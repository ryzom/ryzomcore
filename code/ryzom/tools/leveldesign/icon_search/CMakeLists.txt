FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(icon_search ${SRC})

TARGET_LINK_LIBRARIES(icon_search ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(icon_search "Ryzom, Tools, Misc: Icon Search")
NL_ADD_RUNTIME_FLAGS(icon_search)

INSTALL(TARGETS icon_search RUNTIME DESTINATION bin COMPONENT tools)
