# - Find DirectInput
# Find the DirectSound includes and libraries
#
#  MAXSDK_DIR         - 3DSMAX SDK root directory
#  MAXSDK_INCLUDE_DIR - where to find baseinterface.h
#  MAXSDK_LIBRARIES   - List of libraries when using 3DSMAX.
#  MAXSDK_FOUND       - True if MAX SDK found.

if(MAXSDK_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MAXSDK_FIND_QUIETLY TRUE)
endif()

set(_pf_x86 "PROGRAMFILES(x86)")

FIND_PATH(MAXSDK_DIR
  "include/maxversion.h"
  HINTS
  "$ENV{MAXSDK_DIR}"
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_2021}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2020}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2019}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2018}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2017}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2016}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2015}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2014}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2013}/maxsdk"
  "$ENV{ADSK_3DSMAX_SDK_2012}/maxsdk"
  "$ENV{3DSMAX_2011_SDK_PATH}/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2021 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2020 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2019 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2018 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2017 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2016 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2015 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2014 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2013 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2012 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2011 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2010 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2009 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 2008 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3ds Max 9 SDK/maxsdk"
  "$ENV{${_pf_x86}}/Autodesk/3dsMax8/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2021 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2020 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2019 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2018 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2017 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2016 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2015 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2014 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2013 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2012 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2011 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2010 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2009 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2008 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 9 SDK/maxsdk"
  "$ENV{PROGRAMFILES}/Autodesk/3dsMax8/maxsdk"
)

FIND_PATH(MAXSDK_INCLUDE_DIR
  max.h
  HINTS
  ${MAXSDK_DIR}/include
)

FIND_PATH(MAXSDK_CS_INCLUDE_DIR bipexp.h
  HINTS
  ${MAXSDK_DIR}/include/CS
)

IF(TARGET_X64)
  SET(MAXSDK_LIBRARY_DIRS ${MAXSDK_DIR}/x64/lib ${MAXSDK_DIR}/lib/x64/Release)
ELSE()
  SET(MAXSDK_LIBRARY_DIRS ${MAXSDK_DIR}/lib)
ENDIF()

MACRO(FIND_3DS_LIBRARY MYLIBRARY MYLIBRARYNAME)
  FIND_LIBRARY(${MYLIBRARY}
    NAMES ${MYLIBRARYNAME}
    HINTS
    ${MAXSDK_LIBRARY_DIRS}
  )
ENDMACRO()

FIND_3DS_LIBRARY(MAXSDK_CORE_LIBRARY core)
FIND_3DS_LIBRARY(MAXSDK_GEOM_LIBRARY geom)
FIND_3DS_LIBRARY(MAXSDK_GFX_LIBRARY gfx)
FIND_3DS_LIBRARY(MAXSDK_MESH_LIBRARY mesh)
FIND_3DS_LIBRARY(MAXSDK_MAXUTIL_LIBRARY maxutil)
FIND_3DS_LIBRARY(MAXSDK_MAXSCRIPT_LIBRARY maxscrpt)
FIND_3DS_LIBRARY(MAXSDK_PARAMBLK2_LIBRARY paramblk2)
FIND_3DS_LIBRARY(MAXSDK_BMM_LIBRARY bmm)

# Handle the QUIETLY and REQUIRED arguments and set MAXSDK_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MAXSDK DEFAULT_MSG
    MAXSDK_INCLUDE_DIR MAXSDK_CORE_LIBRARY)

if(MAXSDK_FOUND)
    SET(MAXSDK_LIBRARIES
      ${MAXSDK_CORE_LIBRARY}
      ${MAXSDK_GEOM_LIBRARY}
      ${MAXSDK_GFX_LIBRARY}
      ${MAXSDK_MESH_LIBRARY}
      ${MAXSDK_MAXUTIL_LIBRARY}
      ${MAXSDK_MAXSCRIPT_LIBRARY}
      ${MAXSDK_PARAMBLK2_LIBRARY}
      ${MAXSDK_BMM_LIBRARY})

  # parse maxversion.h to determine SDK version
  IF(EXISTS "${MAXSDK_DIR}/include/maxversion.h")
    FILE(STRINGS "${MAXSDK_DIR}/include/maxversion.h" LINES REGEX "#define MAX_PRODUCT_YEAR_NUMBER ([0-9]+)")

    STRING(REGEX REPLACE ".+MAX_PRODUCT_YEAR_NUMBER ([0-9]+)" "\\1" MAXSDK_VERSION "${LINES}")
    UNSET(LINES)
  ELSE()
    SET(MAXSDK_VERSION "Unknown")
  ENDIF()

  MESSAGE(STATUS "Found 3dsmax version ${MAXSDK_VERSION} in ${MAXSDK_DIR}")

  # 3ds Max 2013 and later are always Unicode
  IF(MAXSDK_VERSION VERSION_GREATER 2012)
    SET(MAXSDK_DEFINITIONS -DUNICODE -D_UNICODE)
  ELSE()
    SET(MAXSDK_DEFINITIONS)
  ENDIF()
ELSE()
    set(MAXSDK_LIBRARIES)
ENDIF()

mark_as_advanced(MAXSDK_INCLUDE_DIR MAXSDK_LIBRARY)
