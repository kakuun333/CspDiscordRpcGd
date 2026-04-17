#pragma once

#include "CspDiscordRpcGdCppWorkData.h"
#include "godot_cpp/classes/window.hpp"
#include <vector>

namespace godot
{

class Button;
class GridContainer;
class InputEvent;
class VBoxContainer;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppWorkItem;

class CspDiscordRpcGdCppWorksWindow final : public godot::Window
{
    GDCLASS(CspDiscordRpcGdCppWorksWindow, godot::Window)

protected:
    static void _bind_methods();

public:
    CspDiscordRpcGdCppWorksWindow() = default;

    virtual void _ready() override;

    void SetWorks(const std::vector<CspDiscordRpcGdCppWorkData>& InWorks);

    [[nodiscard]] const godot::String& GetSelectedWorkName() const;
    [[nodiscard]] const godot::String& GetSelectedWorkPath() const;

private:
    void EnsureUiBuilt();
    void RebuildWorkItems();
    void UpdateChooseButtonState() const;
    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void OnWorkItemPressed(const godot::String& WorkName, const godot::String& WorkPath);
    void OnCancelPressed();
    void OnChoosePressed();

private:
    std::vector<CspDiscordRpcGdCppWorkData> Works;
    godot::VBoxContainer* RootContainer = nullptr;
    godot::GridContainer* WorkGridContainer = nullptr;
    godot::Button* ChooseButton = nullptr;
    godot::String SelectedWorkName;
    godot::String SelectedWorkPath;
};

} // namespace CspDiscordRpcGdCpp
