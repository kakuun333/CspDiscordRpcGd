#include "CspDiscordRpcGdCppMainControl.h"

#include "CspDiscordRpcGdCppCloseWindow.h"
#include "CspDiscordRpcGdCppNavigationView.h"
#include "CspDiscordRpcGdCppWorkData.h"
#include "CspDiscordRpcGdCppWorksWindow.h"
#include "CspDiscordRpcService.h"
#include "Generated/EmbeddedBinaryResources.h"
#include "Generated/EmbeddedSvgResources.h"
#if defined(MACOS_ENABLED)
#include "Platform/MacOsWindowUtils.h"
#endif
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/color_rect.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/grid_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/line_edit.hpp"
#include "godot_cpp/classes/margin_container.hpp"
#include "godot_cpp/classes/option_button.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/popup_menu.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/scroll_container.hpp"
#include "godot_cpp/classes/status_indicator.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/texture_rect.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/classes/xml_parser.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{

constexpr float ResizeBorderThickness = 3.0f;
constexpr float ResizeCornerExtent = 12.0f;
constexpr const char* SettingsFileName = "Settings.json";
constexpr int32_t CspIconMaxWidth = 48;
constexpr int32_t CspIconTextureSize = CspIconMaxWidth * 2;

using CspDiscordRpcGdCpp::CspDiscordRpcGdCppWorkData;

enum class ERichPresenceTextType : uint8_t
{
    State,
    Details,
    LargeImageText,
};

enum class EWindowControlButtonStyle : int32_t
{
    Default = 0,
    Close,
};

[[nodiscard]] bool IsMacOs()
{
    const godot::OS* OperatingSystem{ godot::OS::get_singleton() };
    return OperatingSystem != nullptr && OperatingSystem->get_name() == godot::String("macOS");
}

[[nodiscard]] godot::Vector2i RoundVector2ToVector2i(const godot::Vector2& Value)
{
    return { static_cast<int32_t>(Value.x + (Value.x >= 0.0F ? 0.5F : -0.5F)),
             static_cast<int32_t>(Value.y + (Value.y >= 0.0F ? 0.5F : -0.5F)) };
}

[[nodiscard]] godot::Vector2i GetViewportBoundsSize(godot::Control* ControlNode)
{
    if (ControlNode == nullptr)
    {
        return {};
    }

    return RoundVector2ToVector2i(ControlNode->get_viewport_rect().size);
}
[[nodiscard]] godot::Ref<godot::Texture2D> CreateTextureFromSvg(const godot::String SvgContent)
{
    ERR_FAIL_COND_V_MSG(SvgContent.is_empty(), {}, "Missing embedded SVG content.");

    godot::Ref<godot::Image> Image;
    Image.instantiate();

    const godot::Error LoadError = Image->load_svg_from_string(SvgContent, 1.0f);
    ERR_FAIL_COND_V_MSG(LoadError != godot::OK, {}, godot::vformat("Failed to load embedded SVG. Error code: %d", static_cast<int32_t>(LoadError)));

    return godot::ImageTexture::create_from_image(Image);
}

[[nodiscard]] godot::Ref<godot::Texture2D> CreateTextureFromPng(const uint8_t* PngData, int64_t PngDataSize)
{
    ERR_FAIL_NULL_V_MSG(PngData, {}, "Missing embedded PNG data.");
    ERR_FAIL_COND_V_MSG(PngDataSize <= 0, {}, "Embedded PNG data is empty.");

    godot::PackedByteArray PngBuffer;
    PngBuffer.resize(PngDataSize);
    std::memcpy(PngBuffer.ptrw(), PngData, static_cast<size_t>(PngDataSize));

    godot::Ref<godot::Image> Image;
    Image.instantiate();

    const godot::Error LoadError = Image->load_png_from_buffer(PngBuffer);
    ERR_FAIL_COND_V_MSG(LoadError != godot::OK, {}, godot::vformat("Failed to load embedded PNG. Error code: %d", static_cast<int32_t>(LoadError)));

    Image->resize(CspIconTextureSize, CspIconTextureSize, godot::Image::INTERPOLATE_LANCZOS);

    const godot::Error MipmapError = Image->generate_mipmaps();
    ERR_FAIL_COND_V_MSG(MipmapError != godot::OK, {}, godot::vformat("Failed to generate mipmaps for embedded PNG. Error code: %d", static_cast<int32_t>(MipmapError)));

    return godot::ImageTexture::create_from_image(Image);
}

[[nodiscard]] bool IsLineEditUnderMouse(godot::Control* ControlNode, const godot::Vector2& MousePosition)
{
    if (ControlNode == nullptr || !ControlNode->is_visible_in_tree())
    {
        return false;
    }

    if (godot::Object::cast_to<godot::LineEdit>(ControlNode) != nullptr && ControlNode->get_global_rect().has_point(MousePosition))
    {
        return true;
    }

    for (int32_t ChildIndex{}; ChildIndex < ControlNode->get_child_count(); ++ChildIndex)
    {
        godot::Control* ChildControl{ godot::Object::cast_to<godot::Control>(ControlNode->get_child(ChildIndex)) };
        if (ChildControl != nullptr && IsLineEditUnderMouse(ChildControl, MousePosition))
        {
            return true;
        }
    }

    return false;
}

