FIND_PACKAGE(ZLIB REQUIRED)

FILE(GLOB SRC *.cpp)
FILE(GLOB SRC_TIME time_weather_season/*.cpp)

FILE(GLOB PRIV_H *.h)
FILE(GLOB PRIV_TIME_H time_weather_season/*.h)

SET(HEADER ${PRIV_H})

FILE(GLOB R2 
	dms.h dms.cpp
	scenario.h scenario.cpp
	user_connection_mgr.h user_connection_mgr.cpp
	object.h object.cpp
	server_animation_module.h server_animation_module.cpp
	server_admin_module.h server_admin_module.cpp
	server_edition_module.h server_edition_module.cpp
	string_mgr_module.h string_mgr_module.cpp
	scenario_entry_points.h scenario_entry_points.cpp
	small_string_manager.h small_string_manager.cpp
	ai_wrapper.h ai_wrapper.cpp
	r2_*.h r2_*.cpp
	ring_*.h ring_*.cpp)

LIST(REMOVE_ITEM SRC R2)
LIST(REMOVE_ITEM HEADER R2)

SOURCE_GROUP("" FILES ${SRC} ${HEADER})
SOURCE_GROUP("time_weather_season" FILES ${SRC_TIME} ${PRIV_TIME_H})
SOURCE_GROUP("R2" FILES ${R2})

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.cpp)
LIST(REMOVE_ITEM HEADER ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.h)

NL_TARGET_LIB(ryzom_gameshare ${HEADER} ${PRIV_TIME_H} ${SRC} ${SRC_TIME} ${R2})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_gameshare ${NELMISC_LIBRARIES} ${NELNET_LIBRARIES} ${NELLIGO_LIBRARIES} ${NELGEORGES_LIBRARIES})
NL_DEFAULT_PROPS(ryzom_gameshare "Ryzom, Library: Game Share")
NL_ADD_RUNTIME_FLAGS(ryzom_gameshare)
NL_ADD_LIB_SUFFIX(ryzom_gameshare)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(ryzom_gameshare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_gameshare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()

IF(WITH_INSTALL_LIBRARIES)
  INSTALL(FILES ${PRIV_H} DESTINATION include/ryzom/game_share COMPONENT headers)
  INSTALL(FILES ${PRIV_TIME_H} DESTINATION include/ryzom/game_share/time_weather_season COMPONENT headers)
ENDIF()

