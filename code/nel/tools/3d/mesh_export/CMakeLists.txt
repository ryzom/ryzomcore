FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

SOURCE_GROUP("" FILES ${SRCS} ${HDRS})

ADD_EXECUTABLE(mesh_export ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(mesh_export mesh_utils nel3d nelmisc)
NL_DEFAULT_PROPS(mesh_export "NeL, Tools, 3D: Mesh Export")
NL_ADD_RUNTIME_FLAGS(mesh_export)

INSTALL(TARGETS mesh_export RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
