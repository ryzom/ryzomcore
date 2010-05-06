#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = -DFAST_MIRROR $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_COMMON_SRC) \
		-I..


LDFLAGS      = 	$(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_COMMON_SRC)/game_share \
		-L../admin_modules \
		-lgame_share \
		-ladmin_modules \
		-lnelgeorges \
		-lnelnet \
		-lnelmisc \
		-lxml2 \
		-lrt\
		-ldl

# The flags for the linker
LDFLAGS2      = 	-L$(HOME)/install/$(DIR_DBG_$(DBG))/lib \
		-lnelnet \
		-lxml2 \
		-L/usr/lib \
		-L/usr/X11R6/lib \
		-L$(HOME)/cvs/code/ryzom/src_v2/game_share \
		-lc \
		-lgame_share \
		-lnelgeorges \
		-lnelmisc \
		-lxml2 \
		-lz \
		-lm \
		-lpthread \
		-lcrypt


#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= frontend_service
DSP_TARGET	= $(TARGETS).vcproj

# The default build rule
all: exe packed_sheets

exe: check-deps $(TARGETS)

$(TARGETS): $(OBJS) ../admin_modules/libadmin_modules.a \
		$(RYZOM_COMMON_SRC)/game_share/libgame_share.a
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
	cp -a $(TARGETS) $(TARGETS)_debug
	strip $(TARGETS)

packed_sheets:
# this service has no packed_sheets to pack
# ./$(TARGETS) $(PACK_SHEETS_FLAGS)


#############################################################################
# Rules

include ../../../Rules.mk

