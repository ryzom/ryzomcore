CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

# ROOT_DIR should be set to root of the repository (where to find the .svn or .hg directory)
# SOURCE_DIR should be set to root of your code (where to find CMakeLists.txt)

IF(SOURCE_DIR)
  # Replace spaces by semi-columns
  IF(CMAKE_MODULE_PATH)
    STRING(REPLACE " " ";" CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
  ENDIF(CMAKE_MODULE_PATH)

  SET(CMAKE_MODULE_PATH ${SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

  IF(NOT ROOT_DIR AND SOURCE_DIR)
    SET(ROOT_DIR ${SOURCE_DIR})
  ENDIF()

  IF(NOT SOURCE_DIR AND ROOT_DIR)
    SET(SOURCE_DIR ${ROOT_DIR})
  ENDIF()
ELSE()
  SET(SOURCE_DIR ${CMAKE_SOURCE_DIR})
  SET(ROOT_DIR ${CMAKE_SOURCE_DIR})
ENDIF()

MACRO(NOW RESULT)
  IF (WIN32)
    EXECUTE_PROCESS(COMMAND "wmic" "os" "get" "localdatetime" OUTPUT_VARIABLE DATETIME)
    IF(NOT DATETIME MATCHES "ERROR")
      STRING(REGEX REPLACE ".*\n([0-9][0-9][0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9])([0-9][0-9]).*" "\\1-\\2-\\3 \\4:\\5:\\6" ${RESULT} "${DATETIME}")
    ENDIF()
  ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE DATETIME)
    STRING(REGEX REPLACE "([0-9: -]+).*" "\\1" ${RESULT} "${DATETIME}")
  ELSE()
    MESSAGE(SEND_ERROR "date not implemented")
    SET(${RESULT} "0000-00-00 00:00:00")
  ENDIF()
ENDMACRO(NOW)

IF(EXISTS "${ROOT_DIR}/.svn/")
  FIND_PACKAGE(Subversion QUIET)

  IF(SUBVERSION_FOUND)
    Subversion_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
  ENDIF(SUBVERSION_FOUND)

  FIND_PACKAGE(TortoiseSVN QUIET)

  IF(TORTOISESVN_FOUND)
    TORTOISESVN_GET_REVISION(${ROOT_DIR} REVISION)
  ENDIF(TORTOISESVN_FOUND)
ENDIF(EXISTS "${ROOT_DIR}/.svn/")

IF(EXISTS "${ROOT_DIR}/.hg/")
  FIND_PACKAGE(Mercurial)

  IF(MERCURIAL_FOUND)
    Mercurial_WC_INFO(${ROOT_DIR} ER)
    SET(REVISION ${ER_WC_REVISION})
    SET(CHANGESET ${ER_WC_CHANGESET})
    SET(BRANCH ${ER_WC_BRANCH})
  ENDIF()
ENDIF()

# if processing exported sources, use "revision" file if exists
IF(SOURCE_DIR AND NOT DEFINED REVISION)
  SET(REVISION_FILE ${SOURCE_DIR}/revision)
  IF(EXISTS ${REVISION_FILE})
    FILE(STRINGS ${REVISION_FILE} REVISION LIMIT_COUNT 1)
    MESSAGE(STATUS "Read revision ${REVISION} from file")
  ENDIF()
ENDIF()

IF(SOURCE_DIR AND DEFINED REVISION)
  IF(EXISTS ${SOURCE_DIR}/revision.h.in)
    MESSAGE(STATUS "Revision: ${REVISION}")
    NOW(BUILD_DATE)
    CONFIGURE_FILE(${SOURCE_DIR}/revision.h.in revision.h.txt)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy revision.h.txt revision.h) # copy_if_different
  ENDIF()
ENDIF()
