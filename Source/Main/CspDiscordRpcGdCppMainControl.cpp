#include "CspDiscordRpcGdCppMainControl.h"

#include "CspDiscordRpcGdCppWorkData.h"
#include "CspDiscordRpcGdCppWorksWindow.h"
#include "CspDiscordRpcService.h"
#include "Generated/EmbeddedSvgResources.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/grid_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/line_edit.hpp"
#include "godot_cpp/classes/margin_container.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/scroll_container.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <initializer_list>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{

constexpr float ResizeBorderThickness = 3.0f;
constexpr float ResizeCornerExtent = 12.0f;

using CspDiscordRpcGdCpp::CspDiscordRpcGdCppWorkData;

enum class EWindowControlIcon : uint8_t
{
    Minimize,
    Maximize,
    Restore,
    Close,
};

[[nodiscard]] godot::String GetEmbeddedSvgContent(EWindowControlIcon Icon)
{
    switch (Icon)
    {
        case EWindowControlIcon::Minimize:
            return CspDiscordRpcGdCpp::EmbeddedSvgResources::CurveConstant;
        case EWindowControlIcon::Maximize:
            return CspDiscordRpcGdCpp::EmbeddedSvgResources::PanelContainer;
        case EWindowControlIcon::Restore:
            return CspDiscordRpcGdCpp::EmbeddedSvgResources::ActionCopy;
        case EWindowControlIcon::Close:
            return CspDiscordRpcGdCpp::EmbeddedSvgResources::Close;
        default:
            return {};
    }
}

[[nodiscard]] godot::Ref<godot::Texture2D> CreateTextureFromEmbeddedSvg(EWindowControlIcon Icon)
{
    const godot::String SvgContent = GetEmbeddedSvgContent(Icon);
    ERR_FAIL_COND_V_MSG(SvgContent.is_empty(), {}, "Missing embedded SVG content.");

    godot::Ref<godot::Image> Image;
    Image.instantiate();

    const godot::Error LoadError = Image->load_svg_from_string(SvgContent, 1.0F);
    ERR_FAIL_COND_V_MSG(LoadError != godot::OK, {}, godot::vformat("Failed to load embedded SVG. Error code: %d", static_cast<int32_t>(LoadError)));

    if (Icon == EWindowControlIcon::Restore)
    {
        Image->rotate_90(godot::CLOCKWISE);
    }

    return godot::ImageTexture::create_from_image(Image);
}

[[nodiscard]] const godot::Ref<godot::Texture2D>& GetWindowControlIconTexture(EWindowControlIcon Icon)
{
    static godot::Ref<godot::Texture2D> MinimizeIconTexture;
    static godot::Ref<godot::Texture2D> MaximizeIconTexture;
    static godot::Ref<godot::Texture2D> RestoreIconTexture;
    static godot::Ref<godot::Texture2D> CloseIconTexture;

    godot::Ref<godot::Texture2D>* IconTexture = nullptr;

    switch (Icon)
    {
        case EWindowControlIcon::Minimize:
            IconTexture = &MinimizeIconTexture;
            break;
        case EWindowControlIcon::Maximize:
            IconTexture = &MaximizeIconTexture;
            break;
        case EWindowControlIcon::Restore:
            IconTexture = &RestoreIconTexture;
            break;
        case EWindowControlIcon::Close:
            IconTexture = &CloseIconTexture;
            break;
        default:
            return MinimizeIconTexture;
    }

    if (!IconTexture->is_valid())
    {
        *IconTexture = CreateTextureFromEmbeddedSvg(Icon);
    }

    return *IconTexture;
}

godot::Label* CreatePropertyLabel(const godot::String& Text)
{
    godot::Label* PropertyLabel = memnew(godot::Label);
    PropertyLabel->set_text(Text);
    PropertyLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
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
    return PanelStyle;
}

[[nodiscard]] godot::String ToGodotString(const std::filesystem::path& Path)
{
    return godot::String(Path.wstring().c_str());
}

