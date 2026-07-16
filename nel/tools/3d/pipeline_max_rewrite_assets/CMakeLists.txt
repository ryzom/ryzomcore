FIND_PACKAGE(Boost REQUIRED)

ADD_EXECUTABLE(pipeline_max_rewrite_assets)

target_sources(pipeline_max_rewrite_assets
		PRIVATE
		main.cpp
)

TARGET_LINK_LIBRARIES(pipeline_max_rewrite_assets
	PRIVATE
	pipeline_max
	NeL::misc
	Boost::boost
)

NL_DEFAULT_PROPS(pipeline_max_dump "NeL, Tools, 3D: Pipeline Max Rewrite Assets")
NL_ADD_RUNTIME_FLAGS(pipeline_max_rewrite_assets)

INSTALL(TARGETS pipeline_max_rewrite_assets RUNTIME DESTINATION bin COMPONENT toolsmisc)

