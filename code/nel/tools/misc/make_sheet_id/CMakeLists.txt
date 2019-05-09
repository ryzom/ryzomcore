FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(make_sheet_id ${SRC})

ADD_DEFINITIONS(-DNL_MK_SH_ID_CFG="\\"${NL_ETC_PREFIX}/\\"")

TARGET_LINK_LIBRARIES(make_sheet_id nelmisc)
NL_DEFAULT_PROPS(make_sheet_id "NeL, Tools, Misc: make_sheet_id")
NL_ADD_RUNTIME_FLAGS(make_sheet_id)

INSTALL(TARGETS make_sheet_id RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
INSTALL(FILES make_sheet_id.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT toolsmisc)
