# - Try to find OpenGL ES 3.0
# Once done this will define
#
#  OPENGLES3_FOUND           - system has OpenGL ES 3.0
#  OPENGLES3_INCLUDE_DIR     - the OpenGL ES 3.0 include directory
#  OPENGLES3_LIBRARIES       - Link these to use OpenGL ES 3.0
#

FIND_PATH(OPENGLES3_INCLUDE_DIR
  NAMES GLES3/gl3.h
  PATHS
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_LIBRARY(OPENGLES3_GLES_LIBRARY
  NAMES GLESv2 libGLESv2
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

FIND_LIBRARY(OPENGLES3_EGL_LIBRARY
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

IF(OPENGLES3_GLES_LIBRARY AND OPENGLES3_INCLUDE_DIR)
  SET(OPENGLES3_FOUND "YES")
  SET(OPENGLES3_LIBRARIES ${OPENGLES3_GLES_LIBRARY})
  IF(OPENGLES3_EGL_LIBRARY)
    SET(OPENGLES3_LIBRARIES ${OPENGLES3_EGL_LIBRARY} ${OPENGLES3_LIBRARIES})
  ENDIF()
ENDIF()
