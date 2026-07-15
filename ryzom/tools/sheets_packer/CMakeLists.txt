FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(sheets_packer ${SRC}
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/continent_manager_build.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/continent_manager_build.h
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.h)

TARGET_INCLUDE_DIRECTORIES(sheets_packer PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/client/src)

TARGET_LINK_LIBRARIES(sheets_packer
        PRIVATE
        Ryzom::clientsheets
        Ryzom::gameshare
        NeL::misc
        NeL::georges
        NeL::net
        NeL::ligo
)

NL_DEFAULT_PROPS(sheets_packer "Ryzom, Tools: Sheets Packer")
NL_ADD_RUNTIME_FLAGS(sheets_packer)

IF (WITH_PCH)
    target_precompile_headers(sheets_packer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS sheets_packer RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
INSTALL(FILES sheets_packer.cfg DESTINATION ${RYZOM_SHARE_PREFIX} COMPONENT tools)
