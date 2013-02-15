FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(phrase_generator ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(phrase_generator
						${LIBXML2_LIBRARIES}
						nelmisc
						nelgeorges)

NL_DEFAULT_PROPS(phrase_generator "Ryzom, Tools: Phrase Generator")
NL_ADD_RUNTIME_FLAGS(phrase_generator)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS phrase_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
