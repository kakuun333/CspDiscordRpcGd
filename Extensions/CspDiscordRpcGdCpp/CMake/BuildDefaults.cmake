include_guard(GLOBAL)

# Configure compiler defaults that should be shared by every target in the project.
function(configure_cpp_defaults)
    set(options)
    set(one_value_args STANDARD)
    set(multi_value_args)
    cmake_parse_arguments(DEFAULTS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT DEFAULTS_STANDARD)
        set(DEFAULTS_STANDARD 20)
    endif()

    set(CMAKE_CXX_STANDARD ${DEFAULTS_STANDARD} PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)

    if(MSVC)
        add_compile_options("/utf-8")

        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded" PARENT_SCOPE)
        add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)

        if(POLICY CMP0141)
            cmake_policy(SET CMP0141 NEW)
            set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
                    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
                    PARENT_SCOPE
            )
        endif()
    endif()
endfunction()

# Convert CMake's host platform and processor names into Godot's binary naming convention.
function(detect_godot_platform out_platform out_arch)
    if(WIN32)
        set(DETECTED_PLATFORM "windows")
    elseif(APPLE)
        set(DETECTED_PLATFORM "macos")
    elseif(UNIX)
        set(DETECTED_PLATFORM "linux")
    else()
        set(DETECTED_PLATFORM "${CMAKE_SYSTEM_NAME}")
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64)")
        set(DETECTED_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(DETECTED_ARCH "arm64")
    else()
        set(DETECTED_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(${out_platform} "${DETECTED_PLATFORM}" PARENT_SCOPE)
    set(${out_arch} "${DETECTED_ARCH}" PARENT_SCOPE)
endfunction()
