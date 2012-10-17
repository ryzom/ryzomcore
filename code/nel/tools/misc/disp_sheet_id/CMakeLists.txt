FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(disp_sheet_id ${SRC})

TARGET_LINK_LIBRARIES(disp_sheet_id nelmisc)
NL_DEFAULT_PROPS(disp_sheet_id "NeL, Tools, Misc: disp_sheet_id")
NL_ADD_RUNTIME_FLAGS(disp_sheet_id)

INSTALL(TARGETS disp_sheet_id RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
