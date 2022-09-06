# Macros to check if a library needs to be manually linked to another one
# because it's using a symbol from it but it's not linked to

# CHECK_UNDEFINED_SYMBOL
# Macro to check if a library is calling an undefined symbol
#
# Syntax:
# CHECK_UNDEFINED_SYMBOL(MYLIBRARY SYMBOL SYMBOL_FOUND)
# SYMBOL_FOUND will be set to TRUE if UNDEFINED
#
# Example:
# CHECK_UNDEFINED_SYMBOL(PNG_LIBRARY inflate INFLATE_FOUND)
#
MACRO(CHECK_UNDEFINED_SYMBOL MYLIBRARY SYMBOL SYMBOL_FOUND)
  SET(${SYMBOL_FOUND} TRUE)
  IF(WIN32)
    # Always TRUE under Windows because we are using static libraries
  ELSEIF(APPLE)
    SET(CMAKE_NM nm)
    IF(CMAKE_NM)
      # Use nm to check if a library is using an external symbol
      EXEC_PROGRAM(${CMAKE_NM} ARGS "-gu ${${MYLIBRARY}} | grep ${SYMBOL}" OUTPUT_VARIABLE NM_SYMBOL)
#      MESSAGE(STATUS "Checking for undefined symbol ${SYMBOL} in ${${MYLIBRARY}}")
      IF(NOT NM_SYMBOL MATCHES ${SYMBOL})
        SET(${SYMBOL_FOUND} FALSE)
#        MESSAGE(STATUS "Defined symbol ${SYMBOL} detected in ${${MYLIBRARY}}")
      ENDIF()
    ENDIF()
  ELSEIF(UNIX)
    SET(CMAKE_OBJDUMP objdump)
    IF(CMAKE_OBJDUMP)
      # Use objdump to check if a library is using an external symbol
      #MESSAGE(STATUS "exec ${CMAKE_OBJDUMP} -T ${${MYLIBRARY}} | grep ${SYMBOL}")
      EXEC_PROGRAM(${CMAKE_OBJDUMP} ARGS "-T ${${MYLIBRARY}} | grep ${SYMBOL}" OUTPUT_VARIABLE OBJDUMP_SYMBOL)
      IF(NOT OBJDUMP_SYMBOL MATCHES "UND")
        #MESSAGE(STATUS "${${MYLIBRARY}} does not use symbol ${SYMBOL}")
        SET(${SYMBOL_FOUND} FALSE)
      ELSE()
        #MESSAGE(STATUS "${${MYLIBRARY}} uses symbol ${SYMBOL}")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO(CHECK_UNDEFINED_SYMBOL)

# CHECK_LINKED_LIBRARY
# Macro to check if a library is linked to another one
#
# Syntax:
# CHECK_LINKED_LIBRARY(MYLIBRARY OTHERLIBRARY LIBRARY_FOUND)
# LIBRARY_FOUND will be set to TRUE if LINKED
#
# Example:
# CHECK_LINKED_LIBRARY(PNG_LIBRARY ZLIB_LIBRARY ZLIB_FOUND)
#
MACRO(CHECK_LINKED_LIBRARY MYLIBRARY OTHERLIBRARY LIBRARY_FOUND)
  SET(${LIBRARY_FOUND} FALSE)
  IF(WIN32)
    # Always FALSE under Windows because we are using static libraries
  ELSEIF(APPLE)
    SET(CMAKE_OTOOL otool)
    IF(CMAKE_OTOOL)
      # Use otool to check if a library is linked to another library
      GET_FILENAME_COMPONENT(LIBNAME "${${OTHERLIBRARY}}" NAME_WE)
      EXEC_PROGRAM(${CMAKE_OTOOL} ARGS "-L ${${MYLIBRARY}} | grep ${LIBNAME}" OUTPUT_VARIABLE OTOOL_LIBRARY)
