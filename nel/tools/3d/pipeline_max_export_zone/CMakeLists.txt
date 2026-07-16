FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_zone
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_zone
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
	NeL::ligo
)

NL_DEFAULT_PROPS(pipeline_max_export_zone "NeL, Tools, 3D: Pipeline Max Export Zone")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_zone)

INSTALL(TARGETS pipeline_max_export_zone RUNTIME DESTINATION bin COMPONENT toolsmisc)
