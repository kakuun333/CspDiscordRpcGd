#include "CspDiscordRpcGdCppWorksWindow.h"

#include "CspDiscordRpcGdCppWorkItem.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/grid_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/input_event_mouse_motion.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/line_edit.hpp"
#include "godot_cpp/classes/margin_container.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/scroll_container.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/color.hpp"

namespace
{


constexpr int32_t PreferredWorksWindowWidth{ 960 };
constexpr int32_t PreferredWorksWindowHeight{ 640 };
constexpr int32_t MinimumWorksWindowWidth{ 420 };
constexpr int32_t MinimumWorksWindowHeight{ 320 };
constexpr int32_t WorksWindowViewportPadding{ 32 };
constexpr int32_t WorksWindowContentHorizontalInsets{ 64 };
constexpr int32_t WorkItemMinimumWidth{ 180 };
constexpr int32_t WorkGridHorizontalSeparation{ 16 };
constexpr float ActionButtonMinimumHeight{ 32.0F };

[[nodiscard]] int32_t ClampInt32(const int32_t Value, const int32_t MinValue, const int32_t MaxValue)
{
    if (MaxValue < MinValue)
    {
        return MinValue;
    }

    if (Value < MinValue)
    {
        return MinValue;
    }

    if (Value > MaxValue)
    {
        return MaxValue;
    }

    return Value;
}


[[nodiscard]] godot::Vector2 GetMouseScreenPosition()
{
    godot::DisplayServer* DisplayServer{ godot::DisplayServer::get_singleton() };
    if (DisplayServer == nullptr)
    {
        return {};
    }

    const godot::Vector2i MousePosition{ DisplayServer->mouse_get_position() };
    return { static_cast<float>(MousePosition.x), static_cast<float>(MousePosition.y) };
}

[[nodiscard]] godot::Vector2i RoundVector2ToVector2i(const godot::Vector2& Value)
{
    return { static_cast<int32_t>(Value.x + (Value.x >= 0.0F ? 0.5F : -0.5F)),
             static_cast<int32_t>(Value.y + (Value.y >= 0.0F ? 0.5F : -0.5F)) };
}
[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateDialogPanelStyle()
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(godot::Color::hex(0x202838ff));
    Style->set_border_width_all(0);
    Style->set_corner_radius_all(12);
    Style->set_shadow_color(godot::Color::hex(0x05070bcc));
    Style->set_shadow_size(8);
    Style->set_shadow_offset({ 0.0F, 2.0F });
    return Style;
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

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateTitleBarPanelStyle()
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(godot::Color::hex(0x1c1f29ff));
    Style->set_border_color(godot::Color::hex(0x6dabe4ff));
    Style->set_border_width(godot::SIDE_BOTTOM, 1);
    Style->set_corner_radius(godot::CORNER_TOP_LEFT, 12);
    Style->set_corner_radius(godot::CORNER_TOP_RIGHT, 12);
    Style->set_corner_radius(godot::CORNER_BOTTOM_LEFT, 0);
    Style->set_corner_radius(godot::CORNER_BOTTOM_RIGHT, 0);
    Style->set_content_margin(godot::SIDE_LEFT, 0.0F);
    Style->set_content_margin(godot::SIDE_TOP, 0.0F);
    Style->set_content_margin(godot::SIDE_RIGHT, 0.0F);
    Style->set_content_margin(godot::SIDE_BOTTOM, 0.0F);
    return Style;
}

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateInputStyle(const godot::Color& BackgroundColor, const godot::Color& BorderColor)
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(BackgroundColor);
    Style->set_border_color(BorderColor);
    Style->set_border_width_all(1);
    Style->set_corner_radius_all(8);
    Style->set_content_margin(godot::SIDE_LEFT, 10.0F);
    Style->set_content_margin(godot::SIDE_TOP, 7.0F);
    Style->set_content_margin(godot::SIDE_RIGHT, 10.0F);
    Style->set_content_margin(godot::SIDE_BOTTOM, 7.0F);
    return Style;
}