#      MESSAGE(STATUS "Checking if ${LIBNAME} is linked to ${${MYLIBRARY}}")
      IF(OTOOL_LIBRARY MATCHES "${LIBNAME}")
        SET(${LIBRARY_FOUND} TRUE)
#        MESSAGE(STATUS "Library ${LIBNAME} already linked to ${${MYLIBRARY}}")
      ENDIF()
    ENDIF()
  ELSEIF(UNIX)
    SET(CMAKE_OBJDUMP objdump)
    IF(CMAKE_OBJDUMP)
      GET_FILENAME_COMPONENT(LIBNAME "${${OTHERLIBRARY}}" NAME)
      # TODO: under Solaris use dump -Lv <lib>
      # Use objdump to check if a library is linked to another library
      #MESSAGE(STATUS "exec ${CMAKE_OBJDUMP} -p ${${MYLIBRARY}} | grep ${LIBNAME}")
      EXEC_PROGRAM(${CMAKE_OBJDUMP} ARGS "-p ${${MYLIBRARY}} | grep ${LIBNAME}" OUTPUT_VARIABLE OBJDUMP_LIBRARY)
      IF(OBJDUMP_LIBRARY MATCHES "NEEDED")
        #MESSAGE(STATUS "${${MYLIBRARY}} references to ${LIBNAME}.")
        SET(${LIBRARY_FOUND} TRUE)
      ELSE()
        #MESSAGE(STATUS "${${MYLIBRARY}} does not reference to ${LIBNAME}!")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO(CHECK_LINKED_LIBRARY)

MACRO(CHECK_DEPENDS MYLIBRARY OTHERLIBRARY SYMBOL MUSTLINK)
  CHECK_UNDEFINED_SYMBOL(MYLIBRARY SYMBOL SYMBOL_FOUND)

  IF(SYMBOL_FOUND)
    CHECK_LINKED_LIBRARY(MYLIBRARY OTHERLIBRARY LIBRARY_FOUND)
  ENDIF()

  IF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
    SET(${MUSTLINK} YES)
  ELSE()
    SET(${MUSTLINK} NO)
  ENDIF()
ENDMACRO(CHECK_DEPENDS)

# LINK_DEPENDS
# Macro to link a library if a symbol is used but is not already linked to it
#
# Syntax:
# LINK_DEPENDS(LIBRARIES MYLIBRARY OTHERLIBRARY SYMBOL)
# OTHERLIBRARY_LINKED will be set to TRUE or FALSE
#
# Example:
# LINK_DEPENDS(PNG_LIBRARIES PNG_LIBRARY ZLIB_LIBRARY inflate)
#
MACRO(LINK_DEPENDS LIBRARIES MYLIBRARY OTHERLIBRARY SYMBOL)
  SET(MUST_LINK FALSE)
  IF(${MYLIBRARY} AND ${OTHERLIBRARY} AND NOT ${OTHERLIBRARY}_LINKED)
    IF(WIN32 OR WITH_STATIC)
      # In static, we link all libraries because it will keep only used symbols
      SET(MUST_LINK TRUE)
    ELSE()
      CHECK_UNDEFINED_SYMBOL(${MYLIBRARY} ${SYMBOL} SYMBOL_FOUND)

      IF(SYMBOL_FOUND)
        CHECK_LINKED_LIBRARY(${MYLIBRARY} ${OTHERLIBRARY} LIBRARY_FOUND)
      ENDIF()

      IF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
        MESSAGE(STATUS "Underlinking found: ${${MYLIBRARY}} needs ${${OTHERLIBRARY}} but is not linked to, manually linking...")
        SET(MUST_LINK TRUE)
      ENDIF()
    ENDIF()
  ENDIF()
  IF(MUST_LINK)
    MESSAGE(STATUS "Linking with ${${OTHERLIBRARY}}")
    SET(${LIBRARIES} ${${LIBRARIES}} ${${OTHERLIBRARY}})
    SET(${OTHERLIBRARY}_LINKED TRUE)
  ENDIF()
ENDMACRO(LINK_DEPENDS)

