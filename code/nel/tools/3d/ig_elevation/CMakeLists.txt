FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(ig_elevation ${SRC})

TARGET_LINK_LIBRARIES(ig_elevation nelmisc nel3d nelligo)
NL_DEFAULT_PROPS(ig_elevation "NeL, Tools, 3D: IG Elevation")
NL_ADD_RUNTIME_FLAGS(ig_elevation)

INSTALL(TARGETS ig_elevation RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
