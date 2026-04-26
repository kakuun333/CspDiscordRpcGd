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
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/color.hpp"

namespace
{

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreatePanelStyle(const godot::Color& BackgroundColor)
{
    godot::Ref<godot::StyleBoxFlat> PanelStyle;
    PanelStyle.instantiate();
    PanelStyle->set_bg_color(BackgroundColor);
    return PanelStyle;
}

[[nodiscard]] godot::Button* CreateTitleBarButton(const godot::String& Text)
{
    godot::Button* TitleBarButton = memnew(godot::Button);
    TitleBarButton->set_name("TitleBarButton");
    TitleBarButton->set_text(Text);
    TitleBarButton->set_flat(true);
    TitleBarButton->set_focus_mode(godot::Control::FOCUS_NONE);
    TitleBarButton->set_clip_text(true);
    TitleBarButton->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    TitleBarButton->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    return TitleBarButton;
}

[[nodiscard]] godot::Button* CreateActionButton(const godot::String& Text)
{
    godot::Button* ActionButton = memnew(godot::Button);
    ActionButton->set_text(Text);
    ActionButton->set_custom_minimum_size(godot::Vector2(120.0f, 32.0f));
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
    set_transient(true);
    set_wrap_controls(true);
    set_min_size(godot::Vector2i(520, 280));

    RootContainer = memnew(godot::VBoxContainer);
    RootContainer->set_name("RootContainer");
    RootContainer->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    RootContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->add_theme_constant_override("separation", 0);
    add_child(RootContainer);

    godot::PanelContainer* TitleBarPanel = memnew(godot::PanelContainer);
    TitleBarPanel->set_name("TitleBarPanel");
    TitleBarPanel->set_custom_minimum_size(godot::Vector2(0.0f, 40.0f));
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.11f, 0.12f, 0.16f, 1.0f)));
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer = memnew(godot::HBoxContainer);
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->add_theme_constant_override("separation", 4);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::Button* TitleBarButton = CreateTitleBarButton("Closing Window");
    TitleBarButton->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput));
    TitleBarContainer->add_child(TitleBarButton);

    godot::PanelContainer* ContentPanel = memnew(godot::PanelContainer);
    ContentPanel->set_name("ContentPanel");
    ContentPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.16f, 0.17f, 0.22f, 1.0f)));
    RootContainer->add_child(ContentPanel);

    godot::MarginContainer* ContentMargin = memnew(godot::MarginContainer);
    ContentMargin->set_name("ContentMargin");
    ContentMargin->add_theme_constant_override("margin_left", 16);
    ContentMargin->add_theme_constant_override("margin_top", 16);
    ContentMargin->add_theme_constant_override("margin_right", 16);
    ContentMargin->add_theme_constant_override("margin_bottom", 16);
    ContentMargin->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    ContentPanel->add_child(ContentMargin);

    godot::VBoxContainer* ContentContainer = memnew(godot::VBoxContainer);
    ContentContainer->set_name("ContentContainer");
    ContentContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentContainer->add_theme_constant_override("separation", 16);
    ContentMargin->add_child(ContentContainer);

    godot::Label* DescriptionLabel = memnew(godot::Label);
    DescriptionLabel->set_name("DescriptionLabel");
    DescriptionLabel->set_text("Choose what to do when the Close button is pressed.");
    ContentContainer->add_child(DescriptionLabel);

    CloseActionOptionButton = memnew(godot::OptionButton);
    CloseActionOptionButton->set_name("CloseActionOptionButton");
    CloseActionOptionButton->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    CloseActionOptionButton->add_item("Minimize to System Tray", static_cast<int32_t>(ECloseAction::MinimizeToSystemTray));
    CloseActionOptionButton->add_item("Close", static_cast<int32_t>(ECloseAction::Close));
    ContentContainer->add_child(CloseActionOptionButton);

    DontShowAgainCheckBox = memnew(godot::CheckBox);
    DontShowAgainCheckBox->set_name("DontShowAgainCheckBox");
    DontShowAgainCheckBox->set_text("Don't show again");
    ContentContainer->add_child(DontShowAgainCheckBox);

    godot::Control* Spacer = memnew(godot::Control);
    Spacer->set_name("Spacer");
    Spacer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentContainer->add_child(Spacer);

    godot::HBoxContainer* FooterContainer = memnew(godot::HBoxContainer);
    FooterContainer->set_name("FooterContainer");
    FooterContainer->set_alignment(godot::BoxContainer::ALIGNMENT_END);
    FooterContainer->add_theme_constant_override("separation", 12);
    ContentContainer->add_child(FooterContainer);

    godot::Button* CancelButton = CreateActionButton("Cancel");
    CancelButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnCancelPressed));
    FooterContainer->add_child(CancelButton);

    godot::Button* ConfirmButton = CreateActionButton("Confirm");
    ConfirmButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppCloseWindow::OnConfirmPressed));
    FooterContainer->add_child(ConfirmButton);
}

void CspDiscordRpcGdCppCloseWindow::OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    godot::DisplayServer::get_singleton()->window_start_drag();
}

void CspDiscordRpcGdCppCloseWindow::OnCancelPressed()
{
    hide();
}

void CspDiscordRpcGdCppCloseWindow::OnConfirmPressed()
{
    const int32_t CloseAction =
        CloseActionOptionButton != nullptr ? CloseActionOptionButton->get_selected_id() : static_cast<int32_t>(ECloseAction::MinimizeToSystemTray);
    const bool bDontShowAgain = DontShowAgainCheckBox != nullptr && DontShowAgainCheckBox->is_pressed();

    emit_signal("confirmed", CloseAction, bDontShowAgain);
    hide();
}

} // namespace CspDiscordRpcGdCpp
