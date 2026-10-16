if (NOT TARGET generate)
    add_custom_target(generate)
endif ()

function(add_module_interface)
    set(xslt_template ${CMAKE_SOURCE_DIR}/ryzom/common/src/game_share/generate_module_interface.xslt)
    #set(options OPTIONAL FAST)
    #set(oneValueArgs XML_SOURCE)
    set(multiValueArgs XML_SOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 arg
            "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    if (NOT EXISTS "${xslt_template}")
        message(FATAL_ERROR "Cannot find xslt template '${xslt_template}'")
    endif ()


    foreach (xml_source IN LISTS arg_XML_SOURCES)
        set(source "${CMAKE_CURRENT_SOURCE_DIR}/${xml_source}")
        get_filename_component(interface_filename ${source} NAME_WLE)
        set(header "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.h")
        set(cpp "${CMAKE_CURRENT_SOURCE_DIR}/${interface_filename}.cpp")

        if (NOT EXISTS "${source}")
            message(FATAL_ERROR "Cannot find '${source}'")
        endif ()

        add_custom_command(
                OUTPUT "${header}"
                COMMAND LibXslt::xsltproc
                --stringparam output header
                --stringparam filename ${interface_filename}
                --output "${header}"
                ${xslt_template}
                ${source}
                DEPENDS ${xslt_template} ${source}
                VERBATIM
        )

        add_custom_command(
                OUTPUT "${cpp}"
                COMMAND LibXslt::xsltproc
                --stringparam output cpp
                --stringparam filename ${interface_filename}
                --output "${cpp}"
                ${xslt_template}
                ${source}
                DEPENDS ${xslt_template} ${source}
                VERBATIM
        )

        set(target generate_${interface_filename})
        add_custom_target(${target}
                DEPENDS
                ${header}
                ${cpp}
        )

        add_dependencies(generate "${target}")
    endforeach ()
endfunction()
