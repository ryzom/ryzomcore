CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

# ROOT_DIR should be set to root of the repository (where to find the .svn or .hg directory)
# SOURCE_DIR should be set to root of your code (where to find CMakeLists.txt)

SET(CMAKE_MODULE_PATH "${SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

MACRO(NOW RESULT)
  IF (WIN32)
    EXECUTE_PROCESS(COMMAND "wmic" "os" "get" "localdatetime" OUTPUT_VARIABLE DATETIME)
    STRING(REGEX REPLACE ".*\n([0-9][0-9][0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9]).*" "\\1-\\2-\\3 \\4:\\5:\\6" ${RESULT} ${DATETIME})
  ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" "+'%Y-%m-%d %H:%M:%S'" OUTPUT_VARIABLE ${RESULT})
  ELSE (WIN32)
    MESSAGE(SEND_ERROR "date not implemented")
    SET(${RESULT} 000000)
  ENDIF (WIN32)
ENDMACRO(NOW)

IF(EXISTS "${ROOT_DIR}/.svn/")
  FIND_PACKAGE(Subversion)

  IF(SUBVERSION_FOUND)
    Subversion_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
  ENDIF(SUBVERSION_FOUND)
ENDIF(EXISTS "${ROOT_DIR}/.svn/")

IF(EXISTS "${ROOT_DIR}/.hg/")
  FIND_PACKAGE(Mercurial)

  IF(MERCURIAL_FOUND)
    Mercurial_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
  ENDIF(MERCURIAL_FOUND)
ENDIF(EXISTS "${ROOT_DIR}/.hg/")

IF(REVISION)
  IF(EXISTS ${SOURCE_DIR}/revision.h.in)
    NOW(BUILD_DATE)
    CONFIGURE_FILE(${SOURCE_DIR}/revision.h.in revision.h.txt)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy revision.h.txt revision.h) # copy_if_different
  ENDIF(EXISTS ${SOURCE_DIR}/revision.h.in)
ENDIF(REVISION)
