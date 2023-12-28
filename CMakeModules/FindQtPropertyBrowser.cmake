# Locate QtPropertyBrowser library
# This module defines
#  QtPropertyBrowser_FOUND, if false, do not try to link to QtPropertyBrowser
#  QtPropertyBrowser_LIBRARY
#  QtPropertyBrowser_INCLUDE_DIR, where to find qtpropertybrowser.h
#  QtPropertyBrowser_DIR - Can be set to QtPropertyBrowser install path or Windows build path

find_path(QtPropertyBrowser_INCLUDE_DIR qtpropertybrowser.h
        HINTS ${QTPROPERTYBROWSER_DIR}
        PATH_SUFFIXES include QtPropertyBrowser qtpropertybrowser
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

find_library(QtPropertyBrowser_LIBRARY
        NAMES QtPropertyBrowser qtpropertybrowser
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
# handle the QUIETLY and REQUIRED arguments and set QtPropertyBrowser_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QtPropertyBrowser DEFAULT_MSG QtPropertyBrowser_LIBRARY QtPropertyBrowser_INCLUDE_DIR)

mark_as_advanced(QtPropertyBrowser_INCLUDE_DIR QtPropertyBrowser_LIBRARY)

