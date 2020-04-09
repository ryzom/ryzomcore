FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(zone_elevation ${SRC})

TARGET_LINK_LIBRARIES(zone_elevation nel3d nelmisc nelligo)
NL_DEFAULT_PROPS(zone_elevation "NeL, Tools, 3D: Zone Elevation")
NL_ADD_RUNTIME_FLAGS(zone_elevation)

INSTALL(TARGETS zone_elevation RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
