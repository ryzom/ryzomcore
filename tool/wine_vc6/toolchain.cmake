# CMake toolchain file for building ryzomcore with Visual C++ 6.0 (SP5 + Processor Pack) x86 under Wine,
# against STLport 5.2.1, with the toolchain_vc6_prefix wine prefix package. The VC6 counterpart of
# tool/wine_vs2008/toolchain.cmake. Environment:
#   NL_WINE_VC6_PREFIX    extracted prefix                     (default $HOME/toolchain_vc6_prefix)
#   NL_WINE_VC6_SHADOW    shadow VC dir: VC/bin holds cl.exe, link.exe, lib.exe and winecl6-rc wrapper
#                         scripts (the winecl6-* scripts of the prefix package), VC/include and VC/lib
#                         link to the prefix's VC98 Include and Lib    (default <prefix>-msvc-shadow)
#   NL_WINE_VC6_EXTERNAL  externals root, one directory per library (2026q4_external_vc6_x86 layout)
#                                                              (default <prefix>/drive_c/2026q4_external_vc6_x86)
# Configure-time environment expected by the Find modules: PROGRAMFILES, DXSDK_DIR (the DirectX 9.0 SDK,
# Summer 2003, at C:\DX90SDK in the prefix).
# ryzomcore/CMakeModules/nel.cmake uses its own TARGET_CPU convention, not CMake's CMAKE_SYSTEM_PROCESSOR.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

if(DEFINED ENV{NL_WINE_VC6_PREFIX})
	set(_NLWINE_PREFIX "$ENV{NL_WINE_VC6_PREFIX}")
else()
	set(_NLWINE_PREFIX "$ENV{HOME}/toolchain_vc6_prefix")
endif()
if(DEFINED ENV{NL_WINE_VC6_SHADOW})
	set(_NLWINE_SHADOW "$ENV{NL_WINE_VC6_SHADOW}")
else()
	set(_NLWINE_SHADOW "${_NLWINE_PREFIX}-msvc-shadow")
endif()
if(DEFINED ENV{NL_WINE_VC6_EXTERNAL})
	set(_NLWINE_EXTERNAL "$ENV{NL_WINE_VC6_EXTERNAL}")
else()
	set(_NLWINE_EXTERNAL "${_NLWINE_PREFIX}/drive_c/2026q4_external_vc6_x86")
endif()

set(TARGET_CPU x86 CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER   "${_NLWINE_SHADOW}/VC/bin/cl.exe")
set(CMAKE_CXX_COMPILER "${_NLWINE_SHADOW}/VC/bin/cl.exe")
set(CMAKE_LINKER       "${_NLWINE_SHADOW}/VC/bin/link.exe")
set(CMAKE_AR           "${_NLWINE_SHADOW}/VC/bin/lib.exe")
set(CMAKE_RC_COMPILER  "${_NLWINE_SHADOW}/VC/bin/winecl6-rc")
# VC6 binaries bind msvcrt.dll directly: no manifests, no mt.exe

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

# Debug information goes into the objects (/Z7): without a PDB server, parallel compiles writing one
# vc60.pdb fail with C1033 (the compiler wrapper also rewrites /Zi). VC6 has /GZ where later
# compilers have /RTC1.
set(CMAKE_C_FLAGS_DEBUG "/MDd /Z7 /Od /GZ" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Z7 /Od /GZ" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MD /Z7 /O2 /Ob1 /DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Z7 /O2 /Ob1 /DNDEBUG" CACHE STRING "" FORCE)

file(GLOB NLWINE_EXTERNAL_DIRS "${_NLWINE_EXTERNAL}/*")
if(NOT NLWINE_EXTERNAL_DIRS)
	# Fail fast: with an empty prefix path the Find modules silently fall back to the Linux host's own
	# headers (host zlib.h found, no matching .lib ever).
	message(FATAL_ERROR "No externals found under ${_NLWINE_EXTERNAL}: extract 2026q4_external_vc6_x86 there or set NL_WINE_VC6_EXTERNAL")
endif()
set(CMAKE_PREFIX_PATH ${NLWINE_EXTERNAL_DIRS} CACHE STRING "" FORCE)

# STLport is linked as the DLL, like the externals that were built against it (its auto-link picks the
# import library matching /MD or /MDd); linking the static stlport_statix as well would duplicate it.
set(STLPORT_DIR "${_NLWINE_EXTERNAL}/stlport" CACHE PATH "" FORCE)
set(STLPORT_LIBRARY_RELEASE "${_NLWINE_EXTERNAL}/stlport/lib/stlport.5.2.lib" CACHE FILEPATH "" FORCE)
set(STLPORT_LIBRARY_DEBUG "${_NLWINE_EXTERNAL}/stlport/lib/stlportd.5.2.lib" CACHE FILEPATH "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_MySQL TRUE)
