FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

FILE(GLOB EGSSHEETS ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.cpp ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.h)

SOURCE_GROUP("" FILES ${SRC})
SOURCE_GROUP("EGS Sheets" FILES ${EGSSHEETS})

ADD_EXECUTABLE(sheets_packer_shard ${SRC} ${EGSSHEETS}
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager_sheet.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager.h
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.h
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.h
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/sheets.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/sheets.h
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/commands_mlf.cpp)


TARGET_INCLUDE_DIRECTORIES(sheets_packer_shard PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/common/src ${CMAKE_SOURCE_DIR}/ryzom/server/src ${CMAKE_SOURCE_DIR}/ryzom/tools/sheets_packer_shard ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service)

TARGET_LINK_LIBRARIES(sheets_packer_shard
        PRIVATE
        ryzom_gameshare
        ryzom_servershare
        ryzom_aishare
        NeL::misc
        NeL::georges
        NeL::net
        NeL::ligo
)

NL_DEFAULT_PROPS(sheets_packer_shard "Ryzom, Tools: Sheets Packer Shard")
NL_ADD_RUNTIME_FLAGS(sheets_packer_shard)

TARGET_COMPILE_DEFINITIONS(sheets_packer_shard PRIVATE NO_EGS_VARS DNO_AI_COMP)

IF (MSVC)
    SET_TARGET_PROPERTIES(sheets_packer_shard PROPERTIES COMPILE_FLAGS "/bigobj") # TODO replace with target_compile_options
ENDIF ()

INSTALL(TARGETS sheets_packer_shard RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
