FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(translation_tools ${SRC})

TARGET_LINK_LIBRARIES(translation_tools nelmisc nelligo nelgeorges)

NL_DEFAULT_PROPS(translation_tools "Ryzom, Tools, Misc: Translation Tools")
NL_ADD_RUNTIME_FLAGS(translation_tools)

INSTALL(TARGETS translation_tools RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
