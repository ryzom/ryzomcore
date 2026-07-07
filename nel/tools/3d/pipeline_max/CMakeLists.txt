
FILE(GLOB SRCS *.cpp builtin/*.cpp builtin/storage/*.cpp update1/*.cpp epoly/*.cpp biped/*.cpp nelpatch/*.cpp)
FILE(GLOB HDRS *.h builtin/*.h builtin/storage/*.h update1/*.h epoly/*.h biped/*.h nelpatch/*.h)

NL_TARGET_LIB(pipeline_max
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(pipeline_max
	NeL::misc
)

# OLE/CFB container backend. Default: the self-contained native reader/writer (storage_ole.cpp),
# which needs no third-party library and is cross-platform + deterministic. WITH_PIPELINE_NATIVE_OLE
# OFF switches to the libgsf-backed backend (kept for cross-validation); that path requires libgsf +
# glib/gobject.
IF(WITH_PIPELINE_NATIVE_OLE)
	TARGET_COMPILE_DEFINITIONS(pipeline_max PRIVATE NL_PIPELINE_NATIVE_OLE)
ELSE()
	FIND_PACKAGE(LIBGSF)
	IF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))
		MESSAGE(FATAL_ERROR "LIBGSF not found (required when WITH_PIPELINE_NATIVE_OLE is OFF)!")
	ENDIF (NOT (LIBGSF_INCLUDE_DIR AND LIBGSF_LIBRARIES))
	FIND_PACKAGE(GOBJECT2 REQUIRED)
	FIND_PACKAGE(GLIB2 REQUIRED)
	TARGET_INCLUDE_DIRECTORIES(pipeline_max
		PRIVATE
		${GLIB2_INCLUDE_DIRS}
		${LIBGSF_INCLUDE_DIR}
	)
	TARGET_LINK_LIBRARIES(pipeline_max
		${LIBGSF_LIBRARIES}
		${GLIB2_LIBRARIES}
		${GOBJECT2_LIBRARIES}
	)
ENDIF()

SET_TARGET_PROPERTIES(pipeline_max PROPERTIES LINK_INTERFACE_LIBRARIES "")

NL_DEFAULT_PROPS(pipeline_max "NeL, Tools, 3D: Pipeline Max (Library)")
NL_ADD_LIB_SUFFIX(pipeline_max)
NL_ADD_RUNTIME_FLAGS(pipeline_max)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS pipeline_max LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
ENDIF()
