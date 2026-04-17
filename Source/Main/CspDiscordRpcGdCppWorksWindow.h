#pragma once

#include "CspDiscordRpcGdCppWorkData.h"
#include "godot_cpp/classes/window.hpp"
#include <vector>

namespace godot
{

class Button;
class GridContainer;
class InputEvent;
class Label;
class LineEdit;
class VBoxContainer;

} // namespace godot

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppWorkItem;

class CspDiscordRpcGdCppWorksWindow final : public godot::Window
{
    GDCLASS(CspDiscordRpcGdCppWorksWindow, godot::Window)

public:
    CspDiscordRpcGdCppWorksWindow() = default;

    virtual void _ready() override;

    void SetWorks(const std::vector<CspDiscordRpcGdCppWorkData>& InWorks);

    [[nodiscard]] const godot::String& GetSelectedWorkName() const;
    [[nodiscard]] const godot::String& GetSelectedWorkPath() const;

protected:
    static void _bind_methods();

private:
    void EnsureUiBuilt();
    void RebuildWorkItems();
    void SyncSelectionWithFilteredWorks();
    void UpdateChooseButtonState() const;
    [[nodiscard]] bool MatchesSearchText(const CspDiscordRpcGdCppWorkData& Work) const;
    void OnTitleBarGuiInput(const godot::Ref<godot::InputEvent>& Event);
    void OnSearchTextChanged(const godot::String& NewText);
    void OnWorkItemPressed(const godot::String& WorkName, const godot::String& WorkPath);
    void OnCancelPressed();
    void OnChoosePressed();

private:
    std::vector<CspDiscordRpcGdCppWorkData> Works;
    godot::VBoxContainer* RootContainer{ nullptr };
    godot::LineEdit* SearchLineEdit{ nullptr };
    godot::Label* EmptyStateLabel{ nullptr };
    godot::GridContainer* WorkGridContainer{ nullptr };
    godot::Button* ChooseButton{ nullptr };
    godot::String SearchText;
    godot::String SelectedWorkName;
    godot::String SelectedWorkPath;
};

} // namespace CspDiscordRpcGdCpp
