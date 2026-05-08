#include "CspDiscordRpcGdCppNavigationView.h"

#include "CspDiscordRpcGdCppNavigationViewItem.h"
#include "Generated/EmbeddedSvgResources.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/callable.hpp"

namespace CspDiscordRpcGdCpp
{
namespace
{

constexpr float ExpandedWidth{ 180.0f };
constexpr float CollapsedWidth{ 56.0f };

} // namespace

void CspDiscordRpcGdCppNavigationView::_ready()
{
    ConfigureWindow();
    BuildLayout();
}

void CspDiscordRpcGdCppNavigationView::ConfigureWindow()
{
    godot::Engine* Engine{ godot::Engine::get_singleton() };
    if (Engine != nullptr && Engine->is_editor_hint())
    {
        return;
    }

    godot::DisplayServer* DisplayServer{ godot::DisplayServer::get_singleton() };
    if (DisplayServer == nullptr)
    {
        return;
    }

    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_RESIZE_DISABLED, false);
    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_EXTEND_TO_TITLE, false);
}

void CspDiscordRpcGdCppNavigationView::SetExpanded(const bool bNewExpanded)
{
    if (bExpanded == bNewExpanded)
    {
        return;
    }

    bExpanded = bNewExpanded;
    UpdateExpandedState();
}

bool CspDiscordRpcGdCppNavigationView::IsExpanded() const
{
    return bExpanded;
}

void CspDiscordRpcGdCppNavigationView::SetSelectedItem(const int32_t NewSelectedItem)
{
    if (SelectedItem == NewSelectedItem)
    {
        return;
    }

    SelectedItem = NewSelectedItem;
    UpdateSelectionState();

    if (SelectedItemChangedCallable.is_valid())
    {
        SelectedItemChangedCallable.call(SelectedItem);
    }
}

int32_t CspDiscordRpcGdCppNavigationView::GetSelectedItem() const
{
    return SelectedItem;
}

void CspDiscordRpcGdCppNavigationView::SetSelectedItemChangedCallable(
    const godot::Callable& NewSelectedItemChangedCallable)
{
    SelectedItemChangedCallable = NewSelectedItemChangedCallable;
}

void CspDiscordRpcGdCppNavigationView::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("set_expanded", "expanded"),
                                &CspDiscordRpcGdCppNavigationView::SetExpanded);
    godot::ClassDB::bind_method(godot::D_METHOD("is_expanded"), &CspDiscordRpcGdCppNavigationView::IsExpanded);
    godot::ClassDB::bind_method(godot::D_METHOD("set_selected_item", "selected_item"),
                                &CspDiscordRpcGdCppNavigationView::SetSelectedItem);
    godot::ClassDB::bind_method(godot::D_METHOD("get_selected_item"),
                                &CspDiscordRpcGdCppNavigationView::GetSelectedItem);

    ADD_PROPERTY(
        godot::PropertyInfo(godot::Variant::BOOL, "Expanded", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_DEFAULT), "set_expanded", "is_expanded");
    ADD_PROPERTY(
        godot::PropertyInfo(godot::Variant::INT, "Selected Item", godot::PROPERTY_HINT_ENUM, "Home,Settings", godot ::PROPERTY_USAGE_DEFAULT), "set_selected_item", "get_selected_item");

    BIND_ENUM_CONSTANT(NAVIGATION_VIEW_ITEM_HOME);
    BIND_ENUM_CONSTANT(NAVIGATION_VIEW_ITEM_SETTINGS);
}

