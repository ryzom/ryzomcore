# - Find Windows Platform SDK
# Find the Windows includes
#
#  WINSDK_INCLUDE_DIR - where to find Windows.h
#  WINSDK_INCLUDE_DIRS - where to find all Windows headers
#  WINSDK_LIBRARY_DIR - where to find libraries
#  WINSDK_FOUND       - True if Windows SDK found.

IF(WINSDK_FOUND)
  # If Windows SDK already found, skip it
  RETURN()
ENDIF()

# Values can be CURRENT or any existing versions 7.1, 8.0A, etc...
SET(WINSDK_VERSION "CURRENT" CACHE STRING "Windows SDK version to prefer")

MACRO(DETECT_WINSDK_VERSION_HELPER _ROOT _VERSION)
  GET_FILENAME_COMPONENT(WINSDK${_VERSION}_DIR "[${_ROOT}\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v${_VERSION};InstallationFolder]" ABSOLUTE)

  IF(WINSDK${_VERSION}_DIR AND NOT WINSDK${_VERSION}_DIR STREQUAL "/registry")
    SET(WINSDK${_VERSION}_FOUND ON)
    GET_FILENAME_COMPONENT(WINSDK${_VERSION}_VERSION_FULL "[${_ROOT}\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v${_VERSION};ProductVersion]" NAME)
    IF(NOT WindowsSDK_FIND_QUIETLY)
      MESSAGE(STATUS "Found Windows SDK ${_VERSION} in ${WINSDK${_VERSION}_DIR}")
    ENDIF(NOT WindowsSDK_FIND_QUIETLY)
  ELSE(WINSDK${_VERSION}_DIR AND NOT WINSDK${_VERSION}_DIR STREQUAL "/registry")
    SET(WINSDK${_VERSION}_DIR "")
  ENDIF()
ENDMACRO()

MACRO(DETECT_WINSDK_VERSION _VERSION)
  SET(WINSDK${_VERSION}_FOUND OFF)
  DETECT_WINSDK_VERSION_HELPER("HKEY_CURRENT_USER" ${_VERSION})

  IF(NOT WINSDK${_VERSION}_FOUND)
    DETECT_WINSDK_VERSION_HELPER("HKEY_LOCAL_MACHINE" ${_VERSION})
  ENDIF()
ENDMACRO()

SET(WINSDK_VERSIONS "8.1" "8.0" "7.1" "7.1A" "7.0" "7.0A" "6.1" "6.0" "6.0A")
SET(WINSDK_DETECTED_VERSIONS)

# Search all supported Windows SDKs
FOREACH(_VERSION ${WINSDK_VERSIONS})
  DETECT_WINSDK_VERSION(${_VERSION})

  IF(WINSDK${_VERSION}_FOUND)
    LIST(APPEND WINSDK_DETECTED_VERSIONS ${_VERSION})
  ENDIF()
ENDFOREACH()

SET(WINSDK_SUFFIX)

IF(TARGET_ARM)
  SET(WINSDK8_SUFFIX "arm")
ELSEIF(TARGET_X64)
  SET(WINSDK8_SUFFIX "x64")
  SET(WINSDK_SUFFIX "x64")
ELSEIF(TARGET_X86)
  SET(WINSDK8_SUFFIX "x86")
ENDIF()

SET(WINSDKCURRENT_VERSION_INCLUDE $ENV{INCLUDE})

IF(WINSDKCURRENT_VERSION_INCLUDE)
  FILE(TO_CMAKE_PATH "${WINSDKCURRENT_VERSION_INCLUDE}" WINSDKCURRENT_VERSION_INCLUDE)
ENDIF()

SET(WINSDKENV_DIR $ENV{WINSDK_DIR})

IF(NOT WINSDKENV_DIR)
  SET(WINSDKENV_DIR $ENV{WindowsSDKDir})
ENDIF()

