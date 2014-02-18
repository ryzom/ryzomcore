FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheets_packer_shard	${SRC}
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager_sheet.cpp
				${CMAKE_SOURCE_DIR}/ryzom/server/src/input_output_service/string_manager.h)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/ryzom/common/src ${CMAKE_SOURCE_DIR}/ryzom/server/src)
TARGET_LINK_LIBRARIES(sheets_packer_shard
					ryzom_gameshare
					nelmisc
					nelgeorges
					nelnet
					nelligo
					${LIBXML2_LIBRARIES})

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(sheets_packer_shard "Ryzom, Tools: Sheets Packer Shard")
NL_ADD_RUNTIME_FLAGS(sheets_packer_shard)

INSTALL(TARGETS sheets_packer_shard RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