void ApplyButtonVisualStyle(godot::Button* ButtonNode, const bool bPrimary = false)
{
    if (ButtonNode == nullptr)
    {
        return;
    }

    const godot::Color NormalColor{ bPrimary ? godot::Color::hex(0x33445cff) : godot::Color::hex(0x242a36ff) };
    const godot::Color HoverColor{ bPrimary ? godot::Color::hex(0x405a78ff) : godot::Color::hex(0x30394aff) };
    const godot::Color PressedColor{ bPrimary ? godot::Color::hex(0x2d3b52ff) : godot::Color::hex(0x202631ff) };
    const godot::Color BorderColor{ godot::Color::hex(0x526982ff) };

    ButtonNode->set_focus_mode(godot::Control::FOCUS_NONE);
    ButtonNode->add_theme_stylebox_override("normal", CreateInputStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_stylebox_override("hover", CreateInputStyle(HoverColor, godot::Color::hex(0x9fcef7ff)));
    ButtonNode->add_theme_stylebox_override("pressed", CreateInputStyle(PressedColor, godot::Color::hex(0x9fcef7ff)));
    ButtonNode->add_theme_stylebox_override("focus", CreateInputStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
    ButtonNode->add_theme_color_override("font_hover_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_pressed_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_focus_color", godot::Color::hex(0xe0e0e0ff));
    ButtonNode->add_theme_color_override("font_disabled_color", godot::Color::hex(0x7a8293ff));
}

void ApplyLabelVisualStyle(godot::Label* LabelNode, const godot::Color& Color = godot::Color::hex(0xf0f4fbff))
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

    LineEditNode->add_theme_stylebox_override("normal", CreateInputStyle(godot::Color::hex(0x141923ff), godot::Color::hex(0x526982ff)));
    LineEditNode->add_theme_stylebox_override("focus", CreateInputStyle(godot::Color::hex(0x182132ff), godot::Color::hex(0x9fcef7ff)));
    LineEditNode->add_theme_color_override("font_color", godot::Color::hex(0xffffffff));
    LineEditNode->add_theme_color_override("font_placeholder_color", godot::Color::hex(0x9aa8b8ff));
    LineEditNode->add_theme_color_override("caret_color", godot::Color::hex(0x9fcef7ff));
}

void ApplyTitleBarLabelVisualStyle(godot::Label* LabelNode)
{
    if (LabelNode == nullptr)
    {
        return;
    }

    LabelNode->add_theme_font_size_override("font_size", 15);
    LabelNode->add_theme_color_override("font_color", godot::Color::hex(0xffffffff));
}

[[nodiscard]] godot::Label* CreateTitleBarLabel(const godot::String& Text)
{
    godot::Label* TitleBarLabel{ memnew(godot::Label) };
    TitleBarLabel->set_name("TitleBarLabel");
    TitleBarLabel->set_text(Text);
    TitleBarLabel->set_clip_text(true);
    TitleBarLabel->set_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    TitleBarLabel->set_vertical_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    TitleBarLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarLabel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarLabel->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    ApplyTitleBarLabelVisualStyle(TitleBarLabel);
    return TitleBarLabel;
}

[[nodiscard]] godot::Button* CreateActionButton(const godot::String& Text)
{
    godot::Button* ActionButton{ memnew(godot::Button) };
    ActionButton->set_text(Text);
    ActionButton->set_clip_text(true);
    ActionButton->set_custom_minimum_size({ 0.0F, ActionButtonMinimumHeight });
    ActionButton->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ApplyButtonVisualStyle(ActionButton);
    return ActionButton;
}

[[nodiscard]] godot::String NormalizeSearchString(const godot::String& Text)
{
    const godot::String LowerText{ Text.strip_edges().to_lower() };
    godot::String NormalizedText;

    for (int64_t CharacterIndex{}; CharacterIndex < LowerText.length(); ++CharacterIndex)
    {
        int64_t CodePoint{ LowerText.unicode_at(CharacterIndex) };

        if (CodePoint >= 0x30A1 && CodePoint <= 0x30F6)
        {
            CodePoint -= 0x60;
        }

        NormalizedText += godot::String::chr(CodePoint);
    }

    return NormalizedText;
}

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppWorksWindow::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_search_text_changed", "new_text"), &CspDiscordRpcGdCppWorksWindow::OnSearchTextChanged);
    godot::ClassDB::bind_method(godot::D_METHOD("on_window_size_changed"), &CspDiscordRpcGdCppWorksWindow::OnWindowSizeChanged);
    godot::ClassDB::bind_method(godot::D_METHOD("on_work_item_pressed", "work_name", "work_path"), &CspDiscordRpcGdCppWorksWindow::OnWorkItemPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_cancel_pressed"), &CspDiscordRpcGdCppWorksWindow::OnCancelPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_choose_pressed"), &CspDiscordRpcGdCppWorksWindow::OnChoosePressed);

    ADD_SIGNAL(godot::MethodInfo("work_chosen",
                                 godot::PropertyInfo(godot::Variant::STRING, "work_name"),
                                 godot::PropertyInfo(godot::Variant::STRING, "work_path")));
}

