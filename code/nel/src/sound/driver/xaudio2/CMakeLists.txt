FILE(GLOB SRC *.cpp *.h *.def)

SOURCE_GROUP(sfx FILES
        buffer_xaudio2.cpp
        buffer_xaudio2.h
		listener_xaudio2.cpp
		listener_xaudio2.h
		source_xaudio2.cpp
		source_xaudio2.h)
		
SOURCE_GROUP(sfx\\utility FILES
        adpcm_xaudio2.cpp
		adpcm_xaudio2.h)

SOURCE_GROUP(music_remove FILES
        music_channel_xaudio2.cpp
		music_channel_xaudio2.h)
		
SOURCE_GROUP(music_remove\\utility FILES
        music_buffer.cpp
		music_buffer.h
		music_buffer_vorbis.cpp
		music_buffer_vorbis.h)

SOURCE_GROUP(efx FILES
        effect_xaudio2.cpp
        effect_xaudio2.h)
		
NL_TARGET_DRIVER(nel_drv_xaudio2_win ${SRC})

INCLUDE_DIRECTORIES(${DXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_drv_xaudio2_win nelmisc nelsnd_lowlevel ${DXSDK_XAUDIO_LIBRARY} ${DXSDK_GUID_LIBRARY})

NL_DEFAULT_PROPS(nel_drv_xaudio2_win "NeL, Driver, Sound: XAudio2")
NL_ADD_RUNTIME_FLAGS(nel_drv_xaudio2_win)
NL_ADD_LIB_SUFFIX(nel_drv_xaudio2_win)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_drv_xaudio2_win ${CMAKE_CURRENT_SOURCE_DIR}/stdxaudio2.h ${CMAKE_CURRENT_SOURCE_DIR}/stdxaudio2.cpp)
ENDIF(WITH_PCH)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC_DRIVERS) OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS nel_drv_xaudio2_win RUNTIME DESTINATION ${NL_DRIVER_PREFIX} LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT driverssound)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS nel_drv_xaudio2_win RUNTIME DESTINATION maxplugin COMPONENT driverssound)
  ENDIF(WITH_MAXPLUGIN)
ENDIF((WITH_INSTALL_LIBRARIES AND WITH_STATIC_DRIVERS) OR NOT WITH_STATIC_DRIVERS)
