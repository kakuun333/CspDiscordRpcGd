#pragma once

#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/variant/color.hpp"

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppNavigationViewItem : public godot::Button
{
    GDCLASS(CspDiscordRpcGdCppNavigationViewItem, godot::Button)

public:
    CspDiscordRpcGdCppNavigationViewItem() = default;

    void Setup(int32_t NewItemId, const godot::String& NewText, const godot::Ref<godot::Texture2D>& NewIcon);
    void SetExpanded(bool bNewExpanded);
    void SetSelected(bool bNewSelected);
    [[nodiscard]] int32_t GetItemId() const;

protected:
    static void _bind_methods();

private:
    void ApplyVisualState();

private:
    static godot::Ref<godot::StyleBoxFlat> CreateButtonStyle(const godot::Color& Color, const godot::Color& BorderColor);

    godot::String ItemText;
    int32_t ItemId{ 0 };
    bool bExpanded{ true };
    bool bSelected{ false };
};

} // namespace CspDiscordRpcGdCpp
