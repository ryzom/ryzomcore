FILE(GLOB SRC *.cpp *.h *.def *.rc)

SOURCE_GROUP("" FILES
		sound_driver_fmod.cpp
		sound_driver_fmod.h
		stdfmod.cpp
		stdfmod.h
		driver_fmod.def
		resources.rc)

SOURCE_GROUP(sfx FILES
		buffer_fmod.cpp
		buffer_fmod.h
		listener_fmod.cpp
		listener_fmod.h
		source_fmod.cpp
		source_fmod.h)

SOURCE_GROUP(music FILES
		music_channel_fmod.cpp
		music_channel_fmod.h)

NL_TARGET_DRIVER(nel_drv_fmod_win ${SRC})

TARGET_INCLUDE_DIRECTORIES(nel_drv_fmod_win PRIVATE ${FMOD_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_drv_fmod_win ${FMOD_LIBRARY} nelmisc nelsnd_lowlevel)

NL_DEFAULT_PROPS(nel_drv_fmod_win "NeL, Driver, Sound: FMOD")
NL_ADD_RUNTIME_FLAGS(nel_drv_fmod_win)
NL_ADD_LIB_SUFFIX(nel_drv_fmod_win)

IF(WITH_PCH)
  target_precompile_headers(nel_drv_fmod_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdfmod.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS nel_drv_fmod_win RUNTIME DESTINATION ${NL_DRIVER_PREFIX} LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT driverssound)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS nel_drv_fmod_win RUNTIME DESTINATION maxplugin COMPONENT driverssound)
  ENDIF()
ENDIF()
