cmake_minimum_required(VERSION 3.25)

# Build a Universal 2 library by default so the same GDExtension can be loaded
# by both Intel and Apple Silicon Macs.  macOS 10.15 is the oldest deployment
# target supported by the C++ standard library features used by Glaze; Apple
# Silicon itself requires macOS 11.0 or newer.
# Both values remain normal CMake cache variables and can be overridden with
# -DCMAKE_OSX_ARCHITECTURES=... and -DCMAKE_OSX_DEPLOYMENT_TARGET=....
if(APPLE)
    if(NOT DEFINED CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL "")
        set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS target architectures")
    endif()

    if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET OR CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
        if(DEFINED ENV{MACOSX_DEPLOYMENT_TARGET} AND NOT "$ENV{MACOSX_DEPLOYMENT_TARGET}" STREQUAL "")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}" CACHE STRING "Minimum macOS version")
        else()
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
        endif()
    endif()
endif()

project(CspDiscordRpcGdCpp LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(BuildDefaults)
include(EmbeddedBinaryResources)
include(EmbeddedSvgResources)
include(GodotExtension)

configure_cpp_defaults(STANDARD 23)
detect_godot_platform(PLATFORM ARCH)

set(DISCORD_PRESENCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/discord-presence")
set(FMT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/fmt")
set(GLAZE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glaze")

# Generate Resource SVG
set(RESOURCE_SVG_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Icon.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Restore.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Close.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Expand.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Collapse.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Minimize.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Maximize.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Warning.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Home.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Settings.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/Menu.svg"
)

generate_embedded_svg_header(GENERATED_SVG_HEADER
        NAMESPACE "${PROJECT_NAME}::EmbeddedSvgResources"
        FILES ${RESOURCE_SVG_FILES}
)

set(RESOURCE_BINARY_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/CspIconOriginal.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/CspIconVersion5.png"
)

generate_embedded_binary_header(GENERATED_BINARY_HEADER
        NAMESPACE "${PROJECT_NAME}::EmbeddedBinaryResources"
        FILES ${RESOURCE_BINARY_FILES}
)


if(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(DISCORD_RPC_BUILD_TESTS OFF CACHE BOOL "Build discord-presence tests" FORCE)
set(FMT_DOC OFF CACHE BOOL "Generate fmt documentation" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "Generate fmt install target" FORCE)
set(FMT_TEST OFF CACHE BOOL "Build fmt tests" FORCE)
add_subdirectory("${FMT_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Dependencies/fmt")
add_subdirectory("${GLAZE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Dependencies/glaze")
add_subdirectory("${DISCORD_PRESENCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Dependencies/discord-presence")

# Create a Dynamic Link Library Target
# Note: Godot requires GDExtension to be either a MODULE or SHARED library.
add_library(${PROJECT_NAME} SHARED
        Source/RegisterTypes.cpp
        Source/CspDiscordRpcService.cpp
        Source/CspDiscordRpcService.h
        Source/CspDiscordRpcGdCppMainControl.cpp
        Source/CspDiscordRpcGdCppMainControl.h
        Source/CspDiscordRpcGdCppCloseWindow.cpp
        Source/CspDiscordRpcGdCppCloseWindow.h
        Source/CspDiscordRpcGdCppNavigationView.cpp
        Source/CspDiscordRpcGdCppNavigationView.h
        Source/CspDiscordRpcGdCppNavigationViewItem.cpp
        Source/CspDiscordRpcGdCppNavigationViewItem.h
        Source/CspDiscordRpcGdCppWorkData.h
        Source/CspDiscordRpcGdCppWorkItem.cpp
        Source/CspDiscordRpcGdCppWorkItem.h
        Source/CspDiscordRpcGdCppWorksWindow.cpp
        Source/CspDiscordRpcGdCppWorksWindow.h
        ${GENERATED_BINARY_HEADER}
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

target_godot_cpp(${PROJECT_NAME}
        PLATFORM "${PLATFORM}"
        ARCH "${ARCH}"
)

set_gdextension_output(${PROJECT_NAME}
        PLATFORM "${PLATFORM}"
        ARCH "${ARCH}"
        OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Binary"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        discord-rpc
        fmt::fmt
        glaze::glaze
)

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_RPATH ON
            BUILD_RPATH "@loader_path"
            INSTALL_RPATH "@loader_path"
            INSTALL_NAME_DIR "@rpath"
    )
endif()

if(GODOT_PROJECT_BIN_DIR)
    cmake_path(CONVERT "${GODOT_PROJECT_BIN_DIR}" TO_CMAKE_PATH_LIST GODOT_PROJECT_BIN_DIR_CMAKE)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_PROJECT_BIN_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${PROJECT_NAME}>"
                    "${GODOT_PROJECT_BIN_DIR_CMAKE}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Deploying $<TARGET_FILE_NAME:${PROJECT_NAME}> to ${GODOT_PROJECT_BIN_DIR}"
    )
else()
    message(WARNING "GODOT_PROJECT_BIN_DIR is not defined. Please set it as a CMake Cache Variable, otherwise automatic deployment of dynamic libraries will be skipped.")
endif()
