#include "CspDiscordRpcGdCppMainControl.h"

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
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/scroll_container.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include <algorithm>
#include <cstdint>

namespace
{

constexpr float ResizeBorderThickness = 3.0f;
constexpr float ResizeCornerExtent = 12.0f;

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

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppMainControl::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("sync_to_viewport_size"), &CspDiscordRpcGdCppMainControl::SyncToViewportSize);
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppMainControl::OnTitleBarGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_resize_handle_gui_input", "event", "resize_edge"), &CspDiscordRpcGdCppMainControl::OnResizeHandleGuiInput);
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

    godot::CheckButton* DiscordRichPresenceCheckButton = CreateNamedControl<godot::CheckButton>("Discord Rich Presence");
    DiscordRichPresenceCheckButton->set_text("");
    AddPropertyRow(PropertyGridContainer, "Discord Rich Presence", DiscordRichPresenceCheckButton);

    godot::Button* ChooseCSPWorkButton = CreateNamedControl<godot::Button>("Choose CSP Work");
    ChooseCSPWorkButton->set_text("Choose CSP Work");
    AddPropertyRow(PropertyGridContainer, "Choose CSP Work", ChooseCSPWorkButton);

    godot::LineEdit* SmallImageSourceLineEdit = CreateNamedControl<godot::LineEdit>("Small Image Source");
    SmallImageSourceLineEdit->set_placeholder("https://example/image.png");
    AddPropertyRow(PropertyGridContainer, "Small Image Source", SmallImageSourceLineEdit);

    godot::LineEdit* SmallImageTextLineEdit = CreateNamedControl<godot::LineEdit>("Small Image Text");
    SmallImageTextLineEdit->set_placeholder("Drawing something");
    AddPropertyRow(PropertyGridContainer, "Small Image Text", SmallImageTextLineEdit);

    godot::Button* UpdatePresenceButton = CreateNamedControl<godot::Button>("Update Presence");
    UpdatePresenceButton->set_text("Update Presence");
    AddPropertyRow(PropertyGridContainer, "Update Presence", UpdatePresenceButton);

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

void CspDiscordRpcGdCppMainControl::AddPropertyRow(godot::GridContainer* GridContainer,
                                                   const godot::String& LabelText,
                                                   godot::Control* EditorControl)
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

godot::Button* CspDiscordRpcGdCppMainControl::CreateTitleBarButton(const godot::String& Text,
                                                                   const godot::String& TooltipText) const
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
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT ||
        !MouseButtonEvent->is_pressed())
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
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT ||
        !MouseButtonEvent->is_pressed())
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

} // namespace CspDiscordRpcGdCpp
