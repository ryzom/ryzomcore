FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/service.cpp ${CMAKE_CURRENT_SOURCE_DIR}/service.h)

ADD_EXECUTABLE(ryzom_dynamic_scenario_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_dynamic_scenario_service
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

NL_DEFAULT_PROPS(ryzom_dynamic_scenario_service "Ryzom, Services: Dynamic Scenario Service (DSS)")
NL_ADD_RUNTIME_FLAGS(ryzom_dynamic_scenario_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_dynamic_scenario_service RUNTIME DESTINATION sbin COMPONENT services)
