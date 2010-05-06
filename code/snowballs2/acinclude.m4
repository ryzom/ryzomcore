# =========================================================================
#
# Macros used by Nevrax in configure.in files.
#
# =========================================================================

# =========================================================================
# WARNING: The original version of this file is placed in the $CVSROOT/code
#          directory.
#          There is links in the $CVSROOT/code sub-directories to that file
#          (ex: $CVSROOT/code/nel), so be careful of the consequences of
#          any modification of that file.
# =========================================================================

# =========================================================================
# Macros available in that file.
#
#
# AM_NEL_DEBUG
#
#    Option:      none.
#    Description: manage the different debug and the release mode by setting
#                 correctly the CFLAGS and CXXFLAGS variables.
#
#
# AM_PATH_NEL
#
#    Option:      none.
#    Description: check the instalation of the NeL library and set the
#                 CXXFLAGS and LIBS variables to use it.
#
#
# AM_PATH_OPENGL
#
#    Option:      "yes" if the use of the OpenGL library is mandatory.
#    Description: check the instalation of the OpenGL library and set the
#                 OPENGL_CFLAGS and OPENGL_LIBS variables to use it.
#
#
# AM_PATH_FREETYPE
#
#    Option:      "yes" if the use of the Freetype library is mandatory.
#    Description: check the instalation of the OpenGL library and set the
#                 FREETYPE_CFLAGS and FREETYPE_LIBS variables to use it.
#
#
# AM_PATH_XF86VIDMODE
#
#    Option:      none.
#    Description: check the instalation of the OpenGL library and set the
#                 XF86VIDMODE_CFLAGS and XF86VIDMODE_LIBS variables to use it.
#
#
# AM_PATH_OPENAL
#
#    Option:      "yes" if the use of the OpenAL library is mandatory.
#    Description: check the instalation of the OpenGL library and set the
#                 OPENAL_CFLAGS and OPENAL_LIBS variables to use it.
#
#
# AM_PATH_PYTHON
#
#    Option:      "yes" if the use of the Python library is mandatory.
#    Description: check the instalation of the OpenGL library and set the
#                 PYTHON_CFLAGS and PYTHON_LIBS variables to use it.
#
# =========================================================================


# =========================================================================
# AM_NEL_DEBUG

AC_DEFUN([AM_NEL_DEBUG],
[

MAX_C_OPTIMIZE="-O6"

NL_DEBUG="-DNL_DEBUG"
NL_RELEASE="-DNL_RELEASE"

AC_ARG_WITH(debug,
    [  --with-debug[=full|medium|fast]
                          Build a debug version (huge libraries).
                          Full mode set only NeL.
                          Medium mode set NeL debug flags with inline
                          optimization (default mode).
                          Fast mode is like the Medium mode with some basic
                          optimization.
  --without-debug         Build without debugging code (default)],
    [with_debug=$withval],
    [with_debug=no])

# Build optimized or debug version ?
# First check for gcc and g++
if test "$ac_cv_prog_gcc" = "yes"
then
    DEBUG_CFLAGS="-g"
    DEBUG_OPTIMIZE_CC="-O"
    OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
else
    DEBUG_CFLAGS="-g"
    DEBUG_OPTIMIZE_CC=""
    OPTIMIZE_CFLAGS=""
fi

if test "$ac_cv_prog_cxx_g" = "yes"
then
    DEBUG_CXXFLAGS="-g"
    DEBUG_OPTIMIZE_CXX="-O"
    OPTIMIZE_CXXFLAGS="-O3"
    OPTIMIZE_INLINE_CXXFLAGS="-finline-functions"
else
    DEBUG_CXXFLAGS="-g"
    DEBUG_OPTIMIZE_CXX=""
    OPTIMIZE_CXXFLAGS=""
    OPTIMIZE_INLINE_CXXFLAGS=""
fi

if test "$with_debug" = "yes" -o "$with_debug" = "medium"
then
    # Medium debug. Inline optimization
    CFLAGS="$DEBUG_CFLAGS $OPTIMIZE_INLINE_CFLAGS $NL_DEBUG $CFLAGS"
    CXXFLAGS="$DEBUG_CXXFLAGS $OPTIMIZE_INLINE_CXXFLAGS $NL_DEBUG $CXXFLAGS"
else
    if test "$with_debug" = "full"
    then
        # Full debug. Very slow in some cases
        CFLAGS="$DEBUG_CFLAGS $NL_DEBUG $CFLAGS"
        CXXFLAGS="$DEBUG_CXXFLAGS $NL_DEBUG $CXXFLAGS"
    else
        if test "$with_debug" = "fast"
        then
            # Fast debug.
            CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC $OPTIMIZE_INLINE_CFLAGS $NL_DEBUG $CFLAGS"
            CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX $OPTIMIZE_INLINE_CXXFLAGS $NL_DEBUG $CXXFLAGS"
        else
            # Optimized version. No debug
            CFLAGS="$OPTIMIZE_CFLAGS $NL_RELEASE $CFLAGS"
            CXXFLAGS="$OPTIMIZE_CXXFLAGS $NL_RELEASE $CXXFLAGS"
        fi
    fi
fi

# AC_MSG_RESULT([CFLAGS = $CFLAGS])
# AC_MSG_RESULT([CXXGLAGS = $CXXFLAGS])

])