void CspDiscordRpcGdCppWorksWindow::_ready()
{
    EnsureUiBuilt();
    RebuildWorkItems();
    UpdateResponsiveLayout();
}

void CspDiscordRpcGdCppWorksWindow::_input(const godot::Ref<godot::InputEvent>& Event)
{
    ReleaseFocusedLineEditOnOutsideMouseClick(RootContainer, Event);

    if (!bDragging)
    {
        return;
    }

    const godot::Ref<godot::InputEventMouseButton> MouseButton{ Event };
    if (MouseButton.is_valid() && MouseButton->get_button_index() == godot::MOUSE_BUTTON_LEFT && !MouseButton->is_pressed())
    {
        bDragging = false;
        ClampToBounds();
        return;
    }

    const godot::Ref<godot::InputEventMouseMotion> MouseMotion{ Event };
    if (MouseMotion.is_valid())
    {
        UpdateBoundedDrag(GetMouseScreenPosition());
    }
}

void CspDiscordRpcGdCppWorksWindow::SetBoundsSize(const godot::Vector2i& NewBoundsSize)
{
    BoundsSize = NewBoundsSize;
}

void CspDiscordRpcGdCppWorksWindow::ApplyBoundsLayout(const bool bCenterInBounds)
{
    EnsureUiBuilt();
    set_size(GetBoundedWindowSize());
    UpdateResponsiveLayout();

    if (bCenterInBounds)
    {
        set_position(GetCenteredPosition());
    }

    ClampToBounds();
}

void CspDiscordRpcGdCppWorksWindow::ClampToBounds()
{
    set_position(GetClampedPosition(get_position()));
}

void CspDiscordRpcGdCppWorksWindow::SetWorks(const std::vector<CspDiscordRpcGdCppWorkData>& InWorks)
{
    Works = InWorks;
    SearchText = "";
    SelectedWorkName = "";
    SelectedWorkPath = "";

    if (SearchLineEdit != nullptr)
    {
        SearchLineEdit->set_text("");
    }

    RebuildWorkItems();
}

const godot::String& CspDiscordRpcGdCppWorksWindow::GetSelectedWorkName() const
{
    return SelectedWorkName;
}

const godot::String& CspDiscordRpcGdCppWorksWindow::GetSelectedWorkPath() const
{
    return SelectedWorkPath;
}

