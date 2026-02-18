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
      execute_process(COMMAND ${CMAKE_NM} -gu "${${MYLIBRARY}}" OUTPUT_VARIABLE NM_SYMBOL ERROR_QUIET)
      IF(NOT NM_SYMBOL MATCHES "${SYMBOL}")
        SET(${SYMBOL_FOUND} FALSE)
      ENDIF()
    ENDIF()
  ELSEIF(UNIX)
    SET(CMAKE_OBJDUMP objdump)
    IF(CMAKE_OBJDUMP)
      # Use objdump to check if a library is using an external symbol
      execute_process(COMMAND ${CMAKE_OBJDUMP} -T "${${MYLIBRARY}}" OUTPUT_VARIABLE OBJDUMP_OUTPUT ERROR_QUIET)
      string(REGEX MATCH "[^\n]*${SYMBOL}[^\n]*" OBJDUMP_SYMBOL "${OBJDUMP_OUTPUT}")
      IF(NOT OBJDUMP_SYMBOL MATCHES "UND")
        SET(${SYMBOL_FOUND} FALSE)
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
      execute_process(COMMAND ${CMAKE_OTOOL} -L "${${MYLIBRARY}}" OUTPUT_VARIABLE OTOOL_LIBRARY ERROR_QUIET)
      IF(OTOOL_LIBRARY MATCHES "${LIBNAME}")
        SET(${LIBRARY_FOUND} TRUE)
      ENDIF()
    ENDIF()
  ELSEIF(UNIX)
    SET(CMAKE_OBJDUMP objdump)
    IF(CMAKE_OBJDUMP)
      GET_FILENAME_COMPONENT(LIBNAME "${${OTHERLIBRARY}}" NAME)
      # Use objdump to check if a library is linked to another library
      execute_process(COMMAND ${CMAKE_OBJDUMP} -p "${${MYLIBRARY}}" OUTPUT_VARIABLE OBJDUMP_OUTPUT ERROR_QUIET)
      string(REGEX MATCH "[^\n]*${LIBNAME}[^\n]*" OBJDUMP_LIBRARY "${OBJDUMP_OUTPUT}")
      IF(OBJDUMP_LIBRARY MATCHES "NEEDED")
        SET(${LIBRARY_FOUND} TRUE)
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

