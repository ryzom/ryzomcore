#############################################################################
# Simple make file for compiling Ryzom client

#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../Variables.mk

WWWFLAGS = $(shell libwww-config --cflags)
WWWLDFLAGS = $(shell libwww-config --libs)
LUAFLAGS = $(shell pkg-config lua5.1 --cflags)
LUALDFLAGS = $(shell pkg-config lua5.1 --libs)

# The flags for the C++ compiler
CXXFLAGS     = $(WWWFLAGS) $(LUAFLAGS) \
		$(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I$(RYZOM_PATH)/../../code/luabind \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_PATH)/common/src \
		-I$(RYZOM_PATH)/client/src/seven_zip \
		-I.. \
		-I.

LDFLAGS      = 	$(WWWLDFLAGS) $(LUALDFLAGS) $(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L$(RYZOM_PATH)/common/src/game_share \
		-L$(RYZOM_PATH)/client/src/client_sheets \
		-L$(RYZOM_PATH)/client/src/seven_zip \
		-lclient_sheets \
		-lseven_zip \
		-lgame_share \
		-Wl,--start-group -lnel_drv_opengl -lnel3d -Wl,--end-group \
		-Wl,--start-group -lnel_drv_openal -lnelsnd -Wl,--end-group \
		-lnelpacs \
		-lnelligo \
		-lnelgeorges \
		-lnelnet \
		-lnelmisc \
		-lfreetype \
		-lxml2 \
		-ljpeg \
		-lpng \
		-lrt \
		-ldl \
		-lluabind \
		-lopenal \
		-lcurl \
		-lGL \
		-lX11

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= client
DSP_TARGET	= $(TARGETS).vcproj

# The default build rule
all: client_sheets seven_zip exe

exe: check-deps $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
	cp -a $(TARGETS) $(TARGETS)_debug
	strip $(TARGETS)

client_sheets:
	make $(MAKE_ALL_ARGS) -C client_sheets all

seven_zip:
	make $(MAKE_ALL_ARGS) -C seven_zip all

#############################################################################
# Rules

include ../../Rules.mk