void CspDiscordRpcGdCppNavigationView::BuildLayout()
{
    if (ItemsContainer != nullptr)
    {
        return;
    }

    set_name("NavigationView");
    set_h_size_flags(godot::Control::SIZE_SHRINK_BEGIN);
    set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    add_theme_stylebox_override("panel", CreatePanelStyle(godot::Color::hex(0x171a22ff)));

    ItemsContainer = memnew(godot::VBoxContainer);
    ItemsContainer->set_name("NavigationViewItems");
    ItemsContainer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ItemsContainer->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ItemsContainer->add_theme_constant_override("separation", 4);
    add_child(ItemsContainer);

    ToggleButton = memnew(godot::Button);
    ToggleButton->set_name("NavigationViewToggleButton");
    ToggleButton->set_focus_mode(godot::Control::FOCUS_NONE);
    ToggleButton->set_h_size_flags(godot::Control::SIZE_SHRINK_BEGIN);
    ToggleButton->set_custom_minimum_size({ 40.0f, 40.0f });
    ToggleButton->set_button_icon(CreateTextureFromSvg(EmbeddedSvgResources::Menu));
    ToggleButton->set_expand_icon(false);
    ToggleButton->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
    ToggleButton->set_vertical_icon_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    ToggleButton->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppNavigationView::OnTogglePressed));
    ToggleButton->add_theme_stylebox_override("normal", CreateButtonStyle(godot::Color::hex(0x00000000)));
    ToggleButton->add_theme_stylebox_override("hover", CreateButtonStyle(godot::Color::hex(0x6dabe42e)));
    ToggleButton->add_theme_stylebox_override("pressed", CreateButtonStyle(godot::Color::hex(0x6dabe446)));
    ItemsContainer->add_child(ToggleButton);

    HomeItem = CreateItem(NAVIGATION_VIEW_ITEM_HOME, "Home", CreateTextureFromSvg(EmbeddedSvgResources::Home));
    ItemsContainer->add_child(HomeItem);

    SettingsItem = CreateItem(NAVIGATION_VIEW_ITEM_SETTINGS, "Settings", CreateTextureFromSvg(EmbeddedSvgResources::Settings));
    ItemsContainer->add_child(SettingsItem);

    UpdateExpandedState();
    UpdateSelectionState();
}

void CspDiscordRpcGdCppNavigationView::OnTogglePressed()
{
    SetExpanded(!bExpanded);
}

void CspDiscordRpcGdCppNavigationView::OnItemPressed(const int32_t ItemId)
{
    SetSelectedItem(ItemId);
}

void CspDiscordRpcGdCppNavigationView::UpdateExpandedState()
{
    set_custom_minimum_size({ bExpanded ? ExpandedWidth : CollapsedWidth, 0.0f });

    if (ToggleButton != nullptr)
    {
        ToggleButton->set_tooltip_text(bExpanded ? "Collapse navigation" : "Expand navigation");
    }

    if (HomeItem != nullptr)
    {
        HomeItem->SetExpanded(bExpanded);
    }

    if (SettingsItem != nullptr)
    {
        SettingsItem->SetExpanded(bExpanded);
    }
}

void CspDiscordRpcGdCppNavigationView::UpdateSelectionState()
{
    if (HomeItem != nullptr)
    {
        HomeItem->SetSelected(SelectedItem == NAVIGATION_VIEW_ITEM_HOME);
    }

    if (SettingsItem != nullptr)
    {
        SettingsItem->SetSelected(SelectedItem == NAVIGATION_VIEW_ITEM_SETTINGS);
    }
}

godot::Ref<godot::Texture2D> CspDiscordRpcGdCppNavigationView::CreateTextureFromSvg(
    const godot::String& SvgContent) const
{
    if (SvgContent.is_empty())
    {
        return {};
    }

    godot::Ref<godot::Image> Image;
    Image.instantiate();

    if (Image->load_svg_from_string(SvgContent, 1.0f) != godot::OK)
    {
        return {};
    }

    return godot::ImageTexture::create_from_image(Image);
}

CspDiscordRpcGdCppNavigationViewItem* CspDiscordRpcGdCppNavigationView::CreateItem(const int32_t ItemId,
                                                                                   const godot::String& Text,
                                                                                   const godot::Ref<godot::Texture2D>& Icon)
{
    CspDiscordRpcGdCppNavigationViewItem* Item{ memnew(CspDiscordRpcGdCppNavigationViewItem) };
    Item->Setup(ItemId, Text, Icon);
    Item->connect("pressed", callable_mp(this, &CspDiscordRpcGdCppNavigationView::OnItemPressed).bind(ItemId));
    return Item;
}

godot::Ref<godot::StyleBoxFlat> CspDiscordRpcGdCppNavigationView::CreatePanelStyle(const godot::Color& Color)
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(Color);
    Style->set_content_margin(godot::SIDE_LEFT, 8.0f);
    Style->set_content_margin(godot::SIDE_TOP, 8.0f);
    Style->set_content_margin(godot::SIDE_RIGHT, 8.0f);
    Style->set_content_margin(godot::SIDE_BOTTOM, 8.0f);
    Style->set_border_color(godot::Color::hex(0x2a3140ff));
    Style->set_border_width(godot::SIDE_RIGHT, 1);
    return Style;
}

godot::Ref<godot::StyleBoxFlat> CspDiscordRpcGdCppNavigationView::CreateButtonStyle(const godot::Color& Color)
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(Color);
    Style->set_corner_radius_all(4);
    return Style;
}

} // namespace CspDiscordRpcGdCpp
