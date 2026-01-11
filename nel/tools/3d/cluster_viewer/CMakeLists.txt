FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(cluster_viewer WIN32 ${SRC})

TARGET_LINK_LIBRARIES(cluster_viewer nel3d nelmisc)
NL_DEFAULT_PROPS(cluster_viewer "NeL, Tools, 3D: Cluster Viewer")
NL_ADD_RUNTIME_FLAGS(cluster_viewer)

INSTALL(TARGETS cluster_viewer RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
