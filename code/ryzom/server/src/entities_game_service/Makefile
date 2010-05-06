#############################################################################
# Setting up the compiler settings...

# Global variables
include ../../../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I/usr/include/libxml2 \
		-I$(NEL_INCLUDE) \
		-I$(RYZOM_COMMON_SRC) \
		-I.. \
		-I.

LDFLAGS      = 	$(LD_FLAGS_CMN) \
		-L$(NEL_PATH)/lib \
		-L../admin_modules \
		-L$(RYZOM_COMMON_SRC)/game_share \
        -L../ai_share \
        -L../server_share \
		-L../pd_lib \
		-L../gameplay_module_lib \
		-ladmin_modules \
		-lpd_lib \
		-lai_share \
        -lserver_share \
		-lgame_share \
		-lnelligo \
		-lnelgeorges \
		-lgameplay_module_lib \
		-lnelnet \
		-lnelmisc \
		-lpd_lib \
		-lxml2 \
		-lrt\
		-ldl

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= entities_game_service
DSP_TARGET	= $(TARGETS).vcproj
PACKED_SHEETS = \
	egs_action_xp_factor.packed_sheets \
	egs_aiactions.packed_sheets \
	egs_continents.packed_sheets \
	egs_creatures.packed_sheets \
	egs_death_impact.packed_sheets \
	egs_emot.packed_sheets \
	egs_encyclo_album.packed_sheets \
	egs_encyclo_thema.packed_sheets \
	egs_items.packed_sheets \
	egs_loot_set.packed_sheets \
	egs_loot_table.packed_sheets \
	egs_outpost_building.packed_sheets \
	egs_outpost_squads.packed_sheets \
	egs_outposts.packed_sheets \
	egs_race_stats.packed_sheets \
	egs_sbricks.packed_sheets \
	egs_skill_tree.packed_sheets \
	egs_sphrases.packed_sheets \
	egs_starting_role.packed_sheets \
	egs_success_chance_tables.packed_sheets \
	egs_text_emotes.packed_sheets \
	egs_weather_function_params.packed_sheets \
	egs_weather_setup.packed_sheets \
	egs_world.packed_sheets \
	egs_xptables.packed_sheets

# The default build rule
all: exe packed_sheets

exe: check-deps $(TARGETS)

$(TARGETS): $(OBJS) ../admin_modules/libadmin_modules.a \
		$(RYZOM_COMMON_SRC)/game_share/libgame_share.a \
		../server_share/libserver_share.a \
		../pd_lib/libpd_lib.a \
		../gameplay_module_lib/libgameplay_module_lib.a
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
	cp -a $(TARGETS) $(TARGETS)_debug
	strip $(TARGETS)

packed_sheets: $(TARGETS)
	/bin/sh ../update_packed_sheets.sh ./$(TARGETS) $(PACKED_SHEETS)


#############################################################################
# Rules

include ../../../Rules.mk
