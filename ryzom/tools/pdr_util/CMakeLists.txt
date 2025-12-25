ADD_EXECUTABLE(pdr_util pdr_util.cpp)

TARGET_LINK_LIBRARIES(pdr_util
        PRIVATE
        ryzom_gameshare
        NeL::misc
        NeL::net
        NeL::ligo
        NeL::georges
)

NL_DEFAULT_PROPS(pdr_util "Ryzom, Tools: PDR Utility")
NL_ADD_RUNTIME_FLAGS(pdr_util)

INSTALL(TARGETS pdr_util RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
