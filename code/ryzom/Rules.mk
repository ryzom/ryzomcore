#############################################################################
# A few basic default rules and intrinsic rules

# Load objects dependencies
ifeq (Dependencies.mk,$(wildcard Dependencies.mk))
include Dependencies.mk
check-deps:
	@echo
	@echo Dependencies found [OK]
	@echo
else
check-deps:
	@echo
	@echo "No dependencies found [ERROR]"
	@echo "You should try 'make update' first"
	@echo
	@exit 1
endif

# Start off by over-riding the default build rules with our own intrinsics
.SUFFIXES:
.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

# remove object files and core (if any)
clean:
	find . -name "core*" -exec $(RM) {} \;
	find . -name "*.o" -exec $(RM) {} \;
	find . -name "*~" -exec $(RM) {} \;
	find . -name "Dependencies.mk" -exec $(RM) {} \;
	find . -name "Objects.mk" -exec $(RM) {} \;

cleansheets:
	find . -name "*.packed_sheets" -exec $(RM) {} \;

# remove object files, core dump, and executable (if any)
distclean:
	$(MAKE) clean
	$(RM) $(TARGETS)
	$(RM) $(TARGETS)_debug

# make the thing again from scratch
again:
	$(MAKE) distclean
	$(MAKE) $(TARGETS)

#UPDATE_OBJS=`cat $(DSP_TARGET) | grep SOURCE | sed -e 's/\r$$//' | grep "\.cpp$$" | cut -d\\\\ -f3- | tr '\n' ' ' | sed -e 's/=/..\\\\/g' | tr '\n' ' ' | sed -e 's/\\\\/\\//g' | sed -e 's/\.cpp /\.o /g'`

#UPDATE_SRCS=`cat $(DSP_TARGET) | grep SOURCE | sed -e 's/\r$$//' | grep "\.cpp$$" | cut -d\\\\ -f3- | tr '\n' ' ' | sed -e 's/=/..\\\\/g' | tr '\n' ' ' | sed -e 's/\\\\/\\//g'`

UPDATE_OBJS=`cat $(DSP_TARGET) | grep RelativePath | sed -e 's/\\"\r$$//' | grep "\.cpp$$"  | cut -d\\\\ -f2- | tr '\n' ' ' | sed -e 's/\\\\/\\//g' | sed -e 's/\.cpp /\.o /g'`
UPDATE_SRCS=`cat $(DSP_TARGET) | grep RelativePath | sed -e 's/\\"\r$$//' | grep "\.cpp$$"  | cut -d\\\\ -f2- | tr '\n' ' ' | sed -e 's/\\\\/\\//g'`

dep: update

update:
#	../gen_compile_flags.sh > RyzomCompilerFlags.mk
	$(RYZOM_PATH)/gen_deps.sh $(CXX) $(CXXFLAGS) -- $(UPDATE_SRCS) > Dependencies.mk
	echo "OBJS=$(UPDATE_OBJS)" > Objects.mk

touch:
	$(RM) $(TARGETS)
	$(RM) $(TARGETS)_debug
