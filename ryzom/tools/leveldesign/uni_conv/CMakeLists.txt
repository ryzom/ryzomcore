FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(uni_conv ${SRC})

TARGET_LINK_LIBRARIES(uni_conv nelmisc)

NL_DEFAULT_PROPS(uni_conv "Ryzom, Tools, Misc: Unicode Conversion Tool")
NL_ADD_RUNTIME_FLAGS(uni_conv)

INSTALL(TARGETS uni_conv RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
