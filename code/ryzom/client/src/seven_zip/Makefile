#############################################################################
# Simple make file for compiling SEVEN ZIP

#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-D_SZ_ONE_DIRECTORY \
		-I$(HOME)/code/nel/include \
		-I$(RYZOM_PATH)/common/src \
		-I.. \
		-I.

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libseven_zip.a
DSP_TARGET	= seven_zip.vcproj

# The default build rule
all:			check-deps $(TARGETS)


#libseven_zip.so: $(OBJS)
#	$(CXX) -o $@ $(OBJS) -shared $(LDFLAGS)

libseven_zip.a: $(OBJS)
	ar r libseven_zip.a $(OBJS)

#############################################################################
# Rules

include ../../../Rules.mk
