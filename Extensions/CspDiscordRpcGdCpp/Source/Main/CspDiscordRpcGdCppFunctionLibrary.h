#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/object.hpp"

namespace CspDiscordRpcGdCpp
{

class CspDiscordRpcGdCppFunctionLibrary final : public godot::Object
{
    GDCLASS(CspDiscordRpcGdCppFunctionLibrary, godot::Object)

protected:
    static void _bind_methods();

public:
    static void ConfigureMainWindow();
    static godot::Control* BuildMainControl();
};

} // namespace CspDiscordRpcGdCpp
