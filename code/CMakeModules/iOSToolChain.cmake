# This file is based off of the Platform/Darwin.cmake and Platform/UnixPaths.cmake
# files which are included with CMake 2.8.4
# It has been altered for iOS development
#
# Options:
#
# IOS_VERSION = last(default) or specific one (4.3, 5.0, 4.1)
#   This decides if SDKS will be selected from the iPhoneOS.platform or iPhoneSimulator.platform folders
#   OS - the default, used to build for iPhone and iPad physical devices, which have an arm arch.
#   SIMULATOR - used to build for the Simulator platforms, which have an x86 arch.
#
# IOS_PLATFORM = OS (default) or SIMULATOR or ALL
#   This decides if SDKS will be selected from the iPhoneOS.platform or iPhoneSimulator.platform folders
#   OS - the default, used to build for iPhone and iPad physical devices, which have an arm arch.
#   SIMULATOR - used to build for the Simulator platforms, which have an x86 arch.
#
# CMAKE_IOS_DEVELOPER_ROOT = automatic(default) or /path/to/platform/Developer folder
#   By default this location is automatcially chosen based on the IOS_PLATFORM value above.
#   If set manually, it will override the default location and force the user of a particular Developer Platform
#
# CMAKE_IOS_SDK_ROOT = automatic(default) or /path/to/platform/Developer/SDKs/SDK folder
#   By default this location is automatcially chosen based on the CMAKE_IOS_DEVELOPER_ROOT value.
#   In this case it will always be the most up-to-date SDK found in the CMAKE_IOS_DEVELOPER_ROOT path.
#   If set manually, this will force the use of a specific SDK version

IF(DEFINED CMAKE_CROSSCOMPILING)
  # subsequent toolchain loading is not really needed
  RETURN()
ENDIF()

# Standard settings
SET(CMAKE_SYSTEM_NAME Darwin)
SET(CMAKE_SYSTEM_VERSION 1) # TODO: determine target Darwin version
SET(UNIX ON)
SET(APPLE ON)
SET(IOS ON)

