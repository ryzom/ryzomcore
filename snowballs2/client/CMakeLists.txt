ADD_SUBDIRECTORY(src)

# The client finds snowballs_client_default.cfg in SNOWBALLS_CONFIG and creates the snowballs_client.cfg that links it
# in the directory where it keeps its files
SET(database "${SNOWBALLS_DATA_ABSOLUTE_FILE}")
CONFIGURE_FILE(client.cfg.in snowballs_client_default.cfg)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/snowballs_client_default.cfg DESTINATION ${SNOWBALLS_CONFIG_FILE} COMPONENT snowballsclient)
INSTALL(DIRECTORY data/
        DESTINATION ${SNOWBALLS_DATA_FILE}
	COMPONENT snowballsdata
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN ".gitkeep" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
