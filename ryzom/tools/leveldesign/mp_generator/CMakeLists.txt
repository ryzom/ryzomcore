FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(mp_generator ${SRC})

TARGET_LINK_LIBRARIES(mp_generator
        PRIVATE
        NeL::misc
)

NL_DEFAULT_PROPS(mp_generator "Ryzom, Tools: MP Generator")
NL_ADD_RUNTIME_FLAGS(mp_generator)

INSTALL(TARGETS mp_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
