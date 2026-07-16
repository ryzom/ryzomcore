# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGOBJECT2
-------

Finds the GOBJECT2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GOBJECT2::GOBJECT2``
  The GOBJECT2 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GOBJECT2_FOUND``
  True if the system has the GOBJECT2 library.
``GOBJECT2_VERSION``
  The version of the GOBJECT2 library which was found.
``GOBJECT2_INCLUDE_DIRS``
  Include directories needed to use GOBJECT2.
``GOBJECT2_LIBRARIES``
  Libraries needed to link to GOBJECT2.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GOBJECT2_INCLUDE_DIR``
  The directory containing ``gobject/gobject.h``.
``GOBJECT2_LIBRARY``
  The path to the GOBJECT2 library.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

# Try to use pkg-config first
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(PC_GOBJECT2 QUIET gobject-2.0)
endif ()

# Find the header files
find_path(GOBJECT2_INCLUDE_DIR
        NAMES gobject/gobject.h
        HINTS ${PC_GOBJECT2_INCLUDEDIR} ${PC_GOBJECT2_INCLUDE_DIRS}
        PATH_SUFFIXES glib-2.0
)

# Find the library
find_library(GOBJECT2_LIBRARY
        NAMES gobject-2.0
        HINTS ${PC_GOBJECT2_LIbDIR} ${PC_GOBJECT2_LIBRARY_DIRS}
)

# Set version from pkg-config if available
set(GOBJECT2_VERSION ${PC_GOBJECT2_VERSION})

# Handle dependencies through pkg-config
set(GOBJECT2_DEPS_FOUND TRUE)
set(GOBJECT2_DEPS_LIBRARIES)
set(GOBJECT2_DEPS_INCLUDE_DIRS)

# Set the include directories
set(GOBJECT2_INCLUDE_DIRS ${GOBJECT2_INCLUDE_DIR})
if (GOBJECT2_DEPS_INCLUDE_DIRS)
    list(APPEND GOBJECT2_INCLUDE_DIRS ${GOBJECT2_DEPS_INCLUDE_DIRS})
endif ()

# Set the libraries
set(GOBJECT2_LIBRARIES ${GOBJECT2_LIBRARY})
if (GOBJECT2_DEPS_LIBRARIES)
    list(APPEND GOBJECT2_LIBRARIES ${GOBJECT2_DEPS_LIBRARIES})
endif ()

# Standard find_package handling
find_package_handle_standard_args(GOBJECT2
        REQUIRED_VARS
        GOBJECT2_LIBRARY
        GOBJECT2_INCLUDE_DIR
        VERSION_VAR
        GOBJECT2_VERSION
)

# Create imported target
if (GOBJECT2_FOUND AND NOT TARGET GOBJECT2::GOBJECT2)
    add_library(GOBJECT2::GOBJECT2 UNKNOWN IMPORTED)
    set_target_properties(GOBJECT2::GOBJECT2 PROPERTIES
            IMPORTED_LOCATION "${GOBJECT2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GOBJECT2_INCLUDE_DIR}"
    )

    # Add compile options from pkg-config if available
    if (PC_GOBJECT2_FOUND AND PC_GOBJECT2_CFLAGS_OTHER)
        set_property(TARGET GOBJECT2::GOBJECT2 PROPERTY
                INTERFACE_COMPILE_OPTIONS "${PC_GOBJECT2_CFLAGS_OTHER}"
        )
    endif ()

    # Add dependencies if not already provided by pkg-config
    if (GOBJECT2_DEPS_LIBRARIES)
        set_property(TARGET GOBJECT2::GOBJECT2 APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES "${GOBJECT2_DEPS_LIBRARIES}"
        )
    endif ()
endif ()

# Hide advanced variables
mark_as_advanced(
        GOBJECT2_INCLUDE_DIR
        GOBJECT2_LIBRARY
)
