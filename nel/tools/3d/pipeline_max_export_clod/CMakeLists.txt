# pipeline_max_export_clod shares scene/mesh evaluation with the shape exporter — reuse those
# source files directly rather than duplicating them. Only main.cpp is clod-specific here.
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_clod
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
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_clod
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(pipeline_max_export_clod "NeL, Tools, 3D: Pipeline Max Export Lod Character")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_clod)

INSTALL(TARGETS pipeline_max_export_clod RUNTIME DESTINATION bin COMPONENT toolsmisc)
