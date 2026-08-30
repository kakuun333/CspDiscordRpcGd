#include "MacOsWindowUtils.h"

#import <AppKit/AppKit.h>

namespace CspDiscordRpcGdCpp::MacOsWindowUtils
{
namespace
{

[[nodiscard]] NSWindow* GetActiveWindow()
{
    NSWindow* ActiveWindow{ NSApp.keyWindow };
    if (ActiveWindow == nil)
    {
        ActiveWindow = NSApp.mainWindow;
    }

    return ActiveWindow;
}

} // namespace

bool MinimizeActiveWindow()
{
    @autoreleasepool
    {
        NSWindow* ActiveWindow{ GetActiveWindow() };
        if (ActiveWindow == nil)
        {
            return false;
        }

        const NSWindowStyleMask OriginalStyleMask{ ActiveWindow.styleMask };
        if ((OriginalStyleMask & NSWindowStyleMaskMiniaturizable) != 0)
        {
            [ActiveWindow miniaturize:nil];
            return true;
        }

        const BOOL bOriginalTitlebarAppearsTransparent{ ActiveWindow.titlebarAppearsTransparent };
        const NSWindowTitleVisibility OriginalTitleVisibility{ ActiveWindow.titleVisibility };

        ActiveWindow.styleMask = OriginalStyleMask |
                                 NSWindowStyleMaskTitled |
                                 NSWindowStyleMaskMiniaturizable |
                                 NSWindowStyleMaskFullSizeContentView;
        ActiveWindow.titlebarAppearsTransparent = YES;
        ActiveWindow.titleVisibility = NSWindowTitleHidden;
        [ActiveWindow standardWindowButton:NSWindowCloseButton].hidden = YES;
        [ActiveWindow standardWindowButton:NSWindowMiniaturizeButton].hidden = YES;
        [ActiveWindow standardWindowButton:NSWindowZoomButton].hidden = YES;

        [ActiveWindow miniaturize:nil];
        ActiveWindow.styleMask = OriginalStyleMask;
        ActiveWindow.titlebarAppearsTransparent = bOriginalTitlebarAppearsTransparent;
        ActiveWindow.titleVisibility = OriginalTitleVisibility;

        return true;
    }
}

bool HideWindow(const int64_t NativeWindowHandle)
{
    @autoreleasepool
    {
        NSWindow* Window{ reinterpret_cast<NSWindow*>(NativeWindowHandle) };
        if (Window == nil)
        {
            return false;
        }

        [Window orderOut:nil];
        return !Window.visible;
    }
}

bool ShowWindow(const int64_t NativeWindowHandle)
{
    @autoreleasepool
    {
        NSWindow* Window{ reinterpret_cast<NSWindow*>(NativeWindowHandle) };
        if (Window == nil)
        {
            return false;
        }

        if (Window.miniaturized)
        {
            [Window deminiaturize:nil];
        }

        [NSApp activateIgnoringOtherApps:YES];
        [Window makeKeyAndOrderFront:nil];

        if (!Window.visible)
        {
            return false;
        }

        return true;
    }
}

} // namespace CspDiscordRpcGdCpp::MacOsWindowUtils