void CspDiscordRpcGdCppWorksWindow::EnsureUiBuilt()
{
    if (RootContainer != nullptr)
    {
        return;
    }

    set_name("CspDiscordRpcGdCppWorksWindow");
    set_title("Choose CSP Work");
    set_flag(godot::Window::FLAG_BORDERLESS, true);
    set_flag(godot::Window::FLAG_TRANSPARENT, true);
    set_transient(true);
    set_wrap_controls(false);
    set_size({ PreferredWorksWindowWidth, PreferredWorksWindowHeight });
    set_min_size({ 1, 1 });
    connect("size_changed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnWindowSizeChanged));


    godot::PanelContainer* DialogPanel{ memnew(godot::PanelContainer) };
    DialogPanel->set_name("DialogPanel");
    DialogPanel->set_anchor(godot::SIDE_LEFT, 0.0F);
    DialogPanel->set_anchor(godot::SIDE_TOP, 0.0F);
    DialogPanel->set_anchor(godot::SIDE_RIGHT, 1.0F);
    DialogPanel->set_anchor(godot::SIDE_BOTTOM, 1.0F);
    DialogPanel->set_offset(godot::SIDE_LEFT, 8.0F);
    DialogPanel->set_offset(godot::SIDE_TOP, 8.0F);
    DialogPanel->set_offset(godot::SIDE_RIGHT, -8.0F);
    DialogPanel->set_offset(godot::SIDE_BOTTOM, -22.0F);
    DialogPanel->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    DialogPanel->add_theme_stylebox_override("panel", CreateDialogPanelStyle());
    DialogPanel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput));
    add_child(DialogPanel);

    RootContainer = memnew(godot::VBoxContainer);
    RootContainer->set_name("RootContainer");
    RootContainer->set_anchors_preset(godot::Control::PRESET_FULL_RECT);
    RootContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->add_theme_constant_override("separation", 0);
    DialogPanel->add_child(RootContainer);

    godot::PanelContainer* TitleBarPanel{ memnew(godot::PanelContainer) };
    TitleBarPanel->set_name("TitleBarPanel");
    TitleBarPanel->set_custom_minimum_size({ 0.0F, 40.0F });
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->set_clip_contents(true);
    TitleBarPanel->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    TitleBarPanel->add_theme_stylebox_override("panel", CreateTitleBarPanelStyle());
    TitleBarPanel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput));
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer{ memnew(godot::HBoxContainer) };
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->set_clip_contents(true);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::MarginContainer* TitleBarMargin{ memnew(godot::MarginContainer) };
    TitleBarMargin->set_name("TitleBarMargin");
    TitleBarMargin->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarMargin->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarMargin->add_theme_constant_override("margin_left", 10);
    TitleBarMargin->add_theme_constant_override("margin_right", 10);
    TitleBarContainer->add_child(TitleBarMargin);

    godot::Label* TitleBarLabel{ CreateTitleBarLabel("Choose CSP Work") };
    TitleBarLabel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput));
    TitleBarMargin->add_child(TitleBarLabel);

    godot::MarginContainer* DialogMargin{ memnew(godot::MarginContainer) };
    DialogMargin->set_name("DialogMargin");
    DialogMargin->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    DialogMargin->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    DialogMargin->add_theme_constant_override("margin_left", 16);
    DialogMargin->add_theme_constant_override("margin_top", 8);
    DialogMargin->add_theme_constant_override("margin_right", 16);
    DialogMargin->add_theme_constant_override("margin_bottom", 12);
    RootContainer->add_child(DialogMargin);

    godot::VBoxContainer* BodyContainer{ memnew(godot::VBoxContainer) };
    BodyContainer->set_name("BodyContainer");
    BodyContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    BodyContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    BodyContainer->add_theme_constant_override("separation", 10);
    DialogMargin->add_child(BodyContainer);

    godot::Control* ContentViewport{ memnew(godot::Control) };
    ContentViewport->set_name("ContentViewport");
    ContentViewport->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentViewport->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentViewport->set_clip_contents(true);
    BodyContainer->add_child(ContentViewport);

    godot::VBoxContainer* ContentContainer{ memnew(godot::VBoxContainer) };
    ContentContainer->set_name("ContentContainer");
    ContentContainer->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    ContentContainer->add_theme_constant_override("separation", 10);
    ContentViewport->add_child(ContentContainer);

    SearchLineEdit = memnew(godot::LineEdit);
    SearchLineEdit->set_name("SearchLineEdit");
    SearchLineEdit->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SearchLineEdit->set_custom_minimum_size({ 0.0F, 36.0F });
    SearchLineEdit->set_placeholder("Search work name...");
    SearchLineEdit->set_clear_button_enabled(true);
    ApplyLineEditVisualStyle(SearchLineEdit);
    SearchLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnSearchTextChanged));
    ContentContainer->add_child(SearchLineEdit);

    godot::ScrollContainer* ScrollContainer{ memnew(godot::ScrollContainer) };
    ScrollContainer->set_name("ScrollContainer");
    ScrollContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ScrollContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ScrollContainer->set_horizontal_scroll_mode(godot::ScrollContainer::SCROLL_MODE_DISABLED);
    ScrollContainer->set_vertical_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentContainer->add_child(ScrollContainer);

    WorkGridContainer = memnew(godot::GridContainer);
    WorkGridContainer->set_name("WorkGridContainer");
    WorkGridContainer->set_columns(1);
    WorkGridContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    WorkGridContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    WorkGridContainer->add_theme_constant_override("h_separation", WorkGridHorizontalSeparation);
    WorkGridContainer->add_theme_constant_override("v_separation", WorkGridHorizontalSeparation);
    ScrollContainer->add_child(WorkGridContainer);

    EmptyStateLabel = memnew(godot::Label);
    EmptyStateLabel->set_name("EmptyStateLabel");
    EmptyStateLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    EmptyStateLabel->set_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
    ApplyLabelVisualStyle(EmptyStateLabel, godot::Color::hex(0xb7d7f2ff));
    EmptyStateLabel->set_text("No works match the current search.");
    EmptyStateLabel->set_visible(false);
    ContentContainer->add_child(EmptyStateLabel);

    godot::HBoxContainer* FooterContainer{ memnew(godot::HBoxContainer) };
    FooterContainer->set_name("FooterContainer");
    FooterContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    FooterContainer->set_alignment(godot::BoxContainer::ALIGNMENT_END);
    FooterContainer->add_theme_constant_override("separation", 8);
    BodyContainer->add_child(FooterContainer);

    godot::Button* CancelButton{ CreateActionButton("Cancel") };
    CancelButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnCancelPressed));
    FooterContainer->add_child(CancelButton);

    ChooseButton = CreateActionButton("Choose");
    ApplyButtonVisualStyle(ChooseButton, true);
    ChooseButton->set_disabled(true);
    ChooseButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnChoosePressed));
    FooterContainer->add_child(ChooseButton);

    UpdateResponsiveLayout();
}

