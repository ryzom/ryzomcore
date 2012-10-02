FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_soundbank ${SRC})

ADD_DEFINITIONS( ${LIBXML2_DEFINITIONS})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(build_soundbank nelmisc nelsound)
NL_DEFAULT_PROPS(build_soundbank "NeL, Tools, Sound: build_soundbank")
NL_ADD_RUNTIME_FLAGS(build_soundbank)

INSTALL(TARGETS build_soundbank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolssound)
