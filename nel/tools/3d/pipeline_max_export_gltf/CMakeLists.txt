# max2gltf shares the direct shape exporter's extraction sources (branch at the pre-build
# model — design doc drafts/max2gltf_plan.md): the same scene_lib/mesh_eval/material_build/
# mesh_build/interface_build objects feed both the .shape serializer and the glTF encoder.
SET(SHAPE_SHARED
	../pipeline_max_export_shape/scene_lib.cpp
	../pipeline_max_export_shape/mesh_eval.cpp
	../pipeline_max_export_shape/material_build.cpp
	../pipeline_max_export_shape/mesh_build.cpp
	../pipeline_max_export_shape/interface_build.cpp
)

# The ig exporter's whole per-file flow (selection, XRef resolution, buildInstanceGroup) backs
# the nel_igs blob — compiled in with its standalone main() guarded out.
SET(IG_SHARED ../pipeline_max_export_ig/main.cpp)
SET_SOURCE_FILES_PROPERTIES(${IG_SHARED} PROPERTIES COMPILE_DEFINITIONS PMB_IG_NO_MAIN)

FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_gltf
	${SRCS}
	${HDRS}
	${SHAPE_SHARED}
	${IG_SHARED}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_gltf
	pipeline_max
	pipeline_max_export_common
	nel_gltf
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(pipeline_max_export_gltf "NeL, Tools, 3D: Pipeline Max Export glTF")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_gltf)

INSTALL(TARGETS pipeline_max_export_gltf RUNTIME DESTINATION bin COMPONENT toolsmisc)
