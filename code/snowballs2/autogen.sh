#!/bin/sh -

WANT_AUTOMAKE="1.6"

case `uname -s` in
Darwin)
	LIBTOOLIZE=glibtoolize
	;;
*)
	LIBTOOLIZE=libtoolize
	;;
esac

echo "Creating macros..." && \
aclocal && \
echo "Creating library tools..." && \
$LIBTOOLIZE --force && \
echo "Creating header templates..." && \
autoheader && \
echo "Creating Makefile templates..." && \
automake --gnu --add-missing && \
echo "Creating 'configure'..." && \
autoconf && \
echo "" && \
echo "Run: ./configure; make; make install" && \
echo ""
