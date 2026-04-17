#include "CspDiscordRpcGdCppFunctionLibrary.h"

#include "CspDiscordRpcGdCppMainControl.h"
#include "godot_cpp/classes/display_server.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"

namespace CspDiscordRpcGdCpp
{

void CspDiscordRpcGdCppFunctionLibrary::_bind_methods()
{
    godot::ClassDB::bind_static_method("CspDiscordRpcGdCppFunctionLibrary",
                                       godot::D_METHOD("configure_main_window"),
                                       &CspDiscordRpcGdCppFunctionLibrary::ConfigureMainWindow);
    godot::ClassDB::bind_static_method("CspDiscordRpcGdCppFunctionLibrary",
                                       godot::D_METHOD("build_main_control"),
                                       &CspDiscordRpcGdCppFunctionLibrary::BuildMainControl);
}

void CspDiscordRpcGdCppFunctionLibrary::ConfigureMainWindow()
{
    godot::DisplayServer* DisplayServer = godot::DisplayServer::get_singleton();
    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_BORDERLESS, true);
    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_RESIZE_DISABLED, false);
    DisplayServer->window_set_flag(godot::DisplayServer::WINDOW_FLAG_EXTEND_TO_TITLE, false);
}

godot::Control* CspDiscordRpcGdCppFunctionLibrary::BuildMainControl()
{
    return memnew(CspDiscordRpcGdCppMainControl);
}

} // namespace CspDiscordRpcGdCpp
