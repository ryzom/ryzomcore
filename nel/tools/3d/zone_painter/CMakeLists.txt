FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

# The include-meshes context display (P3d) shares the shape exporter's evaluation sources —
# the clod reuse pattern (compile-in, no exporter source modified). lm_scene_build carries the
# scene light decode the paint-light setup rides on.
ADD_EXECUTABLE(zone_painter
	${SRCS}
	${HDRS}
	../pipeline_max_export_shape/scene_lib.cpp
	../pipeline_max_export_shape/scene_lib.h
	../pipeline_max_export_shape/mesh_eval.cpp
	../pipeline_max_export_shape/mesh_eval.h
	../pipeline_max_export_shape/material_build.cpp
	../pipeline_max_export_shape/material_build.h
	../pipeline_max_export_shape/mesh_build.cpp
	../pipeline_max_export_shape/mesh_build.h
	../pipeline_max_export_shape/lm_scene_build.cpp
	../pipeline_max_export_shape/lm_scene_build.h
	)

TARGET_LINK_LIBRARIES(zone_painter
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(zone_painter "NeL, Tools, 3D: Zone Painter")
NL_ADD_RUNTIME_FLAGS(zone_painter)

INSTALL(TARGETS zone_painter RUNTIME DESTINATION bin COMPONENT toolsmisc)
