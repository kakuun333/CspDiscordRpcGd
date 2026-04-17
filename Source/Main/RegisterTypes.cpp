#include "CspDiscordRpcGdCppFunctionLibrary.h"
#include "CspDiscordRpcGdCppMainControl.h"
#include "godot_cpp/classes/editor_plugin_registration.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/godot.hpp"


// 初始化函數：註冊類別
void InitializeModule(godot::ModuleInitializationLevel InitLevel)
{
    // 註冊核心資源 (SCENE 層級)
    if (InitLevel == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        godot::ClassDB::register_class<CspDiscordRpcGdCpp::CspDiscordRpcGdCppMainControl>();
        godot::ClassDB::register_class<CspDiscordRpcGdCpp::CspDiscordRpcGdCppFunctionLibrary>();
    }

    // 註冊編輯器插件 (EDITOR 層級)
    if (InitLevel == godot::MODULE_INITIALIZATION_LEVEL_EDITOR)
    {
    }
}

// 卸載函數：清理資源（即使為空也建議寫上）
void UninitializeModule(godot::ModuleInitializationLevel InitLevel)
{
    // 這裡可以留空
}

// Godot DLL 載入時唯一認的入口
extern "C"
{
GDExtensionBool GDE_EXPORT CspDiscordRpcGdCppEntryPoint(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization* r_initialization)
{
    // 初始化綁定物件
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    // 連結你的初始化與卸載函數
    init_obj.register_initializer(InitializeModule);
    init_obj.register_terminator(UninitializeModule);

    // 設定最低初始化等級
    init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
} // extern "C"
