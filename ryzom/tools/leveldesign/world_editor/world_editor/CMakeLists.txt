FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/export_cb_dlg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/builder_logic.cpp)

ENABLE_LANGUAGE(RC)
ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)

ADD_EXECUTABLE(world_editor WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(world_editor PRIVATE ${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(world_editor
        PRIVATE
        NeL::misc
        NeL::ligo
        NeL::3d
        NeL::georges
        NeL::pacs
)

NL_DEFAULT_PROPS(world_editor "Ryzom, Tools, World: World Editor")
NL_ADD_RUNTIME_FLAGS(world_editor)

IF (WITH_PCH)
    target_precompile_headers(world_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)
ENDIF ()

INSTALL(TARGETS world_editor RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
