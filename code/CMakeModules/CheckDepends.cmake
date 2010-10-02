# Macros to check if a library needs to be manually linked to another one
# because it's using a symbol from it but it's not linked to
#
# Syntax:
# CHECK_DEPENDS(MYLIBRARY OTHERLIBRARY SYMBOL MUSTLINK)
#
# Sets the following variables:
#   XYZ_LIBRARY_MUSTLINK
#
# Example:
# Check if we need to explicitly link to libz with libpng
# CHECK_DEPENDS(PNG_LIBRARY ZLIB_LIBRARY inflate)
# objdump -p /usr/lib/libwwwxml.so | grep expat
# objdump -T /usr/lib/libwwwxml.so | grep "*UND*" | grep XML_ParserCreate
# objdump -p <lib> | grep <libray>
# objdump -T <lib> | grep "*UND*" | grep <function>
#

# CHECK_UNDEFINED_SYMBOL
# Macro to check if a library is calling an undefined symbol
#
# Syntax:
# CHECK_UNDEFINED_SYMBOL(MYLIBRARY SYMBOL SYMBOL_FOUND)
# SYMBOL_FOUND will be set to TRUE or FALSE
#
# Example:
# CHECK_UNDEFINED_SYMBOL(PNG_LIBRARY inflate INFLATE_FOUND)
#
MACRO(CHECK_UNDEFINED_SYMBOL MYLIBRARY SYMBOL SYMBOL_FOUND)
  SET(${SYMBOL_FOUND} TRUE)
  IF(WIN32)
    # Always TRUE under Windows because we are using static libraries
  ELSEIF(APPLE)
    # TODO: use otool to detect if a library is using a symbol
  ELSEIF(UNIX)
    IF(CMAKE_OBJDUMP)
      # Use objdump to check if a library is using an external symbol
      EXEC_PROGRAM(${CMAKE_OBJDUMP} ARGS "-T ${MYLIBRARY} | grep ${SYMBOL}" OUTPUT_VARIABLE OBJDUMP_SYMBOL)
      message(STATUS "OBJDUMP_SYMBOL = ${OBJDUMP_SYMBOL}")
      IF(NOT OBJDUMP_SYMBOL MATCHES "UND")
        SET(${SYMBOL_FOUND} FALSE)
      ENDIF(OBJDUMP_SYMBOL MATCHES "UND")
    ENDIF(CMAKE_OBJDUMP)
  ENDIF(WIN32)
ENDMACRO(CHECK_UNDEFINED_SYMBOL)

MACRO(CHECK_LINKED_LIBRARY MYLIBRARY OTHERLIBRARY LIBRARY_FOUND)
  SET(${LIBRARY_FOUND} FALSE)
  IF(WIN32)
    # Always FALSE under Windows because we are using static libraries
  ELSEIF(APPLE)
    # TODO: use otool to detect if a library is using a symbol
  ELSEIF(UNIX)
    IF(CMAKE_OBJDUMP)
      # Use objdump to check if a library is linked to another library
      EXEC_PROGRAM(${CMAKE_OBJDUMP} ARGS "-p ${MYLIBRARY} | grep ${OTHERLIBRARY}" OUTPUT_VARIABLE OBJDUMP_LIBRARY)
      IF(OBJDUMP_LIBRARY MATCHES "NEEDED")
        SET(${LIBRARY_FOUND} TRUE)
      ENDIF(OBJDUMP_LIBRARY MATCHES "NEEDED")
    ENDIF(CMAKE_OBJDUMP)
  ENDIF(WIN32)
ENDMACRO(CHECK_LINKED_LIBRARY)

MACRO(CHECK_DEPENDS MYLIBRARY OTHERLIBRARY SYMBOL MUSTLINK)
  CHECK_UNDEFINED_SYMBOL(MYLIBRARY SYMBOL SYMBOL_FOUND)

  IF(SYMBOL_FOUND)
    CHECK_LINKED_LIBRARY(MYLIBRARY OTHERLIBRARY LIBRARY_FOUND)
  ENDIF(SYMBOL_FOUND)

  IF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
    SET(${MUSTLINK} YES)
  ELSE(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
    SET(${MUSTLINK} NO)
  ENDIF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
ENDMACRO(CHECK_DEPENDS)

MACRO(LINK_DEPENDS LIBRARIES MYLIBRARY OTHERLIBRARY SYMBOL)
  SET(MUST_LINK FALSE)
  IF(${MYLIBRARY} AND ${OTHERLIBRARY} AND NOT ${OTHERLIBRARY}_LINKED)
    IF(WIN32 OR WITH_STATIC)
      # In static, we link all libraries because it will keep only used symbols
      SET(MUST_LINK TRUE)
    ELSE(WIN32 OR WITH_STATIC)
      CHECK_UNDEFINED_SYMBOL(${MYLIBRARY} ${SYMBOL} SYMBOL_FOUND)

      IF(SYMBOL_FOUND)
        CHECK_LINKED_LIBRARY(${MYLIBRARY} ${OTHERLIBRARY} LIBRARY_FOUND)
      ENDIF(SYMBOL_FOUND)

#      CHECK_DEPENDS(${MYLIBRARY} ${${OTHERLIBRARY}} ${SYMBOL} MUST_LINK)
      IF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
        MESSAGE(STATUS "Underlinking found: ${${MYLIBRARY}} needs ${${OTHERLIBRARY}} but is not linked to, manually linking...")
        SET(MUST_LINK TRUE)
      ENDIF(SYMBOL_FOUND AND NOT LIBRARY_FOUND)
    ENDIF(WIN32 OR WITH_STATIC)
  ENDIF(${MYLIBRARY} AND ${OTHERLIBRARY} AND NOT ${OTHERLIBRARY}_LINKED)
  IF(MUST_LINK)
    MESSAGE(STATUS "Linking with ${${OTHERLIBRARY}}")
    SET(${OTHERLIBRARY}_LINKED TRUE)
    SET(${LIBRARIES} ${${LIBRARIES}} ${${OTHERLIBRARY}})
  ENDIF(MUST_LINK)
ENDMACRO(LINK_DEPENDS)
