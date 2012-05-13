# - Try to find OpenGL ES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGL ES
#  OPENGLES_EGL_FOUND    - system has EGL
#  OPENGLES_LIBRARIES    - Link these to use OpenGL ES and EGL
#   
# If you want to use just GL ES you can use these values
#  OPENGLES_GLES_LIBRARY - Path to OpenGL ES Library
#  OPENGLES_EGL_LIBRARY  - Path to EGL Library

FIND_LIBRARY(OPENGLES_GLES_LIBRARY
  NAMES GLESv1_CM libGLESv1_CM gles_cm libgles_cm
  PATHS
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

FIND_LIBRARY(OPENGLES_EGL_LIBRARY
  NAMES EGL libEGL
  PATHS
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

IF(OPENGLES_GLES_LIBRARY)
  SET(OPENGLES_FOUND "YES")
  SET(OPENGLES_LIBRARIES ${OPENGLES_GLES_LIBRARY} ${OPENGLES_LIBRARIES})
  IF(OPENGLES_EGL_LIBRARY)
    SET(OPENGLES_EGL_FOUND "YES")
    SET(OPENGLES_LIBRARIES ${OPENGLES_EGL_LIBRARY} ${OPENGLES_LIBRARIES})
  ELSE(OPENGLES_EGL_LIBRARY)
    SET(OPENGLES_EGL_FOUND "NO")
  ENDIF(OPENGLES_EGL_LIBRARY)
ENDIF(OPENGLES_GLES_LIBRARY)
