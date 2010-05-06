ifeq (Dependencies.mk,$(wildcard Dependencies.mk))
include Dependencies.mk
endif

BUILD_DIRS = \
misc\
net\
pacs\
memory\
georges\
ligo\
logic\

SRC_DIRS=$(foreach dir,$(BUILD_DIRS),src/$(dir))
INCLUDE_DIRS=$(foreach dir,$(BUILD_DIRS),include/nel/$(dir))

BIN_DIR=$HOME/static/script/linux

TARGET=src_nel.tgz

CXX=distcc
CC=distcc
MAKE_ARGS=-j20

night: all
mono: all
all: $(TARGET)

update:
	if [ -e FileList ]; then rm FileList; fi
	for f in $(INCLUDE_DIRS); \
		do \
		find $$f -name "*.h" >> FileList; \
		find $$f -name "Makefile" >> FileList; \
	done
	for f in $(SRC_DIRS); \
		do \
		find $$f -name "*.cpp" >>FileList; \
		find $$f -name "*.h" >>FileList; \
		find $$f -name "*.dsp" >>FileList; \
		find $$f -name "*.def" >>FileList; \
		find $$f -name "Makefile" >>FileList; \
		find $$f -name "*.h" >>FileList; \
	done
	if [ -e Dependencies.mk ] ; then rm Dependencies.mk; fi
	echo -n src_nel.tgz:>> Dependencies.mk
	awk '{printf "\\\n" $$0 }' FileList >> Dependencies.mk
	echo >> Dependencies.mk
	for f in $(SRC_DIRS); do echo -e \\t+make -C $$f 'CXX=$$(CXX) CC=$$(CC) $$(MAKE_ARGS)'>> Dependencies.mk ; done
	echo -e \\t'if [ -e '$(TARGET)' ] ; then rm '$(TARGET)'; fi'>> Dependencies.mk
	echo -e \\ttar cvzf $(TARGET) -T FileList>> Dependencies.mk
	for f in $(SRC_DIRS); do make -C $$f update ; done

clean:
	for f in $(SRC_DIRS); do make -C $$f clean ; done

touch:
	for f in $(SRC_DIRS); do make -C $$f touch ; done

.NOTPARALLEL:
