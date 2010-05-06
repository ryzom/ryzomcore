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
		-L../admin_modules \
		-lpd_lib \
		-lgame_share \
		-lnelgeorges \
		-ladmin_modules \
		-lnelnet \
		-lnelmisc \
		-lxml2 \
		-lrt\
		-ldl

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= log_analyser_service
DSP_TARGET	= $(TARGETS).vcproj

# The default build rule
all: exe packed_sheets

exe: check-deps $(TARGETS)

$(TARGETS): $(OBJS) ../admin_modules/libadmin_modules.a
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
	cp -a $(TARGETS) $(TARGETS)_debug
	strip $(TARGETS)

packed_sheets:
# this service has no packed_sheets to pack
# ./$(TARGETS) $(PACK_SHEETS_FLAGS)


#############################################################################
# Rules

include ../../../Rules.mk