void ReleaseFocusedLineEditOnOutsideMouseClick(godot::Control* RootControl, const godot::Ref<godot::InputEvent>& Event)
{
    const godot::Ref<godot::InputEventMouseButton> MouseButton{ Event };
    if (!MouseButton.is_valid() || !MouseButton->is_pressed() || MouseButton->get_button_index() != godot::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    if (RootControl == nullptr)
    {
        return;
    }

    godot::Viewport* Viewport{ RootControl->get_viewport() };
    if (Viewport == nullptr)
    {
        return;
    }

    godot::LineEdit* FocusedLineEdit{ godot::Object::cast_to<godot::LineEdit>(Viewport->gui_get_focus_owner()) };
    if (FocusedLineEdit == nullptr || IsLineEditUnderMouse(RootControl, MouseButton->get_position()))
    {
        return;
    }

    FocusedLineEdit->release_focus();
}

void ApplyLabelVisualStyle(godot::Label* LabelNode, const godot::Color& Color = godot::Color::hex(0xe0e0e0ff));

godot::Label* CreatePropertyLabel(const godot::String& Text)
{
    godot::Label* PropertyLabel = memnew(godot::Label);
    PropertyLabel->set_text(Text);
    PropertyLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    PropertyLabel->set_vertical_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    ApplyLabelVisualStyle(PropertyLabel);
    return PropertyLabel;
}

template<typename TControl>
TControl* CreateNamedControl(const godot::String& Name)
{
    TControl* Control = memnew(TControl);
    Control->set_name(Name);
    Control->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    return Control;
}

godot::Ref<godot::StyleBoxFlat> CreatePanelStyle(const godot::Color& BackgroundColor)
{
    godot::Ref<godot::StyleBoxFlat> PanelStyle;
    PanelStyle.instantiate();
    PanelStyle->set_bg_color(BackgroundColor);
    PanelStyle->set_content_margin(godot::SIDE_LEFT, 0.0F);
    PanelStyle->set_content_margin(godot::SIDE_TOP, 0.0F);
    PanelStyle->set_content_margin(godot::SIDE_RIGHT, 0.0F);
    PanelStyle->set_content_margin(godot::SIDE_BOTTOM, 0.0F);
    return PanelStyle;
}

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreatePanelStyle(const godot::Color& BackgroundColor, const godot::Color& BorderColor)
{
    godot::Ref<godot::StyleBoxFlat> PanelStyle = CreatePanelStyle(BackgroundColor);
    PanelStyle->set_border_color(BorderColor);
    PanelStyle->set_border_width(godot::SIDE_LEFT, 1);
    PanelStyle->set_border_width(godot::SIDE_TOP, 1);
    PanelStyle->set_border_width(godot::SIDE_RIGHT, 1);
    PanelStyle->set_border_width(godot::SIDE_BOTTOM, 1);
    return PanelStyle;
}

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateButtonStyle(const godot::Color& BackgroundColor, const godot::Color& BorderColor)
{
    godot::Ref<godot::StyleBoxFlat> ButtonStyle = CreatePanelStyle(BackgroundColor);
    ButtonStyle->set_corner_radius_all(4);
    ButtonStyle->set_border_color(BorderColor);
    ButtonStyle->set_border_width(godot::SIDE_LEFT, 1);
    ButtonStyle->set_border_width(godot::SIDE_TOP, 1);
    ButtonStyle->set_border_width(godot::SIDE_RIGHT, 1);
    ButtonStyle->set_border_width(godot::SIDE_BOTTOM, 1);
    ButtonStyle->set_content_margin(godot::SIDE_LEFT, 10.0F);
    ButtonStyle->set_content_margin(godot::SIDE_TOP, 4.0F);
    ButtonStyle->set_content_margin(godot::SIDE_RIGHT, 10.0F);
    ButtonStyle->set_content_margin(godot::SIDE_BOTTOM, 4.0F);
    return ButtonStyle;
}

void ApplyButtonVisualStyle(godot::Button* ButtonNode, const bool bPrimary = false)
{
    if (ButtonNode == nullptr)
    {
        return;
    }

    const godot::Color NormalColor = bPrimary ? godot::Color::hex(0x33445cff) : godot::Color::hex(0x242a36ff);
    const godot::Color HoverColor = bPrimary ? godot::Color::hex(0x405a78ff) : godot::Color::hex(0x30394aff);
    const godot::Color PressedColor = bPrimary ? godot::Color::hex(0x2d3b52ff) : godot::Color::hex(0x202631ff);
    const godot::Color BorderColor = bPrimary ? godot::Color::hex(0x526982ff) : godot::Color::hex(0x3b4558ff);

    ButtonNode->set_focus_mode(godot::Control::FOCUS_NONE);
    ButtonNode->add_theme_stylebox_override("normal", CreateButtonStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_stylebox_override("hover", CreateButtonStyle(HoverColor, godot::Color::hex(0x6dabe4ff)));
    ButtonNode->add_theme_stylebox_override("pressed", CreateButtonStyle(PressedColor, godot::Color::hex(0x6dabe4ff)));
    ButtonNode->add_theme_stylebox_override("focus", CreateButtonStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
    ButtonNode->add_theme_color_override("font_hover_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_pressed_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_focus_color", godot::Color::hex(0xe0e0e0ff));
    ButtonNode->add_theme_color_override("font_disabled_color", godot::Color::hex(0x7a8293ff));
}

void ApplyCheckButtonVisualStyle(godot::CheckButton* CheckButtonNode)
{
    if (CheckButtonNode == nullptr)
    {
        return;
    }

    CheckButtonNode->set_focus_mode(godot::Control::FOCUS_NONE);
    CheckButtonNode->add_theme_stylebox_override("normal", CreateButtonStyle(godot::Color::hex(0x33445cff), godot::Color::hex(0x526982ff)));
    CheckButtonNode->add_theme_stylebox_override("hover", CreateButtonStyle(godot::Color::hex(0x405a78ff), godot::Color::hex(0x6dabe4ff)));
    CheckButtonNode->add_theme_stylebox_override("pressed", CreateButtonStyle(godot::Color::hex(0x2f6fa6ff), godot::Color::hex(0x9fcef7ff)));
    CheckButtonNode->add_theme_stylebox_override("hover_pressed", CreateButtonStyle(godot::Color::hex(0x3c83c2ff), godot::Color::hex(0xb8dcfbff)));
    CheckButtonNode->add_theme_stylebox_override("focus", CreateButtonStyle(godot::Color::hex(0x33445cff), godot::Color::hex(0x6dabe4ff)));
    CheckButtonNode->add_theme_color_override("font_color", godot::Color::hex(0xffffffff));
    CheckButtonNode->add_theme_color_override("font_hover_color", godot::Color::hex(0xffffffff));
    CheckButtonNode->add_theme_color_override("font_pressed_color", godot::Color::hex(0xffffffff));
    CheckButtonNode->add_theme_color_override("font_hover_pressed_color", godot::Color::hex(0xffffffff));
    CheckButtonNode->add_theme_color_override("font_focus_color", godot::Color::hex(0xffffffff));
}

void ApplyTitleBarLabelVisualStyle(godot::Label* LabelNode)
{
    if (LabelNode == nullptr)
    {
        return;
    }

    LabelNode->add_theme_font_size_override("font_size", 15);
    LabelNode->add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
}

void ApplyLabelVisualStyle(godot::Label* LabelNode, const godot::Color& Color)
{
    if (LabelNode == nullptr)
    {
        return;
    }

    LabelNode->add_theme_color_override("font_color", Color);
    LabelNode->add_theme_font_size_override("font_size", 14);
}

void ApplyLineEditVisualStyle(godot::LineEdit* LineEditNode)
{
    if (LineEditNode == nullptr)
    {
        return;
    }

    LineEditNode->add_theme_stylebox_override("normal", CreateButtonStyle(godot::Color::hex(0x202631ff), godot::Color::hex(0x3b4558ff)));
    LineEditNode->add_theme_stylebox_override("focus", CreateButtonStyle(godot::Color::hex(0x202631ff), godot::Color::hex(0x6dabe4ff)));
    LineEditNode->add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
    LineEditNode->add_theme_color_override("font_placeholder_color", godot::Color::hex(0x7a8293ff));
    LineEditNode->add_theme_color_override("caret_color", godot::Color::hex(0xffffffff));
}

[[nodiscard]] godot::String ToGodotString(const std::filesystem::path& Path)
{
    return godot::String(Path.wstring().c_str());
}

[[nodiscard]] std::filesystem::path ToPath(const godot::String& Value)
{
    if (Value.is_empty())
    {
        return {};
    }

    const godot::PackedByteArray Utf16Buffer = Value.to_utf16_buffer();
    if (Utf16Buffer.is_empty())
    {
        return {};
    }

    const char16_t* Utf16Chars = reinterpret_cast<const char16_t*>(Utf16Buffer.ptr());
    const std::size_t Utf16CharCount = static_cast<std::size_t>(Utf16Buffer.size()) / sizeof(char16_t);

    std::wstring WideString;
    WideString.reserve(Utf16CharCount);

    for (std::size_t CharIndex = 0; CharIndex < Utf16CharCount; ++CharIndex)
    {
        const char16_t Character = Utf16Chars[CharIndex];
        if (Character == u'\0')
        {
            break;
        }

        WideString.push_back(static_cast<wchar_t>(Character));
    }

    return std::filesystem::path(WideString);
}

[[nodiscard]] bool HasSupportedWorkExtension(const std::filesystem::path& Path)
{
    const std::wstring Extension = Path.extension().wstring();
    return Extension == L".clip" || Extension == L".cmc" || Extension == L".lip";
}

void SetWindowVisibility(godot::Window* Window, const bool bVisible)
{
    if (Window == nullptr)
    {
        return;
    }

#if defined(MACOS_ENABLED)
    if (IsMacOs())
    {
        if (godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton())
        {
            const int64_t NativeWindowHandle{ DisplayServer->window_get_native_handle(godot::DisplayServer::WINDOW_HANDLE, Window->get_window_id()) };
            const bool bVisibilityChanged{ bVisible ? CspDiscordRpcGdCpp::MacOsWindowUtils::ShowWindow(NativeWindowHandle)
                                                    : CspDiscordRpcGdCpp::MacOsWindowUtils::HideWindow(NativeWindowHandle) };
            if (bVisibilityChanged)
            {
                return;
            }
        }
    }
#endif

    if (bVisible)
    {
        Window->set_mode(godot::Window::MODE_WINDOWED);
        Window->grab_focus();
        return;
    }

    Window->set_mode(godot::Window::MODE_MINIMIZED);
}

[[nodiscard]] std::filesystem::path GetPropertySettingsFilePath()
{
    return std::filesystem::current_path() / SettingsFileName;
}

[[nodiscard]] bool MatchesXmlPath(const std::vector<godot::String>& CurrentPath, std::initializer_list<const char*> ExpectedPath)
{
    if (CurrentPath.size() != ExpectedPath.size())
    {
        return false;
    }

    std::size_t PathIndex = 0;
    for (const char* ExpectedSegment : ExpectedPath)
    {
        if (CurrentPath[PathIndex] != godot::String(ExpectedSegment))
        {
            return false;
        }

        ++PathIndex;
    }

    return true;
}

[[nodiscard]] godot::String ReadXmlTextValue(const godot::Ref<godot::XMLParser>& XmlParser)
{
    if (!XmlParser.is_valid())
    {
        return {};
    }

    if (XmlParser->read() != godot::OK)
    {
        return {};
    }

    return XmlParser->get_node_type() == godot::XMLParser::NODE_TEXT ? XmlParser->get_node_data() : godot::String();
}

[[nodiscard]] godot::String ConvertMillisecondsToDHMS(int64_t Milliseconds)
{
    const int64_t TotalSeconds = Milliseconds / 1000;
    const int64_t Days = TotalSeconds / 86400;
    const int64_t Hours = (TotalSeconds % 86400) / 3600;
    const int64_t Minutes = (TotalSeconds % 3600) / 60;
    const int64_t Seconds = TotalSeconds % 60;
    return godot::vformat("%02d:%02d:%02d:%02d", Days, Hours, Minutes, Seconds);
}

[[nodiscard]] godot::String GetRichPresenceFormatText(CspDiscordRpcGdCpp::CspDiscordRpcGdCppMainControl::ERichPresenceTextLanguage RichPresenceTextLanguage,
                                                      ERichPresenceTextType RichPresenceTextType)
{
    using ERichPresenceTextLanguage = CspDiscordRpcGdCpp::CspDiscordRpcGdCppMainControl::ERichPresenceTextLanguage;

    switch (RichPresenceTextType)
    {
        case ERichPresenceTextType::State:
        {
            switch (RichPresenceTextLanguage)
            {
                case ERichPresenceTextLanguage::Japanese:
                    return godot::String::utf8("\"%s\" で作業している");
                case ERichPresenceTextLanguage::TraditionalChinese:
                    return godot::String::utf8("於 \"%s\" 中作業");
                case ERichPresenceTextLanguage::SimplifiedChinese:
                    return godot::String::utf8("于 \"%s\" 中作业");
                default:
                    return godot::String::utf8("Working on \"%s\"");
            }
        }
        case ERichPresenceTextType::Details:
        {
            switch (RichPresenceTextLanguage)
            {
                case ERichPresenceTextLanguage::Japanese:
                    return godot::String::utf8("総作業時間: %s");
                case ERichPresenceTextLanguage::TraditionalChinese:
                    return godot::String::utf8("總作業時長: %s");
                case ERichPresenceTextLanguage::SimplifiedChinese:
                    return godot::String::utf8("总作业时长: %s");
                default:
                    return godot::String::utf8("Total working time: %s");
            }
        }
        case ERichPresenceTextType::LargeImageText:
        {
            return "CLIP STUDIO PAINT Ver.%s";
        }
        default:
            return {};
    }
}

[[nodiscard]] CspDiscordRpcGdCppWorkData GetCspWorkCacheData(const std::filesystem::path& CspWorkCachePath)
{
    CspDiscordRpcGdCppWorkData WorkData;
    if (CspWorkCachePath.empty())
    {
        return WorkData;
    }

    const std::filesystem::path CatalogPath = CspWorkCachePath / "catalog.xml";
    if (std::filesystem::exists(CatalogPath))
    {
        godot::Ref<godot::XMLParser> XmlParser;
        XmlParser.instantiate();
        if (XmlParser->open(ToGodotString(CatalogPath)) == godot::OK)
        {
            std::vector<godot::String> CurrentXmlPath;
            bool bInsidePngFile = false;

            while (XmlParser->read() == godot::OK)
            {
                const godot::XMLParser::NodeType NodeType = XmlParser->get_node_type();
                if (NodeType == godot::XMLParser::NODE_ELEMENT)
                {
                    const godot::String NodeName = XmlParser->get_node_name();
                    CurrentXmlPath.push_back(NodeName);

                    if (MatchesXmlPath(CurrentXmlPath, { "archive", "catalog", "name" }) && WorkData.Name.is_empty())
                    {
                        WorkData.Name = ReadXmlTextValue(XmlParser);
                    }
                    else if (MatchesXmlPath(CurrentXmlPath, { "archive", "catalog", "groups", "group", "tool" }) && WorkData.CspVersion.is_empty())
                    {
                        WorkData.CspVersion = XmlParser->get_named_attribute_value_safe("version");
                    }
                    else if (MatchesXmlPath(CurrentXmlPath, { "archive", "files", "file" }))
                    {
                        bInsidePngFile = XmlParser->get_named_attribute_value_safe("mime").contains("image/png");
                    }
                    else if (bInsidePngFile && MatchesXmlPath(CurrentXmlPath, { "archive", "files", "file", "path" }) && WorkData.ThumbnailPath.is_empty())
                    {
                        const godot::String RelativeThumbnailPath = ReadXmlTextValue(XmlParser);
                        if (!RelativeThumbnailPath.is_empty())
                        {
                            WorkData.ThumbnailPath = ToGodotString(CspWorkCachePath / ToPath(RelativeThumbnailPath));
                        }
                    }

                    if (XmlParser->is_empty() && !CurrentXmlPath.empty())
                    {
                        CurrentXmlPath.pop_back();
                    }
                }
                else if (NodeType == godot::XMLParser::NODE_ELEMENT_END)
                {
                    if (XmlParser->get_node_name() == godot::String("file"))
                    {
                        bInsidePngFile = false;
                    }

                    if (!CurrentXmlPath.empty())
                    {
                        CurrentXmlPath.pop_back();
                    }
                }
            }
        }
    }

    const std::filesystem::path WorkingTimePath = CspWorkCachePath / "workingTime.json";
    if (std::filesystem::exists(WorkingTimePath))
    {
        std::ifstream WorkingTimeStream(WorkingTimePath, std::ios::binary);
        if (WorkingTimeStream.is_open())
        {
            const std::string WorkingTimeJson{ std::istreambuf_iterator<char>(WorkingTimeStream), std::istreambuf_iterator<char>() };
            const godot::Variant ParsedJson = godot::JSON::parse_string(godot::String(WorkingTimeJson.c_str()));
            if (ParsedJson.get_type() == godot::Variant::DICTIONARY)
            {
                const godot::Dictionary WorkingTimeDictionary = ParsedJson;
                if (WorkingTimeDictionary.has("totalworkingtime"))
                {
                    const int64_t TotalWorkingTimeMilliseconds = static_cast<int64_t>(WorkingTimeDictionary["totalworkingtime"]);
                    WorkData.TotalWorkingTime = ConvertMillisecondsToDHMS(TotalWorkingTimeMilliseconds);
                }
            }
        }
    }

    if (WorkData.Name.is_empty())
    {
        WorkData.Name = godot::String(CspWorkCachePath.filename().wstring().c_str());
    }

    if (WorkData.ThumbnailPath.is_empty())
    {
        const std::filesystem::path DefaultThumbnailPath = CspWorkCachePath / "thumbnail" / "thumbnail.png";
        if (std::filesystem::exists(DefaultThumbnailPath))
        {
            WorkData.ThumbnailPath = ToGodotString(DefaultThumbnailPath);
        }
    }

    WorkData.CacheDataPath = ToGodotString(CspWorkCachePath);
    return WorkData;
}

[[nodiscard]] std::vector<std::filesystem::path> GetCandidateRootPaths(const godot::String& ClipStudioCommonRootPath)
{
    std::vector<std::filesystem::path> CandidateRoots;

    const godot::OS* OperatingSystem = godot::OS::get_singleton();
    const godot::String LocalAppData = OperatingSystem->get_environment("LOCALAPPDATA");
    const godot::String AppData = OperatingSystem->get_environment("APPDATA");
    const godot::String UserProfile = OperatingSystem->get_environment("USERPROFILE");
    const godot::String DocumentsDir = OperatingSystem->get_system_dir(godot::OS::SYSTEM_DIR_DOCUMENTS);

    if (!LocalAppData.is_empty())
    {
        CandidateRoots.emplace_back(ToPath(LocalAppData) / "CELSYS");
        CandidateRoots.emplace_back(ToPath(LocalAppData) / "CELSYSUserData" / "CELSYS" / "CLIPStudioCommon" / "Document");
    }

    if (!AppData.is_empty())
    {
        CandidateRoots.emplace_back(ToPath(AppData) / "CELSYS");
        CandidateRoots.emplace_back(ToPath(AppData) / "CELSYSUserData" / "CELSYS" / "CLIPStudioCommon" / "Document");
    }

    if (!DocumentsDir.is_empty())
    {
        CandidateRoots.emplace_back(ToPath(DocumentsDir) / "CELSYS");
    }

    if (!ClipStudioCommonRootPath.is_empty())
    {
        const std::filesystem::path ConfiguredRootPath = ToPath(ClipStudioCommonRootPath);
        CandidateRoots.push_back(ConfiguredRootPath);

        if (ConfiguredRootPath.filename().wstring() != L"Document")
        {
            CandidateRoots.push_back(ConfiguredRootPath / "Document");
        }
    }

    if (!UserProfile.is_empty())
    {
        CandidateRoots.emplace_back(ToPath(UserProfile) / "Documents" / "CELSYS");
        CandidateRoots.emplace_back(ToPath(UserProfile) / "AppData" / "Roaming" / "CELSYSUserData" / "CELSYS" / "CLIPStudioCommon" / "Document");
    }

    std::vector<std::filesystem::path> UniqueRoots;
    std::unordered_set<std::wstring> SeenPaths;

    for (const std::filesystem::path& CandidateRoot : CandidateRoots)
    {
        std::error_code ErrorCode;
        const std::filesystem::path NormalizedPath = std::filesystem::weakly_canonical(CandidateRoot, ErrorCode);
        const std::filesystem::path FinalPath = ErrorCode ? CandidateRoot : NormalizedPath;
        const std::wstring Key = FinalPath.lexically_normal().wstring();

        if (!SeenPaths.insert(Key).second)
        {
            continue;
        }

        if (std::filesystem::exists(FinalPath))
        {
            UniqueRoots.push_back(FinalPath);
        }
    }

    return UniqueRoots;
}

[[nodiscard]] std::vector<CspDiscordRpcGdCppWorkData> DiscoverCspWorks(const godot::String& ClipStudioCommonRootPath)
{
    std::vector<CspDiscordRpcGdCppWorkData> Works;
    std::unordered_set<std::wstring> SeenPaths;

    try
    {
        for (const std::filesystem::path& RootPath : GetCandidateRootPaths(ClipStudioCommonRootPath))
        {
            std::error_code ErrorCode;
            for (const std::filesystem::directory_entry& DirectoryEntry : std::filesystem::recursive_directory_iterator(RootPath, std::filesystem::directory_options::skip_permission_denied, ErrorCode))
            {
                if (ErrorCode || !DirectoryEntry.is_regular_file(ErrorCode))
                {
                    continue;
                }

                const std::filesystem::path EntryPath = DirectoryEntry.path();
                if (EntryPath.filename().wstring() == L"catalog.xml")
                {
                    const std::filesystem::path CacheDirectoryPath = EntryPath.parent_path();
                    const std::wstring EntryKey = CacheDirectoryPath.lexically_normal().wstring();
                    if (!SeenPaths.insert(EntryKey).second)
                    {
                        continue;
                    }

                    const CspDiscordRpcGdCppWorkData WorkData = GetCspWorkCacheData(CacheDirectoryPath);
                    if (!WorkData.CacheDataPath.is_empty())
                    {
                        Works.push_back(WorkData);
                    }

                    continue;
                }

                if (!HasSupportedWorkExtension(EntryPath))
                {
                    continue;
                }

                if (const std::wstring EntryKey = EntryPath.lexically_normal().wstring(); !SeenPaths.insert(EntryKey).second)
                {
                    continue;
                }

                CspDiscordRpcGdCppWorkData WorkData;
                WorkData.Name = godot::String(EntryPath.stem().wstring().c_str());
                WorkData.CacheDataPath = ToGodotString(EntryPath);
                WorkData.ThumbnailPath = "";
                WorkData.CspVersion = "";
                WorkData.TotalWorkingTime = "";
                Works.push_back(WorkData);
            }
        }

        std::sort(Works.begin(), Works.end(), [](const CspDiscordRpcGdCppWorkData& Left, const CspDiscordRpcGdCppWorkData& Right)
        {
            return Left.Name.naturalnocasecmp_to(Right.Name) < 0;
        });
    }
    catch (std::exception& e)
    {
        godot::UtilityFunctions::print("Exception: ", e.what());
    }


    return Works;
}

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppMainControl::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("set_rich_presence_text_language", "rich_presence_text_language"), &CspDiscordRpcGdCppMainControl::SetRichPresenceTextLanguage);
    godot::ClassDB::bind_method(godot::D_METHOD("get_rich_presence_text_language"), &CspDiscordRpcGdCppMainControl::GetRichPresenceTextLanguage);
    godot::ClassDB::bind_method(godot::D_METHOD("sync_to_viewport_size"), &CspDiscordRpcGdCppMainControl::SyncToViewportSize);
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_resize_handle_gui_input", "event", "resize_edge"), &CspDiscordRpcGdCppMainControl::OnResizeHandleGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_choose_csp_work_pressed"), &CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_csp_work_chosen", "work_name", "work_path"), &CspDiscordRpcGdCppMainControl::OnCspWorkChosen);
    godot::ClassDB::bind_method(godot::D_METHOD("on_close_status_indicator_menu_id_pressed", "id"),
                                &CspDiscordRpcGdCppMainControl::OnCloseStatusIndicatorMenuIdPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_window_control_button_mouse_entered", "window_control_button", "window_control_button_style"),
                                &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseEntered);
    godot::ClassDB::bind_method(godot::D_METHOD("on_window_control_button_mouse_exited", "window_control_button"),
                                &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseExited);
    godot::ClassDB::bind_method(godot::D_METHOD("on_collapsible_property_group_toggled", "toggle_button", "content_container"), &CspDiscordRpcGdCppMainControl::OnCollapsiblePropertyGroupToggled);
    godot::ClassDB::bind_method(godot::D_METHOD("on_csp_icon_selected", "selected_index"), &CspDiscordRpcGdCppMainControl::OnCspIconSelected);
    godot::ClassDB::bind_method(godot::D_METHOD("on_rich_presence_text_language_selected", "selected_index"), &CspDiscordRpcGdCppMainControl::OnRichPresenceTextLanguageSelected);
    godot::ClassDB::bind_method(godot::D_METHOD("on_update_presence_pressed"), &CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed);

    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT,
                                     "Rich Presence Text Language",
                                     godot::PROPERTY_HINT_ENUM,
                                     godot::String::utf8("English,日本語,繁體中文,简体中文"),
                                     godot::PROPERTY_USAGE_DEFAULT),
                 "set_rich_presence_text_language",
                 "get_rich_presence_text_language");
}

