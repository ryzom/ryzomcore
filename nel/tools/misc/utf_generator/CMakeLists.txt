FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(nl_utf_generator ${SRC})

TARGET_LINK_LIBRARIES(nl_utf_generator nelmisc)
NL_DEFAULT_PROPS(nl_utf_generator "NeL, Tools, Misc: UTF Generator")
NL_ADD_RUNTIME_FLAGS(nl_utf_generator)

INSTALL(TARGETS nl_utf_generator RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
