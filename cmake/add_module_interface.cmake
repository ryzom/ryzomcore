if (NOT TARGET generate)
    add_custom_target(generate)
endif ()

function(add_module_interface)
    set(xslt_template ${CMAKE_SOURCE_DIR}/ryzom/common/src/game_share/generate_module_interface.xslt)
    #set(options OPTIONAL FAST)
    set(oneValueArgs XML_SOURCE)
    #set(multiValueArgs XML_SOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 arg
            "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    set(source "${CMAKE_CURRENT_SOURCE_DIR}/${arg_XML_SOURCE}")
    get_filename_component(interface_filename ${arg_XML_SOURCE} NAME_WLE)

    if(NOT EXISTS "${xslt_template}")
        message(FATAL_ERROR "Cannot find xslt template '${xslt_template}'")
    endif()
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "Cannot find XML_SOURCE '${source}'")
    endif()

    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.h"
            COMMAND LibXslt::xsltproc
            --stringparam output header
            --stringparam filename ${interface_filename}
            --output "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.h"
            ${xslt_template}
            ${source}
            DEPENDS ${xslt_template} ${arg_XML_SOURCE}
            VERBATIM
    )

    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.cpp"
            COMMAND LibXslt::xsltproc
            --stringparam output cpp
            --stringparam filename ${interface_filename}
            --output "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.cpp"
            ${xslt_template}
            ${source}
            DEPENDS ${xslt_template} ${arg_XML_SOURCE}
            VERBATIM
    )

    set(target generate_${interface_filename})
    add_custom_target(${target}
            DEPENDS
            ${interface_filename}.h
            ${interface_filename}.cpp
    )

    add_dependencies(generate "${target}")
endfunction()
