IF(msquic_FIND_REQUIRED)
  SET(msquic_FIND_REQUIRED ON)
ENDIF()

FIND_PATH(
	msquic_INCLUDE_DIRS
	NAMES msquic.h
	PATHS /usr/local/include/
)

FIND_LIBRARY(
	msquic_LIBRARY_RELEASE
	NAMES msquic
	PATHS /usr/local/lib/
)

FIND_LIBRARY(
	msquic_LIBRARY_DEBUG
	NAMES msquicd
	PATHS /usr/local/lib/
)

IF (msquic_INCLUDE_DIRS)
    SET(msquic_FOUND TRUE)
    IF(msquic_LIBRARY_RELEASE AND msquic_LIBRARY_DEBUG)
      SET(msquic_LIBRARIES ${msquic_LIBRARIES} optimized ${msquic_LIBRARY_RELEASE})
      SET(msquic_LIBRARIES ${msquic_LIBRARIES} debug ${msquic_LIBRARY_DEBUG})
    ENDIF()
    IF(msquic_LIBRARY_RELEASE)
      SET(msquic_LIBRARIES ${msquic_LIBRARIES} ${msquic_LIBRARY_RELEASE})
    ENDIF()
ENDIF()

IF (msquic_FOUND)
    IF (NOT msquic_FIND_QUIETLY)
        MESSAGE(STATUS "Found msquic library: ${msquic_LIBRARIES}")
    ENDIF ()
ELSE (msquic_FOUND)
    IF (msquic_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find msquic library")
    ENDIF ()
ENDIF ()
