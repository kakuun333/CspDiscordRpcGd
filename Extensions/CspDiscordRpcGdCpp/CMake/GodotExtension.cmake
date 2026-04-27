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
            NAMES libgodot-cpp.${GODOT_PLATFORM}.template_debug.dev.${GODOT_ARCH}
            PATHS "${godot_cpp_lib_dir}"
    )

    find_library(GODOT_CPP_LIB_RELEASE
            NAMES libgodot-cpp.${GODOT_PLATFORM}.template_release.${GODOT_ARCH}
            PATHS "${godot_cpp_lib_dir}"
    )

    target_link_libraries(${target} PRIVATE
            $<$<CONFIG:Debug>:${GODOT_CPP_LIB_DEBUG}>
            $<$<CONFIG:RelWithDebInfo>:${GODOT_CPP_LIB_DEBUG}>
            $<$<CONFIG:Release>:${GODOT_CPP_LIB_RELEASE}>
    )

    target_compile_definitions(${target} PUBLIC
            $<$<CONFIG:Debug>:
            DEBUG_ENABLED
            DEBUG_METHODS_ENABLED
            TOOLS_ENABLED
            >
            $<$<CONFIG:RelWithDebInfo>:
            DEBUG_ENABLED
            DEBUG_METHODS_ENABLED
            TOOLS_ENABLED
            >
            $<$<CXX_COMPILER_ID:MSVC>:
            TYPED_METHOD_BIND
            >
    )
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
    )

    if(MSVC)
        target_link_options(${target} PRIVATE
                $<$<CONFIG:Debug,RelWithDebInfo>:/DEBUG:FULL>
        )
    endif()
endfunction()
