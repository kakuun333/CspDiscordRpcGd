#include "CspDiscordRpcGdCppCloseWindow.h"

#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/check_box.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/margin_container.hpp"
#include "godot_cpp/classes/option_button.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/text_server.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/color.hpp"

namespace
{

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

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateButtonStyle(const godot::Color& BackgroundColor, const godot::Color& BorderColor)
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(BackgroundColor);
    Style->set_border_color(BorderColor);
    Style->set_border_width_all(1);
    Style->set_corner_radius_all(8);
    Style->set_content_margin(godot::SIDE_LEFT, 10.0F);
    Style->set_content_margin(godot::SIDE_TOP, 4.0F);
    Style->set_content_margin(godot::SIDE_RIGHT, 10.0F);
    Style->set_content_margin(godot::SIDE_BOTTOM, 4.0F);
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
    const godot::Color BorderColor{ bPrimary ? godot::Color::hex(0x526982ff) : godot::Color::hex(0x526982ff) };

    ButtonNode->set_focus_mode(godot::Control::FOCUS_NONE);
    ButtonNode->add_theme_stylebox_override("normal", CreateButtonStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_stylebox_override("hover", CreateButtonStyle(HoverColor, godot::Color::hex(0x9fcef7ff)));
    ButtonNode->add_theme_stylebox_override("pressed", CreateButtonStyle(PressedColor, godot::Color::hex(0x9fcef7ff)));
    ButtonNode->add_theme_stylebox_override("focus", CreateButtonStyle(NormalColor, BorderColor));
    ButtonNode->add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
    ButtonNode->add_theme_color_override("font_hover_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_pressed_color", godot::Color::hex(0xffffffff));
    ButtonNode->add_theme_color_override("font_focus_color", godot::Color::hex(0xe0e0e0ff));
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
    ActionButton->set_custom_minimum_size({ 76.0F, 24.0F });
    ApplyButtonVisualStyle(ActionButton);
    return ActionButton;
}

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppCloseWindow::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput);
    godot::ClassDB::bind_method(godot::D_METHOD("on_cancel_pressed"), &CspDiscordRpcGdCppCloseWindow::OnCancelPressed);
    godot::ClassDB::bind_method(godot::D_METHOD("on_confirm_pressed"), &CspDiscordRpcGdCppCloseWindow::OnConfirmPressed);

    ADD_SIGNAL(godot::MethodInfo("confirmed",
                                 godot::PropertyInfo(godot::Variant::INT, "close_action"),
                                 godot::PropertyInfo(godot::Variant::BOOL, "dont_show_again")));
    ADD_SIGNAL(godot::MethodInfo("cancelled"));
}

void CspDiscordRpcGdCppCloseWindow::_ready()
{
    EnsureUiBuilt();
}

void CspDiscordRpcGdCppCloseWindow::SetSelectedCloseAction(ECloseAction InCloseAction) const
{
    if (CloseActionOptionButton != nullptr)
    {
        CloseActionOptionButton->select(static_cast<int32_t>(InCloseAction));
    }
}

void CspDiscordRpcGdCppCloseWindow::SetDontShowAgain(bool bInDontShowAgain) const
{
    if (DontShowAgainCheckBox != nullptr)
    {
        DontShowAgainCheckBox->set_pressed(bInDontShowAgain);
    }
}

