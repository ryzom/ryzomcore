FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(unbuild_interface ${SRC})

TARGET_LINK_LIBRARIES(unbuild_interface nelmisc)
NL_DEFAULT_PROPS(unbuild_interface "NeL, Tools, 3D: unbuild_interface")
NL_ADD_RUNTIME_FLAGS(unbuild_interface)

INSTALL(TARGETS unbuild_interface RUNTIME DESTINATION bin COMPONENT tools3d)
