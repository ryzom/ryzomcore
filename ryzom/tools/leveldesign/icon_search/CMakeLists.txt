ADD_EXECUTABLE(icon_search icon_search.cpp)

TARGET_LINK_LIBRARIES(icon_search
        PRIVATE
        NeL::misc
)
NL_DEFAULT_PROPS(icon_search "Ryzom, Tools, Misc: Icon Search")
NL_ADD_RUNTIME_FLAGS(icon_search)

INSTALL(TARGETS icon_search RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
