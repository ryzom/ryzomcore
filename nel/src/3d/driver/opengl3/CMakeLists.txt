SET(OpenGL_GL_PREFERENCE "LEGACY")
IF(NOT WIN32)
  IF(APPLE)
    FIND_LIBRARY(CARBON NAMES Carbon)
    FIND_LIBRARY(COCOA NAMES Cocoa)
    FIND_PACKAGE(OpenGL REQUIRED)
  ELSE()
    SET(OLD_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .so)
    FIND_PACKAGE(X11)
    FIND_PACKAGE(XF86VidMode)
    FIND_PACKAGE(OpenGL REQUIRED)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ${OLD_CMAKE_FIND_LIBRARY_SUFFIXES})
  ENDIF()
ELSE()
  FIND_PACKAGE(OpenGL REQUIRED)
ENDIF()

FILE(GLOB SRC *.cpp *.h *.def)

SOURCE_GROUP("" FILES
		driver_opengl.cpp
		driver_opengl.h
		driver_opengl.def
		stdopengl.cpp
		stdopengl.h)

SOURCE_GROUP(rendering FILES
		driver_opengl_rendering.cpp
		driver_opengl_material.cpp
		driver_opengl_texture.cpp
		driver_opengl_light.cpp
		driver_opengl_matrix.cpp
		driver_opengl_states.cpp
		driver_opengl_states.h)

SOURCE_GROUP(program FILES
		driver_opengl_program.cpp
		driver_opengl_program.h
		driver_opengl_vertex_program.cpp
		driver_opengl_pixel_program.cpp
		driver_opengl_vertex_mega_program.cpp
		driver_opengl_pixel_mega_program.cpp)

SOURCE_GROUP(vertex FILES
		driver_opengl_vertex.cpp
		driver_opengl_vertex_buffer.cpp
		driver_opengl_vertex_buffer.h
		driver_opengl_uniform_buffer.cpp
		driver_opengl_uniform_buffer.h)

SOURCE_GROUP(extension FILES
		driver_opengl_extension.cpp
		driver_opengl_extension.h
		driver_opengl_extension_def.h)

SOURCE_GROUP(window FILES
		driver_opengl_window.cpp
		driver_opengl_inputs.cpp
		unix_event_emitter.cpp
		unix_event_emitter.h)

IF(APPLE)
  FILE(GLOB MAC_SRC mac/*.h mac/*.m mac/*.mm mac/*.cpp)
  SET(SRC ${SRC} ${MAC_SRC})
  SET_SOURCE_FILES_PROPERTIES(${SRC}
    PROPERTIES COMPILE_FLAGS "-x objective-c++")
ENDIF()

IF(WIN32)
  SET(NLDRV_OGL_LIB "nel_drv_opengl3_win")
ELSE(WIN32)
  SET(NLDRV_OGL_LIB "nel_drv_opengl3")
ENDIF(WIN32)

NL_TARGET_DRIVER(${NLDRV_OGL_LIB} ${SRC})

TARGET_INCLUDE_DIRECTORIES(${NLDRV_OGL_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} nel3d nelmisc OpenGL::GL)
NL_DEFAULT_PROPS(${NLDRV_OGL_LIB} "NeL, Driver, Video: OpenGL3")
NL_ADD_LIB_SUFFIX(${NLDRV_OGL_LIB})
NL_ADD_RUNTIME_FLAGS(${NLDRV_OGL_LIB})

IF(WIN32)
  TARGET_COMPILE_DEFINITIONS(${NLDRV_OGL_LIB} PRIVATE DRIVER_OPENGL_EXPORTS)
ENDIF()

IF(APPLE)
  TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${CARBON})
  TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${COCOA})
ENDIF()

IF(UNIX AND NOT APPLE)
  TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${X11_X11_LIB})
  IF(XF86VidMode_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${NLDRV_OGL_LIB} PRIVATE ${XF86VidMode_INCLUDE_DIR})
    TARGET_COMPILE_DEFINITIONS(${NLDRV_OGL_LIB} PRIVATE ${XF86VidMode_DEFINITIONS})
    TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${XF86VidMode_LIBRARY})
  ENDIF()
  IF(X11_Xrandr_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${NLDRV_OGL_LIB} PRIVATE ${X11_Xrandr_INCLUDE_PATH})
    TARGET_COMPILE_DEFINITIONS(${NLDRV_OGL_LIB} PRIVATE HAVE_XRANDR)
    TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${X11_Xrandr_LIB})
  ENDIF()
  IF(X11_Xrender_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${NLDRV_OGL_LIB} PRIVATE ${X11_Xrender_INCLUDE_PATH})
    TARGET_COMPILE_DEFINITIONS(${NLDRV_OGL_LIB} PRIVATE HAVE_XRENDER)
    TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${X11_Xrender_LIB})
  ENDIF()
  IF(X11_Xcursor_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${NLDRV_OGL_LIB} PRIVATE ${X11_Xcursor_INCLUDE_PATH})
    TARGET_COMPILE_DEFINITIONS(${NLDRV_OGL_LIB} PRIVATE HAVE_XCURSOR)
    TARGET_LINK_LIBRARIES(${NLDRV_OGL_LIB} ${X11_Xcursor_LIB})
  ENDIF()
ENDIF()

IF(WITH_PCH)
  target_precompile_headers(${NLDRV_OGL_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdopengl.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS ${NLDRV_OGL_LIB} LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_DRIVER_PREFIX} COMPONENT drivers3d)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS ${NLDRV_OGL_LIB} RUNTIME DESTINATION maxplugin COMPONENT drivers3d)
  ENDIF()
ENDIF()
