# CMake toolchain file for building ryzomcore with VS2008 x86 under Wine.
# Pair with setup.sh (creates the shadow VC dir and symlink farms) and the
# toolchain_v90_prefix wine prefix package. Environment:
#   NL_WINE_VS2008_PREFIX    extracted prefix       (default $HOME/toolchain_v90_prefix)
#   NL_WINE_VS2008_SHADOW    shadow VC dir           (default <prefix>-msvc-shadow)
#   NL_WINE_VS2008_EXTERNAL  externals root          (default <prefix>/drive_c/2019q4_external_v90_x86)
# Configure-time environment expected by the Find modules (see setup.sh /
# the workflow): PROGRAMFILES, WINSDK_DIR, DXSDK_DIR.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

if(DEFINED ENV{NL_WINE_VS2008_PREFIX})
	set(_NLWINE_PREFIX "$ENV{NL_WINE_VS2008_PREFIX}")
else()
	set(_NLWINE_PREFIX "$ENV{HOME}/toolchain_v90_prefix")
endif()
if(DEFINED ENV{NL_WINE_VS2008_SHADOW})
	set(_NLWINE_SHADOW "$ENV{NL_WINE_VS2008_SHADOW}")
else()
	set(_NLWINE_SHADOW "${_NLWINE_PREFIX}-msvc-shadow")
endif()
if(DEFINED ENV{NL_WINE_VS2008_EXTERNAL})
	set(_NLWINE_EXTERNAL "$ENV{NL_WINE_VS2008_EXTERNAL}")
else()
	set(_NLWINE_EXTERNAL "${_NLWINE_PREFIX}/drive_c/2019q4_external_v90_x86")
endif()

# ryzomcore/CMakeModules/nel.cmake uses its own TARGET_CPU convention, not
# CMake's built-in CMAKE_SYSTEM_PROCESSOR - falls back to HOST_CPU (the
# build machine's arch, x86_64) when unset, which would make TARGET_X64
# true and point every Find*.cmake module (MAXSDK, DXSDK, MFC) at the
# 64-bit library directories.
set(TARGET_CPU x86 CACHE STRING "" FORCE)

# CMAKE_C/CXX_COMPILER live under a fake .../VC/bin/ shadow directory (real
# files are the wrapper scripts, just named cl.exe/link.exe/lib.exe) because
# CMakeModules/FindMSVC.cmake derives VC_DIR by regexing "/bin/.+" off the
# compiler path, assuming it points directly at a real VC/bin/cl.exe.
# include/lib/atlmfc/redist under the shadow VC/ are symlinks to the real
# toolchain's copies (setup.sh builds all of this).
set(CMAKE_C_COMPILER   "${_NLWINE_SHADOW}/VC/bin/cl.exe")
set(CMAKE_CXX_COMPILER "${_NLWINE_SHADOW}/VC/bin/cl.exe")
set(CMAKE_LINKER       "${_NLWINE_SHADOW}/VC/bin/link.exe")
set(CMAKE_AR           "${_NLWINE_SHADOW}/VC/bin/lib.exe")
set(CMAKE_MT           "${CMAKE_CURRENT_LIST_DIR}/winecl-mt")
set(CMAKE_RC_COMPILER  "${CMAKE_CURRENT_LIST_DIR}/winecl-rc")

# NEVER, not ONLY: ONLY restricts find_path/find_library to paths prefixed
# by CMAKE_FIND_ROOT_PATH, which we never set (empty) - with an empty root
# path, ONLY mode finds nothing at all, even with correct explicit HINTS.
# We aren't doing genuine sysroot-based cross-compiling - every path handed
# to Find*.cmake modules is already an absolute concrete Linux path.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

