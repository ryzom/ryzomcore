#############################################################################
# Simple make file for compiling NeL examples

#############################################################################
# Setting up the compiler settings...

# The names of the executables
CXX          = c++
RM           = rm -f
MAKE         = make

# The flags for the C++ compiler
CXXFLAGS     = -g -pipe -D_REENTRANT -D_GNU_SOURCE \
				-I$(HOME)/cvs/code/nel/include \
				-I$(HOME)/installs/STLport-4.0/stlport \
				-I/usr/local/include \
				-I/usr/local/include/freetype2

# The flags for the linker
LDFLAGS      = -L$(HOME)/build/debug/nel/lib \
				-L/home/installs/STLport-4.0/lib \
				-L/usr/local/lib \
				-lnelmisc \
				-lnelnet \
				-lstlport_gcc \
				-lpthread \
				-lfreetype

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= frontend_service client ping_service

# The default build rule
all:			$(TARGETS)

frontend_service: frontend_service.o
	$(CXX) -o $@ $< $(LDFLAGS)

ping_service: ping_service.o
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
