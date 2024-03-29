FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/sound/*.h)

IF(NOT FFMPEG_FOUND)
  LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/audio_decoder_ffmpeg.cpp)
  LIST(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../../include/nel/sound/audio_decoder_ffmpeg.h)
ENDIF()

FILE(GLOB ANIMATION
	sound_anim_manager.cpp ../../include/nel/sound/sound_anim_manager.h
	sound_anim_marker.cpp ../../include/nel/sound/sound_anim_marker.h
	sound_animation.cpp ../../include/nel/sound/sound_animation.h
)

FILE(GLOB BACKGROUND_SOUND
	background_sound.cpp ../../include/nel/sound/background_sound.h
	background_sound_manager.cpp ../../include/nel/sound/background_sound_manager.h
	background_source.cpp ../../include/nel/sound/background_source.h
	clustered_sound.cpp ../../include/nel/sound/clustered_sound.h
	context_sound.cpp ../../include/nel/sound/context_sound.h
)

FILE(GLOB BANKS
	async_file_manager_sound.cpp ../../include/nel/sound/async_file_manager_sound.h
	sample_bank.cpp ../../include/nel/sound/sample_bank.h
	sample_bank_manager.cpp ../../include/nel/sound/sample_bank_manager.h
	sound_bank.cpp ../../include/nel/sound/sound_bank.h
)

FILE(GLOB MIXER
	audio_mixer_user.cpp ../../include/nel/sound/audio_mixer_user.h
	../../include/nel/sound/containers.h
	group_controller.cpp ../../include/nel/sound/group_controller.h
	group_controller_root.cpp ../../include/nel/sound/group_controller_root.h
	listener_user.cpp ../../include/nel/sound/listener_user.h
	mixing_track.cpp ../../include/nel/sound/mixing_track.h
)

FILE(GLOB MUSIC
	music_channel_fader.cpp ../../include/nel/sound/music_channel_fader.h
	music_sound.cpp ../../include/nel/sound/music_sound.h
	music_sound_manager.cpp ../../include/nel/sound/music_sound_manager.h
	music_source.cpp ../../include/nel/sound/music_source.h
	source_music_channel.cpp ../../include/nel/sound/source_music_channel.h
)

FILE(GLOB SOUND
	complex_sound.cpp ../../include/nel/sound/complex_sound.h
	complex_source.cpp ../../include/nel/sound/complex_source.h
	simple_sound.cpp ../../include/nel/sound/simple_sound.h
	simple_source.cpp ../../include/nel/sound/simple_source.h
	sound.cpp ../../include/nel/sound/sound.h
	../../include/nel/sound/sound_pattern.h
	source_common.cpp ../../include/nel/sound/source_common.h
)

FILE(GLOB STREAM
	stream_sound.cpp ../../include/nel/sound/stream_sound.h
	stream_source.cpp ../../include/nel/sound/stream_source.h
)

FILE(GLOB STREAM_FILE
	audio_decoder.cpp ../../include/nel/sound/audio_decoder.h
	audio_decoder_vorbis.cpp ../../include/nel/sound/audio_decoder_vorbis.h
	audio_decoder_mp3.cpp ../../include/nel/sound/audio_decoder_mp3.h
	audio_decoder_ffmpeg.cpp ../../include/nel/sound/audio_decoder_ffmpeg.h
	stream_file_sound.cpp ../../include/nel/sound/stream_file_sound.h
	stream_file_source.cpp ../../include/nel/sound/stream_file_source.h
)

FILE(GLOB USER_CLASSES
	../../include/nel/sound/u_audio_mixer.h
	../../include/nel/sound/u_group_controller.h
	../../include/nel/sound/u_listener.h
	../../include/nel/sound/u_source.h
	../../include/nel/sound/u_stream_source.h
)

SOURCE_GROUP("" FILES ${SRC} ${HEADERS})
SOURCE_GROUP("animation" FILES ${ANIMATION})
SOURCE_GROUP("background_sound" FILES ${BACKGROUND_SOUND})
SOURCE_GROUP("banks" FILES ${BANKS})
SOURCE_GROUP("mixer" FILES ${MIXER})
SOURCE_GROUP("music_deprecated" FILES ${MUSIC})
SOURCE_GROUP("sound" FILES ${SOUND})
SOURCE_GROUP("stream" FILES ${STREAM})
SOURCE_GROUP("stream_file" FILES ${STREAM_FILE})
SOURCE_GROUP("user_classes" FILES ${USER_CLASSES})


NL_TARGET_LIB(nelsound ${HEADERS} ${SRC})

IF(HUNTER_ENABLED)
  HUNTER_ADD_PACKAGE(libogg)
  FIND_PACKAGE(libogg CONFIG REQUIRED)
  SET(OGG_LIBRARY libogg::ogg)
  SET(OGG_LIBRARIES libogg::ogg)

  HUNTER_ADD_PACKAGE(vorbis)
  FIND_PACKAGE(vorbis CONFIG REQUIRED)
  SET(VORBIS_LIBRARY vorbis::vorbis)
  SET(VORBISFILE_LIBRARY vorbis::vorbisfile)

  IF(WITH_FFMPEG)
    HUNTER_ADD_PACKAGE(ffmpeg)
    FIND_PACKAGE(ffmpeg CONFIG REQUIRED)
    SET(FFMPEG_LIBRARIES ffmpeg::avcodec ffmpeg::avformat ffmpeg::avutil ffmpeg::swresample)
  ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR}/vorbis)
INCLUDE_DIRECTORIES(${OGG_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nelsound ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY})

IF(WITH_STATIC OR ${VORBISFILE_LIBRARY} MATCHES "\\.a")
  # Add libogg dependency only if target is static because to libvorbisfile
  TARGET_LINK_LIBRARIES(nelsound ${OGG_LIBRARY})
ENDIF()

IF(FFMPEG_FOUND)
	ADD_DEFINITIONS(-DFFMPEG_ENABLED)
	INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(nelsound ${FFMPEG_LIBRARIES})
ENDIF()


INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nelsound nelmisc nelligo nelgeorges nel3d nelsnd_lowlevel)
NL_DEFAULT_PROPS(nelsound "NeL, Library: NeL Sound")
NL_ADD_RUNTIME_FLAGS(nelsound)

NL_ADD_LIB_SUFFIX(nelsound)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(nelsound PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdsound.h)
ENDIF()

NL_GEN_PC(nel-sound.pc)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS nelsound LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()

ADD_SUBDIRECTORY(driver)
