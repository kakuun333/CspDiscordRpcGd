#pragma once

#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/variant/vector2i.hpp"

namespace godot
{

class GridContainer;
class InputEvent;
class Texture2D;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppMainControl final : public godot::Control
{
    GDCLASS(CspDiscordRpcGdCppMainControl, godot::Control)

protected:
    static void _bind_methods();

public:
    CspDiscordRpcGdCppMainControl() = default;

    void _ready() override;

private:
    void SyncToViewportSize();
    void CaptureRestoreWindowState();
    void RestoreWindowState(bool bRestorePosition) const;
    void RestoreWindowStateForTitleBarDrag(const godot::Vector2& LocalMousePosition) const;
    void UpdateMaximizeButtonIcon();
    void AddPropertyRow(godot::GridContainer* GridContainer, const godot::String& LabelText, godot::Control* EditorControl);
    void AddResizeHandle(const godot::String& Name,
                         godot::DisplayServer::WindowResizeEdge ResizeEdge,
                         godot::Control::CursorShape CursorShape,
                         float AnchorLeft,
                         float AnchorTop,
                         float AnchorRight,
                         float AnchorBottom,
                         float OffsetLeft,
                         float OffsetTop,
                         float OffsetRight,
                         float OffsetBottom);
    godot::Button* CreateTitleBarButton(const godot::String& Text, const godot::String& TooltipText) const;
    godot::Button* CreateWindowControlButton(const godot::Ref<godot::Texture2D>& Icon, const godot::String& TooltipText) const;

    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void OnResizeHandleGuiInput(const godot::Ref<godot::InputEvent>& Event, int32_t ResizeEdge);
    void OnMinimizePressed();
    void OnMaximizePressed();
    void OnClosePressed();

    godot::Button* MaximizeButton = nullptr;
    godot::Vector2i RestoreWindowPosition;
    godot::Vector2i RestoreWindowSize;
    bool bHasRestoreWindowState = false;
    bool bIsWindowMaximized = false;
};

} // namespace CspDiscordRpcGdCpp
