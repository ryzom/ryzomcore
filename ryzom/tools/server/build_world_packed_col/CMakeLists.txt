FILE(GLOB SRC *.cpp *.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.cpp *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_world_packed_col ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(build_world_packed_col	
					ryzom_gameshare
					ryzom_servershare 
					ryzom_aishare 
					nelmisc
					nel3d)

NL_DEFAULT_PROPS(build_world_packed_col "Ryzom, Tools, Server: Build World Packed Collisions")
NL_ADD_RUNTIME_FLAGS(build_world_packed_col)

IF(WITH_PCH)
  target_precompile_headers(build_world_packed_col PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/std_header.h)
ENDIF()

INSTALL(TARGETS build_world_packed_col RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
