MACRO(CONVERT_NUMBER_VERSION _VERSION_NUMBER _BASE _OUT)
  SET(${_OUT})
  SET(_NUMBER ${_VERSION_NUMBER})
  WHILE(_NUMBER GREATER 0)
    MATH(EXPR _TEMP "${_NUMBER} % ${_BASE}")
    LIST(APPEND ${_OUT} ${_TEMP})
    MATH(EXPR _NUMBER "${_NUMBER} / ${_BASE}")
  ENDWHILE()
ENDMACRO(CONVERT_NUMBER_VERSION)

FUNCTION(JOIN VALUES GLUE OUTPUT)
  STRING(REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  STRING(REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  SET(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
ENDFUNCTION()

MACRO(PARSE_VERSION_OTHER FILENAME)
  IF(EXISTS ${FILENAME})
    SET(_FILTER_ARRAY ${ARGN})
    JOIN("${_FILTER_ARRAY}" "|" _FILTER_REGEX)
    FILE(STRINGS ${FILENAME} _FILE REGEX "(${_FILTER_REGEX})[: \t=\(\)\"]+([0-9.]+)")

    IF(_FILE)
      FOREACH(_LINE ${_FILE})
        FOREACH(_VAR ${_FILTER_ARRAY})
          IF("${${_VAR}}" STREQUAL "")
            STRING(REGEX REPLACE "^.*${_VAR}[: \t=\(\)\"]+([0-9.]+).*$" "\\1" ${_VAR} "${_LINE}")
            IF(${_VAR} STREQUAL "${_LINE}")
              SET(${_VAR})
            ENDIF()
            IF(NOT ${_VAR} AND NOT STREQUAL "0")
              SET(${_VAR} 0)
            ENDIF()
          ENDIF()
        ENDFOREACH()
      ENDFOREACH()
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(FIND_PACKAGE_HELPER NAME INCLUDE)
  # Looks for a directory containing NAME.
  #
  # NAME is the name of the library, lowercase and uppercase can be mixed
  # It should be EXACTLY (same case) the same part as XXXX in FindXXXX.cmake
  #
  # INCLUDE is the file to check for includes
  #
  # Following parameters are optional variables and must be prefixed by:
  #
  # RELEASE is the list of libraries to check in release mode
  # DEBUG is the list of libraries to check in debug mode
  # SUFFIXES is the PATH_SUFFIXES to check for include file
  # QUIET don't display anything
  # VERBOSE display more details if not found
  # REQUIRED throw an error if not found
  # DIR is the base directory where to look for
  #
  # The first match will be used in the specified order and next matches will be ignored
  #
  # The following values are defined
  # NAME_INCLUDE_DIR - where to find NAME
  # NAME_LIBRARIES   - link against these to use NAME
  # NAME_FOUND       - True if NAME is available.

  SET(_PARAMS ${ARGN})

  SET(_RELEASE_LIBRARIES)
  SET(_DEBUG_LIBRARIES)
  SET(_SUFFIXES)
  SET(_BASE_DIRECTORIES)

  SET(_IS_RELEASE OFF)
  SET(_IS_DEBUG OFF)
  SET(_IS_SUFFIXES OFF)
  SET(_IS_VERBOSE OFF)
  SET(_IS_DIR OFF)

  IF(_PARAMS)
    FOREACH(_PARAM ${_PARAMS})
      IF(_PARAM STREQUAL "RELEASE")
        SET(_IS_RELEASE ON)
        SET(_IS_DEBUG OFF)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR OFF)
      ELSEIF(_PARAM STREQUAL "DEBUG")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG ON)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR OFF)
      ELSEIF(_PARAM STREQUAL "SUFFIXES")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_DIR OFF)
        SET(_IS_SUFFIXES ON)
      ELSEIF(_PARAM STREQUAL "QUIET")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR OFF)
        SET(${NAME}_FIND_QUIETLY ON)
      ELSEIF(_PARAM STREQUAL "VERBOSE")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR OFF)
        SET(_IS_VERBOSE ON)
      ELSEIF(_PARAM STREQUAL "REQUIRED")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR OFF)
        SET(${NAME}_FIND_REQUIRED ON)
      ELSEIF(_PARAM STREQUAL "DIR")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_SUFFIXES OFF)
        SET(_IS_DIR ON)
      ELSE()
        IF(_IS_RELEASE)
          LIST(APPEND _RELEASE_LIBRARIES ${_PARAM})
        ELSEIF(_IS_DEBUG)
          LIST(APPEND _DEBUG_LIBRARIES ${_PARAM})
        ELSEIF(_IS_SUFFIXES)
          LIST(APPEND _SUFFIXES ${_PARAM})
        ELSEIF(_IS_DIR)
          LIST(APPEND _BASE_DIRECTORIES ${_PARAM})
        ELSE()
          MESSAGE(STATUS "parameter ${_PARAM} with no prefix")
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  # Fixes names if invalid characters are found
  IF("${NAME}" MATCHES "^[a-zA-Z0-9]+$")
    SET(_NAME_FIXED ${NAME})
  ELSE()
    # if invalid characters are detected, replace them by valid ones
    STRING(REPLACE "+" "p" _NAME_FIXED ${NAME})
  ENDIF()

  # Create uppercase and lowercase versions of NAME
  STRING(TOUPPER ${NAME} _UPNAME)
  STRING(TOLOWER ${NAME} _LOWNAME)

  STRING(TOUPPER ${_NAME_FIXED} _UPNAME_FIXED)
  STRING(TOLOWER ${_NAME_FIXED} _LOWNAME_FIXED)

  SET(_SUFFIXES ${_SUFFIXES} ${_LOWNAME} ${_LOWNAME_FIXED} ${NAME})

