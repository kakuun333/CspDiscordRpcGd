include_guard(GLOBAL)

# Add godot-cpp include paths, link the matching debug/release library, and define Godot debug macros.
function(target_godot_cpp target)
    set(options)
    set(one_value_args GODOT_CPP_DIR PLATFORM ARCH)
    set(multi_value_args)
    cmake_parse_arguments(GODOT "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT GODOT_GODOT_CPP_DIR)
        set(GODOT_GODOT_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/godot-cpp")
    endif()

    if(NOT GODOT_PLATFORM)
        message(FATAL_ERROR "target_godot_cpp requires PLATFORM")
    endif()

    if(NOT GODOT_ARCH)
        message(FATAL_ERROR "target_godot_cpp requires ARCH")
    endif()

    set(godot_cpp_include "${GODOT_GODOT_CPP_DIR}/include")
    set(godot_cpp_lib_dir "${GODOT_GODOT_CPP_DIR}/bin")

    target_include_directories(${target} PUBLIC
            "${GODOT_GODOT_CPP_DIR}"
            "${godot_cpp_include}"
            "${GODOT_GODOT_CPP_DIR}/gdextension"
            "${GODOT_GODOT_CPP_DIR}/gen/include"
    )

    find_library(GODOT_CPP_LIB_DEBUG
            NAMES
            godot-cpp.${GODOT_PLATFORM}.template_debug.${GODOT_ARCH}
            godot-cpp.${GODOT_PLATFORM}.template_debug.dev.${GODOT_ARCH}
            PATHS "${godot_cpp_lib_dir}"
            NO_DEFAULT_PATH
    )

    find_library(GODOT_CPP_LIB_RELEASE
            NAMES
            godot-cpp.${GODOT_PLATFORM}.template_release.${GODOT_ARCH}
            godot-cpp.${GODOT_PLATFORM}.template_release.dev.${GODOT_ARCH}
            PATHS "${godot_cpp_lib_dir}"
            NO_DEFAULT_PATH
    )

    if(CMAKE_CONFIGURATION_TYPES)
        if(NOT GODOT_CPP_LIB_DEBUG)
            message(FATAL_ERROR "godot-cpp debug library not found in: ${godot_cpp_lib_dir}")
        endif()
        if(NOT GODOT_CPP_LIB_RELEASE)
            message(FATAL_ERROR "godot-cpp release library not found in: ${godot_cpp_lib_dir}")
        endif()
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(NOT GODOT_CPP_LIB_RELEASE)
            message(FATAL_ERROR "godot-cpp release library not found in: ${godot_cpp_lib_dir}")
        endif()
    else()
        if(NOT GODOT_CPP_LIB_DEBUG)
            message(FATAL_ERROR "godot-cpp debug library not found in: ${godot_cpp_lib_dir}")
        endif()
    endif()

    target_link_libraries(${target} PRIVATE
            $<$<CONFIG:Debug>:${GODOT_CPP_LIB_DEBUG}>
            $<$<CONFIG:RelWithDebInfo>:${GODOT_CPP_LIB_DEBUG}>
            $<$<CONFIG:Release>:${GODOT_CPP_LIB_RELEASE}>
    )

    target_compile_definitions(${target} PUBLIC
            $<$<CONFIG:Debug>:
            DEBUG_ENABLED
            HOT_RELOAD_ENABLED
            TOOLS_ENABLED
            >
            $<$<CONFIG:RelWithDebInfo>:
            DEBUG_ENABLED
            HOT_RELOAD_ENABLED
            TOOLS_ENABLED
            >
            $<$<CXX_COMPILER_ID:MSVC>:
            TYPED_METHOD_BIND
            >
    )

    if(GODOT_PLATFORM STREQUAL "macos")
        target_compile_definitions(${target} PUBLIC MACOS_ENABLED UNIX_ENABLED)
    elseif(GODOT_PLATFORM STREQUAL "linux")
        target_compile_definitions(${target} PUBLIC LINUXBSD_ENABLED UNIX_ENABLED)
    elseif(GODOT_PLATFORM STREQUAL "windows")
        target_compile_definitions(${target} PUBLIC WINDOWS_ENABLED)
    endif()
endfunction()

# Apply the project's platform/build/architecture binary naming convention and place outputs in one folder.
function(set_gdextension_output target)
    set(options)
    set(one_value_args PLATFORM ARCH BUILD_TYPE OUTPUT_DIR)
    set(multi_value_args)
    cmake_parse_arguments(OUTPUT "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT OUTPUT_PLATFORM)
        message(FATAL_ERROR "set_gdextension_output requires PLATFORM")
    endif()

    if(NOT OUTPUT_ARCH)
        message(FATAL_ERROR "set_gdextension_output requires ARCH")
    endif()

    if(NOT OUTPUT_OUTPUT_DIR)
        set(OUTPUT_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Binary")
    endif()

    if(NOT OUTPUT_BUILD_TYPE)
        set(OUTPUT_BUILD_TYPE "$<IF:$<CONFIG:Release>,release,debug>")
    endif()

    set(output_name "${target}.${OUTPUT_PLATFORM}.${OUTPUT_BUILD_TYPE}.${OUTPUT_ARCH}")

    set_target_properties(${target} PROPERTIES
            PREFIX ""
            OUTPUT_NAME "${output_name}"

            RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${OUTPUT_OUTPUT_DIR}"

            LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${OUTPUT_OUTPUT_DIR}"

            ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${OUTPUT_OUTPUT_DIR}"
    )

    if(MSVC)
        target_link_options(${target} PRIVATE
                $<$<CONFIG:Debug,RelWithDebInfo>:/DEBUG:FULL>
        )
    endif()
endfunction()
