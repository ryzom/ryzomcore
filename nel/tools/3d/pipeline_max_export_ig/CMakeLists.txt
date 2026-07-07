FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_ig
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_ig
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(pipeline_max_export_ig "NeL, Tools, 3D: Pipeline Max Export Ig")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_ig)

INSTALL(TARGETS pipeline_max_export_ig RUNTIME DESTINATION bin COMPONENT toolsmisc)
