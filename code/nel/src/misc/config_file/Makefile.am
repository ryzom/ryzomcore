#
#

MAINTAINERCLEANFILES    = Makefile.in

noinst_LTLIBRARIES      = libconfig.la

AM_YFLAGS = --defines=cf_gramatical.h -p cf

AM_LFLAGS = -f -8 -Pcf -olex.yy.c

EXTRA_DIST = cf_gramatical.h cf_bison.simple cf_flex.skl do.bat

libconfig_la_SOURCES    = cf_lexical.lpp             \
                          cf_gramatical.ypp          \
                          config_file.cpp 

#libconfig_la_SOURCES    = config_file.cpp 

# End of Makefile.am

