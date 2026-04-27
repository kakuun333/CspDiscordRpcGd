include_guard(GLOBAL)

# Append one SVG file as a raw string literal. Large files are split to keep generated C++ readable.
function(_append_embedded_svg output_header resource_name resource_file)
    set(resource_svg_chunk_size 12000)
    file(READ "${resource_file}" resource_svg_content)
    string(LENGTH "${resource_svg_content}" resource_svg_content_length)

    file(APPEND "${output_header}" "inline constexpr const char* ${resource_name} =\n")

    set(resource_svg_offset 0)
    while(resource_svg_offset LESS resource_svg_content_length)
        math(EXPR resource_svg_remaining_length "${resource_svg_content_length} - ${resource_svg_offset}")

        if(resource_svg_remaining_length GREATER resource_svg_chunk_size)
            set(resource_svg_current_chunk_length ${resource_svg_chunk_size})
        else()
            set(resource_svg_current_chunk_length ${resource_svg_remaining_length})
        endif()

        string(SUBSTRING "${resource_svg_content}" ${resource_svg_offset} ${resource_svg_current_chunk_length} resource_svg_chunk)
        file(APPEND "${output_header}" "R\"__RESOURCE_SVG__(${resource_svg_chunk})__RESOURCE_SVG__\"\n")
        math(EXPR resource_svg_offset "${resource_svg_offset} + ${resource_svg_current_chunk_length}")
    endwhile()

    file(APPEND "${output_header}" ";\n")
endfunction()

# Generate a header containing inline constexpr SVG strings under the requested namespace.
# This function also works with an empty FILES list so projects without SVG resources can reuse it.
function(generate_embedded_svg_header out_header)
    set(options)
    set(one_value_args NAMESPACE OUTPUT_DIR HEADER_NAME)
    set(multi_value_args FILES)
    cmake_parse_arguments(SVG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT SVG_NAMESPACE)
        message(FATAL_ERROR "generate_embedded_svg_header requires NAMESPACE")
    endif()

    if(NOT SVG_OUTPUT_DIR)
        set(SVG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Generated")
    endif()

    if(NOT SVG_HEADER_NAME)
        set(SVG_HEADER_NAME "EmbeddedSvgResources.h")
    endif()

    if(SVG_FILES)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SVG_FILES})
    endif()

    set(generated_svg_header "${SVG_OUTPUT_DIR}/${SVG_HEADER_NAME}")
    file(MAKE_DIRECTORY "${SVG_OUTPUT_DIR}")
    file(WRITE "${generated_svg_header}" "#pragma once\n\nnamespace ${SVG_NAMESPACE}\n{\n")

    if(SVG_FILES)
        foreach(resource_svg_file IN LISTS SVG_FILES)
            get_filename_component(resource_svg_name "${resource_svg_file}" NAME_WE)
            _append_embedded_svg("${generated_svg_header}" "${resource_svg_name}" "${resource_svg_file}")
        endforeach()
    endif()

    file(APPEND "${generated_svg_header}" "} // namespace ${SVG_NAMESPACE}\n")

    set(${out_header} "${generated_svg_header}" PARENT_SCOPE)
endfunction()
