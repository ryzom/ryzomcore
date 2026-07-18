FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(zone_painter
	${SRCS}
	${HDRS}
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
