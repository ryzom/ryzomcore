FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(unbuild_interface ${SRC})

TARGET_LINK_LIBRARIES(unbuild_interface nelmisc)
NL_DEFAULT_PROPS(unbuild_interface "NeL, Tools, 3D: Unbuild Interface")
NL_ADD_RUNTIME_FLAGS(unbuild_interface)

INSTALL(TARGETS unbuild_interface RUNTIME DESTINATION bin COMPONENT tools3d)
