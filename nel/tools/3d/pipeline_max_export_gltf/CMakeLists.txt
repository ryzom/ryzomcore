# max2gltf shares the direct shape exporter's extraction sources (branch at the pre-build
# model — design doc drafts/max2gltf_plan.md): the same scene_lib/mesh_eval/material_build/
# mesh_build/interface_build objects feed both the .shape serializer and the glTF encoder.
SET(SHAPE_SHARED
	../pipeline_max_export_shape/scene_lib.cpp
	../pipeline_max_export_shape/mesh_eval.cpp
	../pipeline_max_export_shape/material_build.cpp
	../pipeline_max_export_shape/mesh_build.cpp
	../pipeline_max_export_shape/interface_build.cpp
	../pipeline_max_export_shape/water_build.cpp
	../pipeline_max_export_shape/remanence_build.cpp
	../pipeline_max_export_shape/flare_build.cpp
)

# The ig exporter's whole per-file flow (selection, XRef resolution, buildInstanceGroup) backs
# the nel_igs blob — compiled in with its standalone main() guarded out.
SET(IG_SHARED ../pipeline_max_export_ig/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${IG_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_IG_NO_MAIN)

# The anim exporter's whole per-file flow (Bip01-first selection, biped COM decode, track
# build) backs the nel_anim blob — same pattern. biped_anim/bip_file ride along for the
# BIPANIM sampling the selection path uses (biped_author is main()-only, guarded out).
SET(ANIM_SHARED
	../pipeline_max_export_anim/main.cpp
	../pipeline_max_export_anim/biped_anim.cpp
	../pipeline_max_export_anim/bip_file.cpp
)
SET_SOURCE_FILES_PROPERTIES(../pipeline_max_export_anim/main.cpp PROPERTIES COMPILE_DEFINITIONS PMB_ANIM_NO_MAIN)

# The ligo zone exporter's whole per-file flow backs the nel_zones blob + the tessellated
# nel_proxy viewing meshes — same guarded-main pattern.
SET(ZONE_SHARED ../pipeline_max_export_zone/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${ZONE_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_ZONE_NO_MAIN)

# Single-output processes (full-parity rule): swt + pacs_prim share their tools' only code
# path — the blob and the standalone file cannot drift.
SET(SWT_SHARED ../pipeline_max_export_swt/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${SWT_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_SWT_NO_MAIN)
SET(PACS_PRIM_SHARED ../pipeline_max_export_pacs_prim/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${PACS_PRIM_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_PACS_PRIM_NO_MAIN)
SET(VEGET_SHARED ../pipeline_max_export_veget/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${VEGET_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_VEGET_NO_MAIN)

FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_gltf
	${SRCS}
	${HDRS}
	${SHAPE_SHARED}
	${IG_SHARED}
	${ANIM_SHARED}
	${ZONE_SHARED}
	${SWT_SHARED}
	${PACS_PRIM_SHARED}
	${VEGET_SHARED}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_gltf
	pipeline_max
	pipeline_max_export_common
	nel_gltf
	NeL::misc
	NeL::3d
	NeL::ligo
	NeL::pacs
)

NL_DEFAULT_PROPS(pipeline_max_export_gltf "NeL, Tools, 3D: Pipeline Max Export glTF")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_gltf)

INSTALL(TARGETS pipeline_max_export_gltf RUNTIME DESTINATION bin COMPONENT toolsmisc)
