#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I/usr/include/mysql \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_COMMON_SRC) \
		-I.. \
		-I.

LDFLAGS      = $(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_COMMON_SRC)/game_share \
		-L../admin_modules \
		-L../server_share \
		-lmysqlclient \
		-lserver_share \
		-lgame_share \
		-ladmin_modules \
		-lnelgeorges \
		-lnelnet \
		-lnelmisc \
		-lxml2 \
		-lrt\
		-ldl

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= session_browser_server
DSP_TARGET	= $(TARGETS).vcproj

# The default build rule
all: exe packed_sheets

exe: check-deps $(TARGETS)

$(TARGETS): $(OBJS) \
		../admin_modules/libadmin_modules.a \
		../server_share/libserver_share.a \
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

