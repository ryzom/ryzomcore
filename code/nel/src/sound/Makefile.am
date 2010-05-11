#
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver

lib_LTLIBRARIES      = libnelsnd.la

libnelsnd_la_SOURCES = async_file_manager_sound.cpp \
	async_file_manager_sound.h \
	audio_mixer_user.cpp \
	audio_mixer_user.h \
	background_sound.cpp \
	background_sound.h \
	background_sound_manager.cpp \
	background_sound_manager.h \
	background_source.cpp \
	background_source.h \
	clustered_sound.cpp \
	clustered_sound.h \
	complex_sound.cpp \
	complex_sound.h \
	complex_source.cpp \
	complex_source.h \
	context_sound.cpp \
	context_sound.h \
	listener_user.cpp \
	listener_user.h \
	mixing_track.cpp \
	mixing_track.h \
	music_channel_fader.h \
	music_channel_fader.cpp \
	music_sound.cpp \
	music_sound.h \
	music_sound_manager.cpp \
	music_sound_manager.h \
	music_source.cpp \
	music_source.h \
	sample_bank.cpp \
	sample_bank.h \
	sample_bank_manager.cpp \
	sample_bank_manager.h \
	simple_sound.cpp \
	simple_sound.h \
	simple_source.cpp \
	simple_source.h \
	sound_animation.cpp \
	sound_anim_manager.cpp \
	sound_anim_marker.cpp \
	sound_bank.cpp \
	sound_bank.h \
	sound.cpp \
	sound.h \
	sound_pattern.h \
	source_common.cpp \
	source_common.h \
	stdsound.cpp \
	stdsound.h \
	stream_source.cpp \
	stream_source.h \
	stream_sound.cpp \
	stream_sound.h

AM_CXXFLAGS          = -I$(top_srcdir)/src

noinst_HEADERS	     = stdsound.h

libnelsnd_la_LIBADD  = driver/libnelsnd_lowlevel.la

libnelsnd_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

