# Script mode (cmake -P): writes OUTPUT with the build date and, when
# available, the DESCRIBE version string. Run at build time rather than at
# configure time so the values stay current without regenerating the project
# (a regenerated Xcode project forces a full rebuild).
#
# Inputs: SOURCE_DIR, OUTPUT, DESCRIBE (optional, computed from git if empty)

STRING(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

IF(NOT DESCRIBE)
  FIND_PACKAGE(Git QUIET)

  IF(GIT_FOUND AND EXISTS "${SOURCE_DIR}/.git")
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count .
      WORKING_DIRECTORY "${SOURCE_DIR}/ryzom/client/src"
      OUTPUT_VARIABLE _REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list --abbrev-commit HEAD -n 1
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE _COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      WORKING_DIRECTORY "${SOURCE_DIR}"
      OUTPUT_VARIABLE _BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(_BRANCH STREQUAL "main/yubo-dev")
      SET(_DOMAIN "Alpha /")
    ELSEIF(_BRANCH STREQUAL "main/gingo-test")
      SET(_DOMAIN "Beta /")
    ELSE()
      SET(_DOMAIN "Omega /")
    ENDIF()

    STRING(TIMESTAMP _YEAR "%y")
    STRING(TIMESTAMP _MONTH "%m")
    SET(DESCRIBE "${_DOMAIN} v${_YEAR}.${_MONTH}.${_REVISION} #${_COMMIT}")
  ENDIF()
ENDIF()

SET(_CONTENT "#ifndef RYZOM_BUILD_INFO_H\n#define RYZOM_BUILD_INFO_H\n\n")
IF(DESCRIBE)
  SET(_CONTENT "${_CONTENT}#undef RYZOM_VERSION\n#define RYZOM_VERSION \"${DESCRIBE}\"\n\n")
ENDIF()
SET(_CONTENT "${_CONTENT}#undef BUILD_DATE\n#define BUILD_DATE \"${BUILD_DATE}\"\n\n#endif // RYZOM_BUILD_INFO_H\n")

FILE(WRITE "${OUTPUT}" "${_CONTENT}")
