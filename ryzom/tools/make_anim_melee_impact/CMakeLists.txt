ADD_EXECUTABLE(make_anim_melee_impact main.cpp)

TARGET_LINK_LIBRARIES(make_anim_melee_impact
        PRIVATE
        NeL::misc
)
NL_DEFAULT_PROPS(make_anim_melee_impact "Ryzom, Tools, Misc: Make Anim Melee Impact")
NL_ADD_RUNTIME_FLAGS(make_anim_melee_impact)

INSTALL(TARGETS make_anim_melee_impact RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
