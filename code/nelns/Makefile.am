#
# $Id: Makefile.am,v 1.9 2002/10/21 10:19:03 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs depcomp $(DISTCLEANFILES)

DISTCLEANFILES       = stamp-h.in config.cache config.log config.status 

EXTRA_DIST	     = acconfig.h services.dsw bootstrap doc

SUBDIRS              = login_service          \
                       naming_service         \
                       admin_executor_service \
                       admin_service          \
                       welcome_service

dist-hook:
	find $(distdir) -name CVS -print | xargs rm -fr
	find $(distdir) -name .svn -print | xargs rm -fr

# End of Makefile.am
