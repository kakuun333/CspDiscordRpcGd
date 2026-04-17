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
class OptionButton;
class PopupMenu;
class StatusIndicator;
class Texture2D;
class TextureRect;
class VBoxContainer;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppWorksWindow;
class CspDiscordRpcGdCppCloseWindow;

class CspDiscordRpcGdCppMainControl final : public godot::Control
{
    GDCLASS(CspDiscordRpcGdCppMainControl, godot::Control)

public:
    enum class ECloseAction : int32_t
    {
        MinimizeToSystemTray = 0,
        Close,
    };

    enum class ERichPresenceTextLanguage : int32_t
    {
        English = 0,
        Japanese,
        TraditionalChinese,
        SimplifiedChinese,
    };

    CspDiscordRpcGdCppMainControl() = default;

    virtual void _ready() override;
    virtual void _exit_tree() override;
    void SetRichPresenceTextLanguage(int32_t NewRichPresenceTextLanguage);
    [[nodiscard]] int32_t GetRichPresenceTextLanguage() const;

protected:
    static void _bind_methods();

private:
    void LoadPropertySettings();
    void SavePropertySettings() const;
    void ApplySelectedCspWorkPath(const godot::String& WorkPath, const godot::String& FallbackWorkName);
    void OnPropertySettingsChanged();
    void OnPropertySettingsBoolChanged(bool bValue);
    void OnPropertySettingsIndexChanged(int32_t Value);
    void OnPropertySettingsTextChanged(const godot::String& Value);
    void EnsureCloseStatusIndicator();
    void ExecuteCloseAction(ECloseAction CloseAction);
    void SetWindowControlButtonHighlight(godot::Button* WindowControlButton, const godot::Color& HighlightColor) const;
    void UpdateStatusText(const godot::String& StatusText) const;
    void SyncToViewportSize();
    void CaptureRestoreWindowState();
    void RestoreWindowState(bool bRestorePosition) const;
    void RestoreWindowStateForTitleBarDrag(const godot::Vector2& LocalMousePosition) const;
    void UpdateMaximizeButtonIcon();
    void AddPropertyRow(godot::GridContainer* GridContainer, const godot::String& LabelText, godot::Control* EditorControl);
    godot::GridContainer* CreateCollapsiblePropertyGroup(godot::VBoxContainer* ParentContainer,
                                                         const godot::String& Name,
                                                         const godot::String& Title,
                                                         bool bExpandedByDefault,
                                                         const godot::String& WarningTooltipText);
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
    godot::TextureRect* CreateHeaderWarningIcon(const godot::String& TooltipText) const;

    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void OnResizeHandleGuiInput(const godot::Ref<godot::InputEvent>& Event, int32_t ResizeEdge);
    void OnMinimizePressed();
    void OnMaximizePressed();
    void OnClosePressed();
    void OnChooseCspWorkPressed();
    void OnCspWorkChosen(const godot::String& WorkName, const godot::String& WorkPath);
    void OnCloseWindowConfirmed(int32_t CloseAction, bool bDontShowAgain);
    void OnCloseWindowTreeExited();
    void OnCloseStatusIndicatorPressed(int32_t MouseButton, const godot::Vector2i& MousePosition);
    void OnCloseStatusIndicatorMenuIdPressed(int32_t Id);
    void OnWindowControlButtonMouseEntered(godot::Button* WindowControlButton, int32_t WindowControlButtonStyle);
    void OnWindowControlButtonMouseExited(godot::Button* WindowControlButton);
    void OnWorksWindowTreeExited();
    void OnCollapsiblePropertyGroupToggled(godot::Button* ToggleButton, godot::Control* ContentContainer);
    void OnDiscordRichPresenceToggled(bool bToggled);
    void OnRichPresenceTextLanguageSelected(int32_t SelectedIndex);
    void OnUpdatePresencePressed();

    godot::Button* MinimizeButton{ nullptr };
    godot::Button* MaximizeButton{ nullptr };
    godot::Button* CloseButton{ nullptr };
    godot::CheckButton* DiscordRichPresenceCheckButton{ nullptr };
    godot::Button* ChooseCSPWorkButton{ nullptr };
    godot::OptionButton* RichPresenceTextLanguageOptionButton{ nullptr };
    godot::Button* UpdatePresenceButton{ nullptr };
    godot::LineEdit* ClipStudioCommonRootPathLineEdit{ nullptr };
    godot::LineEdit* SmallImageKeyLineEdit{ nullptr };
    godot::LineEdit* SmallImageTextLineEdit{ nullptr };
    godot::LineEdit* Button1LabelLineEdit{ nullptr };
    godot::LineEdit* Button1UrlLineEdit{ nullptr };
    godot::LineEdit* Button2LabelLineEdit{ nullptr };
    godot::LineEdit* Button2UrlLineEdit{ nullptr };
    godot::Label* StatusLabel{ nullptr };
    godot::PopupMenu* CloseStatusIndicatorMenu{ nullptr };
    godot::StatusIndicator* CloseStatusIndicator{ nullptr };
    CspDiscordRpcGdCppCloseWindow* CloseWindow{ nullptr };
    CspDiscordRpcGdCppWorksWindow* WorksWindow{ nullptr };
    godot::Vector2i RestoreWindowPosition;
    godot::Vector2i RestoreWindowSize;
    CspDiscordRpcGdCppWorkData SelectedCspWorkData;
    godot::String SelectedCSPWorkPath;
    int64_t PresenceStartTimestamp{ 0 };
    ECloseAction CloseAction{ ECloseAction::MinimizeToSystemTray };
    ERichPresenceTextLanguage RichPresenceTextLanguage{ ERichPresenceTextLanguage::English };
    bool bDontShowCloseWindowAgain{ false };
    bool bIsApplyingPropertySettings{ false };
    bool bHasRestoreWindowState{ false };
    bool bIsWindowMaximized{ false };
};

} // namespace CspDiscordRpcGdCpp
