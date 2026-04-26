#pragma once

#include "godot_cpp/variant/string.hpp"
#include <cstdint>
#include <string>

namespace CspDiscordRpcGdCpp
{

struct DiscordRichPresenceData
{
    godot::String State;
    godot::String Details;
    godot::String LargeImageKey;
    godot::String LargeImageText;
    godot::String SmallImageKey;
    godot::String SmallImageText;
    godot::String Button1Label;
    godot::String Button1Url;
    godot::String Button2Label;
    godot::String Button2Url;
    int64_t StartTimestamp = 0;
};

class CspDiscordRpcService final
{
public:
    static CspDiscordRpcService& Get();

    [[nodiscard]] bool Initialize(const godot::String& ClientId);
    void Shutdown();
    void ClearPresence();
    void UpdatePresence(const DiscordRichPresenceData& PresenceData);

    [[nodiscard]] bool IsInitialized() const;
    [[nodiscard]] const godot::String& GetClientId() const;

private:
    CspDiscordRpcService() = default;

    static std::string ToStdString(const godot::String& Value);

private:
    godot::String ClientId;
    bool bInitialized{ false };
};

} // namespace CspDiscordRpcGdCpp