void CspDiscordRpcGdCppWorksWindow::RebuildWorkItems()
{
    if (WorkGridContainer == nullptr)
    {
        return;
    }

    SyncSelectionWithFilteredWorks();

    for (int32_t ChildIndex = WorkGridContainer->get_child_count() - 1; ChildIndex >= 0; --ChildIndex)
    {
        if (godot::Node* ChildNode = WorkGridContainer->get_child(ChildIndex))
        {
            WorkGridContainer->remove_child(ChildNode);
            ChildNode->queue_free();
        }
    }

    int32_t VisibleWorkCount = 0;

    for (const CspDiscordRpcGdCppWorkData& Work : Works)
    {
        if (!MatchesSearchText(Work))
        {
            continue;
        }

        CspDiscordRpcGdCppWorkItem* WorkItem = memnew(CspDiscordRpcGdCppWorkItem);
        WorkItem->SetWorkData(Work);
        WorkItem->SetSelected(Work.CacheDataPath == SelectedWorkPath && !SelectedWorkPath.is_empty());
        WorkItem->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnWorkItemPressed));
        WorkGridContainer->add_child(WorkItem);
        ++VisibleWorkCount;
    }

    if (EmptyStateLabel != nullptr)
    {
        EmptyStateLabel->set_visible(VisibleWorkCount == 0);
    }

    UpdateChooseButtonState();
}