# Don't use pkg-config
#  IF(NOT WIN32 AND NOT IOS)
#    FIND_PACKAGE(PkgConfig QUIET)
#    SET(_MODULES ${_LOWNAME} ${_RELEASE_LIBRARIES})
#    LIST(REMOVE_DUPLICATES _MODULES)
#    IF(PKG_CONFIG_EXECUTABLE)
#      PKG_SEARCH_MODULE(PKG_${_NAME_FIXED} QUIET ${_MODULES})
#    ENDIF()
#  ENDIF()

  SET(_INCLUDE_PATHS)
  SET(_LIBRARY_PATHS)

  # Check for root directories passed to CMake with -DXXX_DIR=...
  IF(DEFINED ${_UPNAME_FIXED}_DIR)
    SET(_TMP ${${_UPNAME_FIXED}_DIR})
    GET_FILENAME_COMPONENT(_TMP ${_TMP} ABSOLUTE)
    LIST(APPEND _INCLUDE_PATHS ${_TMP}/include ${_TMP})
    LIST(APPEND _LIBRARY_PATHS ${_TMP}/lib${LIB_SUFFIX})

    IF(_IS_VERBOSE)
      MESSAGE(STATUS "Using ${_UPNAME_FIXED}_DIR as root directory ${_TMP}")
    ENDIF()
  ENDIF()

  IF(DEFINED ${_UPNAME}_DIR)
    SET(_TMP ${${_UPNAME}_DIR})
    LIST(APPEND _INCLUDE_PATHS ${_TMP}/include ${_TMP})
    LIST(APPEND _LIBRARY_PATHS ${_TMP}/lib${LIB_SUFFIX})

    IF(_IS_VERBOSE)
      MESSAGE(STATUS "Using ${_UPNAME_FIXED}_DIR as root directory ${_TMP}")
    ENDIF()
  ENDIF()

  IF(_BASE_DIRECTORIES)
    FOREACH(_DIR ${_BASE_DIRECTORIES})
      IF(_DIR)
        LIST(APPEND _INCLUDE_PATHS ${_DIR}/include ${_DIR})
        LIST(APPEND _LIBRARY_PATHS ${_DIR}/lib${LIB_SUFFIX})

        IF(_IS_VERBOSE)
          MESSAGE(STATUS "Using ${_DIR} as root directory")
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  IF(UNIX)
    # Append UNIX standard include paths
    SET(_UNIX_INCLUDE_PATHS)

    # Append multiarch include paths
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      LIST(APPEND _UNIX_INCLUDE_PATHS
        /usr/local/include/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/include/${CMAKE_LIBRARY_ARCHITECTURE})
    ENDIF()

    LIST(APPEND _UNIX_INCLUDE_PATHS
      /usr/local/include
      /usr/include
      /sw/include
      /opt/local/include
      /opt/csw/include
      /opt/include)
  ENDIF()

  IF(_IS_VERBOSE)
    MESSAGE(STATUS "Searching header ${INCLUDE} in: ${_INCLUDE_PATHS} with suffixes ${_SUFFIXES}")
  ENDIF()

  # Search for include directory
  FIND_PATH(${_UPNAME_FIXED}_INCLUDE_DIR
    NAMES ${INCLUDE}
    HINTS
      ${PKG_${_NAME_FIXED}_INCLUDE_DIRS}
      ${_INCLUDE_PATHS}
      $ENV{${_UPNAME}_DIR}/include
      $ENV{${_UPNAME_FIXED}_DIR}/include
      $ENV{${_UPNAME}_DIR}
      $ENV{${_UPNAME_FIXED}_DIR}
    PATHS
    ${_UNIX_INCLUDE_PATHS}
    PATH_SUFFIXES
    ${_SUFFIXES}
    DOC "Include path for ${NAME}"
  )

  IF(_IS_VERBOSE)
    IF(${_UPNAME_FIXED}_INCLUDE_DIR)
      MESSAGE(STATUS "${INCLUDE} found in ${${_UPNAME_FIXED}_INCLUDE_DIR}")
    ELSE()
      MESSAGE(STATUS "${INCLUDE} not found")
    ENDIF()
  ENDIF()

  # Append environment variables XXX_DIR
  LIST(APPEND _LIBRARY_PATHS
    $ENV{${_UPNAME}_DIR}/lib${LIB_SUFFIX}
    $ENV{${_UPNAME_FIXED}_DIR}/lib${LIB_SUFFIX})

  IF(UNIX)
    SET(_UNIX_LIBRARY_PATHS)

    # Append multiarch libraries paths
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      LIST(APPEND _UNIX_LIBRARY_PATHS
        /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE})
    ENDIF()

    # Append UNIX standard libraries paths
    LIST(APPEND _UNIX_LIBRARY_PATHS
      /usr/local/lib
      /usr/lib
      /lib
      /usr/local/X11R6/lib
      /usr/X11R6/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib
      /usr/freeware/lib${LIB_SUFFIX})
  ENDIF()

  LIST(APPEND _RELEASE_LIBRARIES ${_LOWNAME} ${_LOWNAME_FIXED} ${NAME} ${_NAME_FIXED})
  LIST(APPEND _DEBUG_LIBRARIES ${_LOWNAME}d ${_LOWNAME_FIXED}d ${NAME}d ${_NAME_FIXED}d)

  # Under Windows, some libs may need the lib prefix
  IF(WIN32)
    SET(_LIBS ${_RELEASE_LIBRARIES})
    FOREACH(_LIB ${_LIBS})
      LIST(APPEND _RELEASE_LIBRARIES lib${_LIB})
    ENDFOREACH()

    SET(_LIBS ${_DEBUG_LIBRARIES})
    FOREACH(_LIB ${_LIBS})
      LIST(APPEND _DEBUG_LIBRARIES lib${_LIB})
    ENDFOREACH()
  ENDIF()

  LIST(REMOVE_DUPLICATES _RELEASE_LIBRARIES)
  LIST(REMOVE_DUPLICATES _DEBUG_LIBRARIES)

  # Search for release library
  FIND_LIBRARY(${_UPNAME_FIXED}_LIBRARY_RELEASE
    NAMES
    ${_RELEASE_LIBRARIES}
    HINTS ${PKG_${_NAME_FIXED}_LIBRARY_DIRS}
    PATHS
    ${_LIBRARY_PATHS}
    ${_UNIX_LIBRARY_PATHS}
    NO_CMAKE_SYSTEM_PATH
  )

  IF(_IS_VERBOSE)
    IF(${_UPNAME_FIXED}_LIBRARY_RELEASE)
      MESSAGE(STATUS "${NAME} release library found: ${${_UPNAME_FIXED}_LIBRARY_RELEASE}")
    ELSE()
      MESSAGE(STATUS "${NAME} release library not found in ${_LIBRARY_PATHS};${_UNIX_LIBRARY_PATHS}")
    ENDIF()
  ENDIF()

  # Search for debug library
  FIND_LIBRARY(${_UPNAME_FIXED}_LIBRARY_DEBUG
    NAMES
    ${_DEBUG_LIBRARIES}
    HINTS ${PKG_${_NAME_FIXED}_LIBRARY_DIRS}
    PATHS
    ${_LIBRARY_PATHS}
    ${_UNIX_LIBRARY_PATHS}
    NO_CMAKE_SYSTEM_PATH
  )

  IF(_IS_VERBOSE)
    IF(${_UPNAME_FIXED}_LIBRARY_DEBUG)
      MESSAGE(STATUS "${NAME} debug library found: ${${_UPNAME_FIXED}_LIBRARY_DEBUG}")
    ELSE()
      MESSAGE(STATUS "${NAME} debug library not found")
    ENDIF()
  ENDIF()

  SET(${_UPNAME_FIXED}_FOUND OFF)

  IF(${_UPNAME_FIXED}_INCLUDE_DIR)
    # Set also _INCLUDE_DIRS
    SET(${_UPNAME_FIXED}_INCLUDE_DIRS ${${_UPNAME_FIXED}_INCLUDE_DIR})
  ENDIF()

  # Library has been found if at least only one library and include are found
  IF(${_UPNAME_FIXED}_LIBRARY_RELEASE AND ${_UPNAME_FIXED}_LIBRARY_DEBUG)
    # Release and debug libraries found
    SET(${_UPNAME_FIXED}_FOUND ON)
    SET(${_UPNAME_FIXED}_LIBRARIES optimized ${${_UPNAME_FIXED}_LIBRARY_RELEASE} debug ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
  ELSEIF(${_UPNAME_FIXED}_LIBRARY_RELEASE)
    # Release library found
    SET(${_UPNAME_FIXED}_FOUND ON)
    SET(${_UPNAME_FIXED}_LIBRARIES ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
  ELSEIF(${_UPNAME_FIXED}_LIBRARY_DEBUG)
    # Debug library found
    SET(${_UPNAME_FIXED}_FOUND ON)
    SET(${_UPNAME_FIXED}_LIBRARIES ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
  ENDIF()

  IF(${_UPNAME_FIXED}_FOUND)
    IF(NOT ${NAME}_FIND_QUIETLY)
      MESSAGE(STATUS "Found ${NAME}: ${${_UPNAME_FIXED}_LIBRARIES}")
    ENDIF()
  ELSE()
    IF(${NAME}_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Error: Unable to find ${NAME}!")
    ENDIF()
    IF(NOT ${NAME}_FIND_QUIETLY)
      MESSAGE(STATUS "Warning: Unable to find ${NAME}!")
    ENDIF()
  ENDIF()

  MARK_AS_ADVANCED(${_UPNAME_FIXED}_LIBRARY_RELEASE ${_UPNAME_FIXED}_LIBRARY_DEBUG)
ENDMACRO()

MACRO(MESSAGE_VERSION_PACKAGE_HELPER NAME VERSION)
  MESSAGE(STATUS "Found ${NAME} ${VERSION}: ${ARGN}")
ENDMACRO()

MACRO(FIND_LIBRARY_HELPER NAME)
  # Looks for libraries.
  #
  # NAME is the name of the library, lowercase and uppercase can be mixed
  #
  # Following parameters are optional variables and must be prefixed by:
  #
  # RELEASE is the list of libraries to check in release mode
  # DEBUG is the list of libraries to check in debug mode
  # VERBOSE display more details if not found
  # REQUIRED throw an error if not found
  # DIR is the base directory where to look for
  #
  # The first match will be used in the specified order and next matches will be ignored
  #
  # The following values are defined
  # NAME_LIBRARIES   - link against these to use NAME

  SET(_PARAMS ${ARGN})

  SET(_RELEASE_LIBRARIES)
  SET(_DEBUG_LIBRARIES)
  SET(_BASE_DIRECTORIES)

  SET(_IS_RELEASE OFF)
  SET(_IS_DEBUG OFF)
  SET(_IS_VERBOSE OFF)
  SET(_IS_DIR OFF)

  IF(_PARAMS)
    FOREACH(_PARAM ${_PARAMS})
      IF(_PARAM STREQUAL "RELEASE")
        SET(_IS_RELEASE ON)
        SET(_IS_DEBUG OFF)
        SET(_IS_DIR OFF)
      ELSEIF(_PARAM STREQUAL "DEBUG")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG ON)
        SET(_IS_DIR OFF)
      ELSEIF(_PARAM STREQUAL "VERBOSE")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_DIR OFF)
        SET(_IS_VERBOSE ON)
      ELSEIF(_PARAM STREQUAL "REQUIRED")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_DIR OFF)
        SET(${NAME}_FIND_REQUIRED ON)
      ELSEIF(_PARAM STREQUAL "DIR")
        SET(_IS_RELEASE OFF)
        SET(_IS_DEBUG OFF)
        SET(_IS_DIR ON)
      ELSE()
        IF(_IS_RELEASE)
          LIST(APPEND _RELEASE_LIBRARIES ${_PARAM})
        ELSEIF(_IS_DEBUG)
          LIST(APPEND _DEBUG_LIBRARIES ${_PARAM})
        ELSEIF(_IS_DIR)
          LIST(APPEND _BASE_DIRECTORIES ${_PARAM})
        ELSE()
          MESSAGE(STATUS "parameter ${_PARAM} with no prefix")
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  # Fixes names if invalid characters are found
  IF("${NAME}" MATCHES "^[a-zA-Z0-9]+$")
    SET(_NAME_FIXED ${NAME})
  ELSE()
    # if invalid characters are detected, replace them by valid ones
    STRING(REPLACE "+" "p" _NAME_FIXED ${NAME})
  ENDIF()

  # Create uppercase and lowercase versions of NAME
  STRING(TOUPPER ${NAME} _UPNAME)
  STRING(TOLOWER ${NAME} _LOWNAME)

  STRING(TOUPPER ${_NAME_FIXED} _UPNAME_FIXED)
  STRING(TOLOWER ${_NAME_FIXED} _LOWNAME_FIXED)

  SET(_LIBRARY_PATHS)

  # Check for root directories passed to CMake with -DXXX_DIR=...
  IF(DEFINED ${_UPNAME_FIXED}_DIR)
    SET(_TMP ${${_UPNAME_FIXED}_DIR})
    GET_FILENAME_COMPONENT(_TMP ${_TMP} ABSOLUTE)
    LIST(APPEND _LIBRARY_PATHS ${_TMP}/lib${LIB_SUFFIX})

    IF(_IS_VERBOSE)
      MESSAGE(STATUS "Using ${_UPNAME_FIXED}_DIR as root directory ${_TMP}")
    ENDIF()
  ENDIF()

  IF(DEFINED ${_UPNAME}_DIR)
    SET(_TMP ${${_UPNAME}_DIR})
    LIST(APPEND _LIBRARY_PATHS ${_TMP}/lib${LIB_SUFFIX})

    IF(_IS_VERBOSE)
      MESSAGE(STATUS "Using ${_UPNAME_FIXED}_DIR as root directory ${_TMP}")
    ENDIF()
  ENDIF()

  IF(_BASE_DIRECTORIES)
    FOREACH(_DIR ${_BASE_DIRECTORIES})
      IF(_DIR)
        LIST(APPEND _LIBRARY_PATHS ${_DIR}/lib${LIB_SUFFIX})

        IF(_IS_VERBOSE)
          MESSAGE(STATUS "Using ${_DIR} as root directory")
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  # Append environment variables XXX_DIR
  LIST(APPEND _LIBRARY_PATHS
    $ENV{${_UPNAME}_DIR}/lib${LIB_SUFFIX}
    $ENV{${_UPNAME_FIXED}_DIR}/lib${LIB_SUFFIX})

  IF(UNIX)
    SET(_UNIX_LIBRARY_PATHS)

    # Append multiarch libraries paths
    IF(CMAKE_LIBRARY_ARCHITECTURE)
      LIST(APPEND _UNIX_LIBRARY_PATHS
        /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE})
    ENDIF()

    # Append UNIX standard libraries paths
    LIST(APPEND _UNIX_LIBRARY_PATHS
      /usr/local/lib
      /usr/lib
      /lib
      /usr/local/X11R6/lib
      /usr/X11R6/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib
      /usr/freeware/lib${LIB_SUFFIX})
  ENDIF()

  LIST(APPEND _RELEASE_LIBRARIES ${_LOWNAME} ${_LOWNAME_FIXED} ${NAME} ${_NAME_FIXED})
  LIST(APPEND _DEBUG_LIBRARIES ${_LOWNAME}d ${_LOWNAME_FIXED}d ${NAME}d ${_NAME_FIXED}d)

  # Under Windows, some libs may need the lib prefix
  IF(WIN32)
    SET(_LIBS ${_RELEASE_LIBRARIES})
    FOREACH(_LIB ${_LIBS})
      LIST(APPEND _RELEASE_LIBRARIES lib${_LIB})
    ENDFOREACH()

    SET(_LIBS ${_DEBUG_LIBRARIES})
    FOREACH(_LIB ${_LIBS})
      LIST(APPEND _DEBUG_LIBRARIES lib${_LIB})
    ENDFOREACH()
  ENDIF()

  LIST(REMOVE_DUPLICATES _RELEASE_LIBRARIES)
  LIST(REMOVE_DUPLICATES _DEBUG_LIBRARIES)

  # Search for release library
  FIND_LIBRARY(${_UPNAME_FIXED}_LIBRARY_RELEASE
    NAMES
    ${_RELEASE_LIBRARIES}
    HINTS ${PKG_${_NAME_FIXED}_LIBRARY_DIRS}
    PATHS
    ${_LIBRARY_PATHS}
    ${_UNIX_LIBRARY_PATHS}
    NO_CMAKE_SYSTEM_PATH
  )

  IF(_IS_VERBOSE)
    IF(${_UPNAME_FIXED}_LIBRARY_RELEASE)
      MESSAGE(STATUS "${NAME} release library found: ${${_UPNAME_FIXED}_LIBRARY_RELEASE}")
    ELSE()
      MESSAGE(STATUS "${NAME} release library not found in ${_LIBRARY_PATHS};${_UNIX_LIBRARY_PATHS}")
    ENDIF()
  ENDIF()

  # Search for debug library
  FIND_LIBRARY(${_UPNAME_FIXED}_LIBRARY_DEBUG
    NAMES
    ${_DEBUG_LIBRARIES}
    HINTS ${PKG_${_NAME_FIXED}_LIBRARY_DIRS}
    PATHS
    ${_LIBRARY_PATHS}
    ${_UNIX_LIBRARY_PATHS}
    NO_CMAKE_SYSTEM_PATH
  )

  IF(_IS_VERBOSE)
    IF(${_UPNAME_FIXED}_LIBRARY_DEBUG)
      MESSAGE(STATUS "${NAME} debug library found: ${${_UPNAME_FIXED}_LIBRARY_DEBUG}")
    ELSE()
      MESSAGE(STATUS "${NAME} debug library not found")
    ENDIF()
  ENDIF()

  # Library has been found if at least only one library and include are found
  IF(${_UPNAME_FIXED}_LIBRARY_RELEASE AND ${_UPNAME_FIXED}_LIBRARY_DEBUG)
    # Release and debug libraries found
    SET(${_UPNAME_FIXED}_LIBRARIES optimized ${${_UPNAME_FIXED}_LIBRARY_RELEASE} debug ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
  ELSEIF(${_UPNAME_FIXED}_LIBRARY_RELEASE)
    # Release library found
    SET(${_UPNAME_FIXED}_LIBRARIES ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_RELEASE})
  ELSEIF(${_UPNAME_FIXED}_LIBRARY_DEBUG)
    # Debug library found
    SET(${_UPNAME_FIXED}_LIBRARIES ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
    SET(${_UPNAME_FIXED}_LIBRARY ${${_UPNAME_FIXED}_LIBRARY_DEBUG})
  ENDIF()

  MARK_AS_ADVANCED(${_UPNAME_FIXED}_LIBRARY_RELEASE ${_UPNAME_FIXED}_LIBRARY_DEBUG)
ENDMACRO()

MACRO(FIND_LIBCURL)
  IF(NOT CURL_FOUND)
    FIND_PACKAGE(CURL REQUIRED)

    IF(WIN32 OR CURL_LIBRARY MATCHES "\\.a" OR WITH_STATIC_CURL)
      SET(CURL_STATIC ON)
    ELSE()
      SET(CURL_STATIC OFF)
    ENDIF()

    IF(CURL_STATIC)
      SET(CURL_DEFINITIONS -DCURL_STATICLIB)

      IF(UNIX)
        # CURL can depend on libidn
        FIND_LIBRARY(IDN_LIBRARY idn)
        IF(IDN_LIBRARY)
          LIST(APPEND CURL_LIBRARIES ${IDN_LIBRARY})
        ENDIF()

        # CURL Macports version can depend on libidn, libintl and libiconv too
        IF(APPLE)
          FIND_LIBRARY(INTL_LIBRARY intl)
          IF(INTL_LIBRARY)
            LIST(APPEND CURL_LIBRARIES ${INTL_LIBRARY})
          ENDIF()
        ELSE()
          # Only used by libcurl under Linux
          FIND_PACKAGE(OpenSSL REQUIRED)

          #IF(WIN32)
          #  SET(OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} Crypt32.lib)
          #ENDIF()

          # Only Linux version of libcurl depends on OpenSSL
          LIST(APPEND CURL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
          LIST(APPEND CURL_LIBRARIES ${OPENSSL_LIBRARIES})
        ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(FIND_LIBXML2)
  IF(NOT LIBXML2_FOUND)
    FIND_PACKAGE(LibXml2 REQUIRED)

    IF(WIN32 OR WITH_STATIC_LIBXML2)
      LIST(APPEND LIBXML2_DEFINITIONS -DLIBXML_STATIC)
    ENDIF()

    IF(WITH_LIBXML2_ICONV)
      FIND_PACKAGE(Iconv REQUIRED)
#      LIST(APPEND CURL_INCLUDE_DIRS ${ICONV_INCLUDE_DIR})
      LIST(APPEND LIBXML2_LIBRARIES ${ICONV_LIBRARIES})
    ENDIF()

    IF(WITH_STATIC)
      # libxml2 could need winsock2 library
      IF(WIN32)
        FIND_LIBRARY(WINSOCK2_LIB ws2_32)
      
        IF(WINSOCK2_LIB)
          LIST(APPEND LIBXML2_LIBRARIES ${WINSOCK2_LIB})
        ENDIF()

        FIND_LIBRARY(CRYPT32_LIB Crypt32)

        IF(CRYPT32_LIB)
          LIST(APPEND LIBXML2_LIBRARIES ${CRYPT32_LIB})
        ENDIF()
      ELSE()
        # under Linux and OS X, recent libxml2 versions are linked against liblzma
        FIND_PACKAGE(LibLZMA)

        IF(LIBLZMA_LIBRARIES)
          LIST(APPEND LIBXML2_LIBRARIES ${LIBLZMA_LIBRARIES})
        ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(ADD_QT_LIBRARY _NAME)
  IF(WIN32)
    SET(_PREFIX "Qt5")
    SET(_EXT "lib")
  ELSE()
    SET(_PREFIX "libQt5")
    SET(_EXT "a")
  ENDIF()
  SET(_LIB "${QT_LIBRARY_DIR}/${_PREFIX}${_NAME}.${_EXT}")
  IF(EXISTS ${_LIB})
    SET(QT_LIBRARIES ${QT_LIBRARIES} optimized ${_LIB})
  ENDIF()
  SET(_LIB "${QT_LIBRARY_DIR}/${_PREFIX}${_NAME}d.${_EXT}")
  IF(EXISTS ${_LIB})
    SET(QT_LIBRARIES ${QT_LIBRARIES} debug ${_LIB})
  ENDIF()
ENDMACRO()

MACRO(ADD_QT_PLUGIN _TYPE _NAME)
  IF(WIN32)
    SET(_PREFIX "")
    SET(_EXT "lib")
  ELSE()
    SET(_PREFIX "lib")
    SET(_EXT "a")
  ENDIF()
  SET(_LIB "${QT_PLUGINS_DIR}/${_TYPE}/${_PREFIX}${_NAME}.${_EXT}")
  IF(EXISTS ${_LIB})
    SET(QT_LIBRARIES ${QT_LIBRARIES} optimized ${_LIB})
  ENDIF()
  SET(_LIB "${QT_PLUGINS_DIR}/${_TYPE}/${_PREFIX}${_NAME}d.${_EXT}")
  IF(EXISTS ${_LIB})
    SET(QT_LIBRARIES ${QT_LIBRARIES} debug ${_LIB})
  ENDIF()
ENDMACRO()

MACRO(ADD_QT_SYSTEM_LIBRARY _NAME)
  # Save default suffixes
  SET(_OLD_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

  # Define specific suffixes
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})

  # Find the library with specified suffixes
  FIND_LIBRARY(${_NAME}_LIBRARY NAMES ${_NAME})

  # Restore default suffixes
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ${_OLD_SUFFIXES})

  IF(${_NAME}_LIBRARY)
    MESSAGE(STATUS "Found ${${_NAME}_LIBRARY} ${_NAME}")

    LIST(APPEND QT_LIBRARIES ${${_NAME}_LIBRARY})
  ELSE()
    MESSAGE(STATUS "Didn't find ${_NAME}")
  ENDIF()
