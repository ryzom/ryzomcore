FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/service.cpp ${CMAKE_CURRENT_SOURCE_DIR}/service.h)

ADD_EXECUTABLE(dynamic_scenario_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR}  ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(dynamic_scenario_service	admin_modules 
						game_share 
						server_share
						ai_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${ZLIB_LIBRARIES} 
						${NELMISC_LIBRARY} 
						${NELNET_LIBRARY} 
						${NELGEORGES_LIBRARY} 
						${NELLIGO_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(dynamic_scenario_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Dynamic Scenario Service (DSS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS dynamic_scenario_service RUNTIME DESTINATION sbin COMPONENT services)

