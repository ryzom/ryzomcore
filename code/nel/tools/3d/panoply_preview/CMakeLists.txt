
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)
FILE(GLOB RESOURCES *.qrc)

SOURCE_GROUP("" FILES ${SRCS} ${HDRS} ${RESOURCES})

SET(CMAKE_AUTOMOC ON)

QT5_ADD_RESOURCES(RESOURCE_ADDED ${RESOURCES})

ADD_EXECUTABLE(nl_panoply_preview WIN32 ${SRC}
	${SRCS}
	${HDRS}
	${RESOURCE_ADDED}
	)

TARGET_LINK_LIBRARIES(nl_panoply_preview
	nelmisc
	nel3d
	Qt5::Widgets)

NL_DEFAULT_PROPS(nl_panoply_preview "NeL, Tools, 3D: panoply_preview")
NL_ADD_RUNTIME_FLAGS(nl_panoply_preview)

QT5_USE_MODULES(nl_panoply_preview Widgets)

INSTALL(TARGETS nl_panoply_preview RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
