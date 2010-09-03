FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(shard_unifier_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(shard_unifier_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARY}
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(shard_unifier_service "Ryzom, Services: Shard Unifier Service (SUS)")
NL_ADD_RUNTIME_FLAGS(shard_unifier_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(shard_unifier_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS shard_unifier_service RUNTIME DESTINATION sbin COMPONENT services)