void CspDiscordRpcGdCppMainControl::_ready()
{
    set_name("CspDiscordRpcGdCppMainControl");
    set_clip_contents(false);
    set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SyncToViewportSize();

    if (godot::Viewport* Viewport = get_viewport())
    {
        Viewport->connect("size_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::SyncToViewportSize));
    }

    godot::VBoxContainer* RootContainer = memnew(godot::VBoxContainer);
    RootContainer->set_name("RootContainer");
    RootContainer->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    RootContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->set_clip_contents(true);
    RootContainer->add_theme_constant_override("separation", 0);
    add_child(RootContainer);

    godot::PanelContainer* TitleBarPanel = memnew(godot::PanelContainer);
    TitleBarPanel->set_name("TitleBarPanel");
    TitleBarPanel->set_custom_minimum_size(godot::Vector2(0.0f, 40.0f));
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->set_clip_contents(true);
    godot::Ref<godot::StyleBoxFlat> TitleBarPanelStyle = CreatePanelStyle(godot::Color::hex(0x1c1f29ff));
    TitleBarPanelStyle->set_border_color(godot::Color::hex(0x6dabe4ff));
    TitleBarPanelStyle->set_border_width(godot::SIDE_BOTTOM, 1);
    TitleBarPanel->add_theme_stylebox_override("panel", TitleBarPanelStyle);
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer = memnew(godot::HBoxContainer);
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->set_clip_contents(true);
    TitleBarContainer->add_theme_constant_override("separation", 4);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::MarginContainer* TitleBarIconMargin = memnew(godot::MarginContainer);
    TitleBarIconMargin->set_name("TitleBarIconMargin");
    TitleBarIconMargin->add_theme_constant_override("margin_left", 8);

    godot::TextureRect* TitleBarIcon = memnew(godot::TextureRect);
    TitleBarIcon->set_name("TitleBarIcon");
    TitleBarIcon->set_custom_minimum_size(godot::Vector2(20.0f, 20.0f));
    TitleBarIcon->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
    TitleBarIcon->set_expand_mode(godot::TextureRect::EXPAND_IGNORE_SIZE);
    TitleBarIcon->set_texture(CreateTextureFromSvg(EmbeddedSvgResources::Icon));

    TitleBarIconMargin->add_child(TitleBarIcon);
    TitleBarContainer->add_child(TitleBarIconMargin);

    godot::Label* TitleBarLabel = CreateTitleBarLabel("CspDiscordRpcGd", "");
    TitleBarLabel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput));
    TitleBarContainer->add_child(TitleBarLabel);

    MinimizeButton = CreateWindowControlButton(CreateTextureFromSvg(EmbeddedSvgResources::Minimize), "Minimize");
    SetWindowControlButtonHighlight(MinimizeButton, godot::Color(0.0f, 0.0f, 0.0f, 0.0f));
    MinimizeButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnMinimizePressed));
    MinimizeButton->connect("mouse_entered",
                            callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseEntered)
                                .bind(MinimizeButton, static_cast<int32_t>(EWindowControlButtonStyle::Default)));
    MinimizeButton->connect("mouse_exited", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseExited).bind(MinimizeButton));
    TitleBarContainer->add_child(MinimizeButton);

    MaximizeButton = CreateWindowControlButton(CreateTextureFromSvg(EmbeddedSvgResources::Maximize), "Toggle maximize");
    SetWindowControlButtonHighlight(MaximizeButton, godot::Color(0.0f, 0.0f, 0.0f, 0.0f));
    MaximizeButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnMaximizePressed));
    MaximizeButton->connect("mouse_entered",
                            callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseEntered)
                                .bind(MaximizeButton, static_cast<int32_t>(EWindowControlButtonStyle::Default)));
    MaximizeButton->connect("mouse_exited", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseExited).bind(MaximizeButton));
    TitleBarContainer->add_child(MaximizeButton);

    CloseButton = CreateWindowControlButton(CreateTextureFromSvg(EmbeddedSvgResources::Close), "Close");
    SetWindowControlButtonHighlight(CloseButton, godot::Color(0.0f, 0.0f, 0.0f, 0.0f));
    CloseButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnClosePressed));
    CloseButton->connect("mouse_entered",
                         callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseEntered)
                             .bind(CloseButton, static_cast<int32_t>(EWindowControlButtonStyle::Close)));
    CloseButton->connect("mouse_exited", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseExited).bind(CloseButton));
    TitleBarContainer->add_child(CloseButton);

    if (IsMacOs())
    {
        TitleBarContainer->move_child(CloseButton, 0);
        TitleBarContainer->move_child(MinimizeButton, 1);
        TitleBarContainer->move_child(MaximizeButton, 2);
    }

    godot::PanelContainer* ContentPanel = memnew(godot::PanelContainer);
    ContentPanel->set_name("ContentPanel");
    ContentPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_clip_contents(true);
    ContentPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color::hex(0x161a22ff)));
    RootContainer->add_child(ContentPanel);

    godot::HBoxContainer* ContentBodyContainer = memnew(godot::HBoxContainer);
    ContentBodyContainer->set_name("ContentBodyContainer");
    ContentBodyContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentBodyContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentBodyContainer->set_clip_contents(true);
    ContentBodyContainer->add_theme_constant_override("separation", 0);
    ContentPanel->add_child(ContentBodyContainer);

    NavigationView = memnew(CspDiscordRpcGdCppNavigationView);
    NavigationView->SetSelectedItemChangedCallable(callable_mp(this, &CspDiscordRpcGdCppMainControl::OnNavigationViewSelectedItemChanged));
    ContentBodyContainer->add_child(NavigationView);

    ContentScrollContainer = memnew(godot::ScrollContainer);
    ContentScrollContainer->set_name("ContentScrollContainer");
    ContentScrollContainer->set_horizontal_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentScrollContainer->set_vertical_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentScrollContainer->set_follow_focus(true);
    ContentScrollContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentScrollContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentBodyContainer->add_child(ContentScrollContainer);

    godot::MarginContainer* ContentMargin = memnew(godot::MarginContainer);
    ContentMargin->set_name("ContentMargin");
    ContentMargin->add_theme_constant_override("margin_left", 16);
    ContentMargin->add_theme_constant_override("margin_top", 16);
    ContentMargin->add_theme_constant_override("margin_right", 16);
    ContentMargin->add_theme_constant_override("margin_bottom", 16);
    ContentMargin->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentScrollContainer->add_child(ContentMargin);

    godot::VBoxContainer* ContentSectionsContainer = memnew(godot::VBoxContainer);
    ContentSectionsContainer->set_name("ContentSectionsContainer");
    ContentSectionsContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentSectionsContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentSectionsContainer->add_theme_constant_override("separation", 12);
    ContentMargin->add_child(ContentSectionsContainer);

    HomeContentContainer = memnew(godot::VBoxContainer);
    HomeContentContainer->set_name("HomeContent");
    HomeContentContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    HomeContentContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    HomeContentContainer->add_theme_constant_override("separation", 12);
    ContentSectionsContainer->add_child(HomeContentContainer);


    godot::GridContainer* PropertyGridContainer = memnew(godot::GridContainer);
    PropertyGridContainer->set_name("HomePropertyGridContainer");
    PropertyGridContainer->set_columns(2);
    PropertyGridContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    PropertyGridContainer->add_theme_constant_override("h_separation", 12);
    PropertyGridContainer->add_theme_constant_override("v_separation", 12);
    HomeContentContainer->add_child(PropertyGridContainer);

    StatusLabel = CreatePropertyLabel("Discord RPC is disabled.");
    StatusLabel->set_name("StatusLabel");
    AddPropertyRow(PropertyGridContainer, "Status", StatusLabel);

    DiscordRichPresenceCheckButton = CreateNamedControl<godot::CheckButton>("DiscordRichPresence");
    DiscordRichPresenceCheckButton->set_text("");
    ApplyCheckButtonVisualStyle(DiscordRichPresenceCheckButton);
    DiscordRichPresenceCheckButton->connect("toggled", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnDiscordRichPresenceToggled));
    DiscordRichPresenceCheckButton->connect("toggled", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsBoolChanged));
    AddPropertyRow(PropertyGridContainer, "Discord Rich Presence", DiscordRichPresenceCheckButton);

    UpdatePresenceButton = CreateNamedControl<godot::Button>("UpdatePresence");
    UpdatePresenceButton->set_text("Update");
    ApplyButtonVisualStyle(UpdatePresenceButton, true);
    UpdatePresenceButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed));
    AddPropertyRow(PropertyGridContainer, "Update Presence", UpdatePresenceButton);

    const godot::Ref<godot::Texture2D> CspIconOriginalTexture =
        CreateTextureFromPng(EmbeddedBinaryResources::CspIconOriginal, sizeof(EmbeddedBinaryResources::CspIconOriginal));
    const godot::Ref<godot::Texture2D> CspIconVersion5Texture =
        CreateTextureFromPng(EmbeddedBinaryResources::CspIconVersion5, sizeof(EmbeddedBinaryResources::CspIconVersion5));

    CspIconOptionButton = CreateNamedControl<godot::OptionButton>("CspIcon");
    ApplyButtonVisualStyle(CspIconOptionButton);
    CspIconOptionButton->set_texture_filter(godot::CanvasItem::TEXTURE_FILTER_LINEAR_WITH_MIPMAPS);
    CspIconOptionButton->add_theme_constant_override("icon_max_width", CspIconMaxWidth);
    CspIconOptionButton->add_icon_item(CspIconOriginalTexture, "Original", static_cast<int32_t>(ECspIcon::Original));
    CspIconOptionButton->add_icon_item(CspIconVersion5Texture, "Version 5.0", static_cast<int32_t>(ECspIcon::Version5));
    CspIconOptionButton->get_popup()->set_default_canvas_item_texture_filter(godot::Viewport::DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS);
    CspIconOptionButton->get_popup()->set_item_icon_max_width(static_cast<int32_t>(ECspIcon::Original), CspIconMaxWidth);
    CspIconOptionButton->get_popup()->set_item_icon_max_width(static_cast<int32_t>(ECspIcon::Version5), CspIconMaxWidth);
    CspIconOptionButton->select(static_cast<int32_t>(CspIcon));
    CspIconOptionButton->connect("item_selected", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCspIconSelected));
    CspIconOptionButton->connect("item_selected", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsIndexChanged));
    AddPropertyRow(PropertyGridContainer, "CSP Icon", CspIconOptionButton);

    RichPresenceTextLanguageOptionButton = CreateNamedControl<godot::OptionButton>("RichPresenceTextLanguage");
    ApplyButtonVisualStyle(RichPresenceTextLanguageOptionButton);
    RichPresenceTextLanguageOptionButton->add_item(godot::String::utf8("English"), static_cast<int32_t>(ERichPresenceTextLanguage::English));
    RichPresenceTextLanguageOptionButton->add_item(godot::String::utf8("日本語"), static_cast<int32_t>(ERichPresenceTextLanguage::Japanese));
    RichPresenceTextLanguageOptionButton->add_item(godot::String::utf8("繁體中文"), static_cast<int32_t>(ERichPresenceTextLanguage::TraditionalChinese));
    RichPresenceTextLanguageOptionButton->add_item(godot::String::utf8("简体中文"), static_cast<int32_t>(ERichPresenceTextLanguage::SimplifiedChinese));
    RichPresenceTextLanguageOptionButton->select(static_cast<int32_t>(RichPresenceTextLanguage));
    RichPresenceTextLanguageOptionButton->connect("item_selected", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnRichPresenceTextLanguageSelected));
    RichPresenceTextLanguageOptionButton->connect("item_selected", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsIndexChanged));
    AddPropertyRow(PropertyGridContainer, "Rich Presence Text Language", RichPresenceTextLanguageOptionButton);

    ChooseCSPWorkButton = CreateNamedControl<godot::Button>("ChooseCSPWork");
    ChooseCSPWorkButton->set_text("Browse");
    ApplyButtonVisualStyle(ChooseCSPWorkButton);
    ChooseCSPWorkButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed));
    AddPropertyRow(PropertyGridContainer, "Choose CSP Work", ChooseCSPWorkButton);

    SettingsContentContainer = memnew(godot::VBoxContainer);
    SettingsContentContainer->set_name("SettingsContent");
    SettingsContentContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SettingsContentContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SettingsContentContainer->add_theme_constant_override("separation", 12);
    ContentSectionsContainer->add_child(SettingsContentContainer);


    godot::GridContainer* SettingsGridContainer = memnew(godot::GridContainer);
    SettingsGridContainer->set_name("SettingsGridContainer");
    SettingsGridContainer->set_columns(2);
    SettingsGridContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SettingsGridContainer->add_theme_constant_override("h_separation", 12);
    SettingsGridContainer->add_theme_constant_override("v_separation", 12);
    SettingsContentContainer->add_child(SettingsGridContainer);

    ClipStudioCommonRootPathLineEdit = CreateNamedControl<godot::LineEdit>("ClipStudioCommonRootPath");
    ClipStudioCommonRootPathLineEdit->set_placeholder(godot::String::utf8(R"(D:\Documents\CELSYS\CLIPStudioCommon)"));
    ApplyLineEditVisualStyle(ClipStudioCommonRootPathLineEdit);
    ClipStudioCommonRootPathLineEdit->set_tooltip_text("Optional. Set this if you moved the CLIPStudioCommon folder away from the default CELSYS locations.");
    ClipStudioCommonRootPathLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(SettingsGridContainer, "CLIPStudioCommon Root Path", ClipStudioCommonRootPathLineEdit);

    godot::GridContainer* SmallImageGroupGridContainer = CreateCollapsiblePropertyGroup(HomeContentContainer, "SmallImageGroup", "Small Image", true, {});
    SmallImageKeyLineEdit = CreateNamedControl<godot::LineEdit>("SmallImageKey");
    SmallImageKeyLineEdit->set_placeholder("https://example/image.png");
    ApplyLineEditVisualStyle(SmallImageKeyLineEdit);
    SmallImageKeyLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(SmallImageGroupGridContainer, "Small Image Key", SmallImageKeyLineEdit);

    SmallImageTextLineEdit = CreateNamedControl<godot::LineEdit>("SmallImageText");
    SmallImageTextLineEdit->set_placeholder("Drawing something");
    ApplyLineEditVisualStyle(SmallImageTextLineEdit);
    SmallImageTextLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(SmallImageGroupGridContainer, "Small Image Text", SmallImageTextLineEdit);

    static const godot::String BUTTON_WARNING_TOOLTIPS = "Discord may not show activity buttons on your own profile.\nIt might be worth checking from another account to confirm whether they're visible.";

    godot::GridContainer* Button1GroupGridContainer = CreateCollapsiblePropertyGroup(HomeContentContainer,
                                                                                     "Button1Group",
                                                                                     "Button 1",
                                                                                     true,
                                                                                     BUTTON_WARNING_TOOLTIPS);
    Button1LabelLineEdit = CreateNamedControl<godot::LineEdit>("Button1Label");
    Button1LabelLineEdit->set_placeholder("My Instagram");
    ApplyLineEditVisualStyle(Button1LabelLineEdit);
    Button1LabelLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(Button1GroupGridContainer, "Button1 Label", Button1LabelLineEdit);

    Button1UrlLineEdit = CreateNamedControl<godot::LineEdit>("Button1Url");
    Button1UrlLineEdit->set_placeholder("www.instagram.com");
    ApplyLineEditVisualStyle(Button1UrlLineEdit);
    Button1UrlLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(Button1GroupGridContainer, "Button1 URL", Button1UrlLineEdit);

    godot::GridContainer* Button2GroupGridContainer = CreateCollapsiblePropertyGroup(HomeContentContainer,
                                                                                     "Button2Group",
                                                                                     "Button 2",
                                                                                     true,
                                                                                     BUTTON_WARNING_TOOLTIPS);
    Button2LabelLineEdit = CreateNamedControl<godot::LineEdit>("Button2Label");
    Button2LabelLineEdit->set_placeholder("My X");
    ApplyLineEditVisualStyle(Button2LabelLineEdit);
    Button2LabelLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(Button2GroupGridContainer, "Button2 Label", Button2LabelLineEdit);

    Button2UrlLineEdit = CreateNamedControl<godot::LineEdit>("Button2Url");
    Button2UrlLineEdit->set_placeholder("https://x.com");
    ApplyLineEditVisualStyle(Button2UrlLineEdit);
    Button2UrlLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged));
    AddPropertyRow(Button2GroupGridContainer, "Button2 URL", Button2UrlLineEdit);

    UpdateNavigationContentVisibility();

    AddResizeHandle("ResizeTopLeft",
                    godot::DisplayServer::WINDOW_EDGE_TOP_LEFT,
                    godot::Control::CURSOR_FDIAGSIZE,
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    ResizeCornerExtent,
                    ResizeCornerExtent);
    AddResizeHandle("ResizeTop",
                    godot::DisplayServer::WINDOW_EDGE_TOP,
                    godot::Control::CURSOR_VSIZE,
                    0.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    ResizeCornerExtent,
                    0.0f,
                    -ResizeCornerExtent,
                    ResizeBorderThickness);
    AddResizeHandle("ResizeTopRight",
                    godot::DisplayServer::WINDOW_EDGE_TOP_RIGHT,
                    godot::Control::CURSOR_BDIAGSIZE,
                    1.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    -ResizeCornerExtent,
                    0.0f,
                    0.0f,
                    ResizeCornerExtent);
    AddResizeHandle("ResizeLeft",
                    godot::DisplayServer::WINDOW_EDGE_LEFT,
                    godot::Control::CURSOR_HSIZE,
                    0.0f,
                    0.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    ResizeCornerExtent,
                    ResizeBorderThickness,
                    -ResizeCornerExtent);
    AddResizeHandle("ResizeRight",
                    godot::DisplayServer::WINDOW_EDGE_RIGHT,
                    godot::Control::CURSOR_HSIZE,
                    1.0f,
                    0.0f,
                    1.0F,
                    1.0F,
                    -ResizeBorderThickness,
                    ResizeCornerExtent,
                    0.0F,
                    -ResizeCornerExtent);
    AddResizeHandle("ResizeBottomLeft",
                    godot::DisplayServer::WINDOW_EDGE_BOTTOM_LEFT,
                    godot::Control::CURSOR_BDIAGSIZE,
                    0.0F,
                    1.0F,
                    0.0F,
                    1.0F,
                    0.0F,
                    -ResizeCornerExtent,
                    ResizeCornerExtent,
                    0.0F);
    AddResizeHandle("ResizeBottom",
                    godot::DisplayServer::WINDOW_EDGE_BOTTOM,
                    godot::Control::CURSOR_VSIZE,
                    0.0F,
                    1.0F,
                    1.0F,
                    1.0F,
                    ResizeCornerExtent,
                    -ResizeBorderThickness,
                    -ResizeCornerExtent,
                    0.0F);
    AddResizeHandle("ResizeBottomRight",
                    godot::DisplayServer::WINDOW_EDGE_BOTTOM_RIGHT,
                    godot::Control::CURSOR_FDIAGSIZE,
                    1.0F,
                    1.0F,
                    1.0F,
                    1.0F,
                    -ResizeCornerExtent,
                    -ResizeCornerExtent,
                    0.0F,
                    0.0F);

    ModalBackground = memnew(godot::ColorRect);
    ModalBackground->set_name("ModalBackground");
    ModalBackground->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    ModalBackground->set_color(godot::Color(0.0F, 0.0F, 0.0F, 0.52F));
    ModalBackground->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    ModalBackground->set_visible(false);
    add_child(ModalBackground);

    UpdateMaximizeButtonIcon();
    LoadPropertySettings();
}

