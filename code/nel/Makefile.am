#
#

MAINTAINERCLEANFILES	=	Makefile.in		\
				configure		\
				libtool			\
				config.guess		\
				config.sub		\
				ltconfig		\
				aclocal.m4		\
				config.h.in		\
				install-sh		\
				missing			\
				mkinstalldirs		\
				ltmain.sh		\
				include/nelconfig.h	\
				include/nelconfig.h.in	\
				include/nel/nelconfig.h

DISTCLEANFILES		=	include/stamp-h		\
				include/stamp-h.in 

SUBDIRS			= 	include src @TOOLS_SUBDIR@ @SAMPLE_SUBDIR@

bin_SCRIPTS		=	nel-config

EXTRA_DIST		=	nel.dsw			\
				nel.sln			\
				nel_8.sln		\
				autogen.sh		\
				nel.m4			\
				automacros		\
				doc			\
				kdevelop		\
				tools			\
				samples

dist-hook:
	find $(distdir) -name CVS -print | xargs rm -fr
	find $(distdir) -name .svn -print | xargs rm -fr

m4datadir = $(datadir)/aclocal
m4data_DATA = nel.m4

# End of Makefile.am

