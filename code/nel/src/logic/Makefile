#############################################################################
# Simple make file for compiling admin modules

#############################################################################
# Setting up the compiler settings...

# Global variables
include ../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I /usr/include/stlport \
		-I /usr/include/libxml2 \
		-I ../../include \
		-I.. \

# The flags for the linker
LDFLAGS      = 


#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGET		= ../../libnellogic.a
DSP_TARGET	= logic.dsp

# The default build rule
all:			check-deps $(TARGET)

$(TARGET): $(OBJS)
	ar r $@ $(OBJS)


#############################################################################
# Rules

include ../Rules.mk

