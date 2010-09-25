# - Find Windows Platform SDK
# Find the Windows includes
#
#  WINSDK_INCLUDE_DIR - where to find Windows.h
#  WINSDK_FOUND       - True if Windows SDK found.

IF(WINSDK_INCLUDE_DIR)
    # Already in cache, be silent
    SET(WINSDK_FIND_QUIETLY TRUE)
ENDIF(WINSDK_INCLUDE_DIR)

FIND_PATH(WINSDK_INCLUDE_DIR Windows.h
  PATHS
  "[HKEY_CURRENT_USER\\Software\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]/Include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]/Include"
)

IF(WINSDK_INCLUDE_DIR)
  SET(WINSDK_FOUND TRUE)
  IF(NOT WINSDK_FIND_QUIETLY)
    MESSAGE(STATUS "Found Windows SDK.")
  ENDIF(NOT WINSDK_FIND_QUIETLY)
ELSE(WINSDK_INCLUDE_DIR)
  IF(NOT WINSDK_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Windows SDK!")
  ENDIF(NOT WINSDK_FIND_QUIETLY)
ENDIF(WINSDK_INCLUDE_DIR)
