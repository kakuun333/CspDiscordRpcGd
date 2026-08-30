#include "MacOsWindowUtils.h"

#import <AppKit/AppKit.h>

namespace CspDiscordRpcGdCpp::MacOsWindowUtils
{

bool MinimizeActiveWindow()
{
    @autoreleasepool
    {
        NSWindow* ActiveWindow{ NSApp.keyWindow };
        if (ActiveWindow == nil)
        {
            ActiveWindow = NSApp.mainWindow;
        }

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

} // namespace CspDiscordRpcGdCpp::MacOsWindowUtils
