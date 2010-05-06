MAKE_NIGHT_ARGS=-j10
MAKE_ALL_ARGS=-j2
CXX=g++
CC=g++
BIN_DIR=$(HOME)/code/ryzom/tools/scripts/linux

night:
	+make $(MAKE_NIGHT_ARGS) -k -C src night CXX=$(CXX)

all:
	+make $(MAKE_ALL_ARGS) -C src all

mono:
	+make -C src all

clean:
	+make -C src clean

distclean:
	+make -C src distclean

cleansheets:
	+make -C src cleansheets

update:
	+make -C src update

touch:
	+make -C src touch
