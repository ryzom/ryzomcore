FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_shard_unifier_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_shard_unifier_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_servershare
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARY}
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_shard_unifier_service "Ryzom, Services: Shard Unifier Service (SUS)")
NL_ADD_RUNTIME_FLAGS(ryzom_shard_unifier_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_shard_unifier_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_shard_unifier_service RUNTIME DESTINATION sbin COMPONENT services)
