FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_soundbank ${SRC})

TARGET_LINK_LIBRARIES(build_soundbank nelmisc nelsound)
NL_DEFAULT_PROPS(build_soundbank "NeL, Tools, Sound: build_soundbank")
NL_ADD_RUNTIME_FLAGS(build_soundbank)

INSTALL(TARGETS build_soundbank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolssound)
