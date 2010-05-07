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

# be able to customize the aclocal (for example to add extra param)
if test "x$ACLOCAL" = "x"
then
	ACLOCAL=aclocal
fi

echo "Creating macros..." && \
$ACLOCAL -I automacros/ && \
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
