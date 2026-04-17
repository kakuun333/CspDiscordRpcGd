#include "CspDiscordRpcGdCppWorksWindow.h"

#include "CspDiscordRpcGdCppWorkItem.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/grid_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/margin_container.hpp"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/scroll_container.hpp"
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
    ActionButton->set_custom_minimum_size(godot::Vector2(120.0F, 32.0F));
    return ActionButton;
}

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppWorksWindow::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("on_title_bar_gui_input", "event"), &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput);
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
}

void CspDiscordRpcGdCppWorksWindow::SetWorks(const std::vector<CspDiscordRpcGdCppWorkData>& InWorks)
{
    Works = InWorks;
    SelectedWorkName = "";
    SelectedWorkPath = "";
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
    set_transient(true);
    set_wrap_controls(true);
    set_min_size(godot::Vector2i(860, 560));

    RootContainer = memnew(godot::VBoxContainer);
    RootContainer->set_name("RootContainer");
    RootContainer->set_anchors_and_offsets_preset(godot::Control::PRESET_FULL_RECT);
    RootContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    RootContainer->add_theme_constant_override("separation", 0);
    add_child(RootContainer);

    godot::PanelContainer* TitleBarPanel = memnew(godot::PanelContainer);
    TitleBarPanel->set_name("TitleBarPanel");
    TitleBarPanel->set_custom_minimum_size(godot::Vector2(0.0F, 40.0F));
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.11F, 0.12F, 0.16F, 1.0F)));
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer = memnew(godot::HBoxContainer);
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->add_theme_constant_override("separation", 4);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::Button* TitleBarButton = CreateTitleBarButton("Choose CSP Work");
    TitleBarButton->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput));
    TitleBarContainer->add_child(TitleBarButton);

    godot::PanelContainer* ContentPanel = memnew(godot::PanelContainer);
    ContentPanel->set_name("ContentPanel");
    ContentPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ContentPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.16F, 0.17F, 0.22F, 1.0F)));
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

    godot::ScrollContainer* ScrollContainer = memnew(godot::ScrollContainer);
    ScrollContainer->set_name("ScrollContainer");
    ScrollContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ScrollContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ScrollContainer->set_horizontal_scroll_mode(godot::ScrollContainer::SCROLL_MODE_DISABLED);
    ScrollContainer->set_vertical_scroll_mode(godot::ScrollContainer::SCROLL_MODE_AUTO);
    ContentContainer->add_child(ScrollContainer);

    WorkGridContainer = memnew(godot::GridContainer);
    WorkGridContainer->set_name("WorkGridContainer");
    WorkGridContainer->set_columns(3);
    WorkGridContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    WorkGridContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    WorkGridContainer->add_theme_constant_override("h_separation", 16);
    WorkGridContainer->add_theme_constant_override("v_separation", 16);
    ScrollContainer->add_child(WorkGridContainer);

    godot::HBoxContainer* FooterContainer = memnew(godot::HBoxContainer);
    FooterContainer->set_name("FooterContainer");
    FooterContainer->set_alignment(godot::BoxContainer::ALIGNMENT_END);
    FooterContainer->add_theme_constant_override("separation", 12);
    ContentContainer->add_child(FooterContainer);

    godot::Button* CancelButton = CreateActionButton("Cancel");
    CancelButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnCancelPressed));
    FooterContainer->add_child(CancelButton);

    ChooseButton = CreateActionButton("Choose");
    ChooseButton->set_disabled(true);
    ChooseButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnChoosePressed));
    FooterContainer->add_child(ChooseButton);
}

void CspDiscordRpcGdCppWorksWindow::RebuildWorkItems()
{
    if (WorkGridContainer == nullptr)
    {
        return;
    }

    for (int32_t ChildIndex = WorkGridContainer->get_child_count() - 1; ChildIndex >= 0; --ChildIndex)
    {
        if (godot::Node* ChildNode = WorkGridContainer->get_child(ChildIndex))
        {
            WorkGridContainer->remove_child(ChildNode);
            ChildNode->queue_free();
        }
    }

    for (const CspDiscordRpcGdCppWorkData& Work : Works)
    {
        CspDiscordRpcGdCppWorkItem* WorkItem = memnew(CspDiscordRpcGdCppWorkItem);
        WorkItem->SetWorkData(Work);
        WorkItem->SetSelected(Work.CacheDataPath == SelectedWorkPath && !SelectedWorkPath.is_empty());
        WorkItem->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnWorkItemPressed));
        WorkGridContainer->add_child(WorkItem);
    }

    UpdateChooseButtonState();
}

void CspDiscordRpcGdCppWorksWindow::UpdateChooseButtonState() const
{
    if (ChooseButton != nullptr)
    {
        ChooseButton->set_disabled(SelectedWorkPath.is_empty());
    }
}

void CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    godot::DisplayServer::get_singleton()->window_start_drag();
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
