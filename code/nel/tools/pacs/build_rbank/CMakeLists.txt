FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_rbank ${SRC})

ADD_DEFINITIONS(-DNL_BRB_CFG="\\"${NL_ETC_PREFIX}/\\"")

TARGET_LINK_LIBRARIES(build_rbank nelpacs nel3d nelligo nelmisc)
NL_DEFAULT_PROPS(build_rbank "NeL, Tools, PACS: build_rbank")
NL_ADD_RUNTIME_FLAGS(build_rbank)

INSTALL(TARGETS build_rbank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolspacs)
INSTALL(FILES build_rbank.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT toolspacs)
