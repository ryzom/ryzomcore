FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_shard_unifier_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_shard_unifier_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_servershare
						nelmisc
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_shard_unifier_service "Ryzom, Services: Shard Unifier Service (SUS)")
NL_ADD_RUNTIME_FLAGS(ryzom_shard_unifier_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(MSVC)
	SET_TARGET_PROPERTIES(ryzom_shard_unifier_service PROPERTIES COMPILE_FLAGS "/bigobj")
ENDIF()

IF(WITH_PCH)
  target_precompile_headers(ryzom_shard_unifier_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS ryzom_shard_unifier_service RUNTIME DESTINATION sbin COMPONENT services)
