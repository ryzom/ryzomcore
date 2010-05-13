FILE(GLOB SRC	*.cpp *.h 
		backward_compatibility/*.h backward_compatibility/*.cpp
		building_manager
		character_structure
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

ADD_EXECUTABLE(entities_game_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(entities_game_service admin_modules game_share server_share ${PLATFORM_LINKFLAGS} ${LIBXML2_LIBRARIES} ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} ${NELMISC_LIBRARY} ${NELNET_LIBRARY} ${NELGEORGES_LIBRARY} ${NELLIGO_LIBRARY})

IF(WIN32)
  SET_TARGET_PROPERTIES(entities_game_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Entities Game Service (EGS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS entities_game_service RUNTIME DESTINATION sbin COMPONENT services)

