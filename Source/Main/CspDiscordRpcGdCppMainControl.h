#pragma once

#include "CspDiscordRpcGdCppWorkData.h"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/variant/vector2i.hpp"
#include <vector>

namespace godot
{

class CheckButton;
class GridContainer;
class InputEvent;
class Label;
class LineEdit;
class Texture2D;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppWorksWindow;

class CspDiscordRpcGdCppMainControl final : public godot::Control
{
    GDCLASS(CspDiscordRpcGdCppMainControl, godot::Control)

protected:
    static void _bind_methods();

public:
    CspDiscordRpcGdCppMainControl() = default;

    virtual void _ready() override;
    virtual void _exit_tree() override;

private:
    void UpdateStatusText(const godot::String& StatusText) const;
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
    void OnChooseCspWorkPressed();
    void OnCspWorkChosen(const godot::String& WorkName, const godot::String& WorkPath);
    void OnWorksWindowTreeExited();
    void OnDiscordRichPresenceToggled(bool bToggled);
    void OnUpdatePresencePressed();

    godot::Button* MaximizeButton{ nullptr };
    godot::CheckButton* DiscordRichPresenceCheckButton{ nullptr };
    godot::Button* ChooseCSPWorkButton{ nullptr };
    godot::Button* UpdatePresenceButton{ nullptr };
    godot::LineEdit* SmallImageKeyLineEdit{ nullptr };
    godot::LineEdit* SmallImageTextLineEdit{ nullptr };
    godot::LineEdit* Button1LabelLineEdit{ nullptr };
    godot::LineEdit* Button1UrlLineEdit{ nullptr };
    godot::LineEdit* Button2LabelLineEdit{ nullptr };
    godot::LineEdit* Button2UrlLineEdit{ nullptr };
    godot::Label* StatusLabel{ nullptr };
    CspDiscordRpcGdCppWorksWindow* WorksWindow{ nullptr };
    godot::Vector2i RestoreWindowPosition;
    godot::Vector2i RestoreWindowSize;
    CspDiscordRpcGdCppWorkData SelectedCspWorkData;
    godot::String SelectedCSPWorkPath;
    int64_t PresenceStartTimestamp{ 0 };
    bool bHasRestoreWindowState{ false };
    bool bIsWindowMaximized{ false };
};

} // namespace CspDiscordRpcGdCpp
