FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_anim
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_anim
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(pipeline_max_export_anim "NeL, Tools, 3D: Pipeline Max Export Anim")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_anim)

INSTALL(TARGETS pipeline_max_export_anim RUNTIME DESTINATION bin COMPONENT toolsmisc)
