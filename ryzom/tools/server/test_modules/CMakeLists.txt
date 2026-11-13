add_subdirectory(game_share_test)
add_subdirectory(shard_unifier_test)

# the option and what it needs are in ryzom/CMakeLists.txt
if (WITH_RYZOM_R2_SCENARIO_CHAIN)
  add_subdirectory(r2_scenario_chain)
endif ()
