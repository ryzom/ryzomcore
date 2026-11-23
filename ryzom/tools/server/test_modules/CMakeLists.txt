add_subdirectory(game_share_test)
add_subdirectory(shard_unifier_test)

# built from sources of the AI service
if (WITH_RYZOM_SERVER)
  add_subdirectory(ai_service_test)
endif ()

# the option and what it needs are in ryzom/CMakeLists.txt
if (WITH_RYZOM_R2_SCENARIO_CHAIN)
  add_subdirectory(r2_scenario_chain)
endif ()
