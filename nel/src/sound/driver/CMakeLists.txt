FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../../include/nel/sound/driver/*.h)

NL_TARGET_LIB(nelsnd_lowlevel ${HEADERS} ${SRC})
add_library(NeL::snd_lowlevel ALIAS nelsnd_lowlevel)
set_target_properties(nelsnd_lowlevel PROPERTIES EXPORT_NAME snd_lowlevel)

TARGET_LINK_LIBRARIES(nelsnd_lowlevel NeL::misc)
NL_DEFAULT_PROPS(nelsnd_lowlevel "NeL, Library: Sound Lowlevel")
NL_ADD_RUNTIME_FLAGS(nelsnd_lowlevel)
NL_ADD_STATIC_SND_DRIVERS(nelsnd_lowlevel)

NL_ADD_LIB_SUFFIX(nelsnd_lowlevel)

IF(WITH_PCH)
  target_precompile_headers(nelsnd_lowlevel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdsound_lowlevel.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nelsnd_lowlevel EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()

IF(WITH_DRIVER_OPENAL)
  ADD_SUBDIRECTORY(openal)
ENDIF()

IF(WITH_DRIVER_FMOD)
  ADD_SUBDIRECTORY(fmod)
ENDIF()

IF(WITH_DRIVER_DSOUND)
  ADD_SUBDIRECTORY(dsound)
ENDIF()

IF(WITH_DRIVER_XAUDIO2)
  ADD_SUBDIRECTORY(xaudio2)
ENDIF()
