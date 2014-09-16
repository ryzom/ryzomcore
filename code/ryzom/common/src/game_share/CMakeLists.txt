FILE(GLOB SRC *.cpp time_weather_season/*.cpp)
FILE(GLOB PRIV_H *.h time_weather_season/*.h)

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
LIST(REMOVE_ITEM PRIV_H R2)

SOURCE_GROUP("" FILES ${SRC} ${PRIV_H})
SOURCE_GROUP("R2" FILES ${R2})

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.cpp)
LIST(REMOVE_ITEM PRIV_H ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.h)

NL_TARGET_LIB(ryzom_gameshare ${PRIV_H} ${SRC} ${R2})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_gameshare nelmisc nelnet nelligo nelgeorges crypt ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
NL_DEFAULT_PROPS(ryzom_gameshare "Ryzom, Library: Game Share")
NL_ADD_RUNTIME_FLAGS(ryzom_gameshare)
NL_ADD_LIB_SUFFIX(ryzom_gameshare)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_gameshare ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_gameshare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
