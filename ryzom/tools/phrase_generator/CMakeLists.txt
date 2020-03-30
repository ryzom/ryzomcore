FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(phrase_generator ${SRC})

TARGET_LINK_LIBRARIES(phrase_generator
						nelmisc
						nelgeorges)

NL_DEFAULT_PROPS(phrase_generator "Ryzom, Tools: Phrase Generator")
NL_ADD_RUNTIME_FLAGS(phrase_generator)

INSTALL(TARGETS phrase_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
