FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(alias_synchronizer ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(alias_synchronizer ${LIBXML2_LIBRARIES} nelmisc nelligo)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(alias_synchronizer "Ryzom, Tools, Misc: Alias Synchronizer")
NL_ADD_RUNTIME_FLAGS(alias_synchronizer)

INSTALL(TARGETS alias_synchronizer RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
