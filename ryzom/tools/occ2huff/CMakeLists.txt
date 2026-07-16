ADD_EXECUTABLE(occ2huff occ2huff.cpp)

TARGET_LINK_LIBRARIES(occ2huff
        PRIVATE
        NeL::misc
)

NL_DEFAULT_PROPS(occ2huff "Ryzom, Tools: occ2huff")
NL_ADD_RUNTIME_FLAGS(occ2huff)

INSTALL(TARGETS occ2huff RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
