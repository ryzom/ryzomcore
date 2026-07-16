FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_swt
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_swt
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(pipeline_max_export_swt "NeL, Tools, 3D: Pipeline Max Export Swt")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_swt)

INSTALL(TARGETS pipeline_max_export_swt RUNTIME DESTINATION bin COMPONENT toolsmisc)
