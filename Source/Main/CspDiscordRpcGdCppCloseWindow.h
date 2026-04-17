#pragma once

#include "godot_cpp/classes/window.hpp"

namespace godot
{

class Button;
class CheckBox;
class InputEvent;
class OptionButton;
class VBoxContainer;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppCloseWindow final : public godot::Window
{
    GDCLASS(CspDiscordRpcGdCppCloseWindow, godot::Window)

public:
    enum class ECloseAction : int32_t
    {
        MinimizeToSystemTray = 0,
        Close,
    };

    CspDiscordRpcGdCppCloseWindow() = default;

    virtual void _ready() override;

    void SetSelectedCloseAction(ECloseAction InCloseAction) const;
    void SetDontShowAgain(bool bInDontShowAgain) const;

protected:
    static void _bind_methods();

private:
    void EnsureUiBuilt();
    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void OnCancelPressed();
    void OnConfirmPressed();

private:
    godot::VBoxContainer* RootContainer{ nullptr };
    godot::OptionButton* CloseActionOptionButton{ nullptr };
    godot::CheckBox* DontShowAgainCheckBox{ nullptr };
};

} // namespace CspDiscordRpcGdCpp
