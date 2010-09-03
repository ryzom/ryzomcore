FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_indoor_rbank ${SRC})

ADD_DEFINITIONS(-DNL_BIRB_CFG="\\"${NL_ETC_PREFIX}/\\"")

TARGET_LINK_LIBRARIES(build_indoor_rbank ${PLATFORM_LINKFLAGS} nelpacs)
NL_DEFAULT_PROPS(build_indoor_rbank "NeL, Tools, PACS: build_indoor_rbank")
NL_ADD_RUNTIME_FLAGS(build_indoor_rbank)

INSTALL(TARGETS build_indoor_rbank RUNTIME DESTINATION bin COMPONENT toolspacs)
INSTALL(FILES build_indoor_rbank.cfg DESTINATION etc/nel COMPONENT toolspacs)
