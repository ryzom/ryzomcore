#############################################################################
# Setting up the global compiler settings...

# The names of the executables
CXX           = c++
RM            = rm -f
MAKE          = make

DBG           = off

FLAGS_CMN     = -g -pipe -Wno-ctor-dtor-privacy -Wno-multichar -D_REENTRANT -DHAVE_X86

FLAGS_DBG_on  = -O0 -finline-functions -DNL_DEBUG -DNL_DEBUG_FAST
FLAGS_DBG_off = -O3 -ftemplate-depth-24 -funroll-loops -DNL_RELEASE_DEBUG
DIR_DBG_on    = debug
DIR_DBG_off   = release

ifeq (Objects.mk,$(wildcard Objects.mk))
include Objects.mk
endif
