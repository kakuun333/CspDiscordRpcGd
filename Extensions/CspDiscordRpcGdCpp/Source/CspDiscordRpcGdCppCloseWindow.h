#pragma once

#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/variant/vector2i.hpp"

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
    virtual void _input(const godot::Ref<godot::InputEvent>& Event) override;

    void SetBoundsSize(const godot::Vector2i& NewBoundsSize);
    void ApplyResponsiveLayout(bool bCenterInBounds);
    void ClampToBounds();
    void SetSelectedCloseAction(ECloseAction InCloseAction) const;
    void SetDontShowAgain(bool bInDontShowAgain) const;

protected:
    static void _bind_methods();

private:
    void EnsureUiBuilt();
    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void StartBoundedDrag(const godot::Vector2& GlobalMousePosition);
    void UpdateBoundedDrag(const godot::Vector2& GlobalMousePosition);
    [[nodiscard]] godot::Vector2i GetResolvedBoundsSize() const;
    [[nodiscard]] godot::Vector2i GetResponsiveWindowSize() const;
    [[nodiscard]] godot::Vector2i GetCenteredPosition() const;
    [[nodiscard]] godot::Vector2i GetClampedPosition(const godot::Vector2i& CandidatePosition) const;
    void OnCancelPressed();
    void OnConfirmPressed();

private:
    godot::VBoxContainer* RootContainer{ nullptr };
    godot::OptionButton* CloseActionOptionButton{ nullptr };
    godot::CheckBox* DontShowAgainCheckBox{ nullptr };
    godot::Vector2i BoundsSize;
    godot::Vector2 DragMouseOffset;
    bool bDragging{};
};

} // namespace CspDiscordRpcGdCpp
