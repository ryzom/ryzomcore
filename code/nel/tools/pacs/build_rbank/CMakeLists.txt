FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_rbank ${SRC})

ADD_DEFINITIONS(-DNL_BRB_CFG="\\"${NL_ETC_PREFIX}/\\"")

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(build_rbank ${PLATFORM_LINKFLAGS} nelpacs nel3d nelligo nelmisc)
NL_DEFAULT_PROPS(build_rbank "Tools, PACS: build_rbank")
NL_ADD_RUNTIME_FLAGS(build_rbank)

INSTALL(TARGETS build_rbank RUNTIME DESTINATION bin COMPONENT toolspacs)
INSTALL(FILES build_rbank.cfg DESTINATION etc/nel COMPONENT toolspacs)
