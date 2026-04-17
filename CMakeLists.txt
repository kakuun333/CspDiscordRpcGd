cmake_minimum_required(VERSION 3.25)

project(CspDiscordRpcGdCpp LANGUAGES CXX)

# --- Compiler ---
set(COMPILER_IS_CLANG "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")
set(COMPILER_IS_GNU "$<CXX_COMPILER_ID:GNU>")
set(COMPILER_IS_MSVC "$<CXX_COMPILER_ID:MSVC>")

# --- Platform ---
if(WIN32)
    set(PLATFORM "windows")
elseif(APPLE)
    set(PLATFORM "macos")
elseif(UNIX)
    set(PLATFORM "linux")
endif()

# --- Architecture ---
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64)")
    set(ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(ARCH "arm64")
else()
    set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(COMPILER_IS_MSVC)
    add_compile_options("/utf-8")

    # Force CMake to Use Static Runtime Library (/MT and /MTd)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)


    # Keep this policy setting; it's very helpful for Debug mode.
    if(POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
                "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
        )
    endif()
endif()

# Set C++ Standard (GDExtension typically requires C++17 or higher)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GODOT_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/godot-cpp")
set(GODOT_CPP_INCLUDE "${GODOT_CPP_DIR}/include")
set(GODOT_CPP_LIB_DIR "${GODOT_CPP_DIR}/bin")
set(DISCORD_PRESENCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/discord-presence")
set(GLAZE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glaze")

set(RESOURCE_SVG_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/ActionCopy.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Close.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/CurveConstant.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/PanelContainer.svg"
)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${RESOURCE_SVG_FILES})

set(GENERATED_SVG_DIR "${CMAKE_CURRENT_BINARY_DIR}/Generated")
set(GENERATED_SVG_HEADER "${GENERATED_SVG_DIR}/EmbeddedSvgResources.h"
        Source/Main/CspDiscordRpcGdCppConstants.h)

file(MAKE_DIRECTORY "${GENERATED_SVG_DIR}")
file(WRITE "${GENERATED_SVG_HEADER}" "#pragma once\n\nnamespace CspDiscordRpcGdCpp::EmbeddedSvgResources\n{\n")

foreach(ResourceSvgFile IN LISTS RESOURCE_SVG_FILES)
    get_filename_component(ResourceSvgName "${ResourceSvgFile}" NAME_WE)
    file(READ "${ResourceSvgFile}" ResourceSvgContent)
    file(APPEND "${GENERATED_SVG_HEADER}" "inline constexpr const char* ${ResourceSvgName} = R\"__CSP_SVG__(\n${ResourceSvgContent}\n)__CSP_SVG__\";\n")
endforeach()

file(APPEND "${GENERATED_SVG_HEADER}" "} // namespace CspDiscordRpcGdCpp::EmbeddedSvgResources\n")

set(DISCORD_RPC_BUILD_TESTS OFF CACHE BOOL "Build discord-presence tests" FORCE)
add_subdirectory("${GLAZE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Dependencies/glaze")
add_subdirectory("${DISCORD_PRESENCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Dependencies/discord-presence")

# Create a Dynamic Link Library Target
# Note: Godot requires GDExtension to be either a MODULE or SHARED library.
add_library(${PROJECT_NAME} SHARED
        Source/Main/RegisterTypes.cpp
        Source/Main/CspDiscordRpcService.cpp
        Source/Main/CspDiscordRpcService.h
        Source/Main/CspDiscordRpcGdCppMainControl.cpp
        Source/Main/CspDiscordRpcGdCppMainControl.h
        Source/Main/CspDiscordRpcGdCppWorkData.h
        Source/Main/CspDiscordRpcGdCppWorkItem.cpp
        Source/Main/CspDiscordRpcGdCppWorkItem.h
        Source/Main/CspDiscordRpcGdCppWorksWindow.cpp
        Source/Main/CspDiscordRpcGdCppWorksWindow.h
        Source/Main/CspDiscordRpcGdCppFunctionLibrary.cpp
        Source/Main/CspDiscordRpcGdCppFunctionLibrary.h
        ${GENERATED_SVG_HEADER}
)

# Include Directories
target_include_directories(${PROJECT_NAME} PUBLIC
        Source
        ${CMAKE_CURRENT_BINARY_DIR}
        ${GODOT_CPP_DIR}
        ${GODOT_CPP_INCLUDE}
        ${GODOT_CPP_DIR}/gdextension
        ${GODOT_CPP_DIR}/gen/include
)

# Linking
find_library(GODOT_CPP_LIB_DEBUG
        NAMES libgodot-cpp.windows.template_debug.dev.x86_64
        PATHS ${GODOT_CPP_LIB_DIR}
)

find_library(GODOT_CPP_LIB_RELEASE
        NAMES libgodot-cpp.windows.template_release.x86_64
        PATHS ${GODOT_CPP_LIB_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:${GODOT_CPP_LIB_DEBUG}>
        $<$<CONFIG:RelWithDebInfo>:${GODOT_CPP_LIB_DEBUG}>
        $<$<CONFIG:Release>:${GODOT_CPP_LIB_RELEASE}>
        discord-rpc
        glaze::glaze
)

# The godot_cpp macros need to define
target_compile_definitions(${PROJECT_NAME} PUBLIC
        $<$<CONFIG:Debug>:
        DEBUG_ENABLED
        DEBUG_METHODS_ENABLED
        >
        $<${COMPILER_IS_MSVC}:
        TYPED_METHOD_BIND
        >
)

# Build type
set(BUILD_TYPE "$<LOWER_CASE:$<CONFIG>>")

# Final output name
set(OUTPUT_NAME "${PROJECT_NAME}.${PLATFORM}.${BUILD_TYPE}.${ARCH}")

set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${OUTPUT_NAME}"
)

if(COMPILER_IS_MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug,RelWithDebInfo>:/DEBUG:FULL>
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
            PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

if(GODOT_PROJECT_BIN_DIR)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_PROJECT_BIN_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${GODOT_PROJECT_BIN_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Copying compiled output to ${GODOT_PROJECT_BIN_DIR}"
    )
else()
    message(WARNING "GODOT_PROJECT_BIN_DIR is not defined. Please set it as a CMake Cache Variable, otherwise automatic deployment of dynamic libraries will be skipped.")
endif()

