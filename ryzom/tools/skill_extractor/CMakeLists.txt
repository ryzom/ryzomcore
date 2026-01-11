FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(skill_extractor ${SRC})

TARGET_LINK_LIBRARIES(skill_extractor nelmisc)

NL_DEFAULT_PROPS(skill_extractor "Ryzom, Tools, Misc: Skill Extractor")
NL_ADD_RUNTIME_FLAGS(skill_extractor)

INSTALL(TARGETS skill_extractor RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