# =========================================================================
# MY_NEL_HEADER_CHK : NeL header files checking macros

AC_DEFUN([MY_NEL_HEADER_CHK],
[ AC_REQUIRE_CPP()

chk_message_obj="$1"
header="$2"
macro="$3"
is_mandatory="$4"

if test $is_mandatory = "yes"
then

    _CPPFLAGS="$CPPFLAGS"

    CPPFLAGS="$CXXFLAGS $NEL_CFLAGS"

    AC_MSG_CHECKING(for $header)

    AC_EGREP_CPP( yo_header,
[#include <$header>
#ifdef $macro
   yo_header
#endif],
  have_header="yes",
  have_header="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_header" = "yes"
    then
        AC_MSG_RESULT(yes)
    else
        if test "$is_mandatory" = "yes"
        then
            AC_MSG_ERROR([$chk_message_obj must be installed (http://dev.ryzom.com).])
        else
            AC_MSG_RESULT(no)
        fi
    fi
fi

        
])


# =========================================================================
# MY_NEL_LIB_CHK : NeL library checking macros

AC_DEFUN([MY_NEL_LIB_CHK],
[ AC_REQUIRE_CPP()

chk_message_obj="$1"
nel_test_lib="$2"
is_mandatory="$3"

if test $is_mandatory = "yes"
then

    AC_CHECK_LIB($nel_test_lib, main,,[AC_MSG_ERROR([$chk_message_obj must be installed (http://dev.ryzom.com).])])
fi
])


# =========================================================================
# AM_PATH_NEL : NeL checking macros
AC_DEFUN([AM_PATH_NEL],
[ AC_REQUIRE_CPP()

AC_ARG_WITH( nel,
    [  --with-nel=<path>       path to the NeL install files directory.
                          e.g. /usr/local/nel])

AC_ARG_WITH( nel-include,
    [  --with-nel-include=<path>
                          path to the NeL header files directory.
                          e.g. /usr/local/nel/include])

AC_ARG_WITH( nel-lib,
    [  --with-nel-lib=<path>
                          path to the NeL library files directory.
                          e.g. /usr/local/nel/lib])


nelmisc_is_mandatory="$1"
nelnet_is_mandatory="$2"
nel3d_is_mandatory="$3"
nelpacs_is_mandatory="$4"
nelsound_is_mandatory="$5"
nelai_is_mandatory="$6"
nelgeorges_is_mandatory="$7"

# Check for nel-config
AC_PATH_PROG(NEL_CONFIG, nel-config, no)

# 
# Configure options (--with-nel*) have precendence 
# over nel-config only set variables if they are not 
# specified
#
if test "$NEL_CONFIG" != "no"
then
    if test -z "$with_nel" -a -z "$with_nel_include"
    then
	CXXFLAGS="$CXXFLAGS `nel-config --cflags`"
    fi

    if test -z "$with_nel" -a -z "$with_nel_lib"
    then
	LDFLAGS="`nel-config --ldflags` $LDFLAGS"
    fi
fi

#
# Set nel_libraries and nel_includes according to
# user specification (--with-nel*) if any. 
# --with-nel-include and --with-nel-lib have precendence
# over --with-nel
#
if test "$with_nel" = "no"
then
    # The user explicitly disabled the use of the NeL
    AC_MSG_ERROR([NeL is mandatory: do not specify --without-nel])
else
    if test "$with_nel" -a "$with_nel" != "yes"
    then
	nel_includes="$with_nel/include"
	nel_libraries="$with_nel/lib"
    fi
fi

if test "$with_nel_include"
then
    nel_includes="$with_nel_include"
fi

if test "$with_nel_lib"
then
    nel_libraries="$with_nel_lib"
fi

#
# Set compilation variables 
#
if test "$nel_includes"
then
    CXXFLAGS="$CXXFLAGS -I$nel_includes"
fi

if test "$nel_libraries"
then
    LDFLAGS="-L$nel_libraries $LDFLAGS"
fi

#
# Collect headers information and bark if missing and
# mandatory
#

MY_NEL_HEADER_CHK([NeL Misc], [nel/misc/types_nl.h], [NL_TYPES_H], $nelmisc_is_mandatory)
MY_NEL_HEADER_CHK([NeL Network], [nel/net/sock.h], [NL_SOCK_H], $nelnet_is_mandatory)
MY_NEL_HEADER_CHK([NeL 3D], [nel/3d/u_camera.h], [NL_U_CAMERA_H], $nel3d_is_mandatory)
MY_NEL_HEADER_CHK([NeL PACS], [nel/pacs/u_global_position.h], [NL_U_GLOBAL_POSITION_H], $nelpacs_is_mandatory)
MY_NEL_HEADER_CHK([NeL Sound], [nel/sound/u_source.h], [NL_U_SOURCE_H], $nelsound_is_mandatory)
MY_NEL_HEADER_CHK([NeL AI], [nel/ai/nl_ai.h], [_IA_NEL_H], $nelai_is_mandatory)
MY_NEL_HEADER_CHK([NeL Georges], [nel/georges/common.h], [NLGEORGES_COMMON_H], $nelgeorges_is_mandatory)

#
# Collect libraries information and bark if missing and
# mandatory
#

MY_NEL_LIB_CHK([NeL Misc], [nelmisc], $nelmisc_is_mandatory)
MY_NEL_LIB_CHK([NeL Network], [nelnet], $nelnet_is_mandatory)
MY_NEL_LIB_CHK([NeL 3D], [nel3d], $nel3d_is_mandatory)
MY_NEL_LIB_CHK([NeL PACS], [nelpacs], $nelpacs_is_mandatory)
MY_NEL_LIB_CHK([NeL Sound], [nelsnd], $nelsound_is_mandatory)
MY_NEL_LIB_CHK([NeL AI], [nelai], $nelai_is_mandatory)
MY_NEL_LIB_CHK([NeL Georges], [nelgeorges], $nelgeorges_is_mandatory)

])

# =========================================================================
# AM_PATH_OPENGL : OpenGL checking macros

AC_DEFUN([AM_PATH_OPENGL],
[ AC_MSG_CHECKING(for OpenGL headers and GL Version >= 1.2)

is_mandatory="$1"

AC_REQUIRE_CPP()

AC_ARG_WITH( opengl,
    [  --with-opengl=<path>    path to the OpenGL install files directory.
                          e.g. /usr/local])

AC_ARG_WITH( opengl-include,
    [  --with-opengl-include=<path>
                          path to the OpenGL header files directory.
                          e.g. /usr/local/include])

AC_ARG_WITH( opengl-lib,
    [  --with-opengl-lib=<path>
                          path to the OpenGL library files directory.
                          e.g. /usr/local/lib])

opengl_lib="GL"

if test "$with_opengl"
then
    opengl_includes="$with_opengl/include"
    opengl_libraries="$with_opengl/lib"
fi

if test "$with_opengl_include"
then
    opengl_includes="$with_opengl_include"
fi

if test "$with_opengl_lib"
then
    opengl_libraries="$with_opengl_lib"
fi

# Set OPENGL_CFLAGS
if test "$opengl_includes"
then
    OPENGL_CFLAGS="-I$opengl_includes"
fi

# Set OPENGL_LIBS
if test "$opengl_libraries"
then
    OPENGL_LIBS="-L$opengl_libraries"
fi
OPENGL_LIBS="$OPENGL_LIBS -l$opengl_lib"

# Test the headers
_CPPFLAGS="$CPPFLAGS"

CPPFLAGS="$CXXFLAGS $OPENGL_CFLAGS"

AC_EGREP_CPP( yo_opengl,
[#include <GL/gl.h>       
#if defined(GL_VERSION_1_2)
   yo_opengl
#endif],
  have_opengl_headers="yes", 
  have_opengl_headers="no" )

if test "$have_opengl_headers" = "yes"
then
    if test "$opengl_includes"
    then
        AC_MSG_RESULT([$opengl_includes])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

# Checking the GLEXT version >= 7
AC_MSG_CHECKING(for <GL/glext.h> and GLEXT version >= 7)

AC_EGREP_CPP( yo_glext_version,
[#include <GL/glext.h>
#ifdef GL_GLEXT_VERSION
#if GL_GLEXT_VERSION >= 7
   yo_glext_version
#endif
#endif],
  have_glext="yes",
  have_glext="no" )

if test "$have_glext" = "yes"
then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT([no, <GL/glext.h> can be downloaded from http://www.opengl.org/registry/])
fi
    
# Test the libraries
AC_MSG_CHECKING(for OpenGL libraries)

CPPFLAGS="$CXXFLAGS $OPENGL_LIBS"

AC_TRY_LINK( , , have_opengl_libraries="yes", have_opengl_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_opengl_libraries" = "yes"
then
    if test "$opengl_libraries"
    then
        AC_MSG_RESULT([$opengl_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

opengl_libraries="$opengl_libraries"

if test "$have_opengl_headers" = "yes" \
        -a "$have_glext" = "yes" \
        -a "$have_opengl_libraries" = "yes"
then
    have_opengl="yes"
else
    have_opengl="no"
fi

if test "$have_opengl" = "no" -a "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([OpenGL >= 1.2 must be installed (http://www.mesa3d.org)])
fi

AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LIBS)

])


# =========================================================================
# AM_PATH_FREETYPE : FreeType checking macros

AC_DEFUN([AM_PATH_FREETYPE],
[ is_mandatory="$1"

AC_REQUIRE_CPP()

AC_ARG_WITH( freetype,
    [  --with-freetype=<path>   path to the FreeType install files directory.
                          e.g. /usr/local/freetype])

AC_ARG_WITH( freetype-include,
    [  --with-freetype-include=<path>
                          path to the FreeType header files directory.
                          e.g. /usr/local/freetype/include])

AC_ARG_WITH( freetype-lib,
    [  --with-freetype-lib=<path>
                          path to the FreeType library files directory.
                          e.g. /usr/local/freetype/lib])

freetype_lib="freetype"


AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
  
if test "$FREETYPE_CONFIG" = "no"
then
    have_freetype_config="no"
else
    FREETYPE_CFLAGS=`freetype-config --cflags`
    FREETYPE_LIBS=`freetype-config --libs`
    have_freetype_config="yes"
fi

if test "$with_freetype"
then
    freetype_includes="$with_freetype/include"
    freetype_libraries="$with_freetype/lib"
fi

if test "$with_freetype_include"
then
    freetype_includes="$with_freetype_include"
fi

if test "$with_freetype_lib"
then
    freetype_libraries="$with_freetype_lib"
fi

if test "$freetype_includes"
then
    FREETYPE_CFLAGS="-I$freetype_includes"
fi

# Checking the FreeType 2 instalation
_CPPFLAGS="$CPPFLAGS"
CPPFLAGS=" $FREETYPE_CFLAGS $CXXFLAGS"

AC_MSG_CHECKING(for FreeType version = 2)

AC_EGREP_CPP( yo_freetype2,
[#include <freetype/freetype.h>
#if FREETYPE_MAJOR == 2
   yo_freetype2
#endif],
  have_freetype2="yes",
  have_freetype2="no")

if test "$have_freetype2" = "yes"
then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
    
# Test the libraries
AC_MSG_CHECKING(for FreeType libraries)
        
if test $freetype_libraries
then
    FREETYPE_LIBS="-L$freetype_libraries -l$freetype_lib"
fi

CPPFLAGS="$FREETYPE_LIBS $CXXFLAGS"
    
AC_TRY_LINK( , , have_freetype_libraries="yes", have_freetype_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_freetype_libraries" = "yes"
then    
    if test "$freetype_libraries"
    then
        AC_MSG_RESULT([$freetype_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

if test "$have_freetype2" = "yes" && test "$have_freetype_libraries" = "yes"
then
    have_freetype="yes"
else
    have_freetype="no"
fi

if test "$have_freetype" = "no" && test "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([FreeType 2 must be installed (http://freetype.sourceforge.net)])
fi

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

])


# =========================================================================
# AM_PATH_XF86VIDMODE : XF86VidMode checking macros

AC_DEFUN([AM_PATH_XF86VIDMODE],
[ AC_MSG_CHECKING(for XF86VidMode extension)

AC_REQUIRE_CPP()

AC_ARG_WITH( xf86vidmode-lib,
    [  --with-xf86vidmode-lib=<path>
                          path to the XF86VidMode library.
                          e.g. /usr/X11R6/lib] )

xf86vidmode_lib="Xxf86vm"

if test "$with_xf86vidmode_lib" = no
then
    # The user explicitly disabled the use of XF86VidMode
    have_xf86vidmode="disabled"
    AC_MSG_RESULT(disabled)
else
    if test "$with_xf86vidmode_lib"
    then
        xf86vidmode_libraries="$with_xf86vidmode_lib"
    fi

    XF86VIDMODE_CFLAGS="-DXF86VIDMODE"
fi

if test -z "$have_xf86vidmode"
# -a "$with_xf86vidmode_lib"
then
    if test "$xf86vidmode_libraries"
    then
        XF86VIDMODE_LIBS="-L$xf86vidmode_libraries"
    fi

    XF86VIDMODE_LIBS="$XF86VIDMODE_LIBS -l$xf86vidmode_lib"

    _CPPFLAGS="$CPPFLAGS"

    CPPFLAGS="$CXXFLAGS $XF86VIDMODE_LIBS"

    AC_TRY_LINK( , , have_xf86vidmode_libraries="yes", have_xf86vidmode_libraries="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_xf86vidmode_libraries" = "yes"
    then
        have_xf86vidmode="yes"
        if test "$xf86vidmode_libraries"
        then
            AC_MSG_RESULT($xf86vidmode_libraries)
        else
            AC_MSG_RESULT(yes)
        fi
    else
        have_xf86vidmode="no"
        AC_MSG_RESULT(no, no fullscreen support available.)
    fi

    xf86vidmode_libraries="$xf86vidmode_libraries"

fi

AC_SUBST(XF86VIDMODE_CFLAGS)
AC_SUBST(XF86VIDMODE_LIBS)

])


# =========================================================================
# AM_PATH_OPENAL : OpenAL checking macros

AC_DEFUN([AM_PATH_OPENAL],
[ is_mandatory="$1"

AC_REQUIRE_CPP()

# Get from the user option the path to the OpenAL files location
AC_ARG_WITH( openal,
    [  --with-openal=<path>   path to the OpenAL install files directory.
                          e.g. /usr/local])

AC_ARG_WITH( openal-include,
    [  --with-openal-include=<path>
                          path to the OpenAL header files directory.
                          e.g. /usr/local/include])

AC_ARG_WITH( openal-lib,
    [  --with-openal-lib=<path>
                          path to the OpenAL library files directory.
                          e.g. /usr/local/lib])

openal_lib="openal"
alut_lib="alut"

if test $with_openal
then
    openal_includes="$with_openal/include"
    openal_libraries="$with_openal/lib"
fi

if test "$with_openal_include"
then
    openal_includes="$with_openal_include"
fi

if test "$with_openal_lib"
then
    openal_libraries="$with_openal_lib"
fi


# Set OPENAL_CFLAGS
if test "$openal_includes"
then
    OPENAL_CFLAGS="-I$openal_includes"
fi

# Set OPENAL_LIBS
if test "$openal_libraries"
then
    OPENAL_LIBS="-L$openal_libraries"
fi
OPENAL_LIBS="$OPENAL_LIBS -l$openal_lib -l$alut_lib"

_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CXXFLAGS $OPENAL_CFLAGS"

AC_MSG_CHECKING(for OpenAL headers)
AC_EGREP_CPP( yo_openal,
[#include <AL/al.h>
#include <AL/alut.h>
#ifdef AL_VERSION
   yo_openal
#endif],
  have_openal_headers="yes",
  have_openal_headers="no" )

if test "$have_openal_headers" = "yes"
then
    if test "$openal_includes"
    then
        AC_MSG_RESULT([$openal_includes])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

# Test the libraries
AC_MSG_CHECKING(for OpenAL libraries)

CPPFLAGS="$CXXFLAGS $OPENAL_LIBS"

AC_TRY_LINK( , , have_openal_libraries="yes", have_openal_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_openal_libraries" = "yes"
then
    if test "$openal_libraries"
    then
        AC_MSG_RESULT([$openal_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

openal_libraries="$openal_libraries"

if test "$have_openal_headers" = "yes" \
   && test "$have_openal_libraries" = "yes"
then
    have_openal="yes"
else
    have_openal="no"
fi

if test "$have_openal" = "no" -a "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([OpenAL is needed to compile NeL (http://www.openal.org).])
fi

AC_SUBST(OPENAL_CFLAGS)
AC_SUBST(OPENAL_LIBS)
AC_SUBST([have_openal])

])


# =========================================================================
# AM_PATH_PYTHON : Python checking macros

AC_DEFUN([AM_PATH_PYTHON],
[ python_version_required="$1"

is_mandatory="$2"

AC_REQUIRE_CPP()

# Get from the user option the path to the Python files location
AC_ARG_WITH( python,
    [  --with-python=<path>    path to the Python prefix installation directory.
                          e.g. /usr/local],
    [ PYTHON_PREFIX=$with_python ]
)

AC_ARG_WITH( python-version,
    [  --with-python-version=<version>
                          Python version to use, e.g. 1.5],
    [ PYTHON_VERSION=$with_python_version ]
)

if test ! "$PYTHON_PREFIX" = ""
then
    PATH="$PYTHON_PREFIX/bin:$PATH"
fi

if test ! "$PYTHON_VERSION" = ""
then
    PYTHON_EXEC="python$PYTHON_VERSION"
else
    PYTHON_EXEC="python python2.1 python2.0 python1.5"
fi

AC_PATH_PROGS(PYTHON, $PYTHON_EXEC, no, $PATH)

if test "$PYTHON" != "no"
then
    PYTHON_PREFIX=`$PYTHON -c 'import sys; print "%s" % (sys.prefix)'`
    PYTHON_VERSION=`$PYTHON -c 'import sys; print "%s" % (sys.version[[:3]])'`

    is_python_version_enough=`expr $python_version_required \<= $PYTHON_VERSION`
fi


if test "$PYTHON" = "no" || test "$is_python_version_enough" != "1"
then

    if test "$is_mandatory" = "yes"
    then
        AC_MSG_ERROR([Python $python_version_required must be installed (http://www.python.org)])
    else
        have_python="no"
    fi

else

    python_includes="$PYTHON_PREFIX/include/python$PYTHON_VERSION"
    python_libraries="$PYTHON_PREFIX/lib/python$PYTHON_VERSION/config"
    python_lib="python$PYTHON_VERSION"

    PYTHON_CFLAGS="-I$python_includes"
    PYTHON_LIBS="-L$python_libraries -l$python_lib"

    _CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CXXFLAGS ${PYTHON_CFLAGS}"

    # Test the headers
    AC_MSG_CHECKING(for Python headers)

    AC_EGREP_CPP( yo_python,
    [#include <Python.h>
   yo_python
    ],
      have_python_headers="yes",
      have_python_headers="no" )

    if test "$have_python_headers" = "yes"
    then
        AC_MSG_RESULT([$python_includes])
    else
        AC_MSG_RESULT(no)
    fi

    # Test the libraries
    AC_MSG_CHECKING(for Python libraries)

    CPPFLAGS="$CXXFLAGS $PYTHON_CFLAGS"

    AC_TRY_LINK( , , have_python_libraries="yes", have_python_libraries="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_python_libraries" = "yes"
    then
        if test "$python_libraries"
        then
            AC_MSG_RESULT([$python_libraries])
        else
            AC_MSG_RESULT(yes)
        fi
    else
        AC_MSG_RESULT(no)
    fi

    if test "$have_python_headers" = "yes" \
       && test "$have_python_libraries" = "yes"
    then
        have_python="yes"
    else
        have_python="no"
    fi

    if test "$have_python" = "no" -a "$is_mandatory" = "yes"
    then
        AC_MSG_ERROR([Python is needed to compile NeL (http://www.python.org).])
    fi

    AC_SUBST(PYTHON_CFLAGS)
    AC_SUBST(PYTHON_LIBS)

fi

])

# =========================================================================
# AM_PATH_MYSQL : MySQL library

# AM_PATH_MYSQL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
# Test for MYSQL, and define MYSQL_CFLAGS and MYSQL_LIBS
#
AC_DEFUN([AM_PATH_MYSQL],
[# 
# Get the cflags and libraries from the mysql_config script
#
AC_ARG_WITH(mysql-prefix,[  --with-mysql-prefix=PFX   Prefix where MYSQL is installed (optional)],
            mysql_prefix="$withval", mysql_prefix="")
AC_ARG_WITH(mysql-exec-prefix,[  --with-mysql-exec-prefix=PFX Exec prefix where MYSQL is installed (optional)],
            mysql_exec_prefix="$withval", mysql_exec_prefix="")
AC_ARG_ENABLE(mysqltest, [  --disable-mysqltest       Do not try to compile and run a test MYSQL program],
		    , enable_mysqltest=yes)

  if test x$mysql_exec_prefix != x ; then
     mysql_args="$mysql_args --exec-prefix=$mysql_exec_prefix"
     if test x${MYSQL_CONFIG+set} != xset ; then
        MYSQL_CONFIG=$mysql_exec_prefix/bin/mysql_config
     fi
  fi
  if test x$mysql_prefix != x ; then
     mysql_args="$mysql_args --prefix=$mysql_prefix"
     if test x${MYSQL_CONFIG+set} != xset ; then
        MYSQL_CONFIG=$mysql_prefix/bin/mysql_config
     fi
  fi

  AC_REQUIRE([AC_CANONICAL_TARGET])
  AC_PATH_PROG(MYSQL_CONFIG, mysql_config, no)
  min_mysql_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for MYSQL - version >= $min_mysql_version)
  no_mysql=""
  if test "$MYSQL_CONFIG" = "no" ; then
    no_mysql=yes
  else
    MYSQL_CFLAGS=`$MYSQL_CONFIG $mysqlconf_args --cflags | sed -e "s/'//g"`
    MYSQL_LIBS=`$MYSQL_CONFIG $mysqlconf_args --libs | sed -e "s/'//g"`

    mysql_major_version=`$MYSQL_CONFIG $mysql_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    mysql_minor_version=`$MYSQL_CONFIG $mysql_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    mysql_micro_version=`$MYSQL_CONFIG $mysql_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_mysqltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $MYSQL_CFLAGS"
      LIBS="$LIBS $MYSQL_LIBS"
#
# Now check if the installed MYSQL is sufficiently new. (Also sanity
# checks the results of mysql_config to some extent
#
      rm -f conf.mysqltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.mysqltest");
  */
  { FILE *fp = fopen("conf.mysqltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_mysql_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_mysql_version");
     exit(1);
   }

   if (($mysql_major_version > major) ||
      (($mysql_major_version == major) && ($mysql_minor_version > minor)) ||
      (($mysql_major_version == major) && ($mysql_minor_version == minor) && ($mysql_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'mysql_config --version' returned %d.%d.%d, but the minimum version\n", $mysql_major_version, $mysql_minor_version, $mysql_micro_version);
      printf("*** of MYSQL required is %d.%d.%d. If mysql_config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If mysql_config was wrong, set the environment variable MYSQL_CONFIG\n");
      printf("*** to point to the correct copy of mysql_config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_mysql=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_mysql" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$MYSQL_CONFIG" = "no" ; then
       echo "*** The mysql_config script installed by MYSQL could not be found"
       echo "*** If MYSQL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the MYSQL_CONFIG environment variable to the"
       echo "*** full path to mysql_config."
     else
       if test -f conf.mysqltest ; then
        :
       else
          echo "*** Could not run MYSQL test program, checking why..."
          CFLAGS="$CFLAGS $MYSQL_CFLAGS"
          LIBS="$LIBS $MYSQL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <mysql.h>

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding MYSQL or finding the wrong"
          echo "*** version of MYSQL. If it is not finding MYSQL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means MYSQL was incorrectly installed"
          echo "*** or that you have moved MYSQL since it was installed. In the latter case, you"
          echo "*** may want to edit the mysql_config script: $MYSQL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     MYSQL_CFLAGS=""
     MYSQL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(MYSQL_CFLAGS)
  AC_SUBST(MYSQL_LIBS)
  rm -f conf.mysqltest
])

# =========================================================================
# AM_PATH_FMOD : FMOD checking macros

AC_DEFUN([AM_PATH_FMOD],
[ is_mandatory="$1"

AC_REQUIRE_CPP()

# Get from the user option the path to the FMOD files location
AC_ARG_WITH( fmod,
    [  --with-fmod=<path>   path to the FMOD install files directory.
                          e.g. /usr/local])

AC_ARG_WITH( fmod-include,
    [  --with-fmod-include=<path>
                          path to the FMOD header files directory.
                          e.g. /usr/local/include])

AC_ARG_WITH( fmod-lib,
    [  --with-fmod-lib=<path>
                          path to the FMOD library files directory.
                          e.g. /usr/local/lib])

fmod_lib="fmod"

if test $with_fmod
then
    fmod_includes="$with_fmod/include"
    fmod_libraries="$with_fmod/lib"
fi

if test "$with_fmod_include"
then
    fmod_includes="$with_fmod_include"
fi

if test "$with_fmod_lib"
then
    fmod_libraries="$with_fmod_lib"
fi


# Set FMOD_CFLAGS
if test "$fmod_includes"
then
    FMOD_CFLAGS="-I$fmod_includes"
fi

# Set FMOD_LIBS
if test "$fmod_libraries"
then
    FMOD_LIBS="-L$fmod_libraries"
fi
FMOD_LIBS="$FMOD_LIBS -l$fmod_lib"

_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CXXFLAGS $FMOD_CFLAGS"

AC_MSG_CHECKING(for FMOD headers)
AC_EGREP_CPP( yo_fmod,
[#include <fmod.h>
#ifdef FMOD_VERSION
   yo_fmod
#endif],
  have_fmod_headers="yes",
  have_fmod_headers="no" )

if test "$have_fmod_headers" = "yes"
then
    if test "$fmod_includes"
    then
        AC_MSG_RESULT([$fmod_includes])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

# Test the libraries
AC_MSG_CHECKING(for FMOD libraries)

CPPFLAGS="$CXXFLAGS $FMOD_LIBS"

AC_TRY_LINK( , , have_fmod_libraries="yes", have_fmod_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_fmod_libraries" = "yes"
then
    if test "$fmod_libraries"
    then
        AC_MSG_RESULT([$fmod_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

fmod_libraries="$fmod_libraries"

if test "$have_fmod_headers" = "yes" \
   && test "$have_fmod_libraries" = "yes"
then
    have_fmod="yes"
else
    have_fmod="no"
fi

if test "$have_fmod" = "no" -a "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([FMOD is needed to compile NeL (http://www.fmod.org).])
fi

AC_SUBST(FMOD_CFLAGS)
AC_SUBST(FMOD_LIBS)
AC_SUBST([have_fmod])

])

# =========================================================================
# End of file

