
FIND_PACKAGE(LIBGSF)
FIND_PACKAGE(Boost REQUIRED)

IF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))
	MESSAGE(FATAL_ERROR "LIBGSF not found!")
ENDIF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))

FIND_PACKAGE(GOBJECT2 REQUIRED)
FIND_PACKAGE(GLIB2 REQUIRED)

ADD_EXECUTABLE(pipeline_max_rewrite_assets)

target_sources(pipeline_max_rewrite_assets
		PRIVATE
		main.cpp
)

TARGET_INCLUDE_DIRECTORIES(pipeline_max_rewrite_assets
		PRIVATE
		${GLIB2_INCLUDE_DIRS}
		${LIBGSF_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(pipeline_max_rewrite_assets
	PRIVATE
	${LIBGSF_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GOBJECT2_LIBRARIES}
	pipeline_max
	nelmisc
	Boost::boost
)

NL_DEFAULT_PROPS(pipeline_max_dump "NeL, Tools, 3D: Pipeline Max Rewrite Assets")
NL_ADD_RUNTIME_FLAGS(pipeline_max_rewrite_assets)

INSTALL(TARGETS pipeline_max_rewrite_assets RUNTIME DESTINATION bin COMPONENT toolsmisc)

