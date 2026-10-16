FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_dump
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_dump
	pipeline_max
	NeL::misc
)

NL_DEFAULT_PROPS(pipeline_max_dump "NeL, Tools, 3D: Pipeline Max Dump (Temporary Tool)")
NL_ADD_RUNTIME_FLAGS(pipeline_max_dump)

INSTALL(TARGETS pipeline_max_dump RUNTIME DESTINATION bin COMPONENT toolsmisc)

