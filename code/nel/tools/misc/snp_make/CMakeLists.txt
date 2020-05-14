FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snp_make ${SRC})

TARGET_LINK_LIBRARIES(snp_make nelmisc)
NL_DEFAULT_PROPS(snp_make "NeL, Tools, Misc: snp_make")
NL_ADD_RUNTIME_FLAGS(snp_make)

INSTALL(TARGETS snp_make RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