[[nodiscard]] std::filesystem::path ToPath(const godot::String& Value)
{
    const godot::PackedByteArray WideCharBuffer = Value.to_wchar_buffer();
    if (WideCharBuffer.is_empty())
    {
        return {};
    }

    const wchar_t* WideChars = reinterpret_cast<const wchar_t*>(WideCharBuffer.ptr());
    return std::filesystem::path(WideChars);
}

[[nodiscard]] bool HasSupportedImageExtension(const std::filesystem::path& Path)
{
    const std::wstring Extension = Path.extension().wstring();
    return Extension == L".png" || Extension == L".jpg" || Extension == L".jpeg" || Extension == L".webp" || Extension == L".bmp";
}

[[nodiscard]] bool HasSupportedWorkExtension(const std::filesystem::path& Path)
{
    const std::wstring Extension = Path.extension().wstring();
    return Extension == L".clip" || Extension == L".cmc" || Extension == L".lip";
}

[[nodiscard]] std::string ReadTextFile(const std::filesystem::path& FilePath)
{
    std::ifstream InputStream(FilePath, std::ios::binary);
    if (!InputStream.is_open())
    {
        return {};
    }

    return {std::istreambuf_iterator<char>(InputStream), std::istreambuf_iterator<char>()};
}

[[nodiscard]] std::string ExtractTagValue(const std::string& Text, const std::string& TagName)
{
    const std::string StartTag = "<" + TagName + ">";
    const std::string EndTag = "</" + TagName + ">";

    const std::size_t StartIndex = Text.find(StartTag);
    if (StartIndex == std::string::npos)
    {
        return {};
    }

    const std::size_t ValueStartIndex = StartIndex + StartTag.size();
    const std::size_t EndIndex = Text.find(EndTag, ValueStartIndex);
    if (EndIndex == std::string::npos || EndIndex <= ValueStartIndex)
    {
        return {};
    }

    return Text.substr(ValueStartIndex, EndIndex - ValueStartIndex);
}

[[nodiscard]] std::string ExtractAttributeValue(const std::string& Text, const std::string& ElementName, const std::string& AttributeName)
{
    const std::string ElementToken = "<" + ElementName;
    const std::size_t ElementIndex = Text.find(ElementToken);
    if (ElementIndex == std::string::npos)
    {
        return {};
    }

    const std::size_t ElementEndIndex = Text.find('>', ElementIndex);
    if (ElementEndIndex == std::string::npos)
    {
        return {};
    }

    const std::string AttributeToken = AttributeName + "=\"";
    const std::size_t AttributeIndex = Text.find(AttributeToken, ElementIndex);
    if (AttributeIndex == std::string::npos || AttributeIndex > ElementEndIndex)
    {
        return {};
    }

    const std::size_t ValueStartIndex = AttributeIndex + AttributeToken.size();
    const std::size_t ValueEndIndex = Text.find('"', ValueStartIndex);
    if (ValueEndIndex == std::string::npos || ValueEndIndex <= ValueStartIndex)
    {
        return {};
    }

    return Text.substr(ValueStartIndex, ValueEndIndex - ValueStartIndex);
}

[[nodiscard]] std::string ExtractPngThumbnailRelativePath(const std::string& CatalogXmlContent)
{
    std::size_t SearchStartIndex = 0;

    while (true)
    {
        const std::size_t FileStartIndex = CatalogXmlContent.find("<file", SearchStartIndex);
        if (FileStartIndex == std::string::npos)
        {
            return {};
        }

        const std::size_t FileEndIndex = CatalogXmlContent.find("</file>", FileStartIndex);
        if (FileEndIndex == std::string::npos)
        {
            return {};
        }

        const std::string FileBlock = CatalogXmlContent.substr(FileStartIndex, FileEndIndex - FileStartIndex);
        if (FileBlock.find("image/png") != std::string::npos)
        {
            return ExtractTagValue(FileBlock, "path");
        }

        SearchStartIndex = FileEndIndex + 7;
    }
}

