FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(translation_tools ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(translation_tools ${LIBXML2_LIBRARIES} nelmisc nelligo nelgeorges)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(translation_tools "Ryzom, Tools, Misc: Translation Tools")
NL_ADD_RUNTIME_FLAGS(translation_tools)

INSTALL(TARGETS translation_tools RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