MACRO(FIND_WINSDK_VERSION_HEADERS)
  IF(WINSDK_DIR AND NOT WINSDK_VERSION)
    # Search version in headers
    FIND_FILE(_MSI_FILE Msi.h
      PATHS
      ${WINSDK_DIR}/Include/um
      ${WINSDK_DIR}/Include
    )

    IF(_MSI_FILE)
      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 8.1
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WINBLUE")

        IF(_CONTENT)
          SET(WINSDK_VERSION "8.1")
        ENDIF()
      ENDIF()
      
      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 8.0
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WIN8")

        IF(_CONTENT)
          SET(WINSDK_VERSION "8.0")
        ENDIF()
      ENDIF()
      
      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 7.0
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WIN7")
        
        IF(_CONTENT)
          FIND_FILE(_WINSDKVER_FILE winsdkver.h WinSDKVer.h
            PATHS
            ${WINSDK_DIR}/Include/um
            ${WINSDK_DIR}/Include
          )

          IF(_WINSDKVER_FILE)
            # Load WinSDKVer.h content
            FILE(STRINGS ${_WINSDKVER_FILE} _CONTENT REGEX "^#define NTDDI_MAXVER")

            # Get NTDDI_MAXVER value
            STRING(REGEX REPLACE "^.*0x([0-9A-Fa-f]+).*$" "\\1" _WINSDKVER "${_CONTENT}")

            # In Windows SDK 7.1, NTDDI_MAXVER is wrong
            IF(_WINSDKVER STREQUAL "06010000")
              SET(WINSDK_VERSION "7.1")
            ELSEIF(_WINSDKVER STREQUAL "0601")
              SET(WINSDK_VERSION "7.0A")
            ELSE()
              MESSAGE(FATAL_ERROR "Can't determine Windows SDK version with NTDDI_MAXVER 0x${_WINSDKVER}")
            ENDIF()
          ELSE()
            SET(WINSDK_VERSION "7.0")
          ENDIF()
        ENDIF()
      ENDIF()

      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 6.0
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_VISTA")
        
        IF(_CONTENT)
          SET(WINSDK_VERSION "6.0")
        ENDIF()
      ENDIF()

      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 5.2
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WS03SP1")

        IF(_CONTENT)
          SET(WINSDK_VERSION "5.2")
        ENDIF()
      ENDIF()

      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 5.1
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WINXP")

        IF(_CONTENT)
          SET(WINSDK_VERSION "5.1")
        ENDIF()
      ENDIF()

      IF(NOT WINSDK_VERSION)
        # Look for Windows SDK 5.0
        FILE(STRINGS ${_MSI_FILE} _CONTENT REGEX "^#ifndef NTDDI_WIN2K")

        IF(_CONTENT)
          SET(WINSDK_VERSION "5.0")
        ENDIF()
      ENDIF()
    ELSE()
      MESSAGE(FATAL_ERROR "Unable to find Msi.h in ${WINSDK_DIR}")
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(USE_CURRENT_WINSDK)
  SET(WINSDK_DIR "")
  SET(WINSDK_VERSION "")
  SET(WINSDK_VERSION_FULL "")

  # Use WINSDK environment variable
  IF(WINSDKENV_DIR)
    FIND_PATH(WINSDK_DIR Windows.h
      HINTS
      ${WINSDKENV_DIR}/Include/um
      ${WINSDKENV_DIR}/Include
    )
  ENDIF()

  # Use INCLUDE environment variable
  IF(NOT WINSDK_DIR AND WINSDKCURRENT_VERSION_INCLUDE)
    FOREACH(_INCLUDE ${WINSDKCURRENT_VERSION_INCLUDE})
      FILE(TO_CMAKE_PATH ${_INCLUDE} _INCLUDE)

      # Look for Windows.h because there are several paths
      IF(EXISTS ${_INCLUDE}/Windows.h)
        STRING(REGEX REPLACE "/(include|INCLUDE|Include)(.*)" "" WINSDK_DIR ${_INCLUDE})
        MESSAGE(STATUS "Found Windows SDK in INCLUDE environment variable: ${WINSDK_DIR}")
        BREAK()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  IF(WINSDK_DIR)
    # Compare WINSDK_DIR with registered Windows SDKs
    FOREACH(_VERSION ${WINSDK_DETECTED_VERSIONS})
      IF(WINSDK_DIR STREQUAL "${WINSDK${_VERSION}_DIR}")
        SET(WINSDK_VERSION ${_VERSION})
        SET(WINSDK_VERSION_FULL "${WINSDK${WINSDK_VERSION}_VERSION_FULL}")
        BREAK()
      ENDIF()
    ENDFOREACH()

    FIND_WINSDK_VERSION_HEADERS()
  ENDIF()

  IF(NOT WINSDK_DIR)
    # Use Windows SDK versions installed with VC++ when possible
    IF(MSVC12)
      SET(WINSDK_VERSION "8.1")
    ELSEIF(MSVC11)
      SET(WINSDK_VERSION "8.0")
    ELSEIF(MSVC10)
      IF(NOT TARGET_X64 OR NOT MSVC_EXPRESS)
        SET(WINSDK_VERSION "7.0A")
      ENDIF()
    ELSEIF(MSVC90)
      IF(NOT MSVC_EXPRESS)
        SET(WINSDK_VERSION "6.0A")
      ENDIF()
    ELSEIF(MSVC80)
      IF(NOT MSVC_EXPRESS)
        # TODO: fix this version
        SET(WINSDK_VERSION "5.2A")
      ENDIF()
    ELSE()
      MESSAGE(FATAL_ERROR "Your compiler is either too old or too recent, please update this CMake module.")
    ENDIF()

    # Use installed Windows SDK
    IF(NOT WINSDK_VERSION)
      IF(WINSDK8.1_FOUND)
        SET(WINSDK_VERSION "8.1")
      ELSEIF(WINSDK8.0_FOUND)
        SET(WINSDK_VERSION "8.0")
      ELSEIF(WINSDK7.1_FOUND)
        SET(WINSDK_VERSION "7.1")
      ELSEIF(WINSDK7.0_FOUND)
        SET(WINSDK_VERSION "7.0")
      ELSEIF(WINSDK6.1_FOUND)
        SET(WINSDK_VERSION "6.1")
      ELSEIF(WINSDK6.0_FOUND)
        SET(WINSDK_VERSION "6.0")
      ELSE()
        MESSAGE(FATAL_ERROR "You have no compatible Windows SDK installed.")
      ENDIF()
    ENDIF()

    # Look for correct registered Windows SDK version
    FOREACH(_VERSION ${WINSDK_DETECTED_VERSIONS})
      IF(WINSDK_VERSION STREQUAL _VERSION)
        SET(WINSDK_VERSION_FULL "${WINSDK${WINSDK_VERSION}_VERSION_FULL}")
        SET(WINSDK_DIR "${WINSDK${WINSDK_VERSION}_DIR}")
        BREAK()
      ENDIF()
    ENDFOREACH()
  ENDIF()
