
night:
	+make $(MAKE_NIGHT_ARGS) -k -C src night CXX=$(CXX)
	$(BIN_DIR)/publish_nel
	$(BIN_DIR)/publish_src

all:
	+make $(MAKE_ALL_ARGS) -C src all

mono:
	+make -C src all

clean:
	+make -C src clean

cleansheets:
	+make -C src cleansheets

update:
	+make -C src update

touch:
	+make -C src touch
