#
#

MAINTAINERCLEANFILES     = Makefile.in

lib_LTLIBRARIES          = libnelligo.la

libnelligo_la_SOURCES = ligo_config.cpp                        \
                        ligo_error.cpp                 \
                        ligo_error.h                 \
			ligo_material.cpp                    \
			ligo_material.h                    \
			primitive.cpp                    \
			primitive_class.cpp                    \
			primitive_configuration.cpp		\
                        transition.cpp                  \
                        transition.h                  \
                        zone_bank.cpp                    \
                        zone_bank.h                    \
			zone_edge.cpp                      \
			zone_edge.h                      \
			zone_region.cpp                   \
			zone_region.h                   \
			zone_template.cpp		\
			zone_template.h		\
			primitive_utils.cpp

AM_CXXFLAGS         	 = -I$(top_srcdir)/src

libnelligo_la_LIBADD  = -lc -lpthread

libnelligo_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