void CspDiscordRpcGdCppWorksWindow::SyncSelectionWithFilteredWorks()
{
    if (SelectedWorkPath.is_empty())
    {
        return;
    }

    for (const CspDiscordRpcGdCppWorkData& Work : Works)
    {
        if (Work.CacheDataPath == SelectedWorkPath && MatchesSearchText(Work))
        {
            return;
        }
    }

    SelectedWorkName = "";
    SelectedWorkPath = "";
}

void CspDiscordRpcGdCppWorksWindow::UpdateResponsiveLayout() const
{
    if (WorkGridContainer == nullptr)
    {
        return;
    }

    const int32_t AvailableContentWidth{ get_size().x - WorksWindowContentHorizontalInsets };
    const int32_t ThreeColumnMinimumWidth{ WorkItemMinimumWidth * 3 + WorkGridHorizontalSeparation * 2 };
    const int32_t TwoColumnMinimumWidth{ WorkItemMinimumWidth * 2 + WorkGridHorizontalSeparation };

    if (AvailableContentWidth >= ThreeColumnMinimumWidth)
    {
        WorkGridContainer->set_columns(3);
    }
    else if (AvailableContentWidth >= TwoColumnMinimumWidth)
    {
        WorkGridContainer->set_columns(2);
    }
    else
    {
        WorkGridContainer->set_columns(1);
    }
}

void CspDiscordRpcGdCppWorksWindow::UpdateChooseButtonState() const
{
    if (ChooseButton != nullptr)
    {
        ChooseButton->set_disabled(SelectedWorkPath.is_empty());
    }
}

bool CspDiscordRpcGdCppWorksWindow::MatchesSearchText(const CspDiscordRpcGdCppWorkData& Work) const
{
    const godot::String NormalizedSearchText = NormalizeSearchString(SearchText);
    if (NormalizedSearchText.is_empty())
    {
        return true;
    }

    return NormalizeSearchString(Work.Name).contains(NormalizedSearchText);
}

void CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::Ref<godot::InputEventMouseButton> MouseButton{ Event };
    if (!MouseButton.is_valid() || MouseButton->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButton->is_pressed())
    {
        return;
    }

    StartBoundedDrag(GetMouseScreenPosition());
}

void CspDiscordRpcGdCppWorksWindow::StartBoundedDrag(const godot::Vector2& GlobalMousePosition)
{
    bDragging = true;
    const godot::Vector2i CurrentPosition{ get_position() };
    DragMouseOffset = GlobalMousePosition - godot::Vector2(static_cast<float>(CurrentPosition.x), static_cast<float>(CurrentPosition.y));
}

void CspDiscordRpcGdCppWorksWindow::UpdateBoundedDrag(const godot::Vector2& GlobalMousePosition)
{
    if (!bDragging)
    {
        return;
    }

    set_position(GetClampedPosition(RoundVector2ToVector2i(GlobalMousePosition - DragMouseOffset)));
}

godot::Vector2i CspDiscordRpcGdCppWorksWindow::GetResolvedBoundsSize() const
{
    if (BoundsSize.x > 0 && BoundsSize.y > 0)
    {
        return BoundsSize;
    }

    godot::Window* ParentWindow{ godot::Object::cast_to<godot::Window>(get_parent()) };
    if (ParentWindow != nullptr)
    {
        return ParentWindow->get_size();
    }

    godot::Viewport* Viewport{ get_viewport() };
    if (Viewport != nullptr)
    {
        return RoundVector2ToVector2i(Viewport->get_visible_rect().size);
    }

    return get_size();
}

