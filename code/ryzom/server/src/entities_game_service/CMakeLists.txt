FILE(GLOB SRC	*.cpp *.h 
		backward_compatibility/*.h backward_compatibility/*.cpp
		building_manager/*.h building_manager/*.cpp
		character_structure/*.h character_structure/*.cpp
		creature_manager/*.h creature_manager/*.cpp
		egs_sheets/*.h egs_sheets/*.cpp
		entity_manager/*.h entity_manager/*.cpp
		entity_structure/*.h entity_structure/*.cpp
		game_item_manager/*.h game_item_manager/*.cpp
		guild_manager/*.h guild_manager/*.cpp
		mission_manager/*.h mission_manager/*.cpp
		modules/*.h modules/*.cpp
		outpost_manager/*.h outpost_manager/*.cpp
		phrase_manager/*.h phrase_manager/*.cpp
		player_manager/*.h player_manager/*.cpp
		progression/*.h progression/*.cpp
		pvp_manager/*.h pvp_manager/*.cpp
		pvp_manager/pvp_faction_reward_manager/*.h pvp_manager/pvp_faction_reward_manager/*.cpp
		shop_type/*.h shop_type/*.cpp
		stables/*.h stables/*.cpp
		team_manager/*.h team_manager/*.cpp)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/ai_pet_interface.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ai_pet_interface.h
			${CMAKE_CURRENT_SOURCE_DIR}/egs_static_continent.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/egs_static_continent.h
			${CMAKE_CURRENT_SOURCE_DIR}/shutdown_handler.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/shutdown_handler.h
			${CMAKE_CURRENT_SOURCE_DIR}/building_manager/room.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/building_manager/room.h
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_bleed_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_bleed_effect.h
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_stun_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_stun_effect.h
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_cure.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_debuff.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_hot.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/slow_move_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/slow_move_effect.h
			${CMAKE_CURRENT_SOURCE_DIR}/shop_type/solded_items.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/shop_type/solded_items.h)

ADD_EXECUTABLE(entities_game_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(entities_game_service	admin_modules 
						ryzom_gameshare 
						server_share 
						ai_share 
						pd_lib 
						gameplay_module_lib
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES} 
						${ZLIB_LIBRARIES} 
						nelmisc
						nelnet
						nelgeorges
						nelligo)

NL_DEFAULT_PROPS(entities_game_service "Ryzom, Services: Entities Game Service (EGS)")
NL_ADD_RUNTIME_FLAGS(entities_game_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(entities_game_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS entities_game_service RUNTIME DESTINATION sbin COMPONENT services)

