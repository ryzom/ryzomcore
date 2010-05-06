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
		-L../pd_lib \
		-lpd_lib \
		-lgame_share \
		-lnelgeorges \
		-lnelnet \
		-lnelmisc \
		-lxml2 \
		-lrt\
		-ldl

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= reference_builder_service
DSP_TARGET	= pd_reference_builder.vcproj

# The default build rule
all:			check-deps $(TARGETS)

reference_builder_service: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
	cp -a $(TARGETS) $(TARGETS)_debug
	strip $(TARGETS)


#############################################################################
# Rules

include ../../../Rules.mk
