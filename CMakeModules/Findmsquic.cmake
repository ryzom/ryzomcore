# Findmsquic.cmake - MsQuic, Microsoft's QUIC library (https://github.com/microsoft/msquic)
#
# Provides the imported target msquic::msquic, the same target that msquic's own package
# configuration (msquic-config.cmake, installed by vcpkg and by the msquic CMake build)
# defines. The presets set CMAKE_FIND_PACKAGE_PREFER_CONFIG, so whenever that configuration
# is present it is used instead of this module; consumers must therefore link the target
# and not the variables below, which only exist for compatibility and are not set in that case.
#
#   msquic_FOUND        - true when the header and the library were found
#   msquic_INCLUDE_DIRS - directory holding msquic.h
#   msquic_LIBRARIES    - the library, with optimized/debug keywords when both were found
#
# Cache entries that may be preset to point at an unpacked release or NuGet package:
#   msquic_INCLUDE_DIRS, msquic_LIBRARY_RELEASE, msquic_LIBRARY_DEBUG
#
# The Ubuntu libmsquic package ships only the versioned libmsquic.so.2, hence that name.

FIND_PATH(
	msquic_INCLUDE_DIRS
	NAMES msquic.h
	PATHS /usr/local/include /usr/include
)

FIND_LIBRARY(
	msquic_LIBRARY_RELEASE
	NAMES libmsquic.so.2 msquic libmsquic
	PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib /opt/local
)

FIND_LIBRARY(
	msquic_LIBRARY_DEBUG
	NAMES libmsquicd.so.2 msquicd libmsquicd
	PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib /opt/local
)

INCLUDE(SelectLibraryConfigurations)
SELECT_LIBRARY_CONFIGURATIONS(msquic)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(msquic REQUIRED_VARS msquic_LIBRARY msquic_INCLUDE_DIRS)

IF(msquic_FOUND AND NOT TARGET msquic::msquic)
  ADD_LIBRARY(msquic::msquic UNKNOWN IMPORTED)
  SET_TARGET_PROPERTIES(msquic::msquic PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${msquic_INCLUDE_DIRS}")
  IF(msquic_LIBRARY_RELEASE)
    SET_PROPERTY(TARGET msquic::msquic APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    SET_TARGET_PROPERTIES(msquic::msquic PROPERTIES IMPORTED_LOCATION_RELEASE "${msquic_LIBRARY_RELEASE}")
  ENDIF()
  IF(msquic_LIBRARY_DEBUG)
    SET_PROPERTY(TARGET msquic::msquic APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    SET_TARGET_PROPERTIES(msquic::msquic PROPERTIES IMPORTED_LOCATION_DEBUG "${msquic_LIBRARY_DEBUG}")
  ENDIF()
  # configurations without a matching entry (RelWithDebInfo, MinSizeRel) fall back to this
  SET_TARGET_PROPERTIES(msquic::msquic PROPERTIES IMPORTED_LOCATION "${msquic_LIBRARY}")
ENDIF()

MARK_AS_ADVANCED(msquic_INCLUDE_DIRS msquic_LIBRARY_RELEASE msquic_LIBRARY_DEBUG)
