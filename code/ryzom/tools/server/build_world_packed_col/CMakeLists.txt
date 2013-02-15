FILE(GLOB SRC *.cpp *.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.cpp)

ADD_EXECUTABLE(build_world_packed_col ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(build_world_packed_col	
					ryzom_gameshare
					ryzom_servershare 
					ryzom_aishare 
					${LIBXML2_LIBRARIES} 
					nelmisc
					nel3d)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(build_world_packed_col "Ryzom, Tools, Server: Build World Packed Collisions")
NL_ADD_RUNTIME_FLAGS(build_world_packed_col)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(build_world_packed_col ${CMAKE_CURRENT_SOURCE_DIR}/std_header.h ${CMAKE_CURRENT_SOURCE_DIR}/std_header.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS build_world_packed_col RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
