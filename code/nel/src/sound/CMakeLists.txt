FILE(GLOB SRC *.cpp *.h)

# TODO: fixes a linking problem for libnelsound.dylib, but does not look like
# the cleanest way to solve the issue. FIXME!
IF(APPLE)
  SET(SRC ${SRC} driver/sound_driver.cpp driver/buffer.cpp)
ENDIF(APPLE)

nl_target_lib(nelsound ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nelsound ${LIBXML2_LIBRARIES} nelligo nelgeorges nel3d)
nl_default_props(nelsound "Library: NeL Sound")
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

nl_add_lib_suffix(nelsound)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nelsound ${CMAKE_CURRENT_SOURCE_DIR}/stdsound.h ${CMAKE_CURRENT_SOURCE_DIR}/stdsound.cpp)
ENDIF(WITH_PCH)

nl_gen_pc(nel-sound.pc)
INSTALL(TARGETS nelsound LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)

ADD_SUBDIRECTORY(driver)
