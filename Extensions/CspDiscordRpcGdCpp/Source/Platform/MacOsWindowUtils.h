#pragma once

#include <cstdint>

namespace CspDiscordRpcGdCpp::MacOsWindowUtils
{

[[nodiscard]] bool MinimizeActiveWindow();
[[nodiscard]] bool HideWindow(int64_t NativeWindowHandle);
[[nodiscard]] bool ShowWindow(int64_t NativeWindowHandle);

} // namespace CspDiscordRpcGdCpp::MacOsWindowUtils
