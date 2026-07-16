FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(unbuild_land ${SRC})

TARGET_LINK_LIBRARIES(unbuild_land PRIVATE NeL::3d NeL::misc NeL::ligo)
NL_DEFAULT_PROPS(unbuild_land "NeL, Tools, Ligo: Unbuild Land")
NL_ADD_RUNTIME_FLAGS(unbuild_land)

INSTALL(TARGETS unbuild_land RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsligo)
