#!/bin/sh -

WANT_AUTOMAKE="1.6"

echo "Creating macros..." && \
aclocal -I automacros/ && \
echo "Creating library tools..." && \
libtoolize --force && \
echo "Creating header templates..." && \
autoheader && \
echo "Creating Makefile templates..." && \
automake --gnu --add-missing && \
echo "Creating 'configure'..." && \
autoconf && \
echo -e "\nRun: ./configure; make; make install\n"
