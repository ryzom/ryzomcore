FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ENABLE_LANGUAGE(RC)
ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(object_viewer WIN32 ${SRC})

TARGET_LINK_LIBRARIES(	object_viewer 
						nelmisc 
						nel3d
						nelsound
						nelsnd_lowlevel
						object_viewer_dll)

NL_DEFAULT_PROPS(object_viewer "NeL, Tools, 3D: Object Viewer")
NL_ADD_RUNTIME_FLAGS(object_viewer)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(object_viewer ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.cpp)
ENDIF()

INSTALL(TARGETS object_viewer RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS object_viewer RUNTIME DESTINATION maxplugin COMPONENT tools3d)
ENDIF()
