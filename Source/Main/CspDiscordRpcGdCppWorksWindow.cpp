#include "CspDiscordRpcGdCppWorksWindow.h"

#include "CspDiscordRpcGdCppWorkItem.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/classes/grid_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/line_edit.hpp"
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
    ActionButton->set_custom_minimum_size(godot::Vector2(120.0f, 32.0f));
    return ActionButton;
}

[[nodiscard]] godot::String NormalizeSearchString(const godot::String& Text)
{
    const godot::String LowerText = Text.strip_edges().to_lower();
    godot::String NormalizedText;

    for (int64_t CharacterIndex = 0; CharacterIndex < LowerText.length(); ++CharacterIndex)
    {
        int64_t CodePoint = LowerText.unicode_at(CharacterIndex);

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
    TitleBarPanel->set_custom_minimum_size(godot::Vector2(0.0f, 40.0f));
    TitleBarPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.11f, 0.12f, 0.16f, 1.0f)));
    RootContainer->add_child(TitleBarPanel);

    godot::HBoxContainer* TitleBarContainer = memnew(godot::HBoxContainer);
    TitleBarContainer->set_name("TitleBarContainer");
    TitleBarContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    TitleBarContainer->add_theme_constant_override("separation", 4);
    TitleBarPanel->add_child(TitleBarContainer);

    godot::Button* TitleBarButton = CreateTitleBarButton("Choose CSP Work");
    TitleBarButton->connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnTitleBarGuiInput));
    TitleBarContainer->add_child(TitleBarButton);

    godot::PanelContainer* SearchPanel = memnew(godot::PanelContainer);
    SearchPanel->set_name("SearchPanel");
    SearchPanel->set_custom_minimum_size(godot::Vector2(0.0f, 56.0f));
    SearchPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SearchPanel->add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color(0.14f, 0.15f, 0.20f, 1.0f)));
    RootContainer->add_child(SearchPanel);

    godot::MarginContainer* SearchMargin = memnew(godot::MarginContainer);
    SearchMargin->set_name("SearchMargin");
    SearchMargin->add_theme_constant_override("margin_left", 16);
    SearchMargin->add_theme_constant_override("margin_top", 10);
    SearchMargin->add_theme_constant_override("margin_right", 16);
    SearchMargin->add_theme_constant_override("margin_bottom", 10);
    SearchPanel->add_child(SearchMargin);

    SearchLineEdit = memnew(godot::LineEdit);
    SearchLineEdit->set_name("SearchLineEdit");
    SearchLineEdit->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    SearchLineEdit->set_custom_minimum_size(godot::Vector2(0.0f, 36.0f));
    SearchLineEdit->set_placeholder("Search work name...");
    SearchLineEdit->set_clear_button_enabled(true);
    SearchLineEdit->connect("text_changed", callable_mp(this, &CspDiscordRpcGdCppWorksWindow::OnSearchTextChanged));
    SearchMargin->add_child(SearchLineEdit);

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

    EmptyStateLabel = memnew(godot::Label);
    EmptyStateLabel->set_name("EmptyStateLabel");
    EmptyStateLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    EmptyStateLabel->set_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
    EmptyStateLabel->set_modulate(godot::Color(0.76f, 0.79f, 0.86f, 1.0f));
    EmptyStateLabel->set_text("No works match the current search.");
    EmptyStateLabel->set_visible(false);
    ContentContainer->add_child(EmptyStateLabel);

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
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    godot::DisplayServer::get_singleton()->window_start_drag();
}

void CspDiscordRpcGdCppWorksWindow::OnSearchTextChanged(const godot::String& NewText)
{
    SearchText = NewText;
    RebuildWorkItems();
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
