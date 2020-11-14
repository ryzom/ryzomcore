FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(prim_export ${SRC})

TARGET_LINK_LIBRARIES(prim_export	
					nelmisc
					nelligo
					nel3d
					nelgeorges)

NL_DEFAULT_PROPS(prim_export "Ryzom, Tools, World: Primitive Export")
NL_ADD_RUNTIME_FLAGS(prim_export)

INSTALL(TARGETS prim_export RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
