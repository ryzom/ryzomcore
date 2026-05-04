
FILE(GLOB SRC *.cpp *.h)

ADD_LIBRARY(georges_plugin_sound SHARED ${SRC})

TARGET_INCLUDE_DIRECTORIES(georges_plugin_sound PRIVATE ${DXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(georges_plugin_sound
        PUBLIC
        NeL::misc
        NeL::georges
        NeL::sound
        NeL::ligo
        NeL::snd_lowlevel
        georges_dll
        ${DXSDK_DSOUND_LIBRARY}
)
NL_DEFAULT_PROPS(georges_plugin_sound "Ryzom, Tools, Georges: Georges Plugin Sound")
NL_ADD_RUNTIME_FLAGS(georges_plugin_sound)
NL_ADD_LIB_SUFFIX(georges_plugin_sound)
NL_MFC_DLL_WORKAROUND(georges_plugin_sound)

TARGET_COMPILE_DEFINITIONS(georges_plugin_sound PRIVATE ${MFC_DEFINITIONS})

INSTALL(TARGETS georges_plugin_sound LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
