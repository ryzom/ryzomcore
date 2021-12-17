FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(object_viewer_dll SHARED ${SRC})

TARGET_LINK_LIBRARIES(object_viewer_dll 
							nelmisc
							nel3d
							nelsound
							nelsnd_lowlevel
							Version.lib)
							
NL_DEFAULT_PROPS(object_viewer_dll "NeL, Tools, 3D: Object Viewer DLL")
NL_ADD_RUNTIME_FLAGS(object_viewer_dll)
NL_ADD_LIB_SUFFIX(object_viewer_dll)

ADD_DEFINITIONS(${MFC_DEFINITIONS} -DOBJECT_VIEWER_LIB_EXPORTS)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(object_viewer_dll ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.cpp)
ENDIF()

INSTALL(TARGETS object_viewer_dll LIBRARY DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT tools3d)
INSTALL(FILES object_viewer.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT tools3d)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS object_viewer_dll RUNTIME DESTINATION maxplugin COMPONENT tools3d)
  INSTALL(FILES object_viewer.cfg DESTINATION maxplugin COMPONENT tools3d)
ENDIF()
