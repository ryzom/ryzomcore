FILE(GLOB SRC *.cpp *.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.h ${RZ_SERVER_SRC_DIR}/ai_data_service/pacs_scan.cpp)

ADD_EXECUTABLE(ai_build_wmap ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ai_build_wmap	
					ryzom_gameshare
					ryzom_aishare
					${LIBXML2_LIBRARIES} 
					nelmisc)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(ai_build_wmap "Ryzom, Tools, Server: AI Build World Map")
NL_ADD_RUNTIME_FLAGS(ai_build_wmap)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ai_build_wmap RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