[[nodiscard]] godot::String ConvertMillisecondsToDHMS(int64_t Milliseconds)
{
    const int64_t TotalSeconds = Milliseconds / 1000;
    const int64_t Days = TotalSeconds / 86400;
    const int64_t Hours = (TotalSeconds % 86400) / 3600;
    const int64_t Minutes = (TotalSeconds % 3600) / 60;
    const int64_t Seconds = TotalSeconds % 60;

    if (Days > 0)
    {
        return godot::vformat("%dd %02dh %02dm %02ds", Days, Hours, Minutes, Seconds);
    }

    if (Hours > 0)
    {
        return godot::vformat("%02dh %02dm %02ds", Hours, Minutes, Seconds);
    }

    if (Minutes > 0)
    {
        return godot::vformat("%02dm %02ds", Minutes, Seconds);
    }

    return godot::vformat("%02ds", Seconds);
}

[[nodiscard]] CspDiscordRpcGdCppWorkData GetCspWorkCacheData(const std::filesystem::path& CspWorkCachePath)
{
    CspDiscordRpcGdCppWorkData WorkData;
    if (CspWorkCachePath.empty())
    {
        return WorkData;
    }

    const std::filesystem::path CatalogPath = CspWorkCachePath / "catalog.xml";
    if (!std::filesystem::exists(CatalogPath))
    {
        return WorkData;
    }

    const std::string CatalogXmlContent = ReadTextFile(CatalogPath);
    if (CatalogXmlContent.empty())
    {
        return WorkData;
    }

    const std::string WorkName = ExtractTagValue(CatalogXmlContent, "name");
    const std::string CspVersion = ExtractAttributeValue(CatalogXmlContent, "tool", "version");
    const std::string ThumbnailRelativePath = ExtractPngThumbnailRelativePath(CatalogXmlContent);

    if (!WorkName.empty())
    {
        WorkData.Name = godot::String(WorkName.c_str());
    }

    if (!CspVersion.empty())
    {
        WorkData.CspVersion = godot::String(CspVersion.c_str());
    }

    if (!ThumbnailRelativePath.empty())
    {
        WorkData.ThumbnailPath = ToGodotString(CspWorkCachePath / std::filesystem::path(ThumbnailRelativePath));
    }

    const std::filesystem::path WorkingTimePath = CspWorkCachePath / "workingTime.json";
    if (std::filesystem::exists(WorkingTimePath))
    {
        std::ifstream WorkingTimeStream(WorkingTimePath, std::ios::binary);
        if (WorkingTimeStream.is_open())
        {
            const std::string WorkingTimeJson {std::istreambuf_iterator<char>(WorkingTimeStream), std::istreambuf_iterator<char>()};
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

[[nodiscard]] std::vector<std::filesystem::path> GetCandidateRootPaths()
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

[[nodiscard]] std::vector<CspDiscordRpcGdCppWorkData> DiscoverCspWorks()
{
    std::vector<CspDiscordRpcGdCppWorkData> Works;
    std::unordered_set<std::wstring> SeenPaths;

    try
    {
        for (const std::filesystem::path& RootPath : GetCandidateRootPaths())
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
                    if (!WorkData.Name.is_empty())
                    {
                        Works.push_back(WorkData);
                    }

                    continue;
                }

                if (!HasSupportedWorkExtension(EntryPath))
                {
                    continue;
                }

                const std::wstring EntryKey = EntryPath.lexically_normal().wstring();
                if (!SeenPaths.insert(EntryKey).second)
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
    godot::ClassDB::bind_method(godot::D_METHOD("sync_to_viewport_size"), &CspDiscordRpcGdCppMainControl::SyncToViewportSize);
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_resize_handle_gui_input", "event", "resize_edge"), &CspDiscordRpcGdCppMainControl::OnResizeHandleGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_choose_csp_work_pressed"), &CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_csp_work_chosen", "work_name", "work_path"), &CspDiscordRpcGdCppMainControl::OnCspWorkChosen);
    godot::ClassDB::bind_method(godot::D_METHOD("on_update_presence_pressed"), &CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed);
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
    TitleBarPanel->set_custom_minimum_size(godot::Vector2(0.0F, 40.0F));
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->set_clip_contents(true);
    TitleBarPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.11F, 0.12F, 0.16F, 1.0F)));
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer = memnew(godot::HBoxContainer);
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->set_clip_contents(true);
    TitleBarContainer->add_theme_constant_override("separation", 4);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::Button* TitleBarButton = CreateTitleBarButton("CSP Discord RPC", "");
    TitleBarButton->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput));
    TitleBarContainer->add_child(TitleBarButton);

    godot::Button* MinimizeButton = CreateWindowControlButton(GetWindowControlIconTexture(EWindowControlIcon::Minimize), "Minimize");
    MinimizeButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnMinimizePressed));
    TitleBarContainer->add_child(MinimizeButton);

    MaximizeButton = CreateWindowControlButton(GetWindowControlIconTexture(EWindowControlIcon::Maximize), "Toggle maximize");
    MaximizeButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnMaximizePressed));
    TitleBarContainer->add_child(MaximizeButton);

    godot::Button* CloseButton = CreateWindowControlButton(GetWindowControlIconTexture(EWindowControlIcon::Close), "Close");
    CloseButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnClosePressed));
    TitleBarContainer->add_child(CloseButton);

    godot::PanelContainer* ContentPanel = memnew(godot::PanelContainer);
    ContentPanel->set_name("ContentPanel");
    ContentPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_clip_contents(true);
    ContentPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.16F, 0.17F, 0.22F, 1.0F)));
    RootContainer->add_child(ContentPanel);

    godot::ScrollContainer* ContentScrollContainer = memnew(godot::ScrollContainer);
    ContentScrollContainer->set_name("ContentScrollContainer");
    ContentScrollContainer->set_horizontal_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentScrollContainer->set_vertical_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentScrollContainer->set_follow_focus(true);
    ContentScrollContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentScrollContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->add_child(ContentScrollContainer);

    godot::MarginContainer* ContentMargin = memnew(godot::MarginContainer);
    ContentMargin->set_name("ContentMargin");
    ContentMargin->add_theme_constant_override("margin_left", 16);
    ContentMargin->add_theme_constant_override("margin_top", 16);
    ContentMargin->add_theme_constant_override("margin_right", 16);
    ContentMargin->add_theme_constant_override("margin_bottom", 16);
    ContentMargin->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentScrollContainer->add_child(ContentMargin);

    godot::GridContainer* PropertyGridContainer = memnew(godot::GridContainer);
    PropertyGridContainer->set_name("PropertyGridContainer");
    PropertyGridContainer->set_columns(2);
    PropertyGridContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    PropertyGridContainer->add_theme_constant_override("h_separation", 12);
    PropertyGridContainer->add_theme_constant_override("v_separation", 12);
    ContentMargin->add_child(PropertyGridContainer);

    DiscordRichPresenceCheckButton = CreateNamedControl<godot::CheckButton>("DiscordRichPresence");
    DiscordRichPresenceCheckButton->set_text("");
    DiscordRichPresenceCheckButton->connect("toggled", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnDiscordRichPresenceToggled));
    AddPropertyRow(PropertyGridContainer, "Discord Rich Presence", DiscordRichPresenceCheckButton);

    ChooseCSPWorkButton = CreateNamedControl<godot::Button>("ChooseCSPWork");
    ChooseCSPWorkButton->set_text("Browse");
    ChooseCSPWorkButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed));
    AddPropertyRow(PropertyGridContainer, "Choose CSP Work", ChooseCSPWorkButton);

    SmallImageKeyLineEdit = CreateNamedControl<godot::LineEdit>("SmallImageKey");
    SmallImageKeyLineEdit->set_placeholder("small_asset_key");
    AddPropertyRow(PropertyGridContainer, "Small Image Key", SmallImageKeyLineEdit);

    SmallImageTextLineEdit = CreateNamedControl<godot::LineEdit>("SmallImageText");
    SmallImageTextLineEdit->set_placeholder("Drawing something");
    AddPropertyRow(PropertyGridContainer, "Small Image Text", SmallImageTextLineEdit);

    ButtonLabelLineEdit = CreateNamedControl<godot::LineEdit>("ButtonLabel");
    ButtonLabelLineEdit->set_placeholder("Open My Website");
    AddPropertyRow(PropertyGridContainer, "Button Label", ButtonLabelLineEdit);

    ButtonUrlLineEdit = CreateNamedControl<godot::LineEdit>("ButtonUrl");
    ButtonUrlLineEdit->set_placeholder("https://example.com");
    AddPropertyRow(PropertyGridContainer, "Button URL", ButtonUrlLineEdit);

    UpdatePresenceButton = CreateNamedControl<godot::Button>("UpdatePresence");
    UpdatePresenceButton->set_text("Update");
    UpdatePresenceButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed));
    AddPropertyRow(PropertyGridContainer, "Update Presence", UpdatePresenceButton);

    StatusLabel = CreatePropertyLabel("Discord RPC is disabled.");
    StatusLabel->set_name("StatusLabel");
    AddPropertyRow(PropertyGridContainer, "Status", StatusLabel);

    AddResizeHandle("ResizeTopLeft",
                    godot::DisplayServer::WINDOW_EDGE_TOP_LEFT,
                    godot::Control::CURSOR_FDIAGSIZE,
                    0.0F,
                    0.0F,
                    0.0F,
                    0.0F,
                    0.0F,
                    0.0F,
                    ResizeCornerExtent,
                    ResizeCornerExtent);
    AddResizeHandle("ResizeTop",
                    godot::DisplayServer::WINDOW_EDGE_TOP,
                    godot::Control::CURSOR_VSIZE,
                    0.0F,
                    0.0F,
                    1.0F,
                    0.0F,
                    ResizeCornerExtent,
                    0.0F,
                    -ResizeCornerExtent,
                    ResizeBorderThickness);
    AddResizeHandle("ResizeTopRight",
                    godot::DisplayServer::WINDOW_EDGE_TOP_RIGHT,
                    godot::Control::CURSOR_BDIAGSIZE,
                    1.0F,
                    0.0F,
                    1.0F,
                    0.0F,
                    -ResizeCornerExtent,
                    0.0F,
                    0.0F,
                    ResizeCornerExtent);
    AddResizeHandle("ResizeLeft",
                    godot::DisplayServer::WINDOW_EDGE_LEFT,
                    godot::Control::CURSOR_HSIZE,
                    0.0F,
                    0.0F,
                    0.0F,
                    1.0F,
                    0.0F,
                    ResizeCornerExtent,
                    ResizeBorderThickness,
                    -ResizeCornerExtent);
    AddResizeHandle("ResizeRight",
                    godot::DisplayServer::WINDOW_EDGE_RIGHT,
                    godot::Control::CURSOR_HSIZE,
                    1.0F,
                    0.0F,
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

    UpdateMaximizeButtonIcon();
}