void CspDiscordRpcGdCppMainControl::_input(const godot::Ref<godot::InputEvent>& Event)
{
    ReleaseFocusedLineEditOnOutsideMouseClick(this, Event);
}

void CspDiscordRpcGdCppMainControl::_exit_tree()
{
    SavePropertySettings();
    CspDiscordRpcService::Get().Shutdown();
    PresenceStartTimestamp = 0;
}

void CspDiscordRpcGdCppMainControl::SetRichPresenceTextLanguage(int32_t NewRichPresenceTextLanguage)
{
    switch (static_cast<ERichPresenceTextLanguage>(NewRichPresenceTextLanguage))
    {
        case ERichPresenceTextLanguage::English:
        case ERichPresenceTextLanguage::Japanese:
        case ERichPresenceTextLanguage::TraditionalChinese:
        case ERichPresenceTextLanguage::SimplifiedChinese:
            RichPresenceTextLanguage = static_cast<ERichPresenceTextLanguage>(NewRichPresenceTextLanguage);
            break;
        default:
            RichPresenceTextLanguage = ERichPresenceTextLanguage::English;
            break;
    }

    if (RichPresenceTextLanguageOptionButton != nullptr)
    {
        RichPresenceTextLanguageOptionButton->select(static_cast<int32_t>(RichPresenceTextLanguage));
    }
}

