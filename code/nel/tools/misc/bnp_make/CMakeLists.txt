FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(bnp_make ${SRC})

TARGET_LINK_LIBRARIES(bnp_make nelmisc)
NL_DEFAULT_PROPS(bnp_make "NeL, Tools, Misc: bnp_make")
NL_ADD_RUNTIME_FLAGS(bnp_make)

INSTALL(TARGETS bnp_make RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
