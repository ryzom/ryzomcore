FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(sheets_packer	${SRC} 
				${CMAKE_SOURCE_DIR}/ryzom/client/src/continent_manager_build.cpp
				${CMAKE_SOURCE_DIR}/ryzom/client/src/continent_manager_build.h
				${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.cpp
				${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.h)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/ryzom/client/src)

TARGET_LINK_LIBRARIES(sheets_packer	
					ryzom_clientsheets
					ryzom_gameshare
					nelmisc
					nelgeorges
					nelnet
					nelligo)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(sheets_packer "Ryzom, Tools: Sheets Packer")
NL_ADD_RUNTIME_FLAGS(sheets_packer)

IF(WITH_PCH)
  target_precompile_headers(sheets_packer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS sheets_packer RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
INSTALL(FILES sheets_packer.cfg DESTINATION ${RYZOM_SHARE_PREFIX} COMPONENT tools)