int32_t CspDiscordRpcGdCppMainControl::GetRichPresenceTextLanguage() const
{
    return static_cast<int32_t>(RichPresenceTextLanguage);
}

void CspDiscordRpcGdCppMainControl::SetCspIcon(int32_t NewCspIcon)
{
    switch (static_cast<ECspIcon>(NewCspIcon))
    {
        case ECspIcon::Original:
        case ECspIcon::Version5:
            CspIcon = static_cast<ECspIcon>(NewCspIcon);
            break;
        default:
            CspIcon = ECspIcon::Version5;
            break;
    }

    if (CspIconOptionButton != nullptr)
    {
        CspIconOptionButton->select(static_cast<int32_t>(CspIcon));
    }
}

godot::String CspDiscordRpcGdCppMainControl::GetCspIconLargeImageKey() const
{
    switch (CspIcon)
    {
        case ECspIcon::Original:
            return "https://cdn.discordapp.com/app-icons/1351785436850163733/be7cdabb8b164c564f9e844ba209a2b6.png?size=256";
        case ECspIcon::Version5:
        default:
            return "https://cdn.discordapp.com/app-icons/1542670464470614146/d304a3b4a7fc92092a5c6f906fe85769.png?size=256";
    }
}

void CspDiscordRpcGdCppMainControl::LoadPropertySettings()
{
    const std::filesystem::path PropertySettingsPath = GetPropertySettingsFilePath();
    if (!std::filesystem::exists(PropertySettingsPath))
    {
        return;
    }

    std::ifstream PropertySettingsStream(PropertySettingsPath, std::ios::binary);
    if (!PropertySettingsStream.is_open())
    {
        return;
    }

    const std::string PropertySettingsJson((std::istreambuf_iterator<char>(PropertySettingsStream)), std::istreambuf_iterator<char>());
    if (PropertySettingsJson.empty())
    {
        return;
    }

    const godot::Variant ParsedJson = godot::JSON::parse_string(godot::String(PropertySettingsJson.c_str()));
    if (ParsedJson.get_type() != godot::Variant::DICTIONARY)
    {
        return;
    }

    const godot::Dictionary PropertySettings = ParsedJson;

    bIsApplyingPropertySettings = true;

    if (PropertySettings.has("RichPresenceTextLanguage"))
    {
        SetRichPresenceTextLanguage(static_cast<int32_t>(PropertySettings["RichPresenceTextLanguage"]));
    }

    if (PropertySettings.has("CspIcon"))
    {
        SetCspIcon(static_cast<int32_t>(PropertySettings["CspIcon"]));
    }

    if (PropertySettings.has("CloseAction"))
    {
        switch (static_cast<ECloseAction>(static_cast<int32_t>(PropertySettings["CloseAction"])))
        {
            case ECloseAction::MinimizeToSystemTray:
            case ECloseAction::Close:
                CloseAction = static_cast<ECloseAction>(static_cast<int32_t>(PropertySettings["CloseAction"]));
                break;
            default:
                CloseAction = ECloseAction::MinimizeToSystemTray;
                break;
        }
    }

    if (PropertySettings.has("DontShowCloseWindowAgain"))
    {
        bDontShowCloseWindowAgain = static_cast<bool>(PropertySettings["DontShowCloseWindowAgain"]);
    }

    if (PropertySettings.has("SelectedCSPWorkPath"))
    {
        ApplySelectedCspWorkPath(static_cast<godot::String>(PropertySettings["SelectedCSPWorkPath"]), {});
    }

    if (PropertySettings.has("ClipStudioCommonRootPath") && ClipStudioCommonRootPathLineEdit != nullptr)
    {
        ClipStudioCommonRootPathLineEdit->set_text(static_cast<godot::String>(PropertySettings["ClipStudioCommonRootPath"]));
    }

    if (PropertySettings.has("SmallImageKey") && SmallImageKeyLineEdit != nullptr)
    {
        SmallImageKeyLineEdit->set_text(static_cast<godot::String>(PropertySettings["SmallImageKey"]));
    }

    if (PropertySettings.has("SmallImageText") && SmallImageTextLineEdit != nullptr)
    {
        SmallImageTextLineEdit->set_text(static_cast<godot::String>(PropertySettings["SmallImageText"]));
    }

    if (PropertySettings.has("Button1Label") && Button1LabelLineEdit != nullptr)
    {
        Button1LabelLineEdit->set_text(static_cast<godot::String>(PropertySettings["Button1Label"]));
    }

    if (PropertySettings.has("Button1Url") && Button1UrlLineEdit != nullptr)
    {
        Button1UrlLineEdit->set_text(static_cast<godot::String>(PropertySettings["Button1Url"]));
    }

    if (PropertySettings.has("Button2Label") && Button2LabelLineEdit != nullptr)
    {
        Button2LabelLineEdit->set_text(static_cast<godot::String>(PropertySettings["Button2Label"]));
    }

    if (PropertySettings.has("Button2Url") && Button2UrlLineEdit != nullptr)
    {
        Button2UrlLineEdit->set_text(static_cast<godot::String>(PropertySettings["Button2Url"]));
    }

    const bool bDiscordRichPresenceEnabled =
        PropertySettings.has("DiscordRichPresenceEnabled") ? static_cast<bool>(PropertySettings["DiscordRichPresenceEnabled"]) : false;

    bIsApplyingPropertySettings = false;

    if (DiscordRichPresenceCheckButton != nullptr)
    {
        DiscordRichPresenceCheckButton->set_pressed(bDiscordRichPresenceEnabled);
    }
}