ENDMACRO()

IF(WINSDK_VERSION STREQUAL "CURRENT")
  USE_CURRENT_WINSDK()
ELSE()
  IF(WINSDK${WINSDK_VERSION}_FOUND)
    SET(WINSDK_VERSION_FULL "${WINSDK${WINSDK_VERSION}_VERSION_FULL}")
    SET(WINSDK_DIR "${WINSDK${WINSDK_VERSION}_DIR}")
  ELSE()
    USE_CURRENT_WINSDK()
  ENDIF()
ENDIF()

IF(WINSDK_DIR)
  MESSAGE(STATUS "Using Windows SDK ${WINSDK_VERSION}")
ELSE()
  MESSAGE(FATAL_ERROR "Unable to find Windows SDK!")
ENDIF()

# directory where Win32 headers are found
FIND_PATH(WINSDK_INCLUDE_DIR Windows.h
  HINTS
  ${WINSDK_DIR}/Include/um
  ${WINSDK_DIR}/Include
)

# directory where DirectX headers are found
FIND_PATH(WINSDK_SHARED_INCLUDE_DIR d3d9.h
  HINTS
  ${WINSDK_DIR}/Include/shared
  ${WINSDK_DIR}/Include
)

# directory where OpenGL headers are found
FIND_PATH(WINSDK_OPENGL_INCLUDE_DIR GL.h
  HINTS
  ${WINSDK_DIR}/Include/um/gl
  ${WINSDK_DIR}/Include/gl
  ${WINSDK_DIR}/Include
)

# directory where all libraries are found
FIND_PATH(WINSDK_LIBRARY_DIR ComCtl32.lib
  HINTS
  ${WINSDK_DIR}/Lib/winv6.3/um/${WINSDK8_SUFFIX}
  ${WINSDK_DIR}/Lib/win8/um/${WINSDK8_SUFFIX}
  ${WINSDK_DIR}/Lib/${WINSDK_SUFFIX}
)

# signtool is used to sign executables
FIND_PROGRAM(WINSDK_SIGNTOOL signtool
  HINTS
  ${WINSDK_DIR}/Bin/${WINSDK8_SUFFIX}
  ${WINSDK_DIR}/Bin/x86
  ${WINSDK_DIR}/Bin
)

# midl is used to generate IDL interfaces
FIND_PROGRAM(WINSDK_MIDL midl
  HINTS
  ${WINSDK_DIR}/Bin/${WINSDK8_SUFFIX}
  ${WINSDK_DIR}/Bin/x86
  ${WINSDK_DIR}/Bin
)

IF(WINSDK_INCLUDE_DIR)
  SET(WINSDK_FOUND ON)
  SET(WINSDK_INCLUDE_DIRS ${WINSDK_INCLUDE_DIR} ${WINSDK_SHARED_INCLUDE_DIR} ${WINSDK_OPENGL_INCLUDE_DIR})
  SET(CMAKE_LIBRARY_PATH ${WINSDK_LIBRARY_DIR} ${CMAKE_LIBRARY_PATH})
  INCLUDE_DIRECTORIES(${WINSDK_INCLUDE_DIRS}) # TODO: Move this after all other includes somehow...

  # Fix for using Windows SDK 7.1 with Visual C++ 2012
  IF(WINSDK_VERSION STREQUAL "7.1" AND MSVC11)
    ADD_DEFINITIONS(-D_USING_V110_SDK71_)
  ENDIF()
ELSE()
  IF(NOT WindowsSDK_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Windows SDK!")
  ENDIF()
ENDIF()
