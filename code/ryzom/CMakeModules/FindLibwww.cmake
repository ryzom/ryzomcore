#
# Find the W3C libwww includes and library
#
# This module defines
# LIBWWW_INCLUDE_DIR, where to find tiff.h, etc.
# LIBWWW_LIBRARY, where to find the LibWWW library.
# LIBWWW_FOUND, If false, do not try to use LibWWW.

# also defined, but not for general use are
IF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  # in cache already
  SET(LIBWWW_FIND_QUIETLY TRUE)
ENDIF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)

FIND_PATH(LIBWWW_INCLUDE_DIR
  WWWInit.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES libwww w3c-libwww
)

# when installing libwww on mac os x using macports the file wwwconf.h resides
# in /opt/local/include and not in the real libwww include dir :/
FIND_PATH(LIBWWW_ADDITIONAL_INCLUDE_DIR
  wwwconf.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

# combine both include directories into one variable
SET(LIBWWW_INCLUDE_DIR ${LIBWWW_INCLUDE_DIR} ${LIBWWW_ADDITIONAL_INCLUDE_DIR})

# helper to find all the libwww sub libraries
MACRO(FIND_WWW_LIBRARY MYLIBRARY)
  FIND_LIBRARY(${MYLIBRARY}
    NAMES ${ARGN}
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
ENDMACRO(FIND_WWW_LIBRARY MYLIBRARY)

# on eg. mac os x and arch linux, libwww sub libraries are not "inter-linked"
# we need to link them all manually

FIND_WWW_LIBRARY(LIBMD5_LIBRARY md5)
FIND_WWW_LIBRARY(LIBWWWAPP_LIBRARY wwwapp)
FIND_WWW_LIBRARY(LIBWWWCACHE_LIBRARY wwwcache)
FIND_WWW_LIBRARY(LIBWWWCORE_LIBRARY wwwcore)
FIND_WWW_LIBRARY(LIBWWWDIR_LIBRARY wwwdir)
FIND_WWW_LIBRARY(LIBWWWFILE_LIBRARY wwwfile)
FIND_WWW_LIBRARY(LIBWWWHTML_LIBRARY wwwhtml)
FIND_WWW_LIBRARY(LIBWWWHTTP_LIBRARY wwwhttp)
FIND_WWW_LIBRARY(LIBWWWINIT_LIBRARY wwwinit)
FIND_WWW_LIBRARY(LIBWWWMIME_LIBRARY wwwmime)
FIND_WWW_LIBRARY(LIBWWWMUX_LIBRARY wwwmux)
FIND_WWW_LIBRARY(LIBWWWSTREAM_LIBRARY wwwstream)
FIND_WWW_LIBRARY(LIBWWWTRANS_LIBRARY wwwtrans)
FIND_WWW_LIBRARY(LIBWWWUTILS_LIBRARY wwwutils)

# combine all the libraries into one variable
SET(LIBWWW_LIBRARY
  ${LIBMD5_LIBRARY} ${LIBWWWAPP_LIBRARY} ${LIBWWWCACHE_LIBRARY}
  ${LIBWWWCORE_LIBRARY} ${LIBWWWDIR_LIBRARY} ${LIBWWWFILE_LIBRARY}
  ${LIBWWWHTML_LIBRARY} ${LIBWWWHTTP_LIBRARY} ${LIBWWWINIT_LIBRARY}
  ${LIBWWWMIME_LIBRARY} ${LIBWWWMUX_LIBRARY} ${LIBWWWSTREAM_LIBRARY}
  ${LIBWWWTRANS_LIBRARY} ${LIBWWWUTILS_LIBRARY}
)

IF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  SET(LIBWWW_FOUND "YES")
  IF(NOT LIBWWW_FIND_QUIETLY)
    MESSAGE(STATUS "Found LibWWW: ${LIBWWW_LIBRARY}")
  ENDIF(NOT LIBWWW_FIND_QUIETLY)
ELSE(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  IF(NOT LIBWWW_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find LibWWW!")
  ENDIF(NOT LIBWWW_FIND_QUIETLY)
ENDIF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
