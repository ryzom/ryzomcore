# Locate QtPropertyBrowser library
# This module defines
#  QTPROPERTYBROWSER_FOUND, if false, do not try to link to QtPropertyBrowser
#  QTPROPERTYBROWSER_LIBRARY
#  QTPROPERTYBROWSER_INCLUDE_DIR, where to find qtpropertybrowser.h
#  QTPROPERTYBROWSER_DIR - Can be set to QtPropertyBrowser install path or Windows build path

find_path(QTPROPERTYBROWSER_INCLUDE_DIR qtpropertybrowser.h
  HINTS ${QTPROPERTYBROWSER_DIR}
  PATH_SUFFIXES include QtPropertyBrowser
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

find_library(QTPROPERTYBROWSER_LIBRARY 
  NAMES QtPropertyBrowser
  HINTS ${QTPROPERTYBROWSER_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set QTPROPERTYBROWSER_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QTPROPERTYBROWSER DEFAULT_MSG QTPROPERTYBROWSER_LIBRARY QTPROPERTYBROWSER_INCLUDE_DIR)

mark_as_advanced(QTPROPERTYBROWSER_INCLUDE_DIR QTPROPERTYBROWSER_LIBRARY)

