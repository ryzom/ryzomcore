# Define OSX_SDK to force a specific version such as : -DOSX_SDK=10.11
#
# Example:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=$CMAKE_MODULE_PATH/OSXToolChain.cmake -DOSX_SDK=10.14 <other flags>

# Don't forget to define environment variables:
#
# export MACOSX_DEPLOYMENT_TARGET=10.8
# export OSXCROSS_GCC_NO_STATIC_RUNTIME=1
# export PATH=$PATH:/home/src/osxcross/target/bin
#
# ln -s /usr/bin/hg /home/src/osxcross/target/bin/hg
#
# To install all dependencies:
# ./osxcross-macports install libxml2 jpeg curl libogg libvorbis freetype boost openssl zlib lua-5.3 giflib

# to compile Luabind
# cmake .. -DCMAKE_TOOLCHAIN_FILE=$CMAKE_MODULE_PATH/OSXToolChain.cmake -DWITH_SHARED=OFF -DWITH_STATIC=ON -DWITH_LUA51=OFF -DWITH_LUA53=ON -DCMAKE_INSTALL_PREFIX=$HOME/osxcross/target/external

IF(DEFINED CMAKE_CROSSCOMPILING)
  # subsequent toolchain loading is not really needed
  RETURN()
ENDIF()

# OSXCROSS_TARGET
# OSXCROSS_SDK

# Force the compilers to Clang for OS X

SET(OSXCROSS_HOST "x86_64-apple-darwin18")

# C
SET(CMAKE_C_COMPILER ${OSXCROSS_HOST}-clang)
SET(CMAKE_C_STANDARD_COMPUTED_DEFAULT "11")
SET(CMAKE_C_COMPILE_FEATURES "c_std_90;c_function_prototypes;c_std_99;c_restrict;c_variadic_macros;c_std_11;c_static_assert")
SET(CMAKE_C90_COMPILE_FEATURES "c_std_90;c_function_prototypes")
SET(CMAKE_C99_COMPILE_FEATURES "c_std_99;c_restrict;c_variadic_macros")
SET(CMAKE_C11_COMPILE_FEATURES "c_std_11;c_static_assert")

# C++
SET(CMAKE_CXX_COMPILER ${OSXCROSS_HOST}-clang++)
SET(CMAKE_CXX_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters;cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates;cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates;cxx_std_17")
SET(CMAKE_CXX98_COMPILE_FEATURES "cxx_std_98;cxx_template_template_parameters")
SET(CMAKE_CXX11_COMPILE_FEATURES "cxx_std_11;cxx_alias_templates;cxx_alignas;cxx_alignof;cxx_attributes;cxx_auto_type;cxx_constexpr;cxx_decltype;cxx_decltype_incomplete_return_types;cxx_default_function_template_args;cxx_defaulted_functions;cxx_defaulted_move_initializers;cxx_delegating_constructors;cxx_deleted_functions;cxx_enum_forward_declarations;cxx_explicit_conversions;cxx_extended_friend_declarations;cxx_extern_templates;cxx_final;cxx_func_identifier;cxx_generalized_initializers;cxx_inheriting_constructors;cxx_inline_namespaces;cxx_lambdas;cxx_local_type_template_args;cxx_long_long_type;cxx_noexcept;cxx_nonstatic_member_init;cxx_nullptr;cxx_override;cxx_range_for;cxx_raw_string_literals;cxx_reference_qualified_functions;cxx_right_angle_brackets;cxx_rvalue_references;cxx_sizeof_member;cxx_static_assert;cxx_strong_enums;cxx_thread_local;cxx_trailing_return_types;cxx_unicode_literals;cxx_uniform_initialization;cxx_unrestricted_unions;cxx_user_literals;cxx_variadic_macros;cxx_variadic_templates")
SET(CMAKE_CXX14_COMPILE_FEATURES "cxx_std_14;cxx_aggregate_default_initializers;cxx_attribute_deprecated;cxx_binary_literals;cxx_contextual_conversions;cxx_decltype_auto;cxx_digit_separators;cxx_generic_lambdas;cxx_lambda_init_captures;cxx_relaxed_constexpr;cxx_return_type_deduction;cxx_variable_templates")
SET(CMAKE_CXX17_COMPILE_FEATURES "cxx_std_17")

# make
SET(CMAKE_MAKE_PROGRAM make)

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
SET(CMAKE_SYSTEM "Darwin-18.0.0")
SET(CMAKE_SYSTEM_VERSION "18.0.0")
SET(CMAKE_SYSTEM_PROCESSOR "x86_64")

SET(UNIX ON)
SET(APPLE ON)

# Set the find root to the OS X developer roots and to user defined paths
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_TOOLCHAIN_ROOT} ${CMAKE_OSX_SYSROOT} ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} ${MACPORTS_ROOT_DIR} ${EXTERNAL_OSX_PATH} $ENV{EXTERNAL_OSX_PATH} CACHE STRING "OS X find search path root")

# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
  ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks
  ${CMAKE_OSX_SYSROOT}/System/Library/PrivateFrameworks
  ${CMAKE_OSX_SYSROOT}/Developer/Library/Frameworks
)

# only search the OS X sdks, not the remainder of the host filesystem
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# determinate location for bin utils based on CMAKE_FIND_ROOT_PATH
INCLUDE(CMakeFindBinUtils)

set(CMAKE_AR "${OSXCROSS_HOST}-ar" CACHE FILEPATH "ar")
set(CMAKE_RANLIB "${OSXCROSS_HOST}-ranlib" CACHE FILEPATH "ranlib")
set(CMAKE_INSTALL_NAME_TOOL "${OSXCROSS_HOST}-install_name_tool" CACHE FILEPATH "install_name_tool")

set(ENV{PKG_CONFIG_LIBDIR} "${MACPORTS_ROOT_DIR}/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_OSX_TOOLCHAIN_ROOT}/macports/pkgs")
