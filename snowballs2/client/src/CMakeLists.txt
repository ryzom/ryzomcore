FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_client WIN32 ${SRC})

TARGET_LINK_LIBRARIES(snowballs_client
						NeL::misc
						NeL::pacs
						NeL::net
						NeL::3d)

NL_DEFAULT_PROPS(snowballs_client "Snowballs, Client: Snowballs Client")
NL_ADD_RUNTIME_FLAGS(snowballs_client)

# If sound is enabled then add the definitions and link the libraries.
IF(WITH_SOUND)
  TARGET_COMPILE_DEFINITIONS(snowballs_client PRIVATE SBCLIENT_WITH_SOUND)
  TARGET_LINK_LIBRARIES(snowballs_client NeL::sound)
ENDIF()

INSTALL(TARGETS snowballs_client RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT snowballsclient)
