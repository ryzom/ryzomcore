# - Find DirectX
# Find the DirectX includes and libraries
#
#  DXSDK_INCLUDE_DIR - where to find baseinterface.h
#  DXSDK_LIBRARIES   - List of libraries when using 3DSMAX.
#  DXSDK_FOUND       - True if MAX SDK found.

if(DXSDK_INCLUDE_DIR)
    # Already in cache, be silent
    set(DXSDK_FIND_QUIETLY TRUE)
endif(DXSDK_INCLUDE_DIR)

find_path(DXSDK_INCLUDE_DIR dxsdkver.h 
  PATHS
  "$ENV{DXSDK_DIR}" 
  "$ENV{DXSDK_DIR}/Include"
)

MACRO(FIND_DXSDK_LIBRARY MYLIBRARY MYLIBRARYNAME)        
  FIND_LIBRARY(${MYLIBRARY}
               NAMES ${MYLIBRARYNAME}
               PATHS 
               "$ENV{DXSDK_DIR}"
               "$ENV{DXSDK_DIR}/Lib"
               "$ENV{DXSDK_DIR}/Lib/x86"
               )               
ENDMACRO(FIND_DXSDK_LIBRARY MYLIBRARY MYLIBRARYNAME)

FIND_DXSDK_LIBRARY(DXSDK_GUID_LIBRARY dxguid)
FIND_DXSDK_LIBRARY(DXSDK_DINPUT_LIBRARY dinput8)
FIND_DXSDK_LIBRARY(DXSDK_DSOUND_LIBRARY dsound)
FIND_DXSDK_LIBRARY(DXSDK_XAUDIO_LIBRARY x3daudio)
FIND_DXSDK_LIBRARY(DXSDK_D3DX9_LIBRARY d3dx9)
FIND_DXSDK_LIBRARY(DXSDK_D3D9_LIBRARY d3d9)

#FIND_DXSDK_LIBRARY(DXSDK_MESH_LIBRARY mesh)
#FIND_DXSDK_LIBRARY(DXSDK_MAXUTIL_LIBRARY maxutil)
#FIND_DXSDK_LIBRARY(DXSDK_MAXSCRIPT_LIBRARY maxscrpt)
#FIND_DXSDK_LIBRARY(DXSDK_PARAMBLK2_LIBRARY paramblk2)
#FIND_DXSDK_LIBRARY(DXSDK_BMM_LIBRARY bmm)

# Handle the QUIETLY and REQUIRED arguments and set DXSDK_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DirectXSDK DEFAULT_MSG
    DXSDK_INCLUDE_DIR DXSDK_GUID_LIBRARY)

if(DirectXSDK_FOUND)
message(status " directx found.")
    SET(DXSDK_LIBRARIES
      ${DXSDK_GUID_LIBRARY}
      ${DXSDK_DINPUT_LIBRARY}
      ${DXSDK_DSOUND_LIBRARY}
      ${DXSDK_D3DX9_LIBRARY}
      ${DXSDK_D3D9_LIBRARY})

else(DirectXSDK_FOUND)
    set(DXSDK_LIBRARIES)
endif(DirectXSDK_FOUND)

mark_as_advanced(DXSDK_INCLUDE_DIR DXSDK_LIBRARIES)
