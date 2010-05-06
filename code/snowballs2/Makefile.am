MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs

EXTRA_DIST	     = autogen.sh \
		       snowballs2.dsw \
		       snowballs2.sln \
		       snowballs2_8.sln \
		       kdevelop

DIST_SUBDIRS	     = client server

dist-hook:
	find $(distdir) -name CVS -print | xargs rm -fr
	find $(distdir) -name .svn -print | xargs rm -fr

SUBDIRS              = @SNO_SUBDIRS@

# End of Makefile.am

