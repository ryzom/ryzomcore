FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

NL_TARGET_LIB(pipeline_max_export_common
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max_export_common
	pipeline_max
	NeL::misc
	NeL::3d
)

SET_TARGET_PROPERTIES(pipeline_max_export_common PROPERTIES LINK_INTERFACE_LIBRARIES "")

NL_DEFAULT_PROPS(pipeline_max_export_common "NeL, Tools, 3D: Pipeline Max Export Common (Library)")
NL_ADD_LIB_SUFFIX(pipeline_max_export_common)
NL_ADD_RUNTIME_FLAGS(pipeline_max_export_common)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS pipeline_max_export_common LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
ENDIF()
