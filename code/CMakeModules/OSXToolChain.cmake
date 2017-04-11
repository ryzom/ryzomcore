# Define OSX_SDK to force a specific version such as : -DOSX_SDK=10.11
#
# Example:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=../CMakeModules/OSXToolChain.cmake -DWITH_NEL_TESTS=OFF -DWITH_RYZOM_SERVER=OFF -DWITH_NEL_TOOLS=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_LUA51=OFF -DWITH_LUA53=ON -DCMAKE_BUILD_TYPE=Release -DWITH_RYZOM_INSTALLER=OFF -DWITH_RYZOM_PATCH=ON -DWITH_NEL_TESTS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_TOOLS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_WARNINGS=OFF -DWITH_QT5=OFF -DWITH_STATIC=ON -DWITH_STATIC_DRIVERS=ON -DWITH_STATIC_EXTERNAL=ON -DWITH_UNIX_STRUCTURE=OFF -DWITH_INSTALL_LIBRARIES=OFF -DWITH_RYZOM_SANDBOX=OFF -DOSX_SDK=10.11

# Don't forget to define environment variables:
#
# export MACOSX_DEPLOYMENT_TARGET=10.7
# export OSXCROSS_GCC_NO_STATIC_RUNTIME=1
# export PATH=$PATH:/home/src/osxcross/target/bin
#
# To install all dependencies:
# ./osxcross-macports install libxml2 jpeg curl libogg libvorbis freetype boost openssl zlib lua-5.3

IF(DEFINED CMAKE_CROSSCOMPILING)
  # subsequent toolchain loading is not really needed
  RETURN()
ENDIF()

# Force the compilers to Clang for iOS
SET(CMAKE_C_COMPILER x86_64-apple-darwin15-clang)
SET(CMAKE_CXX_COMPILER x86_64-apple-darwin15-clang++)
set(CMAKE_CXX_COMPILER_ID "AppleClang")

# Skip the platform compiler checks for cross compiling.
SET(CMAKE_CXX_COMPILER_FORCED TRUE)
SET(CMAKE_C_COMPILER_FORCED TRUE)

# Check if osxcross is installed
EXECUTE_PROCESS(COMMAND which ${CMAKE_CXX_COMPILER} OUTPUT_VARIABLE COMPILER_FULLPATH OUTPUT_STRIP_TRAILING_WHITESPACE)

IF(NOT COMPILER_FULLPATH)
  MESSAGE(FATAL_ERROR "Unable to find ${CMAKE_CXX_COMPILER}, are you sure osxcross is installed and is in PATH?")
ENDIF()

# Default paths
GET_FILENAME_COMPONENT(CMAKE_OSX_TOOLCHAIN_ROOT ${COMPILER_FULLPATH} DIRECTORY)

# Parent directory
GET_FILENAME_COMPONENT(CMAKE_OSX_TOOLCHAIN_ROOT ${CMAKE_OSX_TOOLCHAIN_ROOT} DIRECTORY)

SET(CMAKE_OSX_SYSROOT ${CMAKE_OSX_TOOLCHAIN_ROOT}/SDK)
SET(MACPORTS_ROOT_DIR ${CMAKE_OSX_TOOLCHAIN_ROOT}/macports/pkgs/opt/local)
SET(EXTERNAL_OSX_PATH ${CMAKE_OSX_TOOLCHAIN_ROOT}/external)

# List of all SDKs that have been found
SET(OSX_SDKS)

FILE(GLOB _CMAKE_OSX_SDKS "${CMAKE_OSX_SYSROOT}/MacOSX*")
IF(_CMAKE_OSX_SDKS)
  LIST(SORT _CMAKE_OSX_SDKS)
  LIST(REVERSE _CMAKE_OSX_SDKS)
  FOREACH(_CMAKE_OSX_SDK ${_CMAKE_OSX_SDKS})
    STRING(REGEX REPLACE ".+MacOSX([0-9.]+)\\.sdk" "\\1" _OSX_SDK "${_CMAKE_OSX_SDK}")
    LIST(APPEND OSX_SDKS ${_OSX_SDK})
  ENDFOREACH()
ENDIF()

# Find and use the most recent OS X sdk
IF(NOT OSX_SDKS)
  MESSAGE(FATAL_ERROR "No OS X SDK's found in default search path ${CMAKE_OSX_SYSROOT}.")
ENDIF()

# if a specific SDK is defined, try to use it
IF(OSX_SDK)
  LIST(FIND OSX_SDKS "${OSX_SDK}" _INDEX)
  IF(_INDEX EQUAL -1)
    # if specified SDK doesn't exist, use the last one
    LIST(GET OSX_SDKS 0 OSX_SDK)
  ENDIF()
ELSE()
  # use the last SDK
  LIST(GET OSX_SDKS 0 OSX_SDK)
ENDIF()

MESSAGE(STATUS "Using OS X SDK ${OSX_SDK}")

# Define final OS X sysroot
SET(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT}/MacOSX${OSX_SDK}.sdk)

# Standard settings
SET(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM "Darwin-15.0.0")
set(CMAKE_SYSTEM_VERSION "15.0.0")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
SET(UNIX ON)
SET(APPLE ON)

# Set the find root to the iOS developer roots and to user defined paths
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_TOOLCHAIN_ROOT} ${CMAKE_OSX_SYSROOT} ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} ${MACPORTS_ROOT_DIR} ${EXTERNAL_OSX_PATH} $ENV{EXTERNAL_OSX_PATH} CACHE STRING "OS X find search path root")

# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
  ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks
  ${CMAKE_OSX_SYSROOT}/System/Library/PrivateFrameworks
  ${CMAKE_OSX_SYSROOT}/Developer/Library/Frameworks
)

# only search the iOS sdks, not the remainder of the host filesystem
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# determinate location for bin utils based on CMAKE_FIND_ROOT_PATH
INCLUDE(CMakeFindBinUtils)

