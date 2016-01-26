FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(make_alias_file ${SRC})

TARGET_LINK_LIBRARIES(make_alias_file nelmisc nelgeorges)

NL_DEFAULT_PROPS(make_alias_file "Ryzom, Tools, Misc: Make Alias File")
NL_ADD_RUNTIME_FLAGS(make_alias_file)

INSTALL(TARGETS make_alias_file RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
