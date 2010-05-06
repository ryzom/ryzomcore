#
# $Id: Makefile.am,v 1.7 2002-03-19 17:42:49 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

sbin_PROGRAMS        = sno_position

sno_position_SOURCES = main.cpp      \
                       physics.h     \
		       physics.cpp

AM_CXXFLAGS		=	-DSNOWBALLS_CONFIG="\"${pkgsysconfdir}\"" -DSNOWBALLS_LOGS="\"${logdir}\""

# End of Makefile.am

