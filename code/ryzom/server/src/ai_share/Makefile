#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_COMMON_SRC) \
		-I..

LDFLAGS      = 	$(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_COMMON_SRC)/game_share \
		-lgame_share \
		-lnelnet \
		-lnelmisc \
		-lxml2

# The flags for the linker
LDFLAGS2      = 	-L$(RYZOM_PATH)/install/$(DIR_DBG_$(DBG))/lib \
		-lnelnet \
		-lnelgeorges \
		-lnelpacs \
		-L/usr/lib \
		-L/usr/X11R6/lib \
		-lnelmisc \
		-lxml2 \
		-lstlport \
		-lpthread

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libai_share.a
DSP_TARGET	= ai_share.vcproj

# The default build rule
all:			check-deps $(TARGETS)

#libgame_share.so: $(OBJS)
#	$(CXX) -o $@ $(OBJS) -shared $(LDFLAGS)

libai_share.a: $(OBJS)
	ar r libai_share.a $(OBJS)

#############################################################################
# Rules

include ../../../Rules.mk
