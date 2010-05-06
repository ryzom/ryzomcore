#
#

MAINTAINERCLEANFILES     = Makefile.in

EXTRA_DIST		= georges_file_format.txt

lib_LTLIBRARIES          = libnelgeorges.la

libnelgeorges_la_SOURCES = form.cpp                        \
			   form.h                          \
                           form_loader.cpp                 \
                           form_loader.h                   \
			   form_dfn.cpp                    \
			   form_dfn.h                      \
			   form_elm.cpp                    \
			   form_elm.h                      \
                           stdgeorges.cpp                  \
                           stdgeorges.h                    \
			   header.cpp                      \
			   header.h                        \
			   load_form.cpp                   \
			   type.cpp                        \
			   type.h  

AM_CXXFLAGS         	 = -I$(top_srcdir)/src

libnelgeorges_la_LIBADD  = -lc -lpthread

libnelgeorges_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