ENDMACRO()

MACRO(FIND_QT5)
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)

  SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QTDIR} $ENV{QTDIR})

  FIND_PACKAGE(Qt5Core QUIET)

  IF(Qt5Core_FOUND)
    # Check if we are using Qt static or shared libraries
    GET_TARGET_PROPERTY(_FILE Qt5::Core IMPORTED_LOCATION_RELEASE)

    SET(QT_VERSION "${Qt5Core_VERSION_STRING}")
    SET(_VERSION "${QT_VERSION}")

    IF(_FILE MATCHES "\\.(lib|a)$")
      SET(QT_STATIC ON)
      SET(_VERSION "${_VERSION} static version")
    ELSE()
      SET(QT_STATIC OFF)
      SET(_VERSION "${_VERSION} shared version")
    ENDIF()

    MESSAGE(STATUS "Found Qt ${_VERSION}")

    # These variables are not defined with Qt5 CMake modules
    SET(QT_BINARY_DIR "${_qt5Core_install_prefix}/bin")
    SET(QT_LIBRARY_DIR "${_qt5Core_install_prefix}/lib")
    SET(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")
    SET(QT_TRANSLATIONS_DIR "${_qt5Core_install_prefix}/translations")

    # Fix wrong include directories with Qt 5 under Mac OS X
    INCLUDE_DIRECTORIES("${_qt5Core_install_prefix}/include")

    FIND_PACKAGE(Qt5Gui)
    FIND_PACKAGE(Qt5Widgets)
    FIND_PACKAGE(Qt5OpenGL)
    FIND_PACKAGE(Qt5Xml)
    FIND_PACKAGE(Qt5LinguistTools)
    FIND_PACKAGE(Qt5Network)

    IF(QT_STATIC)
      FIND_PACKAGE(PNG REQUIRED)
      FIND_PACKAGE(Jpeg REQUIRED)

      ADD_DEFINITIONS(-DQT_STATICPLUGIN)

      SET(QT_LIBRARIES Qt5::Widgets)

      # Gui
      SET(QT_LIBRARIES ${QT_LIBRARIES} Qt5::Gui Qt5::OpenGL)

      ADD_QT_LIBRARY(PrintSupport)

      IF(WIN32)
        SET(QT_LIBRARIES ${QT_LIBRARIES}
          ${WINSDK_LIBRARY_DIR}/Imm32.lib
          ${WINSDK_LIBRARY_DIR}/OpenGL32.lib
          ${WINSDK_LIBRARY_DIR}/WinMM.Lib)
        ADD_QT_PLUGIN(platforms qwindows)
        ADD_QT_LIBRARY(PlatformSupport)
      ELSEIF(APPLE)
        # Cups needs .dylib
        SET(OLD_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .dylib)
        FIND_LIBRARY(CUPS_LIBRARY cups)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ${OLD_CMAKE_FIND_LIBRARY_SUFFIXES})

        FIND_LIBRARY(IOKIT_FRAMEWORK IOKit)
        FIND_LIBRARY(COCOA_FRAMEWORK Cocoa)
        FIND_LIBRARY(SYSTEMCONFIGURATION_FRAMEWORK SystemConfiguration)
        FIND_LIBRARY(OPENGL_FRAMEWORK NAMES OpenGL)

        SET(QT_LIBRARIES ${QT_LIBRARIES}
          ${CUPS_LIBRARY}
          ${COCOA_FRAMEWORK}
          ${SYSTEMCONFIGURATION_FRAMEWORK}
          ${IOKIT_FRAMEWORK}
          ${OPENGL_FRAMEWORK})

        ADD_QT_PLUGIN(printsupport cocoaprintersupport)
        ADD_QT_PLUGIN(platforms qcocoa)
        ADD_QT_LIBRARY(PlatformSupport)
      ELSE()
        # order is very important there
        ADD_QT_PLUGIN(platforms qxcb)
        ADD_QT_PLUGIN(xcbglintegrations qxcb-glx-integration)

        ADD_QT_LIBRARY(XcbQpa)
        ADD_QT_LIBRARY(GlxSupport)
        ADD_QT_LIBRARY(ServiceSupport)
        ADD_QT_LIBRARY(EdidSupport)
        ADD_QT_LIBRARY(FontDatabaseSupport)
        ADD_QT_LIBRARY(ThemeSupport)
        ADD_QT_LIBRARY(EventDispatcherSupport)
        ADD_QT_LIBRARY(PlatformSupport)

        ADD_QT_LIBRARY(DBus)

        IF(EXISTS "${QT_LIBRARY_DIR}/libxcb-static.a")
          SET(QT_LIBRARIES ${QT_LIBRARIES} "${QT_LIBRARY_DIR}/libxcb-static.a")
        ENDIF()

        # always link these in dynamic, API never changes
        ADD_QT_SYSTEM_LIBRARY(X11)
        ADD_QT_SYSTEM_LIBRARY(Xmu)
        ADD_QT_SYSTEM_LIBRARY(X11-xcb)
        ADD_QT_SYSTEM_LIBRARY(Xi)
        ADD_QT_SYSTEM_LIBRARY(SM)
        ADD_QT_SYSTEM_LIBRARY(ICE)
        ADD_QT_SYSTEM_LIBRARY(xcb)
        ADD_QT_SYSTEM_LIBRARY(GL)
        ADD_QT_SYSTEM_LIBRARY(xcb-glx)
        ADD_QT_SYSTEM_LIBRARY(fontconfig)
        ADD_QT_SYSTEM_LIBRARY(Xrender)
      ENDIF()

      ADD_QT_PLUGIN(imageformats qgif)
      ADD_QT_PLUGIN(imageformats qicns)
      ADD_QT_PLUGIN(imageformats qico)
      ADD_QT_PLUGIN(imageformats qjpeg)

      # harfbuzz is needed since Qt 5.3
      IF(UNIX)
        SET(HB_LIB "${QT_LIBRARY_DIR}/libqtharfbuzzng.a")
        IF(NOT EXISTS ${HB_LIB})
          SET(HB_LIB "${QT_LIBRARY_DIR}/libqtharfbuzz.a")
        ENDIF()
      ELSEIF(WIN32)
        SET(HB_LIB "${QT_LIBRARY_DIR}/qtharfbuzzng.lib")
      ENDIF()
      IF(EXISTS ${HB_LIB})
        SET(QT_LIBRARIES ${QT_LIBRARIES} ${HB_LIB})
      ENDIF()

      # freetype is needed since Qt 5.5
      FIND_PACKAGE(Freetype)

      IF(FREETYPE_FOUND)
        SET(QT_LIBRARIES ${QT_LIBRARIES} ${FREETYPE_LIBRARIES})
      ELSE()
        IF(UNIX)
          SET(FREETYPE_LIB "${QT_LIBRARY_DIR}/libqtfreetype.a")
        ELSEIF(WIN32)
          SET(FREETYPE_LIB "${QT_LIBRARY_DIR}/qtfreetype.lib")
        ENDIF()
        IF(EXISTS ${FREETYPE_LIB})
          SET(QT_LIBRARIES ${QT_LIBRARIES} ${FREETYPE_LIB})
        ENDIF()
      ENDIF()

      ADD_QT_PLUGIN(accessible qtaccessiblewidgets)

      SET(QT_LIBRARIES ${QT_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARY})

      # Network
      SET(QT_LIBRARIES ${QT_LIBRARIES} Qt5::Network Qt5::Xml)
      SET(QT_LIBRARIES ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

      IF(WIN32)
        SET(QT_LIBRARIES ${QT_LIBRARIES}
          ${WINSDK_LIBRARY_DIR}/Crypt32.lib
          ${WINSDK_LIBRARY_DIR}/WS2_32.Lib
          ${WINSDK_LIBRARY_DIR}/IPHlpApi.Lib)
      ENDIF()

      # Core
      SET(QT_LIBRARIES ${QT_LIBRARIES} Qt5::Core)

      # pcre is needed since Qt 5.5
      IF(UNIX)
        SET(PCRE_LIB "${QT_LIBRARY_DIR}/libqtpcre.a")
        IF(NOT EXISTS ${PCRE_LIB})
          SET(PCRE_LIB "${QT_LIBRARY_DIR}/libqtpcre2.a")
        ENDIF()
      ELSEIF(WIN32)
        SET(PCRE_LIB "${QT_LIBRARY_DIR}/qtpcre.lib")
      ENDIF()
      IF(EXISTS ${PCRE_LIB})
        SET(QT_LIBRARIES ${QT_LIBRARIES} ${PCRE_LIB})
      ENDIF()

      IF(APPLE)
        FIND_LIBRARY(PCRE_LIBRARY pcre16 pcre)

        FIND_LIBRARY(SECURITY_FRAMEWORK Security)

        SET(QT_LIBRARIES ${QT_LIBRARIES}
          ${PCRE_LIBRARY}
          ${FOUNDATION_FRAMEWORK}
          ${CARBON_FRAMEWORK}
          ${SECURITY_FRAMEWORK})
      ELSEIF(UNIX)
        SET(QT_LIBRARIES ${QT_LIBRARIES} ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} -ldl -lrt)
      ENDIF()
    ELSE()
      SET(QT_LIBRARIES Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Gui Qt5::OpenGL Qt5::Core)
    ENDIF()
  ELSE()
    MESSAGE(WARNING "Unable to find Qt 5")
  ENDIF()
ENDMACRO()
