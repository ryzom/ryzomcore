FILE(GLOB SRC *.cpp *.h)

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
		
NL_TARGET_DRIVER(nel_drv_xaudio2 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${DXSDK_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR} ${OGG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_drv_xaudio2 ${LIBXML2_LIBRARIES} nelsnd_lowlevel ${DXSDK_XAUDIO_LIBRARY} ${DXSDK_GUID_LIBRARY} ${VORBIS_LIBRARIES} ${OGG_LIBRARY})

NL_DEFAULT_PROPS(nel_drv_xaudio2 "Driver, Sound: XAudio2")
NL_ADD_LIB_SUFFIX(nel_drv_xaudio2)
NL_ADD_RUNTIME_FLAGS(nel_drv_xaudio2)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_drv_xaudio2 ${CMAKE_CURRENT_SOURCE_DIR}/stdxaudio2.h ${CMAKE_CURRENT_SOURCE_DIR}/stdxaudio2.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nel_drv_xaudio2 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT driverssound)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS nel_drv_xaudio2 RUNTIME DESTINATION maxplugin COMPONENT driverssound)
ENDIF(WITH_MAXPLUGIN)
