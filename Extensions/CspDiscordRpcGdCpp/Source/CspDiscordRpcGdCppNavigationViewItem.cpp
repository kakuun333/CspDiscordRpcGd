#include "CspDiscordRpcGdCppNavigationViewItem.h"

#include "godot_cpp/core/class_db.hpp"

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppNavigationViewItem::Setup(const int32_t NewItemId, const godot::String& NewText, const godot::Ref<godot::Texture2D>& NewIcon)
{
    ItemId = NewItemId;
    ItemText = NewText;
    set_name(ItemText + godot::String("NavigationViewItem"));
    set_text(ItemText);
    set_tooltip_text(ItemText);
    set_button_icon(NewIcon);
    set_expand_icon(false);
    set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    set_vertical_icon_alignment(godot::VERTICAL_ALIGNMENT_CENTER);
    set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
    set_clip_text(true);
    set_focus_mode(godot::Control::FOCUS_NONE);
    set_custom_minimum_size({ 40.0f, 40.0f });
    add_theme_font_size_override("font_size", 14);
    ApplyVisualState();
}

void CspDiscordRpcGdCppNavigationViewItem::SetExpanded(const bool bNewExpanded)
{
    if (bExpanded == bNewExpanded)
    {
        return;
    }

    bExpanded = bNewExpanded;
    set_text(bExpanded ? ItemText : godot::String());
}

void CspDiscordRpcGdCppNavigationViewItem::SetSelected(const bool bNewSelected)
{
    if (bSelected == bNewSelected)
    {
        return;
    }

    bSelected = bNewSelected;
    ApplyVisualState();
}

int32_t CspDiscordRpcGdCppNavigationViewItem::GetItemId() const
{
    return ItemId;
}

void CspDiscordRpcGdCppNavigationViewItem::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("get_item_id"), &CspDiscordRpcGdCppNavigationViewItem::GetItemId);
}

void CspDiscordRpcGdCppNavigationViewItem::ApplyVisualState()
{
    const godot::Color NormalColor{ godot::Color::hex(0x00000000) };
    const godot::Color HoverColor{ godot::Color::hex(0x6dabe42e) };
    const godot::Color SelectedColor{ godot::Color::hex(0x6dabe446) };
    const godot::Color SelectedBorderColor{ godot::Color::hex(0x6dabe4ff) };

    add_theme_stylebox_override("normal", CreateButtonStyle(bSelected ? SelectedColor : NormalColor, bSelected ? SelectedBorderColor : NormalColor));
    add_theme_stylebox_override("hover", CreateButtonStyle(bSelected ? SelectedColor : HoverColor, bSelected ? SelectedBorderColor : HoverColor));
    add_theme_stylebox_override("pressed", CreateButtonStyle(SelectedColor, SelectedBorderColor));
    add_theme_stylebox_override("focus", CreateButtonStyle(NormalColor, NormalColor));
    add_theme_color_override("font_color", godot::Color::hex(0xe0e0e0ff));
    add_theme_color_override("font_hover_color", godot::Color::hex(0xffffffff));
    add_theme_color_override("font_pressed_color", godot::Color::hex(0xffffffff));
    add_theme_color_override("font_focus_color", godot::Color::hex(0xe0e0e0ff));
}

godot::Ref<godot::StyleBoxFlat> CspDiscordRpcGdCppNavigationViewItem::CreateButtonStyle(const godot::Color& Color, const godot::Color& BorderColor)
{
    godot::Ref<godot::StyleBoxFlat> Style;
    Style.instantiate();
    Style->set_bg_color(Color);
    Style->set_corner_radius_all(4);
    Style->set_content_margin(godot::SIDE_LEFT, 10.0f);
    Style->set_content_margin(godot::SIDE_TOP, 0.0f);
    Style->set_content_margin(godot::SIDE_RIGHT, 10.0f);
    Style->set_content_margin(godot::SIDE_BOTTOM, 0.0f);
    Style->set_border_width(godot::SIDE_LEFT, 3);
    Style->set_border_color(BorderColor);
    return Style;
}

} // namespace CspDiscordRpcGdCpp
