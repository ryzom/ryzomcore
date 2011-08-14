# - Find DirectInput
# Find the DirectSound includes and libraries
#
#  MAXSDK_INCLUDE_DIR - where to find baseinterface.h
#  MAXSDK_LIBRARIES   - List of libraries when using 3DSMAX.
#  MAXSDK_FOUND       - True if MAX SDK found.

if(MAXSDK_INCLUDE_DIR)
    # Already in cache, be silent
    set(MAXSDK_FIND_QUIETLY TRUE)
endif(MAXSDK_INCLUDE_DIR)

find_path(MAXSDK_INCLUDE_DIR max.h 
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_2012}/maxsdk/include"
  "$ENV{3DSMAX_2011_SDK_PATH}/maxsdk/include"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2010 SDK/maxsdk/include"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2009 SDK/maxsdk/include"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2008 SDK/maxsdk/include"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 9 SDK/maxsdk/include"
) 

find_path(MAXSDK_CS_INCLUDE_DIR bipexp.h
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_2012}/maxsdk/include/CS"
  "$ENV{3DSMAX_2011_SDK_PATH}/maxsdk/include/CS"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2010 SDK/maxsdk/include/CS"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2009 SDK/maxsdk/include/CS"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2008 SDK/maxsdk/include/CS"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max 9 SDK/maxsdk/include/CS"
)

MACRO(FIND_3DS_LIBRARY MYLIBRARY MYLIBRARYNAME)          
  FIND_LIBRARY(${MYLIBRARY}
               NAMES ${MYLIBRARYNAME}
               PATHS 
			   "$ENV{ADSK_3DSMAX_SDK_2012}/maxsdk/lib"
			   "$ENV{3DSMAX_2011_SDK_PATH}/maxsdk/lib"
               "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2010 SDK/maxsdk/lib"
               "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2009 SDK/maxsdk/lib"
               "$ENV{PROGRAMFILES}/Autodesk/3ds Max 2008 SDK/maxsdk/lib"
               "$ENV{PROGRAMFILES}/Autodesk/3ds Max 9 SDK/maxsdk/lib"
               )
ENDMACRO(FIND_3DS_LIBRARY MYLIBRARY MYLIBRARYNAME)

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
      ${MAXSDK_BMM_LIBRARY} )

else(MAXSDK_FOUND)
    set(MAXSDK_LIBRARIES)
endif(MAXSDK_FOUND)

mark_as_advanced(MAXSDK_INCLUDE_DIR MAXSDK_LIBRARY)
