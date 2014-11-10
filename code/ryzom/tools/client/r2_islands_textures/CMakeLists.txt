FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(r2_islands_textures ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(r2_islands_textures
					ryzom_gameshare
					ryzom_aishare
					${LIBXML2_LIBRARIES} 
					nelmisc
					nel3d)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(r2_islands_textures "Ryzom, Tools, Server: R2 Islands Textures")
NL_ADD_RUNTIME_FLAGS(r2_islands_textures)

INSTALL(TARGETS r2_islands_textures RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
