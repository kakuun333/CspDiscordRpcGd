#include "CspDiscordRpcGdCppCloseWindow.h"
#include "CspDiscordRpcGdCppMainControl.h"
#include "CspDiscordRpcGdCppWorkItem.h"
#include "CspDiscordRpcGdCppWorksWindow.h"
#include "CspDiscordRpcGdCppNavigationViewItem.h"
#include "CspDiscordRpcGdCppNavigationView.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/godot.hpp"

// Initialization function: register classes.
void InitializeModule(godot::ModuleInitializationLevel InitLevel)
{
    // Register scene-level classes.
    if (InitLevel == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppMainControl);
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppCloseWindow);
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppWorkItem);
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppWorksWindow);
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppNavigationViewItem);
        GDREGISTER_CLASS(CspDiscordRpcGdCpp::CspDiscordRpcGdCppNavigationView);
    }

    // Register editor-level classes.
    if (InitLevel == godot::MODULE_INITIALIZATION_LEVEL_EDITOR)
    {
    }
}

// Shutdown function.
void UninitializeModule(godot::ModuleInitializationLevel InitLevel)
{
    // Intentionally empty.
}

// Entry point recognized by Godot when loading the extension DLL.
extern "C"
{
GDExtensionBool GDE_EXPORT CspDiscordRpcGdCppEntryPoint(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization* r_initialization)
{
    // Initialize the binding object.
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    // Hook up init and shutdown callbacks.
    init_obj.register_initializer(InitializeModule);
    init_obj.register_terminator(UninitializeModule);

    // Set the minimum initialization level.
    init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
} // extern "C"
