include_guard(GLOBAL)

function(_append_embedded_binary output_header resource_name resource_file)
    set(resource_bytes_per_line 16)
    math(EXPR resource_hex_chars_per_line "${resource_bytes_per_line} * 2")

    file(READ "${resource_file}" resource_hex_content HEX)
    string(LENGTH "${resource_hex_content}" resource_hex_content_length)
    math(EXPR resource_byte_count "${resource_hex_content_length} / 2")

    file(APPEND "${output_header}" "inline constexpr std::uint8_t ${resource_name}[${resource_byte_count}] = {\n")

    set(resource_hex_offset 0)
    while(resource_hex_offset LESS resource_hex_content_length)
        math(EXPR resource_hex_remaining_length "${resource_hex_content_length} - ${resource_hex_offset}")
        if(resource_hex_remaining_length GREATER resource_hex_chars_per_line)
            set(resource_hex_current_line_length ${resource_hex_chars_per_line})
        else()
            set(resource_hex_current_line_length ${resource_hex_remaining_length})
        endif()

        string(SUBSTRING
                "${resource_hex_content}"
                ${resource_hex_offset}
                ${resource_hex_current_line_length}
                resource_hex_line
        )
        string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1, " resource_byte_line "${resource_hex_line}")
        file(APPEND "${output_header}" "    ${resource_byte_line}\n")
        math(EXPR resource_hex_offset "${resource_hex_offset} + ${resource_hex_current_line_length}")
    endwhile()

    file(APPEND "${output_header}" "};\n")
endfunction()

function(generate_embedded_binary_header out_header)
    set(options)
    set(one_value_args NAMESPACE OUTPUT_DIR HEADER_NAME)
    set(multi_value_args FILES)
    cmake_parse_arguments(BINARY "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT BINARY_NAMESPACE)
        message(FATAL_ERROR "generate_embedded_binary_header requires NAMESPACE")
    endif()

    if(NOT BINARY_OUTPUT_DIR)
        set(BINARY_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Generated")
    endif()

    if(NOT BINARY_HEADER_NAME)
        set(BINARY_HEADER_NAME "EmbeddedBinaryResources.h")
    endif()

    if(BINARY_FILES)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${BINARY_FILES})
    endif()

    set(generated_binary_header "${BINARY_OUTPUT_DIR}/${BINARY_HEADER_NAME}")
    file(MAKE_DIRECTORY "${BINARY_OUTPUT_DIR}")
    file(WRITE "${generated_binary_header}" "#pragma once\n\n#include <cstdint>\n\nnamespace ${BINARY_NAMESPACE}\n{\n")

    foreach(resource_binary_file IN LISTS BINARY_FILES)
        get_filename_component(resource_binary_name "${resource_binary_file}" NAME_WE)
        _append_embedded_binary("${generated_binary_header}" "${resource_binary_name}" "${resource_binary_file}")
    endforeach()

    file(APPEND "${generated_binary_header}" "} // namespace ${BINARY_NAMESPACE}\n")

    set(${out_header} "${generated_binary_header}" PARENT_SCOPE)
endfunction()
