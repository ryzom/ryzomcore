FIND_PACKAGE(LIBGSF)

IF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))
	MESSAGE(FATAL_ERROR "LIBGSF not found!")
ENDIF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))

FIND_PACKAGE(GOBJECT2 REQUIRED)
FIND_PACKAGE(GLIB2 REQUIRED)

FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_corpus_test
	${SRCS}
	${HDRS}
	)

TARGET_INCLUDE_DIRECTORIES(pipeline_max_corpus_test
		PRIVATE
		${GLIB2_INCLUDE_DIRS}
		${LIBGSF_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(pipeline_max_corpus_test
	${LIBGSF_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GOBJECT2_LIBRARIES}
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

	ADD_TEST(NAME pipeline_max_zone_corpus
		COMMAND ${Python3_EXECUTABLE} zone_corpus.py --all --gate-t3
			--bin $<TARGET_FILE_DIR:pipeline_max_export_zone>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_zone_corpus PROPERTIES SKIP_RETURN_CODE 77)

	ADD_TEST(NAME pipeline_max_shape_corpus
		COMMAND ${Python3_EXECUTABLE} shape_corpus.py --all --gate-t3 --min-identical 700 --max-diff 400
			--bin $<TARGET_FILE_DIR:pipeline_max_export_shape>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	SET_TESTS_PROPERTIES(pipeline_max_shape_corpus PROPERTIES SKIP_RETURN_CODE 77)

	IF (TARGET unit_test)
		ADD_DEPENDENCIES(unit_test pipeline_max_corpus_test pipeline_max_export_skel pipeline_max_export_anim pipeline_max_export_swt pipeline_max_export_ig pipeline_max_export_zone pipeline_max_export_shape)
	ENDIF (TARGET unit_test)
ENDIF (Python3_Interpreter_FOUND)
