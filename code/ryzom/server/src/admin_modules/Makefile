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
LDFLAGS      = 	$(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-lnelnet \
		-lnelmisc \
		-lrt\
		-ldl

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libadmin_modules.a
DSP_TARGET	= admin_modules.vcproj

# The default build rule
all:			check-deps $(TARGETS)

libadmin_modules.a: $(OBJS)
	ar r libadmin_modules.a $(OBJS)

#############################################################################
# Rules

include ../../../Rules.mk
