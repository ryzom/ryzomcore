FILE(GLOB SRC *.cpp *.h ../ig_lighter_lib/*.cpp ../ig_lighter_lib/*.h)

ADD_EXECUTABLE(ig_lighter ${SRC})

TARGET_LINK_LIBRARIES(ig_lighter nelmisc nel3d nelpacs)
NL_DEFAULT_PROPS(ig_lighter "NeL, Tools, 3D: ig_lighter")
NL_ADD_RUNTIME_FLAGS(ig_lighter)

INSTALL(TARGETS ig_lighter RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
