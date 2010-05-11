#############################################################################
# Simple make file for compiling CLIENT SHEETS

#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I$(NEL_PATH)/include \
		-I$(RYZOM_PATH)/common/src \
		-I.. \
		-I.

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libclient_sheets.a
DSP_TARGET	= client_sheets.vcproj

# The default build rule
all:			check-deps $(TARGETS)


#libclient_sheets.so: $(OBJS)
#	$(CXX) -o $@ $(OBJS) -shared $(LDFLAGS)

libclient_sheets.a: $(OBJS)
	ar r libclient_sheets.a $(OBJS)

#############################################################################
# Rules

include ../../../Rules.mk
