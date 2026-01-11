FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(unbuild_elevation ${SRC})

TARGET_LINK_LIBRARIES(unbuild_elevation nel3d nelmisc nelligo)
NL_DEFAULT_PROPS(unbuild_elevation "NeL, Tools, 3D: Unbuild Elevation")
NL_ADD_RUNTIME_FLAGS(unbuild_elevation)

INSTALL(TARGETS unbuild_elevation RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
