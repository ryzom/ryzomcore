FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/ec_event_chat.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_popups.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_popups.h
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_select_with_text.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_select_with_text.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_repository_emitter.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/rr_repository_reciever.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/mfc/*)



SOURCE_GROUP("Documentation" FILES	ss_notes.txt 
									ce_notes.txt 
									ec_notes.txt 
									ee_notes.txt 
									em_notes.txt 
									merge_notes.txt 
									gus_module_notes.txt 
									gus_net_notes.txt 
									saves_notes.txt 
									rs_notes.txt 
									rsaves_notes.txt 
									stats_notes.txt
									server control module notes.txt)

SOURCE_GROUP("GUS Modules\\Contest Control (CC)" FILES cc_commands.cpp  cc_contest_ctrl_script.cpp cc_contest_ctrl_script.h cc_module_messages.h)

SOURCE_GROUP("GUS Modules\\Contest Executor (CE)" FILES ce_commands.cpp ce_contest_executor.cpp ce_module_messages.h ce_contest_executor.h)

SOURCE_GROUP("GUS Modules\\Contest Logger (CL)" FILES cl_contest_logger.cpp cl_module_messages.h)

SOURCE_GROUP("GUS Modules\\Event Chat (EC)" FILES	ec_channel.cpp 
													ec_channel.h 
													ec_ctrl_channel.cpp 
													ec_ctrl_channel.h 
													ec_event_chat.cpp 
													ec_event_chat_module.cpp 
													ec_event_chat_module.h 
													ec_faction_channel.cpp 
													ec_faction_channel.h
													ec_party_channel.cpp
													ec_party_channel.h
													ec_types.h)

SOURCE_GROUP("GUS Modules\\Event Executor (EE)" FILES	ee_event_executor.cpp ee_event_executor.h ee_module_messages.h)

SOURCE_GROUP("GUS Modules\\Event Executor (EE)" FILES	em_commands.cpp em_event_manager.cpp em_event_manager.h em_module_messages.h)
                             
SOURCE_GROUP("GUS Modules\\Shard Merge (MERGE)" FILES	merge_commands.cpp merge_shard_merger.cpp)

SOURCE_GROUP("GUS Modules\\Remote Saves (RSAVES)" FILES	remote_saves_interface.cpp remote_saves_interface.h rs_remote_saves.cpp rs_remote_saves.h rs_module_messages.h)

SOURCE_GROUP("GUS Modules\\Shard Saves (SAVES)" FILES	saves_module_messages.h saves_shard_saves.cpp saves_unit.cpp saves_unit.h)

SOURCE_GROUP("GUS Modules\\Shard Script (SS)" FILES	ss_command_executor.cpp 
													ss_command_executor.h 
													ss_script_manager.cpp 
													ss_commands.cpp 
													ss_script_manager.h 
													ss_service_comms_manager.cpp 
													ss_service_comms_manager.h 
													ss_state_manager.cpp 
													ss_state_manager.h)
                   
				   
SOURCE_GROUP("GUS Modules\\Stats Scan (STATS)" FILES	stats_guild_commands.cpp 
														stats_guild_scan_job.cpp 
														stats_guild_scan_job.h 
														stat_character.cpp
														stat_character.h
														stat_character_scan_job.cpp 
														stat_character_scan_job.h
														stat_char_commands.cpp
														stat_char_filter_factory.cpp
														stat_char_filter_factory.h 
														stat_char_info_extractor_factory.cpp
														stat_char_info_extractor_factory.h 
														stat_char_scan_script.cpp
														stat_char_scan_script.h
														stat_file_list_builder_factory.cpp 
														stat_file_list_builder_factory.h 
														stat_globals.cpp stat_globals.h 
														stat_guild_container.cp 
														stat_guild_container.h 
														stat_job_manager.cpp 
														stat_job_manager.h 
														stat_user_char_filters.cpp 
														stat_user_char_info_extractors.cpp 
														stat_user_file_list_builders.cpp)

SOURCE_GROUP("GUS Modules\\GUS Networking" FILES	gus_net.cpp 
													gus_net.h 
													gus_net_commands.cpp 
													gus_net_connection.cpp 
													gus_net_connection.h 
													gus_net_hub.cpp 
													gus_net_hub.h 
													gus_net_implementation.cpp 
													gus_net_implementation.h 
													gus_net_messages.cpp 
													gus_net_messages.h 
													gus_net_remote_module.cpp 
													gus_net_remote_module.h 
													gus_net_types.h)

SOURCE_GROUP("GUS Modules" FILES gus_module.cpp gus_module.h gus_module_commands.cpp gus_module_factory.cpp gus_module_factory.h gus_module_manager.cpp gus_module_manager.h)
														
SOURCE_GROUP("GUS Core" FILES	gus_chat.cpp 
								gus_chat.h 
								gus_client_manager.cpp 
								gus_client_manager.h 
								gus_mirror.cpp 
								gus_mirror.h 
								gus_text.cpp 
								gus_text.h 
								gus_utils.cpp 
								gus_utils.h 
								gus_util_commands.cpp)

ADD_EXECUTABLE(ryzom_general_utilities_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_general_utilities_service
						ryzom_adminmodules
						ryzom_gameshare 
						ryzom_servershare
						ryzom_aishare
						${LIBXML2_LIBRARIES} 
						${ZLIB_LIBRARIES} 
						nelmisc
						nelnet 
						nelgeorges
						nelligo)

NL_DEFAULT_PROPS(ryzom_general_utilities_service "Ryzom, Services: General Utilities Service (GUS)")
NL_ADD_RUNTIME_FLAGS(ryzom_general_utilities_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_general_utilities_service RUNTIME DESTINATION sbin COMPONENT services)
