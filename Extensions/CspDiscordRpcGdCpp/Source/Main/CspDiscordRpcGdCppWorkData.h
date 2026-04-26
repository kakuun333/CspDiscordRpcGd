#pragma once

#include "godot_cpp/variant/string.hpp"

namespace CspDiscordRpcGdCpp
{

struct CspDiscordRpcGdCppWorkData
{
    godot::String Name;
    godot::String ThumbnailPath;
    godot::String CspVersion;
    godot::String CacheDataPath;
    godot::String TotalWorkingTime;
};

} // namespace CspDiscordRpcGdCpp
