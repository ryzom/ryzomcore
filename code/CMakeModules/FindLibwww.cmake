#
# Find the W3C libwww includes and library
#
# This module defines
# LIBWWW_INCLUDE_DIR, where to find tiff.h, etc.
# LIBWWW_LIBRARY, where to find the LibWWW library.
# LIBWWW_FOUND, If false, do not try to use LibWWW.

SET(LIBWWW_FIND_QUIETLY ${Libwww_FIND_QUIETLY})

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
IF(LIBWWW_ADDITIONAL_INCLUDE_DIR)
  SET(LIBWWW_INCLUDE_DIR ${LIBWWW_INCLUDE_DIR} ${LIBWWW_ADDITIONAL_INCLUDE_DIR})
ENDIF(LIBWWW_ADDITIONAL_INCLUDE_DIR)

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
  
  IF(${MYLIBRARY})
    SET(LIBWWW_LIBRARIES ${LIBWWW_LIBRARIES} ${${MYLIBRARY}})
  ELSE(${MYLIBRARY})
    IF(NOT LIBWWW_FIND_QUIETLY)
      MESSAGE(STATUS "Warning: Libwww: Library not found: ${MYLIBRARY}")
    ENDIF(NOT LIBWWW_FIND_QUIETLY)
  ENDIF(${MYLIBRARY})

  MARK_AS_ADVANCED(${MYLIBRARY})  
ENDMACRO(FIND_WWW_LIBRARY MYLIBRARY)

# on eg. mac os x and arch linux, libwww sub libraries are not "inter-linked"
# we need to link them all manually

FIND_WWW_LIBRARY(LIBWWWAPP_LIBRARY wwwapp)
FIND_WWW_LIBRARY(LIBWWWCORE_LIBRARY wwwcore)
FIND_WWW_LIBRARY(LIBWWWDIR_LIBRARY wwwdir)
FIND_WWW_LIBRARY(LIBWWWFILE_LIBRARY wwwfile)
FIND_WWW_LIBRARY(LIBWWWFTP_LIBRARY wwwftp)
FIND_WWW_LIBRARY(LIBWWWGOPHER_LIBRARY wwwgopher)
FIND_WWW_LIBRARY(LIBWWWHTML_LIBRARY wwwhtml)
FIND_WWW_LIBRARY(LIBWWWHTTP_LIBRARY wwwhttp)
FIND_WWW_LIBRARY(LIBWWWINIT_LIBRARY wwwinit)
FIND_WWW_LIBRARY(LIBWWWMIME_LIBRARY wwwmime)
FIND_WWW_LIBRARY(LIBWWWCACHE_LIBRARY wwwcache)
FIND_WWW_LIBRARY(LIBWWWMUX_LIBRARY wwwmux)
FIND_WWW_LIBRARY(LIBWWWNEWS_LIBRARY wwwnews)
FIND_WWW_LIBRARY(LIBWWWSTREAM_LIBRARY wwwstream)
FIND_WWW_LIBRARY(LIBWWWTELNET_LIBRARY wwwtelnet)
FIND_WWW_LIBRARY(LIBWWWTRANS_LIBRARY wwwtrans)
FIND_WWW_LIBRARY(LIBWWWUTILS_LIBRARY wwwutils)
FIND_WWW_LIBRARY(LIBWWWXML_LIBRARY wwwxml)
FIND_WWW_LIBRARY(LIBWWWZIP_LIBRARY wwwzip)
FIND_WWW_LIBRARY(LIBMD5_LIBRARY md5)

# Windows libwww version needs GNU Regex
IF(WIN32 AND WITH_STATIC)
  FIND_PATH(LIBWWW_REGEX_INCLUDE_DIR
    regex.h
    PATHS
    /usr/local/include
    /usr/include
    /sw/include
    /opt/local/include
    /opt/csw/include
    /opt/include
  )

  # combine both include directories into one variable
  IF(LIBWWW_REGEX_INCLUDE_DIR)
    SET(LIBWWW_INCLUDE_DIR ${LIBWWW_INCLUDE_DIR} ${LIBWWW_REGEX_INCLUDE_DIR})
    FIND_WWW_LIBRARY(LIBREGEX_LIBRARY gnu_regex)
  ENDIF(LIBWWW_REGEX_INCLUDE_DIR)
ENDIF(WIN32 AND WITH_STATIC)

IF(WITH_STATIC)
  # If compiled with expat support
  FIND_PACKAGE(EXPAT QUIET)
  IF(EXPAT_FOUND)
    SET(LIBWWW_INCLUDE_DIR ${LIBWWW_INCLUDE_DIR} ${EXPAT_INCLUDE_DIRS})
    SET(LIBWWW_LIBRARIES ${LIBWWW_LIBRARIES} ${EXPAT_LIBRARIES})
  ENDIF(EXPAT_FOUND)

  # If compiled with OpenSSL support
  FIND_PACKAGE(OpenSSL QUIET)
  IF(OPENSSL_FOUND)
    SET(LIBWWW_INCLUDE_DIR ${LIBWWW_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
    SET(LIBWWW_LIBRARIES ${LIBWWW_LIBRARIES} ${OPENSSL_LIBRARIES})
  ENDIF(OPENSSL_FOUND)
ENDIF(WITH_STATIC)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibWWW DEFAULT_MSG
  LIBWWW_LIBRARIES
  LIBWWW_INCLUDE_DIR
)
