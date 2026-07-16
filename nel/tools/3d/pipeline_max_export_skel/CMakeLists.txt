FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_skel
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_skel
	pipeline_max
	pipeline_max_export_common
	NeL::misc
)

NL_DEFAULT_PROPS(pipeline_max_export_skel "NeL, Tools, 3D: Pipeline Max Export Skel")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_skel)

INSTALL(TARGETS pipeline_max_export_skel RUNTIME DESTINATION bin COMPONENT toolsmisc)
