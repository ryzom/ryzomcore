FILE(GLOB SRC *.cpp *.h)

# Filter out the source files not actually compiled.
#LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/Backup\ 1\ of\ admin_modules.cpp)

IF(WIN32)
  ADD_LIBRARY(gameplay_module_lib STATIC ${SRC})
ELSE(WIN32)
  ADD_LIBRARY(gameplay_module_lib SHARED ${SRC})
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(gameplay_module_lib ${NELMISC_LIBRARY})
SET_TARGET_PROPERTIES(gameplay_module_lib PROPERTIES VERSION ${NL_VERSION})

IF(WIN32)
  SET_TARGET_PROPERTIES(gameplay_module_lib 
		PROJECT_LABEL "Library: Gameplay Module"
		DEBUG_POSTFIX "_d"
		RELEASE_POSTFIX "_r"
		LINK_FLAGS_DEBUG "/NODEFAULTLIB:libc;libcmt;libcmtd;msvcrt"
		LINK_FLAGS_RELEASE "/NODEFAULTLIB:libc;libcmt;libcmtd;msvcrtd")
ENDIF(WIN32)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS gameplay_module_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
