ADD_EXECUTABLE(alias_synchronizer alias_synchronizer.cpp)

TARGET_LINK_LIBRARIES(alias_synchronizer PRIVATE
        NeL::misc
        NeL::ligo
)

NL_DEFAULT_PROPS(alias_synchronizer "Ryzom, Tools, Misc: Alias Synchronizer")
NL_ADD_RUNTIME_FLAGS(alias_synchronizer)

INSTALL(TARGETS alias_synchronizer RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
