# - Locate MFC libraries
# This module defines
#  MFC_FOUND, if false, do not try to link to MFC
#  MFC_LIBRARY_DIR, where to find libraries
#  MFC_INCLUDE_DIR, where to find headers

IF(CustomMFC_FIND_REQUIRED)
  SET(MFC_FIND_REQUIRED TRUE)
ENDIF(CustomMFC_FIND_REQUIRED)

# Try to find MFC using official module, MFC_FOUND is set
FIND_PACKAGE(MFC)

IF(NOT MFC_DIR)
  # If MFC have been found, remember their directory
  IF(MFC_FOUND AND VC_DIR)
    SET(MFC_STANDARD_DIR "${VC_DIR}/atlmfc")
  ENDIF(MFC_FOUND AND VC_DIR)

  FIND_PATH(MFC_DIR
    include/afxwin.h
    HINTS
    ${MFC_STANDARD_DIR}
  )
ENDIF(NOT MFC_DIR)

# Display an error message if MFC are not found, MFC_FOUND is updated
# User will be able to update MFC_DIR to the correct directory
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MFC DEFAULT_MSG MFC_DIR)

IF(MFC_FOUND)
  SET(MFC_INCLUDE_DIR "${MFC_DIR}/include")
  INCLUDE_DIRECTORIES(${MFC_INCLUDE_DIR})

  # Using 32 or 64 bits libraries
  IF(TARGET_X64)
    SET(MFC_LIBRARY_DIR "${MFC_DIR}/lib/amd64")
  ELSE(TARGET_X64)
    SET(MFC_LIBRARY_DIR "${MFC_DIR}/lib")
  ENDIF(TARGET_X64)

  # Add MFC libraries directory to default library path
  LINK_DIRECTORIES(${MFC_LIBRARY_DIR})

  # Set definitions for using MFC in DLL
  SET(MFC_DEFINITIONS -D_AFXDLL)
ENDIF(MFC_FOUND)

# TODO: create a macro which set MFC_DEFINITIONS, MFC_LIBRARY_DIR and MFC_INCLUDE_DIR for a project
