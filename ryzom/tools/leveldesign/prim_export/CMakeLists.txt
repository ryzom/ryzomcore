FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(prim_export ${SRC})

TARGET_LINK_LIBRARIES(prim_export
        PRIVATE
        NeL::misc
        NeL::ligo
        NeL::3d
        NeL::georges
)

NL_DEFAULT_PROPS(prim_export "Ryzom, Tools, World: Primitive Export")
NL_ADD_RUNTIME_FLAGS(prim_export)

INSTALL(TARGETS prim_export RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
