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

# Setup iOS platform
if (NOT DEFINED IOS_PLATFORM)
  set (IOS_PLATFORM "OS")
endif (NOT DEFINED IOS_PLATFORM)
set (IOS_PLATFORM ${IOS_PLATFORM} CACHE STRING "Type of iOS Platform")

SET(IOS_PLATFORM_LOCATION "iPhoneOS.platform")
SET(IOS_SIMULATOR_PLATFORM_LOCATION "iPhoneSimulator.platform")

# Check the platform selection and setup for developer root
if (${IOS_PLATFORM} STREQUAL "OS")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
elseif (${IOS_PLATFORM} STREQUAL "SIMULATOR")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")
elseif (${IOS_PLATFORM} STREQUAL "ALL")
  # This causes the installers to properly locate the output libraries
  set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator;-iphoneos")
else (${IOS_PLATFORM} STREQUAL "OS")
  message (FATAL_ERROR "Unsupported IOS_PLATFORM value selected. Please choose OS or SIMULATOR")
endif (${IOS_PLATFORM} STREQUAL "OS")
set (CMAKE_XCODE_EFFECTIVE_PLATFORMS ${CMAKE_XCODE_EFFECTIVE_PLATFORMS} CACHE PATH "iOS Platform")

# Setup iOS developer location unless specified manually with CMAKE_IOS_DEVELOPER_ROOT
# Note Xcode 4.3 changed the installation location, choose the most recent one available
SET(XCODE_POST_43_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms")
SET(XCODE_PRE_43_ROOT "/Developer/Platforms")

IF(NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
  IF(EXISTS ${XCODE_POST_43_ROOT})
    SET(CMAKE_XCODE_ROOT ${XCODE_POST_43_ROOT})
  ELSEIF(EXISTS ${XCODE_PRE_43_ROOT})
    SET(CMAKE_XCODE_ROOT ${XCODE_PRE_43_ROOT})
  ENDIF(EXISTS ${XCODE_POST_43_ROOT})
  IF(EXISTS ${CMAKE_XCODE_ROOT}/${IOS_PLATFORM_LOCATION}/Developer)
    SET(CMAKE_IOS_DEVELOPER_ROOT ${CMAKE_XCODE_ROOT}/${IOS_PLATFORM_LOCATION}/Developer)
  ENDIF(EXISTS ${CMAKE_XCODE_ROOT}/${IOS_PLATFORM_LOCATION}/Developer)
  IF(EXISTS ${CMAKE_XCODE_ROOT}/${IOS_SIMULATOR_PLATFORM_LOCATION}/Developer)
    SET(CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT ${CMAKE_XCODE_ROOT}/${IOS_SIMULATOR_PLATFORM_LOCATION}/Developer)
  ENDIF(EXISTS ${CMAKE_XCODE_ROOT}/${IOS_SIMULATOR_PLATFORM_LOCATION}/Developer)
ENDIF(NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
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
    ENDFOREACH(_CMAKE_IOS_SDK)
  ENDIF(_CMAKE_IOS_SDKS)
ENDMACRO(GET_AVAILABLE_SDK_VERSIONS)

# Find and use the most recent iOS sdk 
IF(NOT DEFINED CMAKE_IOS_SDK_ROOT)
  # Search for a specific version of a SDK
  GET_AVAILABLE_SDK_VERSIONS(${CMAKE_IOS_DEVELOPER_ROOT} IOS_VERSIONS)

  IF(NOT IOS_VERSIONS)
    MESSAGE(FATAL_ERROR "No iOS SDK's found in default search path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
  ENDIF(NOT IOS_VERSIONS)
  
  IF(IOS_VERSION)
    LIST(FIND IOS_VERSIONS "${IOS_VERSION}" _INDEX)
    IF(_INDEX EQUAL -1)
      LIST(GET IOS_VERSIONS 0 IOS_SDK_VERSION)
    ELSE(_INDEX EQUAL -1)
      SET(IOS_SDK_VERSION ${IOS_VERSION})
    ENDIF(_INDEX EQUAL -1)
  ELSE(IOS_VERSION)
    LIST(GET IOS_VERSIONS 0 IOS_VERSION)
    SET(IOS_SDK_VERSION ${IOS_VERSION})
  ENDIF(IOS_VERSION)

  MESSAGE(STATUS "Target iOS ${IOS_VERSION} and use SDK ${IOS_SDK_VERSION}")

  SET(CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneOS${IOS_SDK_VERSION}.sdk)
  SET(CMAKE_IOS_SIMULATOR_SDK_ROOT ${CMAKE_IOS_SIMULATOR_DEVELOPER_ROOT}/SDKs/iPhoneSimulator${IOS_SDK_VERSION}.sdk)
endif (NOT DEFINED CMAKE_IOS_SDK_ROOT)

SET(CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")
SET(CMAKE_IOS_SIMULATOR_SDK_ROOT ${CMAKE_IOS_SIMULATOR_SDK_ROOT} CACHE PATH "Location of the selected iOS Simulator SDK")

SET(IOS_VERSION ${IOS_VERSION} CACHE STRING "iOS target version")

# Set the sysroot default to the most recent SDK
SET(CMAKE_IOS_SYSROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Sysroot used for iOS support")
SET(CMAKE_IOS_SIMULATOR_SYSROOT ${CMAKE_IOS_SIMULATOR_SDK_ROOT} CACHE PATH "Sysroot used for iOS Simulator support")

IF(CMAKE_GENERATOR MATCHES Xcode)
  SET(ARCHS "$(ARCHS_STANDARD_32_BIT)")
  IF(${IOS_PLATFORM} STREQUAL "OS")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ELSEIF(${IOS_PLATFORM} STREQUAL "SIMULATOR")
    SET(CMAKE_SYSTEM_PROCESSOR "x86")
  ELSEIF(${IOS_PLATFORM} STREQUAL "ALL")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ENDIF(${IOS_PLATFORM} STREQUAL "OS")
ELSE(CMAKE_GENERATOR MATCHES Xcode)
  IF(${IOS_PLATFORM} STREQUAL "OS")
    SET(ARCHS "armv7")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ELSEIF(${IOS_PLATFORM} STREQUAL "SIMULATOR")
    # iPhone simulator targets i386
    SET(ARCHS "i386")
    SET(CMAKE_SYSTEM_PROCESSOR "x86")
  ELSEIF(${IOS_PLATFORM} STREQUAL "ALL")
    SET(ARCHS "armv7;i386")
    SET(CMAKE_SYSTEM_PROCESSOR "armv7")
  ENDIF(${IOS_PLATFORM} STREQUAL "OS")
ENDIF(CMAKE_GENERATOR MATCHES Xcode)

# set the architecture for iOS - using ARCHS_STANDARD_32_BIT sets armv7,armv7s and appears to be XCode's standard. 
# The other value that works is ARCHS_UNIVERSAL_IPHONE_OS but that sets armv7 only
set (CMAKE_OSX_ARCHITECTURES ${ARCHS} CACHE string  "Build architecture for iOS")

# Set the find root to the iOS developer roots and to user defined paths
set (CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} $ENV{EXTERNAL_IOS_PATH} CACHE string  "iOS find search path root")

# default to searching for frameworks first
set (CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
set (CMAKE_SYSTEM_FRAMEWORK_PATH
  ${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
  ${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
  ${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
)

# only search the iOS sdks, not the remainder of the host filesystem
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#SET(CMAKE_SYSTEM_INCLUDE_PATH /include /usr/include)
#SET(CMAKE_SYSTEM_LIBRARY_PATH /lib /usr/lib)
#SET(CMAKE_SYSTEM_PROGRAM_PATH /bin /usr/bin)
