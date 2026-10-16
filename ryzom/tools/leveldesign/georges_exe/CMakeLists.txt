FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(georges WIN32 ${SRC})

TARGET_LINK_LIBRARIES(georges
        PRIVATE
        NeL::misc
        NeL::georges
        georges_dll
)

target_compile_definitions(georges
        PRIVATE
        ${MFC_DEFINITIONS}
)

NL_DEFAULT_PROPS(georges "Ryzom, Tools, Georges: Georges Exe")
NL_ADD_RUNTIME_FLAGS(georges)

INSTALL(TARGETS georges RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
