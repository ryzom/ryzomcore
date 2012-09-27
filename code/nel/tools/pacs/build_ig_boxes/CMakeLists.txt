FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_ig_boxes ${SRC})

ADD_DEFINITIONS(-DNL_BIB_CFG="\\"${NL_ETC_PREFIX}/\\"")

TARGET_LINK_LIBRARIES(build_ig_boxes nelmisc nelpacs nel3d)
NL_DEFAULT_PROPS(build_ig_boxes "NeL, Tools, PACS: build_ig_boxes")
NL_ADD_RUNTIME_FLAGS(build_ig_boxes)

INSTALL(TARGETS build_ig_boxes RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolspacs)
INSTALL(FILES build_ig_boxes.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT toolspacs)
