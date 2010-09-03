FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/std_afx2.h
						${CMAKE_CURRENT_SOURCE_DIR}/std_afx2.cpp)
ENABLE_LANGUAGE(RC)

ADD_DEFINITIONS(-D_AFXDLL)
SET(CMAKE_MFC_FLAG 2)

ADD_LIBRARY(object_viewer_dll SHARED ${SRC} object_viewer.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(object_viewer_dll 
							nelmisc
							nel3d
							nelsound
							nelsnd_lowlevel
							Version.lib)
							
NL_DEFAULT_PROPS(object_viewer_dll "NeL, Tools, 3D: Object Viewer DLL")
NL_ADD_RUNTIME_FLAGS(object_viewer_dll)
NL_ADD_LIB_SUFFIX(object_viewer_dll)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DOBJECT_VIEWER_LIB_EXPORTS)

INSTALL(TARGETS object_viewer_dll LIBRARY DESTINATION lib RUNTIME DESTINATION bin ARCHIVE DESTINATION lib COMPONENT tools3d)
INSTALL(FILES object_viewer.cfg DESTINATION bin COMPONENT tools3d)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS object_viewer_dll RUNTIME DESTINATION maxplugin COMPONENT tools3d)
  INSTALL(FILES object_viewer.cfg DESTINATION maxplugin COMPONENT tools3d)
ENDIF(WITH_MAXPLUGIN)