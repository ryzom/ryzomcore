#############################################################################
# Simple make file for compiling NeL examples

#############################################################################
# Setting up the compiler settings...

# The names of the executables
CXX          = g++
RM           = rm -f
MAKE         = make

# The flags for the C++ compiler
CXXFLAGS     = -g -pipe -D_REENTRANT -D_GNU_SOURCE \
				-I/home/installs/STLport-4.0/stlport \
				-I$(HOME)/install/release/include \
				-I/usr/local/include \

# The flags for the linker
LDFLAGS      = \
				-L/home/installs/STLport-4.0/lib \
				-L$(HOME)/install/release/lib \
				-L/usr/local/lib \
				-L/usr/lib \
				-lnelmisc \
				-lnelnet \
				-lstlport_gcc \
				-lpthread \

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= frontend_service client

# The default build rule
all:			$(TARGETS)

frontend_service: frontend_service.o
	$(CXX) -o $@ $< $(LDFLAGS)

client: client.o
	$(CXX) -o $@ $< $(LDFLAGS)


#############################################################################
# A few basic default rules and intrinsic rules

# Start off by over-riding the default build rules with our own intrinsics
.SUFFIXES:
.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -c $(CXXFLAGS) $<


# remove object files and core (if any)
clean:
	$(RM) *.o core

# remove object files, core dump, and executable (if any)
distclean:
	$(MAKE) clean
	$(RM) $(TARGET)

# make the thing again from scratch
again:
	$(MAKE) distclean
	$(MAKE) $(TARGET)