void CspDiscordRpcGdCppCloseWindow::EnsureUiBuilt()
{
    if (RootContainer != nullptr)
    {
        return;
    }

    set_name("CspDiscordRpcGdCppCloseWindow");
    set_title("Closing Window");
    set_flag(godot::Window::FLAG_BORDERLESS, true);
    set_flag(godot::Window::FLAG_TRANSPARENT, true);
    set_transient(true);
    set_wrap_controls(false);
    set_size(godot::Vector2i(396, 172));
    set_min_size(godot::Vector2i(396, 172));


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
    DialogPanel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput));
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
    TitleBarPanel->set_custom_minimum_size({ 0.0F, 32.0F });
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->set_clip_contents(true);
    TitleBarPanel->set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    TitleBarPanel->add_theme_stylebox_override("panel", CreateTitleBarPanelStyle());
    TitleBarPanel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput));
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

    godot::Label* TitleBarLabel{ CreateTitleBarLabel("Closing Window") };
    TitleBarLabel->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput));
    TitleBarMargin->add_child(TitleBarLabel);

    godot::MarginContainer* DialogMargin{ memnew(godot::MarginContainer) };
    DialogMargin->set_name("DialogMargin");
    DialogMargin->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    DialogMargin->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    DialogMargin->add_theme_constant_override("margin_left", 10);
    DialogMargin->add_theme_constant_override("margin_top", 8);
    DialogMargin->add_theme_constant_override("margin_right", 10);
    DialogMargin->add_theme_constant_override("margin_bottom", 8);
    RootContainer->add_child(DialogMargin);

    godot::VBoxContainer* BodyContainer{ memnew(godot::VBoxContainer) };
    BodyContainer->set_name("BodyContainer");
    BodyContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    BodyContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    BodyContainer->add_theme_constant_override("separation", 4);
    DialogMargin->add_child(BodyContainer);

    CloseActionOptionButton = memnew(godot::OptionButton);
    CloseActionOptionButton->set_name("CloseActionOptionButton");
    CloseActionOptionButton->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    CloseActionOptionButton->set_custom_minimum_size({ 0.0F, 24.0F });
    ApplyButtonVisualStyle(CloseActionOptionButton);
    CloseActionOptionButton->add_item("Minimize to System Tray", static_cast<int32_t>(ECloseAction::MinimizeToSystemTray));
    CloseActionOptionButton->add_item("Close", static_cast<int32_t>(ECloseAction::Close));
    BodyContainer->add_child(CloseActionOptionButton);

    DontShowAgainCheckBox = memnew(godot::CheckBox);
    DontShowAgainCheckBox->set_name("DontShowAgainCheckBox");
    DontShowAgainCheckBox->set_text("Don't show again");
    DontShowAgainCheckBox->set_custom_minimum_size({ 0.0F, 24.0F });
    ApplyButtonVisualStyle(DontShowAgainCheckBox);
    BodyContainer->add_child(DontShowAgainCheckBox);

    godot::HBoxContainer* FooterContainer{ memnew(godot::HBoxContainer) };
    FooterContainer->set_name("FooterContainer");
    FooterContainer->set_alignment(godot::BoxContainer::ALIGNMENT_END);
    FooterContainer->add_theme_constant_override("separation", 8);
    BodyContainer->add_child(FooterContainer);

    godot::Button* CancelButton{ CreateActionButton("Cancel") };
    CancelButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnCancelPressed));
    FooterContainer->add_child(CancelButton);

    godot::Button* ConfirmButton{ CreateActionButton("Confirm") };
    ApplyButtonVisualStyle(ConfirmButton, true);
    ConfirmButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnConfirmPressed));
    FooterContainer->add_child(ConfirmButton);
}

void CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::InputEventMouseButton* MouseButtonEvent{ godot::Object::cast_to<const godot::InputEventMouseButton>(*Event) };
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    start_drag();
}

void CspDiscordRpcGdCppCloseWindow::OnCancelPressed()
{
    emit_signal("cancelled");
    hide();
}

void CspDiscordRpcGdCppCloseWindow::OnConfirmPressed()
{
    const int32_t CloseAction{
        CloseActionOptionButton != nullptr ? CloseActionOptionButton->get_selected_id() : static_cast<int32_t>(ECloseAction::MinimizeToSystemTray)
    };
    const bool bDontShowAgain{ DontShowAgainCheckBox != nullptr && DontShowAgainCheckBox->is_pressed() };

    emit_signal("confirmed", CloseAction, bDontShowAgain);
    hide();
}

} // namespace CspDiscordRpcGdCpp
