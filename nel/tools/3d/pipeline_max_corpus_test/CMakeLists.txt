FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_corpus_test
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_corpus_test
	pipeline_max
	NeL::misc
)

NL_DEFAULT_PROPS(pipeline_max_corpus_test "NeL, Tools, 3D: Pipeline Max Corpus Test")
NL_ADD_RUNTIME_FLAGS(pipeline_max_corpus_test)

INSTALL(TARGETS pipeline_max_corpus_test RUNTIME DESTINATION bin COMPONENT toolsmisc)

# skel_corpus.py drives T1 (structural roundtrip)/T2 (parse-build roundtrip)/T3 (.skel export
# accuracy) over the full skel-source .max corpus enumerated from the ryzomcore_leveldesign
# workspace configs. It needs the private ryzomcore_graphics/ryzomcore_leveldesign asset
# checkouts and reference core4_data .skel files that aren't part of this repo, so it self-skips
# (exit 77) rather than failing when those aren't present — e.g. any machine other than
# Kaetemi's. T1/T2 byte-identity mismatches fail the test; T3 .skel-export accuracy is gated
# against the recorded accuracy floor (--gate-t3: biped size-match 100%, drot exact >= 97%,
# dpos exact+close >= 72%, non-biped bit-exact) so decode regressions fail while byte-exactness
# remains an aspiration, not a requirement.
FIND_PACKAGE(Python3 COMPONENTS Interpreter)
IF (Python3_Interpreter_FOUND)
	ADD_TEST(NAME pipeline_max_skel_corpus
		COMMAND ${Python3_EXECUTABLE} skel_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_skel>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_skel_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# Same corpus, driven through the VS2008/Wine x87 reference-codegen build (see the "VS2008
	# x87 reference build" wiki page) via the winebin/ wrapper scripts, which translate Unix
	# paths to Windows form so the native x64 harness can drive the .exe under Wine. This is a
	# second, independent build tree (not part of this CMake project — ~/build_vs2008_wine_pipeline
	# is a personal path, hence hardcoded rather than a CMake target reference) that matches the
	# Max 2010 reference exporter's own compiler/float model. Confirms T1/T2 byte-identity and the
	# T3 accuracy floor hold independently of SSE-vs-x87 codegen, not just on the x64/SSE build —
	# self-skips (exit 77) on any machine without that build, same as every other line here.
	ADD_TEST(NAME pipeline_max_skel_corpus_vs2008
		COMMAND ${Python3_EXECUTABLE} skel_corpus.py --all --gate-t3
			--bin $ENV{HOME}/build_vs2008_wine_pipeline/winebin
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_skel_corpus_vs2008 PROPERTIES SKIP_RETURN_CODE 77)

	# regen_corpus.py validates the fresh-format (Max 9, figure-version 0x0115 == 0) biped decode
	# against the regenerated corpus in ~/biped_regen: T1/T2 roundtrip over the regenerated .max
	# files plus a bone-by-bone compare of our reconstructed figure-mode world transforms against
	# the manifest.txt ground truth Max 9 logged when writing them (encode-direction
	# cross-validation, see pipeline_max_design.md). Self-skips when ~/biped_regen isn't present.
	ADD_TEST(NAME pipeline_max_regen_corpus
		COMMAND ${Python3_EXECUTABLE} regen_corpus.py --all --gate-gt
			--bin $<TARGET_FILE_DIR:pipeline_max_export_skel>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_regen_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# anim_corpus.py drives the same tiers over the anim-source corpus. T1/T2 run on the
	# non-biped subset here to keep the ctest runtime bounded (the full 4.5k-file anim corpus
	# T1/T2 sweep is a manual/occasional run); T3 exports every non-biped anim and gates:
	# direct pre-optimizer references must be byte-identical, optimizer-processed references
	# must reconstruct within the optimizer's own key-drop tolerance.
	# swt_corpus.py: T1/T2 over the SwtSourceDirectories sources + T3 export byte-compare
	# against the direct reference .swt (era-tolerant for nodes flagged after the reference
	# export; see the ERA_EXTRA_NODES list in the driver).
	ADD_TEST(NAME pipeline_max_swt_corpus
		COMMAND ${Python3_EXECUTABLE} swt_corpus.py
			--bin $<TARGET_FILE_DIR:pipeline_max_export_swt>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_swt_corpus PROPERTIES SKIP_RETURN_CODE 77)

	ADD_TEST(NAME pipeline_max_anim_corpus
		COMMAND ${Python3_EXECUTABLE} anim_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_anim>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_anim_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# Same corpus, driven through the VS2008/Wine x87 reference-codegen build (see the "VS2008
	# x87 reference build" wiki page), same rationale as pipeline_max_skel_corpus_vs2008: confirms
	# the T1/T2 roundtrip and the T3 direct-reference byte-identity/structural floor hold under
	# the Max-2010-matching compiler and float model, not just x64/SSE. anim_builder IS built in
	# that tree too (nel/tools/3d/anim_builder is unconditional under WITH_3D, no pipeline-native-OLE
	# gate), so the optimized-reference tier runs for real there — but it is informational only for
	# both biped and nonbiped (anim_corpus.py never gates on it): it chains our exporter's output
	# through a SEPARATE tool (anim_builder) whose own key-drop/quantization thresholds already sit
	# within a few thousandths of the tolerance on the original x64/SSE build, so switching either
	# tool's codegen can tip an already-borderline fauna file over — confirmed 2026-07-07, see the
	# design doc. The direct tier (byte-identity for nonbiped, structural-floor for biped) and T1/T2
	# are what's actually gated here. Self-skips (exit 77) on any machine without that build.
	ADD_TEST(NAME pipeline_max_anim_corpus_vs2008
		COMMAND ${Python3_EXECUTABLE} anim_corpus.py --all --gate-t3
			--bin $ENV{HOME}/build_vs2008_wine_pipeline/winebin
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_anim_corpus_vs2008 PROPERTIES SKIP_RETURN_CODE 77)

	# ig_corpus.py: T1/T2 over the IgOtherSourceDirectories/IgLandSourceDirectories sources +
	# T3 export with the two-tier reference comparison (direct raw intermediates under
	# ~/pipeline_export with the uninitialized-bytes mask, processed client data under
	# ~/core4_data with lighting/elevation masked). --gate-t3 fails on T1/T2 mismatches, export
	# failures, and direct-tier field-compare failures beyond the recorded known-deviation
	# budget (--max-direct-diff, currently 1: the mirror-modifier default-gizmo case).
	# prim_check.py: parametric primitive topology (--dump-prim) vs the Max 9 ground-truth
	# dump in ~/prim_mesh_dataset (gen_prim_mesh_dataset.ms) — exact face indices, vertex
	# positions to manifest precision. Self-skips without the dataset.
	ADD_TEST(NAME pipeline_max_prim_mesh
		COMMAND ${Python3_EXECUTABLE} prim_check.py
			--bin $<TARGET_FILE_DIR:pipeline_max_export_ig>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_prim_mesh PROPERTIES SKIP_RETURN_CODE 77)

	ADD_TEST(NAME pipeline_max_ig_corpus
		COMMAND ${Python3_EXECUTABLE} ig_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_ig>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_ig_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# ligo_ig_corpus.py: the ligo-brick side of ig export (build_gamedata processes/ligo's
	# exportInstanceGroupFromZone), as opposed to ig_corpus.py's standalone processes/ig path —
	# zonematerial/zonespecial/zonetransition .max under landscape/ligo/<eco>/max, compared
	# against ~/pipeline_export/ecosystems/<eco>/ligo_es/igs. --gate-t3 budgets the known
	# selection-order and cluster-membership diff classes (DIFF_BUDGET; see the script docstring).
	ADD_TEST(NAME pipeline_max_ligo_ig_corpus
		COMMAND ${Python3_EXECUTABLE} ligo_ig_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_ig>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_ligo_ig_corpus PROPERTIES SKIP_RETURN_CODE 77)

	ADD_TEST(NAME pipeline_max_zone_corpus
		COMMAND ${Python3_EXECUTABLE} zone_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_zone>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_zone_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# Same zone corpus, driven through the VS2008/Wine x87 reference-codegen build (see the "VS2008
	# x87 reference build" wiki page), same rationale as pipeline_max_skel_corpus_vs2008 /
	# pipeline_max_anim_corpus_vs2008: the .zone format quantizes patch control points into sint16
	# via (v - PatchBias) / PatchScale, and the reference plugin was a 32-bit x87 build that keeps
	# that quotient at extended precision (one rounding before the truncation), where the x64/SSE
	# build rounds the subtraction and the division separately. This is the codegen-limited residual
	# class documented in pipeline_max_design.md section 10h (the +-1 packed-word flips at
	# quantization boundaries). The VS2008 build confirms how much of the x87-tier closes under the
	# Max-2010-matching compiler/float model and pins T1/T2 byte-identity independent of SSE-vs-x87
	# codegen (the chunk parser/writer never recomputes a float). Self-skips (exit 77) on any machine
	# without that build tree, same convention as every other *_vs2008 line here.
	ADD_TEST(NAME pipeline_max_zone_corpus_vs2008
		COMMAND ${Python3_EXECUTABLE} zone_corpus.py --all --gate-t3
			--bin $ENV{HOME}/build_vs2008_wine_pipeline/winebin
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_zone_corpus_vs2008 PROPERTIES SKIP_RETURN_CODE 77)

	ADD_TEST(NAME pipeline_max_shape_corpus
		COMMAND ${Python3_EXECUTABLE} shape_corpus.py --all --gate-t3 --min-identical 700 --max-diff 400
			--bin $<TARGET_FILE_DIR:pipeline_max_export_shape>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_shape_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# pacs_prim_corpus.py: T1/T2 over PacsPrimSourceDirectories (stuff/<race>/decors/vegetations)
	# + T3 export byte-compare against ~/pipeline_export/ecosystems/<eco>/pacs_prim. Landing state:
	# 509 corpus files, 493 with PACS primitives, all byte-identical.
	ADD_TEST(NAME pipeline_max_pacs_prim_corpus
		COMMAND ${Python3_EXECUTABLE} pacs_prim_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_pacs_prim>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_pacs_prim_corpus PROPERTIES SKIP_RETURN_CODE 77)

	# cmb_corpus.py: T1/T2/T3 over the 16 direct RBankCmbSourceDirectories construction files
	# (gated, DIRECT_DIFF_BUDGET) plus an informational (non-gated) sweep of the ligo brick
	# corpus via --ligo. See the script docstring for the known open diff classes.
	ADD_TEST(NAME pipeline_max_cmb_corpus
		COMMAND ${Python3_EXECUTABLE} cmb_corpus.py --t1 --t2 --t3 --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_cmb>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_cmb_corpus PROPERTIES SKIP_RETURN_CODE 77)

	IF (TARGET unit_test)
		ADD_DEPENDENCIES(unit_test pipeline_max_corpus_test pipeline_max_export_skel pipeline_max_export_anim pipeline_max_export_swt pipeline_max_export_ig pipeline_max_export_zone pipeline_max_export_shape pipeline_max_export_pacs_prim pipeline_max_export_cmb)
	ENDIF (TARGET unit_test)
ENDIF (Python3_Interpreter_FOUND)
