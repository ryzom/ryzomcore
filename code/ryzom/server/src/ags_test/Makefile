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

# The flags for the linker
LDFLAGS      = $(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_COMMON_SRC)/game_share \
		-lgame_share \
		-lnelnet \
		-lnelgeorges \
		-lnelligo \
		-lnelpacs \
		-lnelmisc \
		-lxml2

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= ags_test
DSP_TARGET  = $(TARGETS).vcproj

# The default build rule
all:			check-deps $(TARGETS)

ags_test: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

#############################################################################
# Rules

include ../../../Rules.mk

