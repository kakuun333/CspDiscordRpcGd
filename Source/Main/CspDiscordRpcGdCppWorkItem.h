#pragma once

#include "CspDiscordRpcGdCppWorkData.h"
#include "godot_cpp/classes/panel_container.hpp"
#include "godot_cpp/classes/texture_rect.hpp"
#include "godot_cpp/classes/v_box_container.hpp"

namespace godot
{

class InputEvent;
class Label;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppWorkItem final : public godot::VBoxContainer
{
    GDCLASS(CspDiscordRpcGdCppWorkItem, godot::VBoxContainer)

protected:
    static void _bind_methods();

public:
    CspDiscordRpcGdCppWorkItem() = default;

    virtual void _ready() override;

    void SetWorkData(const CspDiscordRpcGdCppWorkData& InWorkData);
    void SetSelected(bool bInSelected);

    [[nodiscard]] const godot::String& GetWorkName() const;
    [[nodiscard]] const godot::String& GetWorkPath() const;

private:
    void EnsureUIBuilt();
    void RefreshUI();
    void OnGuiInput(const godot::Ref<godot::InputEvent>& Event);

private:
    CspDiscordRpcGdCppWorkData WorkData;
    godot::PanelContainer* ThumbnailPanel = nullptr;
    godot::TextureRect* ThumbnailTextureRect = nullptr;
    godot::Label* WorkNameLabel = nullptr;
    bool bSelected = false;
};

} // namespace CspDiscordRpcGdCpp
