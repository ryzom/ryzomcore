FILE(GLOB SRC *.cpp *.h)
		
ADD_EXECUTABLE(memlog ${SRC})

TARGET_LINK_LIBRARIES(memlog ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(memlog "Tools, Memory: memlog")
NL_ADD_RUNTIME_FLAGS(memlog)

INSTALL(TARGETS memlog RUNTIME DESTINATION bin COMPONENT toolsmisc)
