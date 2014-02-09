
FILE(GLOB SRC *.cpp *.h)

# Bugfix...
IF (NOT DXSDK_INCLUDE_DIR)
	IF (DXSDK_DIR)
		SET(DXSDK_INCLUDE_DIR "${DXSDK_DIR}/Include")
	ENDIF (DXSDK_DIR)
ENDIF (NOT DXSDK_INCLUDE_DIR)

IF (NOT DXSDK_INCLUDE_DIR)
	message(FATAL_ERROR "Configuration bad, cannot find DirectX include.")
ENDIF (NOT DXSDK_INCLUDE_DIR)

ADD_LIBRARY(georges_plugin_sound SHARED ${SRC})

INCLUDE_DIRECTORIES(georges_plugin_sound ${NEL_INCLUDE_DIR} ${DXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(georges_plugin_sound nelmisc nelgeorges nelsound nelligo nelsnd_lowlevel georges_dll ${DXSDK_DSOUND_LIBRARY} ${DXSDK_GUID_LIBRARY})
NL_DEFAULT_PROPS(georges_plugin_sound "Ryzom, Tools, Georges: Georges Plugin Sound")
NL_ADD_RUNTIME_FLAGS(georges_plugin_sound)
NL_ADD_LIB_SUFFIX(georges_plugin_sound)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS georges_plugin_sound LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
