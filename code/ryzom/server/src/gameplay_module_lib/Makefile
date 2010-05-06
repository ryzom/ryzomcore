#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_COMMON_SRC) \
		-I.. \
		-I.

# The flags for the linker
LDFLAGS      = $(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_COMMON_SRC)/game_share \
		-L../admin_modules \
		-lnelnet \
		-L/usr/lib \
		-L/usr/X11R6/lib \
		-lnelmisc \
		-lxml2 \
		-lpthread

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libgameplay_module_lib.a
DSP_TARGET	= gameplay_module_lib.vcproj

# The default build rule
all:			check-deps $(TARGETS)


#libgame_share.so: $(OBJS)
#	$(CXX) -o $@ $(OBJS) -shared $(LDFLAGS)

libgameplay_module_lib.a: $(OBJS)
	ar r libgameplay_module_lib.a $(OBJS)

#############################################################################
# Rules

include ../../../Rules.mk

