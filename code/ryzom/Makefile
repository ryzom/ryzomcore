MAKE_NIGHT_ARGS=-j2
MAKE_ALL_ARGS=-j2
CXX=g++
CC=g++
BIN_DIR=$(RYZOM_PATH)/tools/scripts/linux

night:
	+make $(MAKE_NIGHT_ARGS) -k -C common night CXX=$(CXX)
	+make $(MAKE_NIGHT_ARGS) -k -C server night CXX=$(CXX)

all:
	+make $(MAKE_ALL_ARGS) -C common all
	+make $(MAKE_ALL_ARGS) -C server all

mono:
	+make -C common all
	+make -C server all

clean:
	+make -C common clean
	+make -C server clean

distclean:
	+make -C common distclean
	+make -C server distclean

cleansheets:
	+make -C common cleansheets
	+make -C server cleansheets

update:
	+make -C common update
	+make -C server update

touch:
	+make -C common touch
	+make -C server touch
