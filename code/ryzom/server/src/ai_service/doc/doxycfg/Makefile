PROJECT=ryzom-ais
DOX_DIR=../source
CPP_DIR=../..
OUTPUT_DIR=../doxydoc
CFG_FILE=ryzom-ais.cfg

DOX_FILES=index.dox fight.dox aggro.dox dynamic_bots.dox path_finding.dox primitive_parsing.dox positions.dox
CPP_FILES=script_compiler_native_func.cpp \
	nf_helpers.cpp nf_static.cpp nf_state_instance.cpp nf_grp.cpp nf_grp_npc.cpp \
	ai.h ai.cpp \
	ai_instance.h ai_instance.cpp \
	ai_mgr.h ai_mgr.cpp \
	ai_mgr_npc.h ai_mgr_npc.cpp \
	ai_mgr_fauna.h ai_mgr_fauna.cpp \
	ai_grp.h ai_grp.cpp \
	ai_grp_npc.h ai_grp_npc.cpp \
	ai_grp_fauna.h ai_grp_fauna.cpp \
	event_reaction_container.h \
	ai_pos.h ai_pos_mirror.h ai_pos_mirror_inline.h \
	ai_vector_mirror.h \
	../ai_share/ai_coord.h \
	../ai_share/ai_vector.h \
	../ai_share/world_map.h

SOURCE_FILES=$(patsubst %,$(DOX_DIR)/%,$(DOX_FILES)) $(patsubst %,$(CPP_DIR)/%,$(CPP_FILES))

DOXYGEN_TAG=$(OUTPUT_DIR)/$(PROJECT)/$(PROJECT).tag

all:$(DOXYGEN_TAG)

$(DOXYGEN_TAG):$(SOURCE_FILES) $(CFG_FILE)
	doxygen ryzom-ais.cfg 2>>doxygen.warnings

$(CFG_FILE):Makefile $(CFG_FILE).tmpl $(SOURCE_FILES)
	cat $(CFG_FILE).tmpl > $@
	echo "INPUT = $(SOURCE_FILES)" >> $@
