FILE(GLOB SRC *.cpp *.h)
FILE(GLOB EGSSHEETS ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.cpp ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.h)

ADD_EXECUTABLE(sheets_packer_shard	${SRC} ${EGSSHEETS}
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager_sheet.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.h)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/ryzom/common/src ${CMAKE_SOURCE_DIR}/ryzom/server/src ${CMAKE_SOURCE_DIR}/ryzom/tools/sheets_packer_shard ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service)
TARGET_LINK_LIBRARIES(sheets_packer_shard
					ryzom_gameshare
					ryzom_servershare
					ryzom_aishare
					nelmisc
					nelgeorges
					nelnet
					nelligo
					${LIBXML2_LIBRARIES})

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(sheets_packer_shard "Ryzom, Tools: Sheets Packer Shard")
NL_ADD_RUNTIME_FLAGS(sheets_packer_shard)

SET_TARGET_PROPERTIES(sheets_packer_shard PROPERTIES COMPILE_FLAGS -DNO_EGS_VARS)

INSTALL(TARGETS sheets_packer_shard RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
