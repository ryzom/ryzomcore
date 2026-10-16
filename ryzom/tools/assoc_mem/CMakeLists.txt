FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(assoc_mem ${SRC})

TARGET_LINK_LIBRARIES(assoc_mem
        PRIVATE
        NeL::misc
)

NL_DEFAULT_PROPS(assoc_mem "Ryzom, Tools, Misc: assoc_mem")
NL_ADD_RUNTIME_FLAGS(assoc_mem)

INSTALL(TARGETS assoc_mem RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
