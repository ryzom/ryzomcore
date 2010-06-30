FILE(GLOB SRC *.cpp *.h)

# TODO: fixes a linking problem for libnelsound.dylib, but does not look like
# the cleanest way to solve the issue. FIXME!
IF(APPLE)
  SET(SRC ${SRC} driver/sound_driver.cpp driver/buffer.cpp)
ENDIF(APPLE)

IF(NOT WIN32)
  ADD_LIBRARY(nelsound SHARED ${SRC})
  CONFIGURE_FILE(nel-sound.pc.in nel-sound.pc)
  INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/nel-sound.pc" DESTINATION lib/pkgconfig)
ELSE(NOT WIN32)
  ADD_LIBRARY(nelsound STATIC ${SRC})
ENDIF(NOT WIN32) 

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nelsound ${LIBXML2_LIBRARIES} nelligo nelgeorges nel3d)
SET_TARGET_PROPERTIES(nelsound PROPERTIES 
  VERSION ${NL_VERSION}
  SOVERSION ${NL_VERSION_MAJOR}
  PROJECT_LABEL "Library: NeL Sound")
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WIN32)
  SET_TARGET_PROPERTIES(nelsound PROPERTIES 
    DEBUG_POSTFIX "_d"
    RELEASE_POSTFIX "_r")
ENDIF(WIN32)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nelsound ${CMAKE_CURRENT_SOURCE_DIR}/stdsound.h ${CMAKE_CURRENT_SOURCE_DIR}/stdsound.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nelsound LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)

ADD_SUBDIRECTORY(driver)
