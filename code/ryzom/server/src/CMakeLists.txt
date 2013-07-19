
# Supporting modules and libraries.
# Need servershare for build packed collision tool
# Need aishare for build wmap tool
ADD_SUBDIRECTORY(server_share)
ADD_SUBDIRECTORY(ai_share)

IF(WITH_RYZOM_SERVER)

# Supporting modules and libraries.
ADD_SUBDIRECTORY(admin_modules)
ADD_SUBDIRECTORY(gameplay_module_lib)
ADD_SUBDIRECTORY(pd_lib)

# Ryzom Services
ADD_SUBDIRECTORY(ai_service)
ADD_SUBDIRECTORY(backup_service)
ADD_SUBDIRECTORY(dynamic_scenario_service)
ADD_SUBDIRECTORY(entities_game_service)
ADD_SUBDIRECTORY(frontend_service)
ADD_SUBDIRECTORY(gpm_service)
ADD_SUBDIRECTORY(input_output_service)
ADD_SUBDIRECTORY(ryzom_admin_service)
ADD_SUBDIRECTORY(ryzom_naming_service)
ADD_SUBDIRECTORY(ryzom_welcome_service)
ADD_SUBDIRECTORY(log_analyser_service)
ADD_SUBDIRECTORY(logger_service)
ADD_SUBDIRECTORY(mail_forum_service)
ADD_SUBDIRECTORY(mirror_service)
ADD_SUBDIRECTORY(monitor_service)
ADD_SUBDIRECTORY(pd_reference_builder)
ADD_SUBDIRECTORY(pd_support_service)
ADD_SUBDIRECTORY(persistant_data_service)
ADD_SUBDIRECTORY(session_browser_server)
ADD_SUBDIRECTORY(shard_unifier_service)
ADD_SUBDIRECTORY(tick_service)
ADD_SUBDIRECTORY(patchman_service)


# Deprecated, no longer used.
#ADD_SUBDIRECTORY(ags_test)
#ADD_SUBDIRECTORY(ai_data_service)
#ADD_SUBDIRECTORY(entity_view_service)
ADD_SUBDIRECTORY(general_utilities_service)

# Not sure, no longer used maybe?
#sabrina
#simulation_service
#testing_tool_service

ENDIF(WITH_RYZOM_SERVER)
