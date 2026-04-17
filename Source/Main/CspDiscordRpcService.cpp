#include "CspDiscordRpcService.h"

#include "godot_cpp/variant/char_string.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <discord-rpc.hpp>
#include <string_view>

namespace
{

[[nodiscard]] godot::String Trimmed(const godot::String& Value)
{
    return Value.strip_edges();
}

[[nodiscard]] godot::String NormalizeButtonUrl(const godot::String& Value)
{
    godot::String NormalizedValue = Trimmed(Value);
    if (NormalizedValue.is_empty())
    {
        return {};
    }

    const godot::String LowerValue = NormalizedValue.to_lower();
    if (!LowerValue.begins_with("http://") && !LowerValue.begins_with("https://"))
    {
        NormalizedValue = "https://" + NormalizedValue;
    }

    return NormalizedValue;
}

} // namespace

namespace CspDiscordRpcGdCpp
{

CspDiscordRpcService& CspDiscordRpcService::Get()
{
    static CspDiscordRpcService Service;
    return Service;
}

bool CspDiscordRpcService::Initialize(const godot::String& ClientId)
{
    if (ClientId.is_empty())
    {
        godot::UtilityFunctions::push_warning("Discord RPC client id is empty.");
        return false;
    }

    if (bInitialized && this->ClientId == ClientId)
    {
        return true;
    }

    if (bInitialized)
    {
        Shutdown();
    }

    this->ClientId = ClientId;

    discord::RPCManager::get()
        .setClientID(ToStdString(this->ClientId))
        .onReady([](const discord::User& User)
    {
        godot::UtilityFunctions::print(godot::String("Discord RPC connected: ") + godot::String(User.username.c_str()));
    }).onDisconnected([](int ErrorCode, std::string_view Message)
    {
        godot::UtilityFunctions::push_warning(
            godot::String("Discord RPC disconnected (") + godot::String::num_int64(ErrorCode) + "): " + godot::String(Message.data()));
    }).onErrored([](int ErrorCode, std::string_view Message)
    {
        godot::UtilityFunctions::push_error(
            godot::String("Discord RPC error (") + godot::String::num_int64(ErrorCode) + "): " + godot::String(Message.data()));
    });

    discord::RPCManager::get().initialize();
    bInitialized = true;
    return true;
}

void CspDiscordRpcService::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    discord::RPCManager::get().shutdown();
    bInitialized = false;
}

void CspDiscordRpcService::ClearPresence()
{
    if (!bInitialized)
    {
        return;
    }

    discord::RPCManager::get().clearPresence();
}

void CspDiscordRpcService::UpdatePresence(const DiscordRichPresenceData& PresenceData)
{
    if (!bInitialized)
    {
        godot::UtilityFunctions::push_warning("Discord RPC is not initialized.");
        return;
    }

    discord::Presence& Presence = discord::RPCManager::get().getPresence();
    Presence.clear();
    Presence.setActivityType(discord::ActivityType::Game);
    Presence.setStatusDisplayType(discord::StatusDisplayType::State);
    Presence.setInstance(false);

    const godot::String State = Trimmed(PresenceData.State);
    const godot::String Details = Trimmed(PresenceData.Details);
    const godot::String LargeImageKey = Trimmed(PresenceData.LargeImageKey);
    const godot::String LargeImageText = Trimmed(PresenceData.LargeImageText);
    const godot::String SmallImageKey = Trimmed(PresenceData.SmallImageKey);
    const godot::String SmallImageText = Trimmed(PresenceData.SmallImageText);
    const godot::String ButtonLabel = Trimmed(PresenceData.ButtonLabel);
    const godot::String ButtonUrl = NormalizeButtonUrl(PresenceData.ButtonUrl);

    if (!State.is_empty())
    {
        Presence.setState(ToStdString(State));
    }

    if (!Details.is_empty())
    {
        Presence.setDetails(ToStdString(Details));
    }

    if (PresenceData.StartTimestamp > 0)
    {
        Presence.setStartTimestamp(PresenceData.StartTimestamp);
    }

    if (!LargeImageKey.is_empty())
    {
        Presence.setLargeImageKey(ToStdString(LargeImageKey));
    }

    if (!LargeImageText.is_empty())
    {
        Presence.setLargeImageText(ToStdString(LargeImageText));
    }

    if (!SmallImageKey.is_empty())
    {
        Presence.setSmallImageKey(ToStdString(SmallImageKey));
    }

    if (!SmallImageText.is_empty())
    {
        Presence.setSmallImageText(ToStdString(SmallImageText));
    }

    if (!ButtonLabel.is_empty() && !ButtonUrl.is_empty())
    {
        Presence.setButton1(ToStdString(ButtonLabel), ToStdString(ButtonUrl));
    }

    Presence.refresh();
}

bool CspDiscordRpcService::IsInitialized() const
{
    return bInitialized;
}

const godot::String& CspDiscordRpcService::GetClientId() const
{
    return ClientId;
}

std::string CspDiscordRpcService::ToStdString(const godot::String& Value)
{
    const godot::CharString Utf8Value = Value.utf8();
    return { Utf8Value.get_data() };
}

} // namespace CspDiscordRpcGdCpp
