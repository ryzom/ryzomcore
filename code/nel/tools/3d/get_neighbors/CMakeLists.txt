FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(get_neighbors ${SRC})

TARGET_LINK_LIBRARIES(get_neighbors ${PLATFORM_LINKFLAGS})
NL_DEFAULT_PROPS(get_neighbors "Tools, 3D: get_neighbors")
NL_ADD_RUNTIME_FLAGS(get_neighbors)

INSTALL(TARGETS get_neighbors RUNTIME DESTINATION bin COMPONENT tools3d)