void CspDiscordRpcGdCppMainControl::SavePropertySettings() const
{
    if (bIsApplyingPropertySettings)
    {
        return;
    }

    godot::Dictionary PropertySettings;
    PropertySettings["CspIcon"] = static_cast<int32_t>(CspIcon);
    PropertySettings["RichPresenceTextLanguage"] = static_cast<int32_t>(RichPresenceTextLanguage);
    PropertySettings["CloseAction"] = static_cast<int32_t>(CloseAction);
    PropertySettings["DontShowCloseWindowAgain"] = bDontShowCloseWindowAgain;
    PropertySettings["DiscordRichPresenceEnabled"] =
        DiscordRichPresenceCheckButton != nullptr ? DiscordRichPresenceCheckButton->is_pressed() : false;
    PropertySettings["SelectedCSPWorkPath"] = SelectedCSPWorkPath;
    PropertySettings["ClipStudioCommonRootPath"] =
        ClipStudioCommonRootPathLineEdit != nullptr ? ClipStudioCommonRootPathLineEdit->get_text().strip_edges() : godot::String();
    PropertySettings["SmallImageKey"] = SmallImageKeyLineEdit != nullptr ? SmallImageKeyLineEdit->get_text() : godot::String();
    PropertySettings["SmallImageText"] = SmallImageTextLineEdit != nullptr ? SmallImageTextLineEdit->get_text() : godot::String();
    PropertySettings["Button1Label"] = Button1LabelLineEdit != nullptr ? Button1LabelLineEdit->get_text() : godot::String();
    PropertySettings["Button1Url"] = Button1UrlLineEdit != nullptr ? Button1UrlLineEdit->get_text() : godot::String();
    PropertySettings["Button2Label"] = Button2LabelLineEdit != nullptr ? Button2LabelLineEdit->get_text() : godot::String();
    PropertySettings["Button2Url"] = Button2UrlLineEdit != nullptr ? Button2UrlLineEdit->get_text() : godot::String();

    std::ofstream PropertySettingsStream(GetPropertySettingsFilePath(), std::ios::binary | std::ios::trunc);
    if (!PropertySettingsStream.is_open())
    {
        return;
    }

    const godot::String PropertySettingsJson = godot::JSON::stringify(PropertySettings, "  ");
    PropertySettingsStream << PropertySettingsJson.utf8().get_data();
}

void CspDiscordRpcGdCppMainControl::ApplySelectedCspWorkPath(const godot::String& WorkPath, const godot::String& FallbackWorkName)
{
    SelectedCSPWorkPath = WorkPath;
    SelectedCspWorkData = {};

    godot::String DisplayWorkName = FallbackWorkName;

    if (!WorkPath.is_empty())
    {
        SelectedCspWorkData = GetCspWorkCacheData(ToPath(WorkPath));
        if (SelectedCspWorkData.Name.is_empty())
        {
            SelectedCspWorkData.Name = FallbackWorkName;
        }

        if (DisplayWorkName.is_empty())
        {
            DisplayWorkName = SelectedCspWorkData.Name;
        }

        if (DisplayWorkName.is_empty())
        {
            DisplayWorkName = ToGodotString(ToPath(WorkPath).stem());
        }
    }

    if (ChooseCSPWorkButton != nullptr)
    {
        ChooseCSPWorkButton->set_text(DisplayWorkName.is_empty() ? godot::String("Browse") : DisplayWorkName);
        ChooseCSPWorkButton->set_tooltip_text(WorkPath);
    }

    if (!WorkPath.is_empty())
    {
        UpdateStatusText(godot::vformat("Selected CSP work: %s", DisplayWorkName));
    }
}

void CspDiscordRpcGdCppMainControl::OnPropertySettingsChanged()
{
    SavePropertySettings();
}

void CspDiscordRpcGdCppMainControl::OnPropertySettingsBoolChanged(bool bValue)
{
    static_cast<void>(bValue);
    OnPropertySettingsChanged();
}

void CspDiscordRpcGdCppMainControl::OnPropertySettingsIndexChanged(int32_t Value)
{
    static_cast<void>(Value);
    OnPropertySettingsChanged();
}

void CspDiscordRpcGdCppMainControl::OnPropertySettingsTextChanged(const godot::String& Value)
{
    static_cast<void>(Value);
    OnPropertySettingsChanged();
}

void CspDiscordRpcGdCppMainControl::EnsureCloseStatusIndicator()
{
    if (CloseStatusIndicator != nullptr)
    {
        return;
    }

    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();
    if (DisplayServer == nullptr || !DisplayServer->has_feature(godot::DisplayServer::FEATURE_STATUS_INDICATOR))
    {
        return;
    }

    CloseStatusIndicator = memnew(godot::StatusIndicator);
    CloseStatusIndicator->set_name("CloseStatusIndicator");
    CloseStatusIndicator->set_visible(false);
    CloseStatusIndicator->set_tooltip("CspDiscordRpcGd");
    CloseStatusIndicator->set_icon(CreateTextureFromSvg(EmbeddedSvgResources::Icon));
    CloseStatusIndicator->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCloseStatusIndicatorPressed));

    CloseStatusIndicatorMenu = memnew(godot::PopupMenu);
    CloseStatusIndicatorMenu->set_name("CloseStatusIndicatorMenu");
    CloseStatusIndicatorMenu->add_item("Show", 0);
    CloseStatusIndicatorMenu->add_item("Exit", 1);
    CloseStatusIndicatorMenu->connect("id_pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCloseStatusIndicatorMenuIdPressed));
    add_child(CloseStatusIndicatorMenu);
    add_child(CloseStatusIndicator);
    CloseStatusIndicator->set_menu(CloseStatusIndicatorMenu->get_path());
}

void CspDiscordRpcGdCppMainControl::HideCloseStatusIndicator()
{
    if (CloseStatusIndicator != nullptr)
    {
        CloseStatusIndicator->set_visible(false);
    }
}

void CspDiscordRpcGdCppMainControl::SetWindowControlButtonHighlight(godot::Button* WindowControlButton, const godot::Color& HighlightColor) const
{
    if (WindowControlButton == nullptr)
    {
        return;
    }

    godot::Ref<godot::StyleBoxFlat> PanelStyle = CreatePanelStyle(HighlightColor);
    WindowControlButton->add_theme_stylebox_override("normal", PanelStyle);
    WindowControlButton->add_theme_stylebox_override("hover", PanelStyle);
    WindowControlButton->add_theme_stylebox_override("pressed", PanelStyle);
    WindowControlButton->add_theme_stylebox_override("focus", PanelStyle);
}

void CspDiscordRpcGdCppMainControl::ExecuteCloseAction(ECloseAction InCloseAction)
{
    switch (InCloseAction)
    {
        case ECloseAction::MinimizeToSystemTray:
            EnsureCloseStatusIndicator();

            if (CloseStatusIndicator != nullptr)
            {
                if (godot::Window* OwnerWindow = get_window())
                {
                    SetWindowVisibility(OwnerWindow, false);
                }

                CloseStatusIndicator->set_visible(true);
                UpdateStatusText("Window minimized to the system tray.");
            }
            else
            {
                if (godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton())
                {
                    DisplayServer->window_set_mode(godot::DisplayServer::WINDOW_MODE_MINIMIZED);
                }
                else if (godot::Window* OwnerWindow = get_window())
                {
                    OwnerWindow->set_mode(godot::Window::MODE_MINIMIZED);
                }

                UpdateStatusText("Status indicator is unavailable, so the window was minimized instead.");
            }

            break;
        case ECloseAction::Close:
            if (godot::SceneTree* SceneTree = get_tree())
            {
                SceneTree->quit();
            }

            break;
        default:
            break;
    }
}

void CspDiscordRpcGdCppMainControl::UpdateStatusText(const godot::String& StatusText) const
{
    if (StatusLabel != nullptr)
    {
        StatusLabel->set_text(StatusText);
    }
}

void CspDiscordRpcGdCppMainControl::SyncToViewportSize()
{
    set_size(get_viewport_rect().size);

    const godot::Vector2i BoundsSize{ GetViewportBoundsSize(this) };
    if (CloseWindow != nullptr)
    {
        CloseWindow->SetBoundsSize(BoundsSize);
        if (CloseWindow->is_visible())
        {
            CloseWindow->ApplyResponsiveLayout(false);
        }
    }

    if (WorksWindow != nullptr)
    {
        WorksWindow->SetBoundsSize(BoundsSize);
        if (WorksWindow->is_visible())
        {
            WorksWindow->ApplyBoundsLayout(false);
        }
    }

    UpdateMaximizeButtonIcon();
}

void CspDiscordRpcGdCppMainControl::CaptureRestoreWindowState()
{
    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();
    RestoreWindowPosition = DisplayServer->window_get_position();
    RestoreWindowSize = DisplayServer->window_get_size();
    bHasRestoreWindowState = true;
}

void CspDiscordRpcGdCppMainControl::RestoreWindowState(bool bRestorePosition) const
{
    if (!bHasRestoreWindowState)
    {
        return;
    }

    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();
    DisplayServer->window_set_mode(godot::DisplayServer::WINDOW_MODE_WINDOWED);
    DisplayServer->window_set_size(RestoreWindowSize);

    if (bRestorePosition)
    {
        DisplayServer->window_set_position(RestoreWindowPosition);
    }
}

void CspDiscordRpcGdCppMainControl::RestoreWindowStateForTitleBarDrag(const godot::Vector2& LocalMousePosition) const
{
    if (!bHasRestoreWindowState)
    {
        return;
    }

    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();
    const godot::Vector2i MaximizedWindowSize = DisplayServer->window_get_size();
    const godot::Vector2i MouseScreenPosition = DisplayServer->mouse_get_position();

    DisplayServer->window_set_mode(godot::DisplayServer::WINDOW_MODE_WINDOWED);
    DisplayServer->window_set_size(RestoreWindowSize);

    const float TitleBarXRatio = MaximizedWindowSize.x > 0 ? LocalMousePosition.x / static_cast<float>(MaximizedWindowSize.x) : 0.0F;
    const int32_t RestoredOffsetX = std::clamp(static_cast<int32_t>(TitleBarXRatio * static_cast<float>(RestoreWindowSize.x)),
                                               0,
                                               std::max(RestoreWindowSize.x - 1, 0));
    const int32_t RestoredOffsetY = std::clamp(static_cast<int32_t>(LocalMousePosition.y),
                                               0,
                                               std::max(RestoreWindowSize.y - 1, 0));

    DisplayServer->window_set_position(godot::Vector2i(MouseScreenPosition.x - RestoredOffsetX, MouseScreenPosition.y - RestoredOffsetY));
}

