FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(prim_export ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(prim_export	
					nelmisc
					nelligo
					nel3d
					nelgeorges
					${LIBXML2_LIBRARIES})
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(prim_export "Ryzom, Tools, World: Primitive Export")
NL_ADD_RUNTIME_FLAGS(prim_export)

INSTALL(TARGETS prim_export RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
