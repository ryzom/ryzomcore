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
                                -I$(HOME)/install/debug/include \
                                -I/home/installs/STLport-4.5.1/stlport \
                                -I/usr/local/include \

# The flags for the linker
LDFLAGS      = -L$(HOME)/build/debug/nel/lib \
                                -L/home/installs/STLport-4.5.1/lib \
                                -L/usr/local/lib \
                                -lnelmisc \
                                -lnelnet \
                                -lstlport_gcc \
                                -lpthread

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS         = bench_service client

# The default build rule
all:                    $(TARGETS)

OBJS_SERVICE = bench_service.o receive_task.o simlag.o

bench_service: $(OBJS_SERVICE)
        $(CXX) -o $@ $(OBJS_SERVICE) $(LDFLAGS)

OBJS_CLIENT = client.o simlag.o

client: $(OBJS_CLIENT)
        $(CXX) -o $@ $(OBJS_CLIENT) $(LDFLAGS)


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
