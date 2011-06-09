# - Locate MFC libraries
# This module defines
#  MFC_FOUND, if false, do not try to link to MFC
#  MFC_LIBRARY_DIR, where to find libraries
#  MFC_INCLUDE_DIR, where to find headers

# Try to find MFC using official module, MFC_FOUND is set
FIND_PACKAGE(MFC)

SET(CUSTOM_MFC_DIR FALSE)

# If using STLport and MFC have been found, remember its directory
IF(WITH_STLPORT AND MFC_FOUND AND VC_DIR)
  SET(MFC_STANDARD_DIR "${VC_DIR}/atlmfc")
ENDIF(WITH_STLPORT AND MFC_FOUND AND VC_DIR)

# If using STLport or MFC haven't been found, search for afxwin.h
IF(WITH_STLPORT OR NOT MFC_FOUND)
  FIND_PATH(MFC_DIR
    include/afxwin.h
    PATHS
    ${MFC_STANDARD_DIR}
  )

  IF(CustomMFC_FIND_REQUIRED)
    SET(MFC_FIND_REQUIRED TRUE)
  ENDIF(CustomMFC_FIND_REQUIRED)

  # Display an error message if MFC are not found, MFC_FOUND is updated
  # User will be able to update MFC_DIR to the correct directory
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(MFC DEFAULT_MSG MFC_DIR)

  IF(MFC_FOUND)
    SET(CUSTOM_MFC_DIR TRUE)
    SET(MFC_INCLUDE_DIR "${MFC_DIR}/include")
    INCLUDE_DIRECTORIES(${MFC_INCLUDE_DIR})
  ENDIF(MFC_FOUND)
ENDIF(WITH_STLPORT OR NOT MFC_FOUND)

# Only if using a custom path
IF(CUSTOM_MFC_DIR)
  # Using 32 or 64 bits libraries
  IF(TARGET_X64)
    SET(MFC_LIBRARY_DIR "${MFC_DIR}/lib/amd64")
  ELSE(TARGET_X64)
    SET(MFC_LIBRARY_DIR "${MFC_DIR}/lib")
  ENDIF(TARGET_X64)

  # Add MFC libraries directory to default library path
  LINK_DIRECTORIES(${MFC_LIBRARY_DIR})
ENDIF(CUSTOM_MFC_DIR)

IF(MFC_FOUND)
  # Set definitions for using MFC in DLL
  SET(MFC_DEFINITIONS -D_AFXDLL)
ENDIF(MFC_FOUND)

# TODO: create a macro which set MFC_DEFINITIONS, MFC_LIBRARY_DIR and MFC_INCLUDE_DIR for a project
