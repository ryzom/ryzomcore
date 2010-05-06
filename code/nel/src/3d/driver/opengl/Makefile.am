#
#

MAINTAINERCLEANFILES		= Makefile.in

EXTRA_DIST			=	driver_opengl_8.vcproj	\
					driver_opengl.vcproj	\
					driver_opengl.dsp	\
					driver_opengl.def

lib_LTLIBRARIES			= libnel_drv_opengl.la

libnel_drv_opengl_la_SOURCES = driver_opengl.cpp                 \
                               driver_opengl.h                   \
                               driver_opengl_extension.cpp       \
                               driver_opengl_extension.h         \
                               driver_opengl_extension_def.h     \
                               driver_opengl_light.cpp           \
							   driver_opengl_mac.cpp             \
                               driver_opengl_material.cpp        \
                               driver_opengl_matrix.cpp          \
                               driver_opengl_states.cpp          \
                               driver_opengl_states.h            \
                               driver_opengl_texture.cpp         \
                               driver_opengl_vertex.cpp          \
                               driver_opengl_vertex_buffer_hard.cpp \
                               driver_opengl_vertex_buffer_hard.h \
                               driver_opengl_vertex_program.cpp  \
                               stdopengl.cpp                     \
                               stdopengl.h                       \
                               unix_event_emitter.cpp            \
                               unix_event_emitter.h


noinst_HEADERS		     = stdopengl.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src \
                               @OPENGL_CFLAGS@ @XF86VIDMODE_CFLAGS@

libnel_drv_opengl_la_LIBADD  = @OPENGL_LIBS@ @XF86VIDMODE_LIBS@ -lXmu
libnel_drv_opengl_la_LDFLAGS = -no-undefined

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nel-driverogl.pc

# End of Makefile.am

