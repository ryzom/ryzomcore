dnl ====================================================================
dnl Configuration script for Snowballs 2
dnl ====================================================================


dnl ====================================================================
dnl Process this file with autoconf to produce a configure script.
dnl ====================================================================

# If you want to change the version, must must change AC_INIT
# *and* AC_SUBST(LIBTOOL_VERSION)

AC_PREREQ(2.57)
AC_INIT([snowballs],[0.3.0],[nel-all@nevrax.org])
AM_INIT_AUTOMAKE([tar-ustar])

AM_CONFIG_HEADER(config.h)

AC_SUBST(LIBTOOL_VERSION, [0:3:0])

dnl ====================================================================
dnl Checks for programs.
dnl ====================================================================
AC_CANONICAL_HOST
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AM_SANITY_CHECK

AC_SYS_LARGEFILE

AM_MAINTAINER_MODE

dnl The following hack should ensure that configure doesn't add optimizing
dnl or debugging flags to CFLAGS or CXXFLAGS
CXXFLAGS="$CXXFLAGS -ftemplate-depth-24"

AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([rt], [clock_gettime])

dnl ====================================================================
dnl Checks Snowballs modules (Client / Server) to install / Disable
dnl ====================================================================

SNO_SUBDIRS=""

dnl Client
AC_ARG_ENABLE( client,
    [  --disable-client        disable compilation and install of Snowballs Client.],
    [ AC_MSG_RESULT(disable Snowballs Client.) ],
    [ SNO_SUBDIRS="$SNO_SUBDIRS client"
      enable_client="yes" ] )

dnl Server
AC_ARG_ENABLE( server,
    [  --disable-server        disable compilation and install of Snowballs Server.],
    [ AC_MSG_RESULT(disable Snowballs Server.) ],
    [ SNO_SUBDIRS="$SNO_SUBDIRS server"
      enable_server="yes" ] )

AC_SUBST(SNO_SUBDIRS)


dnl ====================================================================
dnl Configure Settings
dnl ====================================================================

dnl Set the C++ compiler as the default compiler for the configure script
dnl compilation tests
AC_LANG_CPLUSPLUS


dnl ====================================================================
dnl Debug/optimized compilation mode
dnl ====================================================================

AM_NEL_DEBUG


dnl ====================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ====================================================================

# Test endianness
AC_C_BIGENDIAN

# Supress GCC "multi-character character constant" warnings.
if test "X$CC" = "Xgcc";
then
    if test "$with_debug" = "yes"
    then
        #
        # When debugging variables are declared for the sole purpose of
        # inspecting their content with a debugger. They are not used
        # in the code itself and this is legitimate, hence the -Wno-unused
        #
        CXXFLAGS="$CXXFLAGS -Wno-unused"
    fi
fi

# Add some common define
if test "$ac_cv_cxx_compiler_gnu" = "yes";
then
    CXXFLAGS="$CXXFLAGS -D_REENTRANT -Wall -ansi -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-long-long -Wno-unused"
fi


dnl ====================================================================
dnl Checks for header and lib files.
dnl ====================================================================

AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h sys/time.h])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([dl], [dlopen])

dnl ====================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ====================================================================

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_TYPES([size_t])
AC_CHECK_TYPES([uintptr_t])

dnl ====================================================================
dnl X11

AC_PATH_X

if test ! "$no_x" = "yes" ; then
    if test ! X"$x_libraries" = X
    then
        LIBS="$LIBS -L$x_libraries"
    fi
    if test ! X"$x_includes" = X
    then
        CXXFLAGS="$CXXFLAGS -I$x_includes"
    fi
else
    if test "$enable_client" = "yes"
    then
        AC_MSG_ERROR(X11 must be installed.)
    fi
fi


dnl ====================================================================
dnl Checks for libraries.
dnl ====================================================================

dnl ====================================================================
dnl OpenAL

AM_PATH_OPENAL($enable_client)


dnl ====================================================================
dnl LibXML

dnl Use C compiler as a default for the libxml tests.
AC_LANG([C])

AM_PATH_XML2([2.0.0], [], [AC_MSG_ERROR([libxml2 must be installed])])

CXXFLAGS="$CXXFLAGS $XML_CFLAGS $XML_CPPFLAGS"

LIBS="$LIBS $XML_LIBS"

dnl ====================================================================
dnl NeL

dnl         misc   net    3d              pacs            sound ai    georges
AM_PATH_NEL("yes", "yes", $enable_client, $enable_client, "no", "no", "no")

dnl ====================================================================
dnl FreeType 2

AM_PATH_FREETYPE($enable_client)

dnl ====================================================================
dnl Checks for library functions.
dnl ====================================================================

dnl ====================================================================
dnl Snowballs data, configuration and logs file location
dnl ====================================================================

test "x$prefix" = xNONE && prefix=$ac_default_prefix

AC_ARG_WITH(database,
    [  --with-database[=DIR]
                          use DIR instead of the default prefix/share//games/snowballs],
    [database=$withval],
    [database="${prefix}/share/games/snowballs"])
AC_SUBST(database)

SNOWBALLS_DATABASE='$database'
AC_SUBST([SNOWBALLS_DATABASE])

AC_ARG_WITH(sysconfdir,
    [  --with-sysconfdir[=DIR]
                          use DIR instead of the default sysconfdir/snowballs],
    [pkgsysconfdir=$withval],
    [pkgsysconfdir='${sysconfdir}/snowballs'])
AC_SUBST(pkgsysconfdir)

SNOWBALLS_CONFIG='$pkgsysconfdir'
AC_SUBST([SNOWBALLS_CONFIG])

AC_ARG_WITH(logdir,
    [  --with-logdir[=DIR]
                          use DIR instead of the default localstatedir/log/snowballs],
    [logdir=$withval],
    [logdir='${localstatedir}/log/snowballs'])
AC_SUBST(logdir)

SNOWBALLS_LOGS='$logdir'
#AH_TEMPLATE([SNOWBALLS_LOGS], [The path to where Snowballs logs should be stored.])
#AC_DEFINE_UNQUOTED([SNOWBALLS_LOGS], "$log_dir")
AC_SUBST([SNOWBALLS_LOGS])

dnl ====================================================================
dnl Output files to generate.
dnl ====================================================================

AC_CONFIG_FILES([Makefile					\
		client/Makefile					\
		client/client.cfg				\
		client/src/Makefile				\
		client/data/Makefile				\
		client/data/anims/Makefile			\
		client/data/maps/Makefile			\
		client/data/maps/texture_croisi/Makefile	\
		client/data/pacs/Makefile			\
		client/data/shapes/Makefile			\
		client/data/tiles/Makefile			\
		client/data/zones/Makefile			\
		server/Makefile					\
		server/frontend/Makefile			\
		server/frontend/src/Makefile			\
		server/chat/Makefile				\
		server/chat/src/Makefile			\
		server/position/Makefile			\
		server/position/src/Makefile			\
])
AC_OUTPUT

dnl End of configure.in