godot::Vector2i CspDiscordRpcGdCppWorksWindow::GetBoundedWindowSize() const
{
    const godot::Vector2i ResolvedBoundsSize{ GetResolvedBoundsSize() };
    const int32_t AvailableWidth{ ResolvedBoundsSize.x - WorksWindowViewportPadding };
    const int32_t AvailableHeight{ ResolvedBoundsSize.y - WorksWindowViewportPadding };
    const int32_t Width{ AvailableWidth >= MinimumWorksWindowWidth ?
                             ClampInt32(AvailableWidth, MinimumWorksWindowWidth, PreferredWorksWindowWidth) :
                             ClampInt32(AvailableWidth, 1, PreferredWorksWindowWidth) };
    const int32_t Height{ AvailableHeight >= MinimumWorksWindowHeight ?
                              ClampInt32(AvailableHeight, MinimumWorksWindowHeight, PreferredWorksWindowHeight) :
                              ClampInt32(AvailableHeight, 1, PreferredWorksWindowHeight) };
    return { Width, Height };
}

godot::Vector2i CspDiscordRpcGdCppWorksWindow::GetCenteredPosition() const
{
    const godot::Vector2i ResolvedBoundsSize{ GetResolvedBoundsSize() };
    const godot::Vector2i WindowSize{ get_size() };
    return GetClampedPosition({ (ResolvedBoundsSize.x - WindowSize.x) / 2, (ResolvedBoundsSize.y - WindowSize.y) / 2 });
}

godot::Vector2i CspDiscordRpcGdCppWorksWindow::GetClampedPosition(const godot::Vector2i& CandidatePosition) const
{
    const godot::Vector2i ResolvedBoundsSize{ GetResolvedBoundsSize() };
    const godot::Vector2i WindowSize{ get_size() };
    const int32_t MaxX{ ResolvedBoundsSize.x > WindowSize.x ? ResolvedBoundsSize.x - WindowSize.x : 0 };
    const int32_t MaxY{ ResolvedBoundsSize.y > WindowSize.y ? ResolvedBoundsSize.y - WindowSize.y : 0 };
    return { ClampInt32(CandidatePosition.x, 0, MaxX), ClampInt32(CandidatePosition.y, 0, MaxY) };
}

void CspDiscordRpcGdCppWorksWindow::OnSearchTextChanged(const godot::String& NewText)
{
    SearchText = NewText;
    RebuildWorkItems();
}

void CspDiscordRpcGdCppWorksWindow::OnWindowSizeChanged()
{
    UpdateResponsiveLayout();
}

void CspDiscordRpcGdCppWorksWindow::OnWorkItemPressed(const godot::String& WorkName, const godot::String& WorkPath)
{
    SelectedWorkName = WorkName;
    SelectedWorkPath = WorkPath;

    for (int32_t ChildIndex = 0; ChildIndex < WorkGridContainer->get_child_count(); ++ChildIndex)
    {
        CspDiscordRpcGdCppWorkItem* WorkItem = godot::Object::cast_to<CspDiscordRpcGdCppWorkItem>(WorkGridContainer->get_child(ChildIndex));
        if (WorkItem == nullptr)
        {
            continue;
        }

        WorkItem->SetSelected(WorkItem->GetWorkPath() == SelectedWorkPath);
    }

    UpdateChooseButtonState();
}

void CspDiscordRpcGdCppWorksWindow::OnCancelPressed()
{
    queue_free();
}

void CspDiscordRpcGdCppWorksWindow::OnChoosePressed()
{
    if (SelectedWorkPath.is_empty())
    {
        return;
    }

    emit_signal("work_chosen", SelectedWorkName, SelectedWorkPath);
    queue_free();
}

} // namespace CspDiscordRpcGdCpp
