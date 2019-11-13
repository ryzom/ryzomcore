FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(get_neighbors ${SRC})

TARGET_LINK_LIBRARIES(get_neighbors nelmisc)
NL_DEFAULT_PROPS(get_neighbors "NeL, Tools, 3D: get_neighbors")
NL_ADD_RUNTIME_FLAGS(get_neighbors)

INSTALL(TARGETS get_neighbors RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
