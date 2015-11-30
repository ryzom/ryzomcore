FIND_PATH(
	assimp_INCLUDE_DIRS
	NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
	PATHS /usr/local/include/
)

FIND_LIBRARY(
	assimp_LIBRARIES
	NAMES assimp
	PATHS /usr/local/lib/
)

IF (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
    SET(assimp_FOUND TRUE)
    FIND_PACKAGE(ZLIB)
    IF(ZLIB_FOUND)
        SET(assimp_LIBRARIES ${assimp_LIBRARIES} ${ZLIB_LIBRARIES})
    ENDIF()
ENDIF (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)

IF (assimp_FOUND)
    IF (NOT assimp_FIND_QUIETLY)
        MESSAGE(STATUS "Found asset importer library: ${assimp_LIBRARIES}")
    ENDIF (NOT assimp_FIND_QUIETLY)
ELSE (assimp_FOUND)
    IF (assimp_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find asset importer library")
    ENDIF (assimp_FIND_REQUIRED)
ENDIF (assimp_FOUND)
