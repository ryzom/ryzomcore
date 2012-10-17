FILE(GLOB SRC							*.cpp *.h)
FILE(GLOB SRC_BACKWARD_COMPATIBILITY	backward_compatibility/*.h backward_compatibility/*.cpp)
FILE(GLOB SRC_BUILDING_MANAGER			building_manager/*.h building_manager/*.cpp)
FILE(GLOB SRC_CHARACTER_STRUCTURE		character_structure/*.h character_structure/*.cpp)
FILE(GLOB SRC_CREATURE_MANAGER			creature_manager/*.h creature_manager/*.cpp)
FILE(GLOB SRC_EGS_SHEETS				egs_sheets/*.h egs_sheets/*.cpp)
FILE(GLOB SRC_ENTITY_MANAGER			entity_manager/*.h entity_manager/*.cpp)
FILE(GLOB SRC_ENTITY_STRUCTURE			entity_structure/*.h entity_structure/*.cpp)
FILE(GLOB SRC_GAME_ITEM_MANAGER			game_item_manager/*.h game_item_manager/*.cpp)
FILE(GLOB SRC_GUILD_MANAGER				guild_manager/*.h guild_manager/*.cpp)
FILE(GLOB SRC_MISSION_MANAGER			mission_manager/*.h mission_manager/*.cpp)
FILE(GLOB SRC_MODULES					modules/*.h modules/*.cpp)
FILE(GLOB SRC_OUTPOST_MANAGER			outpost_manager/*.h outpost_manager/*.cpp)
FILE(GLOB SRC_PHRASE_MANAGER			phrase_manager/*.h phrase_manager/*.cpp)
FILE(GLOB SRC_PLAYER_MANAGER			player_manager/*.h player_manager/*.cpp)
FILE(GLOB SRC_PROGRESSION				progression/*.h progression/*.cpp)
FILE(GLOB SRC_PVP_MANAGER				pvp_manager/*.h pvp_manager/*.cpp)
FILE(GLOB SRC_PVP_FACTION_MANAGER		pvp_manager/pvp_faction_reward_manager/*.h pvp_manager/pvp_faction_reward_manager/*.cpp)
FILE(GLOB SRC_SHOP_TYPE					shop_type/*.h shop_type/*.cpp)
FILE(GLOB SRC_STABLES					stables/*.h stables/*.cpp)
FILE(GLOB SRC_TEAM_MANAGER				team_manager/*.h team_manager/*.cpp)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/ai_pet_interface.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ai_pet_interface.h
			${CMAKE_CURRENT_SOURCE_DIR}/egs_static_continent.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/egs_static_continent.h
			${CMAKE_CURRENT_SOURCE_DIR}/shutdown_handler.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/shutdown_handler.h)
LIST(REMOVE_ITEM SRC_BUILDING_MANAGER
			${CMAKE_CURRENT_SOURCE_DIR}/building_manager/room.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/building_manager/room.h)
LIST(REMOVE_ITEM SRC_PHRASE_MANAGER
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_bleed_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_bleed_effect.h
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_stun_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/combat_stun_effect.h
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_cure.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_debuff.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/magic_action_hot.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/slow_move_effect.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/phrase_manager/slow_move_effect.h)
LIST(REMOVE_ITEM SRC_SHOP_TYPE
			${CMAKE_CURRENT_SOURCE_DIR}/shop_type/solded_items.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/shop_type/solded_items.h)

SOURCE_GROUP("" FILES ${SRC})
SOURCE_GROUP("Backward Compatibility" FILES ${SRC_BACKWARD_COMPATIBILITY})
SOURCE_GROUP("Building Manager" FILES ${SRC_BUILDING_MANAGER})
SOURCE_GROUP("Character Structure" FILES ${SRC_CHARACTER_STRUCTURE})
SOURCE_GROUP("Creature Manager" FILES ${SRC_CREATURE_MANAGER})
SOURCE_GROUP("EGS Sheets" FILES ${SRC_EGS_SHEETS})
SOURCE_GROUP("Entity Manager" FILES ${SRC_ENTITY_MANAGER})
SOURCE_GROUP("Entity Structure" FILES ${SRC_ENTITY_STRUCTURE})
SOURCE_GROUP("Game Item Manager" FILES ${SRC_GAME_ITEM_MANAGER})
SOURCE_GROUP("Guild Manager" FILES ${SRC_GUILD_MANAGER})
SOURCE_GROUP("Mission Manager" FILES ${SRC_MISSION_MANAGER})
SOURCE_GROUP("Modules" FILES ${SRC_MODULES})
SOURCE_GROUP("Outpost Manager" FILES ${SRC_OUTPOST_MANAGER})
SOURCE_GROUP("Phrase Manager" FILES ${SRC_PHRASE_MANAGER})
SOURCE_GROUP("Player Manager" FILES ${SRC_PLAYER_MANAGER})
SOURCE_GROUP("Progression" FILES ${SRC_PROGRESSION})
SOURCE_GROUP("PvP Manager" FILES ${SRC_PVP_MANAGER})
SOURCE_GROUP("PvP Manager\\PvP Faction Reward Manager" FILES ${SRC_PVP_FACTION_MANAGER})
SOURCE_GROUP("Shop Type" FILES ${SRC_SHOP_TYPE})
SOURCE_GROUP("Stables" FILES ${SRC_STABLES})
SOURCE_GROUP("Team Manager" FILES ${SRC_TEAM_MANAGER})

ADD_EXECUTABLE(ryzom_entities_game_service WIN32
	${SRC}
	${SRC_BACKWARD_COMPATIBILITY}
	${SRC_BUILDING_MANAGER}
	${SRC_CHARACTER_STRUCTURE}
	${SRC_CREATURE_MANAGER}
	${SRC_EGS_SHEETS}
	${SRC_ENTITY_MANAGER}
	${SRC_ENTITY_STRUCTURE}
	${SRC_GAME_ITEM_MANAGER}
	${SRC_GUILD_MANAGER}
	${SRC_MISSION_MANAGER}
	${SRC_MODULES}
	${SRC_OUTPOST_MANAGER}
	${SRC_PHRASE_MANAGER}
	${SRC_PLAYER_MANAGER}
	${SRC_PROGRESSION}
	${SRC_PVP_MANAGER}
	${SRC_PVP_FACTION_MANAGER}
	${SRC_SHOP_TYPE}
	${SRC_STABLES}
	${SRC_TEAM_MANAGER})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_entities_game_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_servershare
						ryzom_aishare
						ryzom_pd
						ryzom_gameplaymodule
						${LIBXML2_LIBRARIES}
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARIES}
						nelmisc
						nelnet
						nelgeorges
						nelligo)

NL_DEFAULT_PROPS(ryzom_entities_game_service "Ryzom, Services: Entities Game Service (EGS)")
NL_ADD_RUNTIME_FLAGS(ryzom_entities_game_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_entities_game_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_entities_game_service RUNTIME DESTINATION sbin COMPONENT services)
