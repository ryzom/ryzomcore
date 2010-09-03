FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(mail_forum_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(mail_forum_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						nelmisc 
						nelnet)

NL_DEFAULT_PROPS(mail_forum_service "Ryzom, Services: Mail Forum Service (MFS)")
NL_ADD_RUNTIME_FLAGS(mail_forum_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(mail_forum_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS mail_forum_service RUNTIME DESTINATION sbin COMPONENT services)


