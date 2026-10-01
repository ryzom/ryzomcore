FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

FILE(GLOB EGSSHEETS ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.cpp ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.h)

SOURCE_GROUP("EGS Sheets" FILES ${EGSSHEETS})

ADD_EXECUTABLE(sheets_dump	${SRC} ${EGSSHEETS}
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager_sheet.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/gpm_service/sheets.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/sheets.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/sheets.h
				${CMAKE_SOURCE_DIR}/ryzom/server/src/ai_service/commands_mlf.cpp)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ryzom/common/src ${CMAKE_SOURCE_DIR}/ryzom/server/src ${CMAKE_SOURCE_DIR}/ryzom/server/tools/sheets_dump ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service)

TARGET_LINK_LIBRARIES(sheets_dump
					ryzom_gameshare
					ryzom_servershare
					ryzom_aishare
					nelmisc
					nelgeorges
					nelnet
					nelligo)

NL_DEFAULT_PROPS(sheets_dump "Ryzom, Tools: Sheets Dump")
NL_ADD_RUNTIME_FLAGS(sheets_dump)

ADD_DEFINITIONS(-DNO_EGS_VARS)
ADD_DEFINITIONS(-DNO_AI_COMP)

IF(MSVC)
	SET_TARGET_PROPERTIES(sheets_dump PROPERTIES COMPILE_FLAGS "/bigobj")
ENDIF()

INSTALL(TARGETS sheets_dump RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
