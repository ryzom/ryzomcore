#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = xml_packer.dsp

bin_PROGRAMS              = xml_packer

xml_packer_SOURCES = xml_packer.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

xml_packer_LDADD   =	../../../src/misc/libnelmisc.la

# End of Makefile.am

