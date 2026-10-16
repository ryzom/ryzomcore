FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(object_viewer_dll SHARED ${SRC})

TARGET_LINK_LIBRARIES(object_viewer_dll
							NeL::misc
							NeL::3d
							NeL::sound
							NeL::snd_lowlevel
							Version.lib)

# MFC manifest dependency for the resource-compiled SxS manifest path (nel.cmake
# NL_EMBED_SXS_MANIFEST_RC; the MFC_DEFINITIONS ADD_DEFINITIONS below runs after
# NL_DEFAULT_PROPS, so the macro's _AFXDLL directory check can't see it).
SET_TARGET_PROPERTIES(object_viewer_dll PROPERTIES NL_SXS_MANIFEST_MFC TRUE)
NL_DEFAULT_PROPS(object_viewer_dll "NeL, Tools, 3D: Object Viewer DLL")
NL_ADD_RUNTIME_FLAGS(object_viewer_dll)
NL_ADD_LIB_SUFFIX(object_viewer_dll)

ADD_DEFINITIONS(${MFC_DEFINITIONS} -DOBJECT_VIEWER_LIB_EXPORTS)

IF(WITH_PCH)
  target_precompile_headers(object_viewer_dll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h)
ENDIF()

INSTALL(TARGETS object_viewer_dll LIBRARY DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT tools3d)
INSTALL(FILES object_viewer.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT tools3d)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS object_viewer_dll RUNTIME DESTINATION maxplugin COMPONENT tools3d)
  INSTALL(FILES object_viewer.cfg DESTINATION maxplugin COMPONENT tools3d)
ENDIF()
