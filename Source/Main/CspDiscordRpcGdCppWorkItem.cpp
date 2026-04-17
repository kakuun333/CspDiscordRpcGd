#include "CspDiscordRpcGdCppWorkItem.h"

#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/style_box_flat.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/color.hpp"

namespace
{

[[nodiscard]] godot::Ref<godot::StyleBoxFlat> CreateWorkItemStyle(bool bSelected)
{
    godot::Ref<godot::StyleBoxFlat> StyleBox;
    StyleBox.instantiate();
    StyleBox->set_bg_color(bSelected ? godot::Color(0.28F, 0.34F, 0.49F, 1.0F) : godot::Color(0.18F, 0.20F, 0.27F, 1.0F));
    StyleBox->set_border_width_all(1);
    StyleBox->set_border_color(bSelected ? godot::Color(0.62F, 0.74F, 1.0F, 1.0F) : godot::Color(0.24F, 0.27F, 0.34F, 1.0F));
    StyleBox->set_corner_radius_all(10);
    StyleBox->set_content_margin_all(10);
    return StyleBox;
}

[[nodiscard]] godot::Ref<godot::Texture2D> LoadTextureFromPath(const godot::String& ThumbnailPath)
{
    if (ThumbnailPath.is_empty())
    {
        return {};
    }

    godot::Ref<godot::Image> Image;
    Image.instantiate();

    if (Image->load(ThumbnailPath) != godot::OK)
    {
        return {};
    }

    return godot::ImageTexture::create_from_image(Image);
}

} // namespace

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppWorkItem::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("set_selected", "selected"), &CspDiscordRpcGdCppWorkItem::SetSelected);
    godot::ClassDB::bind_method(godot::D_METHOD("on_gui_input", "event"), &CspDiscordRpcGdCppWorkItem::OnGuiInput);

    ADD_SIGNAL(godot::MethodInfo("pressed",
                                 godot::PropertyInfo(godot::Variant::STRING, "work_name"),
                                 godot::PropertyInfo(godot::Variant::STRING, "work_path")));
}

void CspDiscordRpcGdCppWorkItem::_ready()
{
    EnsureUIBuilt();
    RefreshUI();
}

void CspDiscordRpcGdCppWorkItem::SetWorkData(const CspDiscordRpcGdCppWorkData& InWorkData)
{
    WorkData = InWorkData;
    RefreshUI();
}

void CspDiscordRpcGdCppWorkItem::SetSelected(bool bInSelected)
{
    bSelected = bInSelected;
    RefreshUI();
}

const godot::String& CspDiscordRpcGdCppWorkItem::GetWorkName() const
{
    return WorkData.Name;
}

const godot::String& CspDiscordRpcGdCppWorkItem::GetWorkPath() const
{
    return WorkData.CacheDataPath;
}

void CspDiscordRpcGdCppWorkItem::EnsureUIBuilt()
{
    if (ThumbnailPanel != nullptr)
    {
        return;
    }

    set_name("CspDiscordRpcGdCppWorkItem");
    set_custom_minimum_size(godot::Vector2(180.0F, 220.0F));
    set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    set_mouse_filter(godot::Control::MOUSE_FILTER_STOP);
    add_theme_constant_override("separation", 10);

    ThumbnailPanel = memnew(godot::PanelContainer);
    ThumbnailPanel->set_name("ThumbnailPanel");
    ThumbnailPanel->set_custom_minimum_size(godot::Vector2(0.0F, 180.0F));
    ThumbnailPanel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ThumbnailPanel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ThumbnailPanel->set_mouse_filter(godot::Control::MOUSE_FILTER_IGNORE);
    add_child(ThumbnailPanel);

    ThumbnailTextureRect = memnew(godot::TextureRect);
    ThumbnailTextureRect->set_name("ThumbnailTextureRect");
    ThumbnailTextureRect->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ThumbnailTextureRect->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
    ThumbnailTextureRect->set_expand_mode(godot::TextureRect::EXPAND_IGNORE_SIZE);
    ThumbnailTextureRect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
    ThumbnailTextureRect->set_mouse_filter(godot::Control::MOUSE_FILTER_IGNORE);
    ThumbnailPanel->add_child(ThumbnailTextureRect);

    WorkNameLabel = memnew(godot::Label);
    WorkNameLabel->set_name("WorkNameLabel");
    WorkNameLabel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
    WorkNameLabel->set_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
    WorkNameLabel->set_mouse_filter(godot::Control::MOUSE_FILTER_IGNORE);
    add_child(WorkNameLabel);

    connect("gui_input", callable_mp(this, &CspDiscordRpcGdCppWorkItem::OnGuiInput));
}

void CspDiscordRpcGdCppWorkItem::RefreshUI()
{
    if (ThumbnailPanel == nullptr || ThumbnailTextureRect == nullptr || WorkNameLabel == nullptr)
    {
        return;
    }

    ThumbnailPanel->add_theme_stylebox_override("panel", CreateWorkItemStyle(bSelected));
    ThumbnailTextureRect->set_texture(LoadTextureFromPath(WorkData.ThumbnailPath));
    WorkNameLabel->set_text(WorkData.Name.is_empty() ? godot::String::utf8("Untitled Work") : godot::String::utf8(WorkData.Name.ascii()));
    WorkNameLabel->set_modulate(bSelected ? godot::Color(1.0F, 1.0F, 1.0F, 1.0F) : godot::Color(0.92F, 0.94F, 0.98F, 1.0F));
}

void CspDiscordRpcGdCppWorkItem::OnGuiInput(const godot::Ref<godot::InputEvent>& Event)
{
    const godot::InputEventMouseButton* MouseButtonEvent = godot::Object::cast_to<const godot::InputEventMouseButton>(*Event);
    if (MouseButtonEvent == nullptr || MouseButtonEvent->get_button_index() != godot::MOUSE_BUTTON_LEFT || !MouseButtonEvent->is_pressed())
    {
        return;
    }

    emit_signal("pressed", WorkData.Name, WorkData.CacheDataPath);
}

} // namespace CspDiscordRpcGdCpp
