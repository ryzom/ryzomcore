FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(alias_synchronizer ${SRC})

TARGET_LINK_LIBRARIES(alias_synchronizer nelmisc nelligo)

NL_DEFAULT_PROPS(alias_synchronizer "Ryzom, Tools, Misc: Alias Synchronizer")
NL_ADD_RUNTIME_FLAGS(alias_synchronizer)

INSTALL(TARGETS alias_synchronizer RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
