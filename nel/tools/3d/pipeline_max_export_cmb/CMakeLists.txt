FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_cmb
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_cmb
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
	NeL::pacs
)

NL_DEFAULT_PROPS(pipeline_max_export_cmb "NeL, Tools, 3D: Pipeline Max Export Cmb")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_cmb)

INSTALL(TARGETS pipeline_max_export_cmb RUNTIME DESTINATION bin COMPONENT toolsmisc)
