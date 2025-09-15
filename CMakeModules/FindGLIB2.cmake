# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGLIB2
-------

Finds the GLIB2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GLIB2::GLIB2``
  The GLIB2 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GLIB2_FOUND``
  True if the system has the GLIB2 library.
``GLIB2_VERSION``
  The version of the GLIB2 library which was found.
``GLIB2_INCLUDE_DIRS``
  Include directories needed to use GLIB2.
``GLIB2_LIBRARIES``
  Libraries needed to link to GLIB2.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GLIB2_INCLUDE_DIR``
  The directory containing ``glib.h``.
``GLIB2_LIBRARY``
  The path to the GLIB2 library.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

# Try to use pkg-config first
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(PC_GLIB2 QUIET glib-2.0)
endif ()

# Find the header files
find_path(GLIB2_INCLUDE_DIR
        NAMES glib.h
        HINTS ${PC_GLIB2_INCLUDEDIR} ${PC_GLIB2_INCLUDE_DIRS}
        PATH_SUFFIXES glib-2.0
)

find_path(GLIB2_CONFIG_INCLUDE_DIR
        NAMES glibconfig.h
        HINTS ${PC_GLIB2_INCLUDEDIR} ${PC_GLIB2_INCLUDE_DIRS}
        PATH_SUFFIXES glib-2.0/include
)

# Find the library
find_library(GLIB2_LIBRARY
        NAMES glib-2.0 glib
        HINTS ${PC_GLIB2_LIBDIR} ${PC_GLIB2_LIBRARY_DIRS}
)

# Set version from pkg-config if available
set(GLIB2_VERSION ${PC_GLIB2_VERSION})

# Handle dependencies through pkg-config
set(GLIB2_DEPS_FOUND TRUE)
set(GLIB2_DEPS_LIBRARIES)
set(GLIB2_DEPS_INCLUDE_DIRS)

# Only manually check for deps if pkg-config didn't provide them
if (NOT PC_GLIB2_FOUND)
    # Check for libintl
    find_library(LIBINTL_LIBRARY NAMES intl)
    find_path(LIBINTL_INCLUDE_DIR NAMES libintl.h)

    if (LIBINTL_LIBRARY AND LIBINTL_INCLUDE_DIR)
        list(APPEND GLIB2_DEPS_LIBRARIES ${LIBINTL_LIBRARY})
        list(APPEND GLIB2_DEPS_INCLUDE_DIRS ${LIBINTL_INCLUDE_DIR})
    endif ()

    # Check for libiconv
    find_library(LIBICONV_LIBRARY NAMES iconv)
    find_path(LIBICONV_INCLUDE_DIR NAMES iconv.h)

    if (LIBICONV_LIBRARY AND LIBICONV_INCLUDE_DIR)
        list(APPEND GLIB2_DEPS_LIBRARIES ${LIBICONV_LIBRARY})
        list(APPEND GLIB2_DEPS_INCLUDE_DIRS ${LIBICONV_INCLUDE_DIR})
    endif ()
endif ()

# Set the include directories
set(GLIB2_INCLUDE_DIRS ${GLIB2_INCLUDE_DIR})
if (GLIB2_CONFIG_INCLUDE_DIR)
    list(APPEND GLIB2_INCLUDE_DIRS ${GLIB2_CONFIG_INCLUDE_DIR})
endif ()
if (GLIB2_DEPS_INCLUDE_DIRS)
    list(APPEND GLIB2_INCLUDE_DIRS ${GLIB2_DEPS_INCLUDE_DIRS})
endif ()

# Set the libraries
set(GLIB2_LIBRARIES ${GLIB2_LIBRARY})
if (GLIB2_DEPS_LIBRARIES)
    list(APPEND GLIB2_LIBRARIES ${GLIB2_DEPS_LIBRARIES})
endif ()

# Check for regex support
if (GLIB2_INCLUDE_DIRS)
    include(CheckIncludeFiles)
    set(CMAKE_REQUIRED_INCLUDES ${GLIB2_INCLUDE_DIRS})
    check_include_files("glib.h;glib/gregex.h" HAVE_GLIB_GREGEX_H)
    unset(CMAKE_REQUIRED_INCLUDES)
endif ()

# Standard find_package handling
find_package_handle_standard_args(GLIB2
        REQUIRED_VARS
        GLIB2_LIBRARY
        GLIB2_INCLUDE_DIR
        VERSION_VAR
        GLIB2_VERSION
)

# Create imported target
if (GLIB2_FOUND AND NOT TARGET GLIB2::GLIB2)
    add_library(GLIB2::GLIB2 UNKNOWN IMPORTED)
    set_target_properties(GLIB2::GLIB2 PROPERTIES
            IMPORTED_LOCATION "${GLIB2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLIB2_INCLUDE_DIRS}"
    )

    # Add compile options from pkg-config if available
    if (PC_GLIB2_FOUND AND PC_GLIB2_CFLAGS_OTHER)
        set_property(TARGET GLIB2::GLIB2 PROPERTY
                INTERFACE_COMPILE_OPTIONS "${PC_GLIB2_CFLAGS_OTHER}"
        )
    endif ()

    # Add dependencies if not already provided by pkg-config
    if (GLIB2_DEPS_LIBRARIES)
        set_property(TARGET GLIB2::GLIB2 APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "${GLIB2_DEPS_LIBRARIES}"
        )
    endif ()
endif ()

# Hide advanced variables
mark_as_advanced(
        GLIB2_INCLUDE_DIR
        GLIB2_CONFIG_INCLUDE_DIR
        GLIB2_LIBRARY
        LIBINTL_INCLUDE_DIR
        LIBINTL_LIBRARY
        LIBICONV_INCLUDE_DIR
        LIBICONV_LIBRARY
)