# /Zi triggers cl.exe's own PDB-writing path via mspdb80.dll, which is
# broken under Wine (C1902 at compile time, LNK1101 at link time) -
# verified empirically, not a DLL version mismatch. CMake's default MSVC
# flags always include /Zi for Debug/RelWithDebInfo, even for the plain
# "does this compiler work" sanity check run before CMAKE_BUILD_TYPE is
# consulted. The toolchain file runs before Platform/Windows-MSVC.cmake
# sets the CMAKE_*_FLAGS_<CONFIG>_INIT defaults, so patching those here is
# a no-op - instead force the final CACHE entries directly (with FORCE),
# so the platform module's later non-FORCE `set(... CACHE ...)` becomes a
# no-op and our /Z7 value sticks. Values match CMake 3.28's own MSVC
# defaults verbatim except s/Zi/Z7/, and s/MDd/MD/ (see below).
#
# /MDd (default Debug CRT) needs msvcr90d.dll/msvcp90d.dll, which live in
# the toolchain's VC/redist/Debug_NonRedist/ but aren't installed into the
# prefix - running an /MDd binary fails silently (exit 53, no output). /MD
# is also the semantically correct choice regardless: the era's shipped
# binaries were Release builds, and matching that CRT is the point.
set(CMAKE_C_FLAGS_DEBUG "/MD /Z7 /Ob0 /Od /RTC1" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "/MD /Z7 /Ob0 /Od /RTC1" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MD /Z7 /O2 /Ob1 /DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Z7 /O2 /Ob1 /DNDEBUG" CACHE STRING "" FORCE)

# The 2019q4_external_v90_x86 archive is one subfolder per library
# (zlib/include, zlib/lib, boost/include, boost/lib, ...), not the single
# unified include/+lib/ tree CMakeModules/FindExternal.cmake's generic
# EXTERNAL_PATH mechanism expects - left alone, that mechanism falls back
# to matching plain /usr (the Linux host's own zlib.h exists there), which
# then pollutes CMAKE_INCLUDE_PATH/CMAKE_LIBRARY_PATH globally: headers get
# found, but the matching .lib never will be (wrong platform's binary
# format). CMAKE_PREFIX_PATH is the standard mechanism each individual
# FindZLIB/FindBoost/etc. module actually searches.
file(GLOB NLWINE_EXTERNAL_DIRS "${_NLWINE_EXTERNAL}/*")
if(NOT NLWINE_EXTERNAL_DIRS)
	# Fail fast: with an empty prefix path the Find modules silently fall back to
	# the Linux host's own headers (host zlib.h found, no matching .lib ever).
	message(FATAL_ERROR "No externals found under ${_NLWINE_EXTERNAL} — extract 2019q4_external_v90_x86 there or set NL_WINE_VS2008_EXTERNAL")
endif()
set(CMAKE_PREFIX_PATH ${NLWINE_EXTERNAL_DIRS} CACHE STRING "" FORCE)

# Manifest embedding follows the traditional VS2008 three-stage flow:
# link /MANIFEST writes the sidecar, a POST_BUILD mt.exe step embeds it as
# RT_MANIFEST (NL_EMBED_SXS_MANIFEST_MT in CMakeModules/nel.cmake). CMake's
# own path can't be used: its non-incremental vs_link_exe flow delegates to
# /MANIFEST:EMBED, VS2010+ syntax that VS2008's link.exe rejects (LNK1117;
# the winecl-link wrapper strips the flag). mt.exe itself requires the real
# .NET 2.0 the prefix package carries - without it, mt instantiates the CLR
# metadata dispenser, gets no engine, and page-faults.
set(NL_EMBED_SXS_MANIFEST_MT ON CACHE BOOL "" FORCE)

# The 2019q4 externals carry no mariadb client for VC9, and with
# CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER the stock FindMySQL would happily
# find the Linux host's own mariadb (headers from /usr/include/mariadb, a
# bare "mariadb" library name that the MSVC link line turns into a
# nonexistent mariadb.lib). Disable the package outright; the DB-connected
# tools skip or link without it.
set(CMAKE_DISABLE_FIND_PACKAGE_MySQL TRUE)
