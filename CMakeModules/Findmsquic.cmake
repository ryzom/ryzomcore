IF(msquic_FIND_REQUIRED)
  SET(msquic_FIND_REQUIRED ON)
ENDIF()

FIND_PATH(
	msquic_INCLUDE_DIRS
	NAMES msquic.h
	PATHS /usr/local/include /usr/include
)

FIND_LIBRARY(
	msquic_LIBRARY_RELEASE
	NAMES libmsquic.so.2 msquic libmsquic
	PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib /opt/local
)

FIND_LIBRARY(
	msquic_LIBRARY_DEBUG
	NAMES libmsquicd.so.2 msquicd libmsquicd
	PATHS /usr/lib/x86_64-linux-gnu /usr/local/lib /usr/lib /opt/local
)

IF (msquic_INCLUDE_DIRS)
  IF(msquic_LIBRARY_RELEASE AND msquic_LIBRARY_DEBUG)
    SET(msquic_FOUND TRUE)
    SET(msquic_LIBRARIES ${msquic_LIBRARIES} optimized ${msquic_LIBRARY_RELEASE})
    SET(msquic_LIBRARIES ${msquic_LIBRARIES} debug ${msquic_LIBRARY_DEBUG})
  ELSEIF(msquic_LIBRARY_RELEASE)
    SET(msquic_FOUND TRUE)
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
