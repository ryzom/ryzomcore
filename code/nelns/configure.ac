# ====================================================================
# Configuration script for NeLNS
# ====================================================================
#
# $Id: configure.ac,v 1.4 2005/01/07 18:29:56 distrib Exp $
# 

# ====================================================================
# Process this file with autoconf to produce a configure script.
# ====================================================================

# If you want to change the version, must must change AC_INIT
# *and* AC_SUBST(LIBTOOL_VERSION)

AC_PREREQ(2.57)
AC_INIT([nelns],[0.5.0],[nel-all@nevrax.org])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([tar-ustar])

AM_CONFIG_HEADER(config.h)

AC_SUBST(LIBTOOL_VERSION, [0:5:0])

# ====================================================================
# Checks for programs.
# ====================================================================


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

# The following hack should ensure that configure doesn't add optimizing
# or debugging flags to CFLAGS or CXXFLAGS
CFLAGS="$CFLAGS "
CXXFLAGS="$CXXFLAGS "

# Template needed to generate the config.h.in
#AH_TEMPLATE([NELNS_CONFIG],[Configuration files directory path])
#AH_TEMPLATE([NELNS_LOGS],[Log files directory path])
#AH_TEMPLATE([NELNS_STATE],[Local state files directory path])
AH_TEMPLATE([HAVE_MYSQL],[Define to 1 if mysql was found])


AX_LIB_MYSQL([3.23.40])
#dnl AM_PATH_MYSQL(3.23.40,,AC_MSG_ERROR([MySQL 3.23.40 or higher must be installed]))

AC_DEFINE(HAVE_MYSQL)

CFLAGS="$CFLAGS $MYSQL_CFLAGS"
CXXFLAGS="$CXXFLAGS $MYSQL_CFLAGS"

# prevent using GLIBC2.4 stuffs
CFLAGS="$CFLAGS -fno-stack-protector"
CXXFLAGS="$CXXFLAGS -fno-stack-protector"

LIBS="$LIBS -lrt $MYSQL_LIBS"


# ====================================================================
# Configure Settings
# ====================================================================

AC_LANG([C++])


# ====================================================================
# Debug/optimized compilation mode
# ====================================================================

AM_NEL_DEBUG

# Set the gcc specific warning level
if test "X$CC" = "Xgcc";
then
    if test "$with_debug" != "no"
    then
	#
	# When debugging variables are declared for the sole purpose of
	# inspecting their content with a debugger. They are not used
	# in the code itself and this is legitimate, hence the -Wno-unused
	#
        CXXFLAGS="$CXXFLAGS -Wall -Wno-multichar -Wno-unused"
    else
        CXXFLAGS="$CXXFLAGS -Wall -Wno-multichar"
    fi
fi

# ====================================================================
# Checks for typedefs, structures, and compiler characteristics.
# ====================================================================

# Add the define _REENTRANT for a correct use of the threads
if test "X$CC" = "Xgcc";
then
    CXXFLAGS="$CXXFLAGS -D_REENTRANT"
fi


# ====================================================================
# Checks for header files.
# ====================================================================

AC_HEADER_STDC([])
AC_CHECK_HEADERS(fcntl.h unistd.h)


# ====================================================================
# Checks for libraries.
# ====================================================================

AC_CHECK_LIB(crypt, crypt)


# ====================================================================
# NeL

#         misc   net    3d    pacs  sound  ai    georges
AM_PATH_NEL("yes", "yes", "no", "no", "no",  "no", "no")

# ====================================================================
# Checks for library functions.
# ====================================================================

# ====================================================================
# nelns configuration and logs file location
# ====================================================================

test "x$prefix" = xNONE && prefix=$ac_default_prefix

AC_ARG_WITH(sysconfdir,
    [  --with-sysconfdir[=DIR]
                          use DIR instead of the default sysconfdir/nelns],
    [pkgsysconfdir=$withval],
    [pkgsysconfdir='${sysconfdir}/nelns'])
AC_SUBST(pkgsysconfdir)

#eval eval NELNS_CONFIG=\"$pkgsysconfdir/\"
#AC_DEFINE_UNQUOTED(NELNS_CONFIG, "$NELNS_CONFIG")

AC_ARG_WITH(localstatedir,
    [  --with-localstatedir[=DIR]
                          use DIR instead of the default localstatedir/game/nelns],
    [pkglocalstatedir=$withval],
    [pkglocalstatedir=${localstatedir}/game/nelns])
AC_SUBST(pkglocalstatedir)

#eval eval NELNS_STATE=\"$pkglocalstatedir/\"
#AC_DEFINE_UNQUOTED(NELNS_STATE, "$NELNS_STATE")

AC_ARG_WITH(logdir,
    [  --with-logdir[=DIR]
                          use DIR instead of the default localstatedir/log/nelns],
    [logdir=$withval],
    [logdir='${localstatedir}/log/nelns'])
AC_SUBST(logdir)

#eval eval NELNS_LOGS=\"$logdir/\"
#AC_DEFINE_UNQUOTED(NELNS_LOGS, "$NELNS_LOGS")

# ====================================================================
# Output files to generate.
# ====================================================================

AC_CONFIG_FILES([Makefile                        \
           login_service/Makefile          \
           naming_service/Makefile         \
           admin_executor_service/Makefile \
           admin_service/Makefile          \
           welcome_service/Makefile        \
])
AC_OUTPUT


# End of configure.ac
