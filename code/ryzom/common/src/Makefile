
BUILD_DIRS = game_share

night: all

all: $(foreach dir,$(BUILD_DIRS),$(dir)-all)

%-all:
	+make -k -C $* all

clean: $(foreach dir,$(BUILD_DIRS),$(dir)-clean)

%-clean:
	+make -C $* clean

distclean: $(foreach dir,$(BUILD_DIRS),$(dir)-distclean)

%-distclean:
	+make -C $* distclean

cleansheets: $(foreach dir,$(BUILD_DIRS),$(dir)-cleansheets)

%-cleansheets:
	+make -C $* cleansheets

update: $(foreach dir,$(BUILD_DIRS),$(dir)-update)

%-update:
	+make -C $* update

touch: $(foreach dir,$(BUILD_DIRS),$(dir)-touch)

%-touch:
	+make -C $* touch

.NOTPARALLEL: night all clean cleansheets update touch
