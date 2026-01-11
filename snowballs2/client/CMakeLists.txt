ADD_SUBDIRECTORY(src)

SET(database "${SNOWBALLS_DATA_FILE}")
CONFIGURE_FILE(client.cfg.in client.cfg)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/client.cfg DESTINATION etc/snowballs COMPONENT snowballsclient)
INSTALL(DIRECTORY data/
        DESTINATION share/games/snowballs
	COMPONENT snowballsdata
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