void CspDiscordRpcGdCppMainControl::UpdateMaximizeButtonIcon()
{
    if (MaximizeButton == nullptr)
    {
        return;
    }

    MaximizeButton->set_button_icon(CreateTextureFromSvg(bIsWindowMaximized ? EmbeddedSvgResources::Restore : EmbeddedSvgResources::Maximize));
}

void CspDiscordRpcGdCppMainControl::AddPropertyRow(godot::GridContainer* GridContainer, const godot::String& LabelText, godot::Control* EditorControl)
{
    godot::Label* PropertyLabel{ CreatePropertyLabel(LabelText) };
    PropertyLabel->set_custom_minimum_size(godot::Vector2(0.0F, 0.0F));
    PropertyLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    GridContainer->add_child(PropertyLabel);

    if (EditorControl != nullptr)
    {
        EditorControl->set_custom_minimum_size(godot::Vector2(0.0F, EditorControl->get_custom_minimum_size().y));
        EditorControl->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    }

    GridContainer->add_child(EditorControl);
}

godot::GridContainer* CspDiscordRpcGdCppMainControl::CreateCollapsiblePropertyGroup(godot::VBoxContainer* ParentContainer,
                                                                                    const godot::String& Name,
                                                                                    const godot::String& Title,
                                                                                    bool bExpandedByDefault,
                                                                                    const godot::String& WarningTooltipText)
{
    godot::VBoxContainer* GroupContainer = memnew(godot::VBoxContainer);
    GroupContainer->set_name(Name);
    GroupContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    GroupContainer->add_theme_constant_override("separation", 8);
    ParentContainer->add_child(GroupContainer);

    godot::HBoxContainer* HeaderContainer = memnew(godot::HBoxContainer);
    HeaderContainer->set_name(godot::String(Name) + godot::String("Header"));
    HeaderContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    HeaderContainer->set_alignment(godot::BoxContainer::ALIGNMENT_BEGIN);
    HeaderContainer->add_theme_constant_override("separation", 8);
    GroupContainer->add_child(HeaderContainer);

    godot::Button* ToggleButton = memnew(godot::Button);
    ToggleButton->set_name(godot::String(Name) + godot::String("ToggleButton"));
    ToggleButton->set_flat(true);
    ToggleButton->set_focus_mode(godot::Control::FOCUS_NONE);
    ToggleButton->set_custom_minimum_size(godot::Vector2(20.0F, 20.0F));
    ToggleButton->set_button_icon(CreateTextureFromSvg(bExpandedByDefault ? EmbeddedSvgResources::Expand : EmbeddedSvgResources::Collapse));
    ToggleButton->set_tooltip_text(bExpandedByDefault ? godot::String("Collapse") : godot::String("Expand"));
    ApplyButtonVisualStyle(ToggleButton);
    HeaderContainer->add_child(ToggleButton);

    godot::HBoxContainer* TitleContainer = memnew(godot::HBoxContainer);
    TitleContainer->set_name(godot::String(Name) + godot::String("TitleContainer"));
    TitleContainer->set_alignment(godot::BoxContainer::ALIGNMENT_BEGIN);
    TitleContainer->add_theme_constant_override("separation", 6);
    HeaderContainer->add_child(TitleContainer);

    godot::Label* TitleLabel = CreatePropertyLabel(Title);
    TitleLabel->set_name(godot::String(Name) + godot::String("TitleLabel"));
    TitleLabel->set_vertical_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    TitleContainer->add_child(TitleLabel);

    if (!WarningTooltipText.is_empty())
    {
        TitleContainer->add_child(CreateHeaderWarningIcon(WarningTooltipText));
    }

    godot::Control* HeaderSpacer = memnew(godot::Control);
    HeaderSpacer->set_name(godot::String(Name) + godot::String("HeaderSpacer"));
    HeaderSpacer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    HeaderContainer->add_child(HeaderSpacer);

    godot::MarginContainer* ContentMarginContainer = memnew(godot::MarginContainer);
    ContentMarginContainer->set_name(godot::String(Name) + godot::String("ContentMargin"));
    ContentMarginContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentMarginContainer->add_theme_constant_override("margin_left", 28);
    ContentMarginContainer->set_visible(bExpandedByDefault);
    GroupContainer->add_child(ContentMarginContainer);

    godot::GridContainer* ContentContainer = memnew(godot::GridContainer);
    ContentContainer->set_name(godot::String(Name) + godot::String("Content"));
    ContentContainer->set_columns(2);
    ContentContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentContainer->add_theme_constant_override("h_separation", 12);
    ContentContainer->add_theme_constant_override("v_separation", 12);
    ContentMarginContainer->add_child(ContentContainer);

    ToggleButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCollapsiblePropertyGroupToggled).bind(ToggleButton, ContentMarginContainer));
    return ContentContainer;
}

godot::TextureRect* CspDiscordRpcGdCppMainControl::CreateHeaderWarningIcon(const godot::String& TooltipText) const
{
    godot::TextureRect* WarningIcon = memnew(godot::TextureRect);
    WarningIcon->set_name("HeaderWarningIcon");
    WarningIcon->set_custom_minimum_size(godot::Vector2(16.0F, 16.0F));
    WarningIcon->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_CENTERED);
    WarningIcon->set_expand_mode(godot::TextureRect::EXPAND_IGNORE_SIZE);
    WarningIcon->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    WarningIcon->set_tooltip_text(TooltipText);

    if (!WarningIcon->get_texture().is_valid())
    {
        WarningIcon->set_texture(CreateTextureFromSvg(EmbeddedSvgResources::Warning));
    }

    return WarningIcon;
}

void CspDiscordRpcGdCppMainControl::UpdateNavigationContentVisibility() const
{
    if (HomeContentContainer != nullptr)
    {
        HomeContentContainer->set_visible(SelectedNavigationItem == CspDiscordRpcGdCppNavigationView::NAVIGATION_VIEW_ITEM_HOME);
    }

    if (SettingsContentContainer != nullptr)
    {
        SettingsContentContainer->set_visible(SelectedNavigationItem == CspDiscordRpcGdCppNavigationView::NAVIGATION_VIEW_ITEM_SETTINGS);
    }

    if (ContentScrollContainer != nullptr)
    {
        ContentScrollContainer->set_v_scroll(0);
        ContentScrollContainer->set_h_scroll(0);
    }
}

void CspDiscordRpcGdCppMainControl::OnNavigationViewSelectedItemChanged(const int32_t SelectedItem)
{
    SelectedNavigationItem = SelectedItem;
    UpdateNavigationContentVisibility();
}

void CspDiscordRpcGdCppMainControl::AddResizeHandle(const godot::String& Name,
                                                    godot::DisplayServer::WindowResizeEdge ResizeEdge,
                                                    godot::Control::CursorShape CursorShape,
                                                    float AnchorLeft,
                                                    float AnchorTop,
                                                    float AnchorRight,
                                                    float AnchorBottom,
                                                    float OffsetLeft,
                                                    float OffsetTop,
                                                    float OffsetRight,
                                                    float OffsetBottom)
{
    godot::Control* ResizeHandle = memnew(godot::Control);
    ResizeHandle->set_name(Name);
    ResizeHandle->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    ResizeHandle->set_default_cursor_shape(CursorShape);
    ResizeHandle->set_anchors_preset(godot::Control::PRESET_TOP_LEFT);
    ResizeHandle->set_anchor(godot::SIDE_LEFT, AnchorLeft);
    ResizeHandle->set_anchor(godot::SIDE_TOP, AnchorTop);
    ResizeHandle->set_anchor(godot::SIDE_RIGHT, AnchorRight);
    ResizeHandle->set_anchor(godot::SIDE_BOTTOM, AnchorBottom);
    ResizeHandle->set_offset(godot::SIDE_LEFT, OffsetLeft);
    ResizeHandle->set_offset(godot::SIDE_TOP, OffsetTop);
    ResizeHandle->set_offset(godot::SIDE_RIGHT, OffsetRight);
    ResizeHandle->set_offset(godot::SIDE_BOTTOM, OffsetBottom);
    ResizeHandle->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnResizeHandleGuiInput).bind(static_cast<int32_t>(ResizeEdge)));
    add_child(ResizeHandle);
}

godot::Label* CspDiscordRpcGdCppMainControl::CreateTitleBarLabel(const godot::String& Text, const godot::String& TooltipText) const
{
    godot::Label* TitleBarLabel = memnew(godot::Label);
    TitleBarLabel->set_name("TitleBarLabel");
    TitleBarLabel->set_text(Text);
    TitleBarLabel->set_clip_text(true);
    TitleBarLabel->set_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    TitleBarLabel->set_vertical_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    TitleBarLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarLabel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarLabel->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    TitleBarLabel->set_tooltip_text(TooltipText);
    ApplyTitleBarLabelVisualStyle(TitleBarLabel);
    return TitleBarLabel;
}

godot::Button* CspDiscordRpcGdCppMainControl::CreateWindowControlButton(const godot::Ref<godot::Texture2D>& Icon, const godot::String& TooltipText) const
{
    godot::Button* WindowControlButton = memnew(godot::Button);
    if (Icon.is_valid())
    {
        WindowControlButton->set_button_icon(Icon);
        WindowControlButton->set_expand_icon(false);
        WindowControlButton->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
        WindowControlButton->set_vertical_icon_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    }

    WindowControlButton->set_flat(false);
    WindowControlButton->set_focus_mode(godot::Control::FOCUS_NONE);
    WindowControlButton->set_custom_minimum_size(godot::Vector2(36.0F, 28.0F));
    WindowControlButton->set_v_size_flags(godot::Control::SIZE_SHRINK_CENTER);
    WindowControlButton->set_tooltip_text(TooltipText);
    return WindowControlButton;
}

void CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    if (MouseButtonEvent->is_double_click())
    {
        if (!bIsWindowMaximized)
        {
            OnMaximizePressed();
        }

        return;
    }

    if (bIsWindowMaximized)
    {
        bIsWindowMaximized = false;
        RestoreWindowStateForTitleBarDrag(MouseButtonEvent->get_position());
        UpdateMaximizeButtonIcon();
    }

    godot::DisplayServer::get_singleton()->window_start_drag();
}

void CspDiscordRpcGdCppMainControl::OnResizeHandleGuiInput(const godot::Ref<godot::InputEvent>& Event, int32_t ResizeEdge)
{
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    godot::DisplayServer::get_singleton()->window_start_resize(static_cast<godot::DisplayServer::WindowResizeEdge>(ResizeEdge));
}

void CspDiscordRpcGdCppMainControl::OnMinimizePressed()
{
#if defined(MACOS_ENABLED)
    if (IsMacOs() && MacOsWindowUtils::MinimizeActiveWindow())
    {
        return;
    }
#endif

    if (godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton())
    {
        DisplayServer->window_set_mode(godot::DisplayServer::WINDOW_MODE_MINIMIZED);
    }
}

