FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(pipeline_max_export_pacs_prim
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_pacs_prim
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::pacs
)

NL_DEFAULT_PROPS(pipeline_max_export_pacs_prim "NeL, Tools, 3D: Pipeline Max Export Pacs Prim")
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_pacs_prim)

INSTALL(TARGETS pipeline_max_export_pacs_prim RUNTIME DESTINATION bin COMPONENT toolsmisc)