void CspDiscordRpcGdCppMainControl::_exit_tree()
{
    CspDiscordRpcService::Get().Shutdown();
    PresenceStartTimestamp = 0;
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

    MaximizeButton->set_button_icon(GetWindowControlIconTexture(bIsWindowMaximized ? EWindowControlIcon::Restore : EWindowControlIcon::Maximize));
}

void CspDiscordRpcGdCppMainControl::AddPropertyRow(godot::GridContainer* GridContainer, const godot::String& LabelText, godot::Control* EditorControl)
{
    GridContainer->add_child(CreatePropertyLabel(LabelText));
    GridContainer->add_child(EditorControl);
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

godot::Button* CspDiscordRpcGdCppMainControl::CreateTitleBarButton(const godot::String& Text, const godot::String& TooltipText) const
{
    godot::Button* TitleBarButton = memnew(godot::Button);
    TitleBarButton->set_name("TitleBarButton");
    TitleBarButton->set_text(Text);
    TitleBarButton->set_flat(true);
    TitleBarButton->set_focus_mode(godot::Control::FOCUS_NONE);
    TitleBarButton->set_clip_text(true);
    TitleBarButton->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    TitleBarButton->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarButton->set_tooltip_text(TooltipText);
    return TitleBarButton;
}

godot::Button* CspDiscordRpcGdCppMainControl::CreateWindowControlButton(const godot::Ref<godot::Texture2D>& Icon, const godot::String& TooltipText) const
{
    godot::Button* WindowControlButton = memnew(godot::Button);
    if (Icon.is_valid())
    {
        WindowControlButton->set_button_icon(Icon);
        WindowControlButton->set_expand_icon(false);
    }

    WindowControlButton->set_flat(true);
    WindowControlButton->set_focus_mode(godot::Control::FOCUS_NONE);
    WindowControlButton->set_custom_minimum_size(godot::Vector2(36.0F, 28.0F));
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
    godot::DisplayServer::get_singleton()->window_set_mode(godot::DisplayServer::WINDOW_MODE_MINIMIZED);
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

void CspDiscordRpcGdCppMainControl::OnClosePressed()
{
    if (godot::SceneTree* SceneTree = get_tree())
    {
        SceneTree->quit();
    }
}

void CspDiscordRpcGdCppMainControl::OnChooseCspWorkPressed()
{
    const std::vector<CspDiscordRpcGdCppWorkData> Works = DiscoverCspWorks();
    if (Works.empty())
    {
        UpdateStatusText("No CSP work files were found under the CELSYS folders.");
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
    WorksWindow->popup_centered(godot::Vector2i(960, 640));
}

void CspDiscordRpcGdCppMainControl::OnCspWorkChosen(const godot::String& WorkName, const godot::String& WorkPath)
{
    SelectedCSPWorkPath = WorkPath;

    if (ChooseCSPWorkButton != nullptr)
    {
        ChooseCSPWorkButton->set_text(WorkName.is_empty() ? godot::String("Browse") : WorkName);
        ChooseCSPWorkButton->set_tooltip_text(WorkPath);
    }

    UpdateStatusText(godot::vformat("Selected CSP work: %s", WorkName));
}

void CspDiscordRpcGdCppMainControl::OnWorksWindowTreeExited()
{
    WorksWindow = nullptr;
}

void CspDiscordRpcGdCppMainControl::OnDiscordRichPresenceToggled(bool bToggled)
{
    if (bToggled)
    {
        static const godot::String CLIENT_ID = "1351785436850163733";

        if (!CspDiscordRpcService::Get().Initialize(CLIENT_ID))
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

void CspDiscordRpcGdCppMainControl::OnUpdatePresencePressed()
{
    if (!CspDiscordRpcService::Get().IsInitialized())
    {
        UpdateStatusText("Enable Discord RPC first.");
        return;
    }

    DiscordRichPresenceData PresenceData;
    PresenceData.State = "";
    PresenceData.Details = "";
    PresenceData.LargeImageKey = "";
    PresenceData.LargeImageText = "";
    PresenceData.SmallImageKey = SmallImageKeyLineEdit != nullptr ? SmallImageKeyLineEdit->get_text() : godot::String();
    PresenceData.SmallImageText = SmallImageTextLineEdit != nullptr ? SmallImageTextLineEdit->get_text() : godot::String();
    PresenceData.ButtonLabel = ButtonLabelLineEdit != nullptr ? ButtonLabelLineEdit->get_text() : godot::String();
    PresenceData.ButtonUrl = ButtonUrlLineEdit != nullptr ? ButtonUrlLineEdit->get_text() : godot::String();
    PresenceData.StartTimestamp = PresenceStartTimestamp;

    CspDiscordRpcService::Get().UpdatePresence(PresenceData);
    UpdateStatusText("Presence update sent. Discord must be running locally for it to appear.");
}

} // namespace CspDiscordRpcGdCpp