# Force the compilers to Clang for iOS
include (CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER (clang Clang)
CMAKE_FORCE_CXX_COMPILER (clang++ Clang)

IF(CMAKE_CXX_COMPILER)
  EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} --version
    OUTPUT_VARIABLE CLANG_VERSION_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  STRING(REGEX REPLACE "Apple LLVM version ([\\.0-9]+).*"
    "\\1" CMAKE_CXX_COMPILER_VERSION "${CLANG_VERSION_RAW}")

  SET(CMAKE_C_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
  SET(CMAKE_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
ENDIF()

# Setup iOS platform
IF(NOT DEFINED IOS_PLATFORM)
  SET(IOS_PLATFORM "OS")
ENDIF()

SET(IOS_PLATFORM ${IOS_PLATFORM} CACHE STRING "Type of iOS Platform")

SET(IOS_PLATFORM_LOCATION "iPhoneOS.platform")
SET(IOS_SIMULATOR_PLATFORM_LOCATION "iPhoneSimulator.platform")

# Check the platform selection and setup for developer root
if(${IOS_PLATFORM} STREQUAL "OS")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
elseif(${IOS_PLATFORM} STREQUAL "SIMULATOR")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")
elseif(${IOS_PLATFORM} STREQUAL "ALL")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator;-iphoneos")
else()
  message (FATAL_ERROR "Unsupported IOS_PLATFORM value selected. Please choose OS or SIMULATOR")
endif()
set (CMAKE_XCODE_EFFECTIVE_PLATFORMS ${CMAKE_XCODE_EFFECTIVE_PLATFORMS} CACHE PATH "iOS Platform")

# Setup iOS developer location unless specified manually with CMAKE_IOS_DEVELOPER_ROOT
# Note Xcode 4.3 changed the installation location, choose the most recent one available
SET(XCODE_DEFAULT_ROOT "/Applications/Xcode.app/Contents")

IF(NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
  IF(NOT DEFINED CMAKE_XCODE_ROOT)
    IF(EXISTS ${XCODE_DEFAULT_ROOT})
      SET(CMAKE_XCODE_ROOT ${XCODE_DEFAULT_ROOT} CACHE STRING "Xcode root")
    ELSE()
      MESSAGE(FATAL_ERROR "Xcode directory ${XCODE_DEFAULT_ROOT} doesn't exist")
    ENDIF()
  ENDIF()
  SET(TMP ${CMAKE_XCODE_ROOT}/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer)
  IF(EXISTS ${TMP})
    SET(CMAKE_IOS_DEVELOPER_ROOT ${TMP})
    MESSAGE(STATUS "Use iOS developer root: ${CMAKE_IOS_DEVELOPER_ROOT}")
  ENDIF()
  SET(TMP ${CMAKE_XCODE_ROOT}/Developer/Platforms/${IOS_SIMULATOR_PLATFORM_LOCATION}/Developer)
  IF(EXISTS ${TMP})
    SET(CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT ${TMP})
    MESSAGE(STATUS "Use iOS simulator developer root: ${CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT}")
  ENDIF()
ENDIF()

SET(CMAKE_IOS_DEVELOPER_ROOT ${CMAKE_IOS_DEVELOPER_ROOT} CACHE PATH "Location of iOS Platform")
SET(CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT ${CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT} CACHE PATH "Location of iOS Simulator Platform")

MACRO(GET_AVAILABLE_SDK_VERSIONS ROOT VERSIONS)
  FILE(GLOB _CMAKE_IOS_SDKS "${ROOT}/SDKs/iPhoneOS*")
  IF(_CMAKE_IOS_SDKS)
    LIST(SORT _CMAKE_IOS_SDKS)
    LIST(REVERSE _CMAKE_IOS_SDKS)
    FOREACH(_CMAKE_IOS_SDK ${_CMAKE_IOS_SDKS})
      STRING(REGEX REPLACE ".+iPhoneOS([0-9.]+)\\.sdk" "\\1" _IOS_SDK "${_CMAKE_IOS_SDK}")
      LIST(APPEND ${VERSIONS} ${_IOS_SDK})
    ENDFOREACH()
  ENDIF()
ENDMACRO()

# Find and use the most recent iOS sdk
IF(NOT DEFINED CMAKE_IOS_SDK_ROOT)
  # Search for a specific version of a SDK
  GET_AVAILABLE_SDK_VERSIONS(${CMAKE_IOS_DEVELOPER_ROOT} IOS_VERSIONS)

  IF(NOT IOS_VERSIONS)
    MESSAGE(FATAL_ERROR "No iOS SDK's found in default search path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
  ENDIF()

  IF(IOS_VERSION)
    LIST(FIND IOS_VERSIONS "${IOS_VERSION}" _INDEX)
    IF(_INDEX EQUAL -1)
      LIST(GET IOS_VERSIONS 0 IOS_SDK_VERSION)
    ELSE()
      SET(IOS_SDK_VERSION ${IOS_VERSION})
    ENDIF()
  ELSE()
    LIST(GET IOS_VERSIONS 0 IOS_VERSION)
    SET(IOS_SDK_VERSION ${IOS_VERSION})
  ENDIF()

  MESSAGE(STATUS "Target iOS ${IOS_VERSION} and use SDK ${IOS_SDK_VERSION}")

  SET(CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneOS${IOS_SDK_VERSION}.sdk)
  SET(CMAKE_IOS_SIMULATOR_SDK_ROOT ${CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT}/SDKs/iPhoneSimulator${IOS_SDK_VERSION}.sdk)
endif()

SET(CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")
SET(CMAKE_IOS_SIMULATOR_SDK_ROOT ${CMAKE_IOS_SIMULATOR_SDK_ROOT} CACHE PATH "Location of the selected iOS Simulator SDK")

SET(IOS_VERSION ${IOS_VERSION} CACHE STRING "iOS target version")

# Set the sysroot default to the most recent SDK
SET(CMAKE_IOS_SYSROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Sysroot used for iOS support")
SET(CMAKE_IOS_SIMULATOR_SYSROOT ${CMAKE_IOS_SIMULATOR_SDK_ROOT} CACHE PATH "Sysroot used for iOS Simulator support")

IF(CMAKE_GENERATOR MATCHES Xcode)
  IF(${IOS_PLATFORM} STREQUAL "OS")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ELSEIF(${IOS_PLATFORM} STREQUAL "SIMULATOR")
    SET(CMAKE_SYSTEM_PROCESSOR "x86")
  ELSEIF(${IOS_PLATFORM} STREQUAL "ALL")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ENDIF()
ELSE()
  IF(${IOS_PLATFORM} STREQUAL "OS")
    SET(ARCHS "armv7;arm64")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ELSEIF(${IOS_PLATFORM} STREQUAL "SIMULATOR")
    # iPhone simulator targets i386
    SET(ARCHS "i386")
    SET(CMAKE_SYSTEM_PROCESSOR "x86")
  ELSEIF(${IOS_PLATFORM} STREQUAL "ALL")
    SET(ARCHS "armv7;arm64;i386;x86_64")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ENDIF()
ENDIF()

# set the architecture for iOS - using ARCHS_STANDARD_32_BIT sets armv7,armv7s and appears to be XCode's standard.
# The other value that works is ARCHS_UNIVERSAL_IPHONE_OS but that sets armv7 only
IF(ARCHS)
  SET(CMAKE_OSX_ARCHITECTURES ${ARCHS} CACHE STRING "Build architecture for iOS")
ENDIF()

# Set the find root to the iOS developer roots and to user defined paths
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} ${CMAKE_SOURCE_DIR}/external $ENV{EXTERNAL_IOS_PATH} CACHE STRING "iOS find search path root")

# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
  ${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
  ${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
  ${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
)

# only search the iOS sdks, not the remainder of the host filesystem
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# determinate location for bin utils based on CMAKE_FIND_ROOT_PATH
include(CMakeFindBinUtils)
