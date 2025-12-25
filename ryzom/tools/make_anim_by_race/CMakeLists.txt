ADD_EXECUTABLE(make_anim_by_race main.cpp)

TARGET_LINK_LIBRARIES(make_anim_by_race
        PRIVATE
        NeL::misc
)

NL_DEFAULT_PROPS(make_anim_by_race "Ryzom, Tools, Misc: Make Anim By Race")
NL_ADD_RUNTIME_FLAGS(make_anim_by_race)

INSTALL(TARGETS make_anim_by_race RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
