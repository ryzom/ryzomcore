FILE(GLOB SRC *.cpp *.h)

FILE(GLOB R2
        r2_*.h r2_*.cpp)

LIST(REMOVE_ITEM SRC R2)

SOURCE_GROUP("" FILES ${SRC})
SOURCE_GROUP("R2" FILES ${R2})

NL_TARGET_LIB(ryzom_servershare ${SRC} ${R2})
add_library(Ryzom::servershare ALIAS ryzom_servershare)

if (WITH_RYZOM_XSLT)
    add_module_interface(XML_SOURCES
            backup_service_itf.xml
            char_name_mapper_itf.xml
            chat_unifier_itf.xml
            command_executor_itf.xml
            entity_locator_itf.xml
            logger_service_itf.xml
            login_service_itf.xml
            mail_forum_itf.xml
    )
endif ()

TARGET_LINK_LIBRARIES(ryzom_servershare
        PUBLIC
        NeL::misc
        NeL::pacs
        NeL::georges
        NeL::net
        Ryzom::gameshare
        MySQL::MySQL
        ${ZLIB_LIBRARIES}
)

NL_DEFAULT_PROPS(ryzom_servershare "Ryzom, Library: Service Shared")
NL_ADD_RUNTIME_FLAGS(ryzom_servershare)
NL_ADD_LIB_SUFFIX(ryzom_servershare)

IF (WITH_PCH)
    target_precompile_headers(ryzom_servershare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

IF (WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS ryzom_servershare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF ()
