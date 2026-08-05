FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(branch_patcher WIN32 ${SRC})
SET_TARGET_PROPERTIES(branch_patcher PROPERTIES NL_MFC_UNICODE_APP ON) # unicode MFC GUI: pre-VS2010 links need the explicit wWinMain entry (see NL_DEFAULT_PROPS)

#TARGET_LINK_LIBRARIES(branch_patcher ${PLATFORM_LINKFLAGS})
NL_DEFAULT_PROPS(branch_patcher "NeL, Tools, Misc: branch_patcher")
NL_ADD_RUNTIME_FLAGS(branch_patcher)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS branch_patcher RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