void CspDiscordRpcGdCppMainControl::OnMaximizePressed()
{
    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();

    if (bIsWindowMaximized)
    {
        bIsWindowMaximized = false;
        RestoreWindowState(true);
    }
    else
    {
        CaptureRestoreWindowState();
        bIsWindowMaximized = true;
        DisplayServer->window_set_mode(godot::DisplayServer::WINDOW_MODE_MAXIMIZED);
    }

    UpdateMaximizeButtonIcon();
}

void CspDiscordRpcGdCppMainControl::SetModalBackgroundVisible(const bool bVisible)
{
    if (ModalBackground == nullptr)
    {
        return;
    }

    ModalBackground->set_visible(bVisible);

    if (bVisible && ModalBackground->get_parent() == this)
    {
        const int32_t LastChildIndex{ get_child_count() - 1 };
        if (LastChildIndex >= 0)
        {
            move_child(ModalBackground, LastChildIndex);
        }
    }
}

void CspDiscordRpcGdCppMainControl::OnClosePressed()
{
    if (bDontShowCloseWindowAgain)
    {
        ExecuteCloseAction(CloseAction);
        return;
    }

    if (CloseWindow == nullptr)
    {
        CloseWindow = memnew(CspDiscordRpcGdCppCloseWindow);
        CloseWindow->connect("confirmed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCloseWindowConfirmed));
        CloseWindow->connect("cancelled", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCloseWindowCancelled));
        CloseWindow->connect("tree_exited", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCloseWindowTreeExited));

        if (godot::Window* OwnerWindow = get_window())
        {
            OwnerWindow->add_child(CloseWindow);
        }
        else
        {
            add_child(CloseWindow);
        }
    }

    CloseWindow->SetSelectedCloseAction(static_cast<CspDiscordRpcGdCppCloseWindow::ECloseAction>(CloseAction));
    CloseWindow->SetDontShowAgain(bDontShowCloseWindowAgain);
    CloseWindow->set_exclusive(false);
    CloseWindow->SetBoundsSize(GetViewportBoundsSize(this));
    CloseWindow->ApplyResponsiveLayout(true);
    SetModalBackgroundVisible(true);
    CloseWindow->popup();
    CloseWindow->ApplyResponsiveLayout(true);
}


void CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed()
{
    const godot::String ClipStudioCommonRootPath =
        ClipStudioCommonRootPathLineEdit != nullptr ? ClipStudioCommonRootPathLineEdit->get_text().strip_edges() : godot::String();
    const std::vector<CspDiscordRpcGdCppWorkData> Works = DiscoverCspWorks(ClipStudioCommonRootPath);
    if (Works.empty())
    {
        UpdateStatusText(ClipStudioCommonRootPath.is_empty() ? "No CSP work files were found under the CELSYS folders."
                                                             : "No CSP work files were found. Check the CLIPStudioCommon Root Path setting.");
        return;
    }

    if (WorksWindow == nullptr)
    {
        WorksWindow = memnew(CspDiscordRpcGdCppWorksWindow);
        WorksWindow->connect("work_chosen", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnCspWorkChosen));
        WorksWindow->connect("tree_exited", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnWorksWindowTreeExited));

        if (godot::Window* OwnerWindow = get_window())
        {
            OwnerWindow->add_child(WorksWindow);
        }
        else
        {
            add_child(WorksWindow);
        }
    }

    WorksWindow->SetWorks(Works);
    WorksWindow->set_exclusive(false);
    WorksWindow->SetBoundsSize(GetViewportBoundsSize(this));
    WorksWindow->ApplyBoundsLayout(true);
    SetModalBackgroundVisible(true);
    WorksWindow->popup();
    WorksWindow->ApplyBoundsLayout(true);
}

void CspDiscordRpcGdCppMainControl::OnCspWorkChosen(const godot::String& WorkName, const godot::String& WorkPath)
{
    ApplySelectedCspWorkPath(WorkPath, WorkName);
    SavePropertySettings();
}

void CspDiscordRpcGdCppMainControl::OnWorksWindowTreeExited()
{
    WorksWindow = nullptr;
    SetModalBackgroundVisible(CloseWindow != nullptr && CloseWindow->is_visible());
}

void CspDiscordRpcGdCppMainControl::OnCloseWindowConfirmed(int32_t InCloseAction, bool bDontShowAgain)
{
    switch (static_cast<ECloseAction>(InCloseAction))
    {
        case ECloseAction::MinimizeToSystemTray:
        case ECloseAction::Close:
            CloseAction = static_cast<ECloseAction>(InCloseAction);
            break;
        default:
            CloseAction = ECloseAction::MinimizeToSystemTray;
            break;
    }

    bDontShowCloseWindowAgain = bDontShowAgain;
    SavePropertySettings();
    SetModalBackgroundVisible(false);
    ExecuteCloseAction(CloseAction);
}

void CspDiscordRpcGdCppMainControl::OnCloseWindowCancelled()
{
    SetModalBackgroundVisible(WorksWindow != nullptr && WorksWindow->is_visible());
}

void CspDiscordRpcGdCppMainControl::OnCloseWindowTreeExited()
{
    CloseWindow = nullptr;
    SetModalBackgroundVisible(WorksWindow != nullptr && WorksWindow->is_visible());
}

void CspDiscordRpcGdCppMainControl::OnCloseStatusIndicatorPressed(int32_t MouseButton, const godot::Vector2i& MousePosition)
{
    static_cast<void>(MousePosition);

    if (MouseButton != godot::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    if (godot::Window* OwnerWindow = get_window())
    {
        SetWindowVisibility(OwnerWindow, true);
    }

    callable_mp(this, &CspDiscordRpcGdCppMainControl::HideCloseStatusIndicator).call_deferred();
    UpdateStatusText("Window restored from the system tray.");
}

void CspDiscordRpcGdCppMainControl::OnCloseStatusIndicatorMenuIdPressed(int32_t Id)
{
    switch (Id)
    {
        case 0:
            OnCloseStatusIndicatorPressed(godot::MOUSE_BUTTON_LEFT, {});
            break;
        case 1:
            ExecuteCloseAction(ECloseAction::Close);
            break;
        default:
            break;
    }
}

void CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseEntered(godot::Button* WindowControlButton, int32_t WindowControlButtonStyle)
{
    switch (static_cast<EWindowControlButtonStyle>(WindowControlButtonStyle))
    {
        case EWindowControlButtonStyle::Close:
            SetWindowControlButtonHighlight(WindowControlButton, godot::Color(0.82F, 0.23F, 0.23F, 1.0F));
            break;
        case EWindowControlButtonStyle::Default:
        default:
            SetWindowControlButtonHighlight(WindowControlButton, godot::Color(0.24F, 0.26F, 0.32F, 1.0F));
            break;
    }
}

void CspDiscordRpcGdCppMainControl::OnWindowControlButtonMouseExited(godot::Button* WindowControlButton)
{
    SetWindowControlButtonHighlight(WindowControlButton, godot::Color(0.0F, 0.0F, 0.0F, 0.0F));
}

void CspDiscordRpcGdCppMainControl::OnCollapsiblePropertyGroupToggled(godot::Button* ToggleButton, godot::Control* ContentContainer)
{
    if (ToggleButton == nullptr || ContentContainer == nullptr)
    {
        return;
    }

    const bool bShouldExpand = !ContentContainer->is_visible();
    ContentContainer->set_visible(bShouldExpand);
    ToggleButton->set_button_icon(CreateTextureFromSvg(bShouldExpand ? EmbeddedSvgResources::Expand : EmbeddedSvgResources::Collapse));
    ToggleButton->set_tooltip_text(bShouldExpand ? godot::String("Collapse") : godot::String("Expand"));
}

void CspDiscordRpcGdCppMainControl::OnDiscordRichPresenceToggled(bool bToggled)
{
    if (bToggled)
    {
        static const godot::String DiscordApplicationId = "1351785436850163733";

        if (!CspDiscordRpcService::Get().Initialize(DiscordApplicationId))
        {
            if (DiscordRichPresenceCheckButton != nullptr)
            {
                DiscordRichPresenceCheckButton->set_pressed_no_signal(false);
            }

            UpdateStatusText("Failed to initialize Discord RPC.");
            return;
        }

        PresenceStartTimestamp = static_cast<int64_t>(std::time(nullptr));
        UpdateStatusText("Discord RPC enabled. Press Update Presence to send the latest values.");
        OnUpdatePresencePressed();
    }
    else
    {
        CspDiscordRpcService::Get().ClearPresence();
        CspDiscordRpcService::Get().Shutdown();
        PresenceStartTimestamp = 0;
        UpdateStatusText("Discord RPC is disabled.");
    }
}

void CspDiscordRpcGdCppMainControl::OnCspIconSelected(int32_t SelectedIndex)
{
    SetCspIcon(SelectedIndex);

    if (CspDiscordRpcService::Get().IsInitialized())
    {
        OnUpdatePresencePressed();
    }
}

void CspDiscordRpcGdCppMainControl::OnRichPresenceTextLanguageSelected(int32_t SelectedIndex)
{
    SetRichPresenceTextLanguage(SelectedIndex);

    if (CspDiscordRpcService::Get().IsInitialized())
    {
        OnUpdatePresencePressed();
    }
}

void CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed()
{
    if (!CspDiscordRpcService::Get().IsInitialized())
    {
        UpdateStatusText("Please enable Discord RPC first.");
        return;
    }

    DiscordRichPresenceData PresenceData;
    PresenceData.State = SelectedCspWorkData.Name.is_empty()
                             ? godot::String()
                             : godot::vformat(GetRichPresenceFormatText(RichPresenceTextLanguage, ERichPresenceTextType::State), SelectedCspWorkData.Name);
    PresenceData.Details = SelectedCspWorkData.TotalWorkingTime.is_empty()
                               ? godot::String()
                               : godot::vformat(GetRichPresenceFormatText(RichPresenceTextLanguage, ERichPresenceTextType::Details), SelectedCspWorkData.TotalWorkingTime);
    PresenceData.LargeImageKey = GetCspIconLargeImageKey();
    PresenceData.LargeImageText = SelectedCspWorkData.CspVersion.is_empty()
                                      ? godot::String()
                                      : godot::vformat(GetRichPresenceFormatText(RichPresenceTextLanguage, ERichPresenceTextType::LargeImageText), SelectedCspWorkData.CspVersion);
    PresenceData.SmallImageKey = SmallImageKeyLineEdit != nullptr ? SmallImageKeyLineEdit->get_text() : godot::String();
    PresenceData.SmallImageText = SmallImageTextLineEdit != nullptr ? SmallImageTextLineEdit->get_text() : godot::String();
    PresenceData.Button1Label = Button1LabelLineEdit != nullptr ? Button1LabelLineEdit->get_text() : godot::String();
    PresenceData.Button1Url = Button1UrlLineEdit != nullptr ? Button1UrlLineEdit->get_text() : godot::String();
    PresenceData.Button2Label = Button2LabelLineEdit != nullptr ? Button2LabelLineEdit->get_text() : godot::String();
    PresenceData.Button2Url = Button2UrlLineEdit != nullptr ? Button2UrlLineEdit->get_text() : godot::String();
    PresenceData.StartTimestamp = PresenceStartTimestamp;

    CspDiscordRpcService::Get().UpdatePresence(PresenceData);
    UpdateStatusText("Presence update sent. Discord must be running locally for it to appear.");
}

} // namespace CspDiscordRpcGdCpp
