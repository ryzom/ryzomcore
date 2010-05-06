
BUILD_DIRS = \
server_share \
ai_share \
admin_modules \
ryzom_admin_service \
ryzom_naming_service \
mirror_service \
ryzom_welcome_service \
tick_service \
frontend_service \
gpm_service \
pd_lib \
input_output_service \
gameplay_module_lib \
entities_game_service \
ai_service \
backup_service \
dynamic_scenario_service \
shard_unifier_service \
session_browser_server \
pd_support_service \
monitor_service \
log_analyser_service \
mail_forum_service \
logger_service

# removed service
#pd_reference_builder \

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

packed_sheets: $(foreach dir,$(BUILD_DIRS),$(dir)-packed_sheets)

%-packed_sheets:
	-make -C $* packed_sheets

exe: $(foreach dir,$(BUILD_DIRS),$(dir)-exe)

%-exe:
	-make -C $* exe

.NOTPARALLEL: night all clean cleansheets update touch
