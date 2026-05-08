#pragma once

#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/color.hpp"

namespace godot
{

class Button;
class VBoxContainer;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppNavigationViewItem;

class CspDiscordRpcGdCppNavigationView : public godot::PanelContainer
{
    GDCLASS(CspDiscordRpcGdCppNavigationView, godot::PanelContainer)

public:
    enum ENavigationViewItem : int32_t
    {
        NAVIGATION_VIEW_ITEM_HOME = 0,
        NAVIGATION_VIEW_ITEM_SETTINGS,
    };

    CspDiscordRpcGdCppNavigationView() = default;

    virtual void _ready() override;

    void SetExpanded(bool bNewExpanded);
    [[nodiscard]] bool IsExpanded() const;
    void SetSelectedItem(int32_t NewSelectedItem);
    [[nodiscard]] int32_t GetSelectedItem() const;
    void SetSelectedItemChangedCallable(const godot::Callable& NewSelectedItemChangedCallable);

protected:
    static void _bind_methods();

private:
    void BuildLayout();
    void OnTogglePressed();
    void OnItemPressed(int32_t ItemId);
    void UpdateExpandedState();
    void UpdateSelectionState();

    [[nodiscard]] godot::Ref<godot::Texture2D> CreateTextureFromSvg(const godot::String& SvgContent) const;
    [[nodiscard]] CspDiscordRpcGdCppNavigationViewItem* CreateItem(int32_t ItemId,
                                                                    const godot::String& Text,
                                                                    const godot::Ref<godot::Texture2D>& Icon);

private:
    static godot::Ref<godot::StyleBoxFlat> CreatePanelStyle(const godot::Color& Color);
    static godot::Ref<godot::StyleBoxFlat> CreateButtonStyle(const godot::Color& Color);

    godot::VBoxContainer* ItemsContainer{ nullptr };
    godot::Button* ToggleButton{ nullptr };
    CspDiscordRpcGdCppNavigationViewItem* HomeItem{ nullptr };
    CspDiscordRpcGdCppNavigationViewItem* SettingsItem{ nullptr };
    godot::Callable SelectedItemChangedCallable;
    int32_t SelectedItem{ NAVIGATION_VIEW_ITEM_HOME };
    bool bExpanded{ true };
};

} // namespace CspDiscordRpcGdCpp

VARIANT_ENUM_CAST(CspDiscordRpcGdCpp::CspDiscordRpcGdCppNavigationView::ENavigationViewItem);
