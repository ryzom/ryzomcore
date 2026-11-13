ADD_SUBDIRECTORY(src)

# The client loads snowballs_client.cfg from SNOWBALLS_CONFIG, which links snowballs_client_default.cfg next to it
SET(database "${SNOWBALLS_DATA_ABSOLUTE_FILE}")
CONFIGURE_FILE(client.cfg.in snowballs_client_default.cfg)
FILE(CONFIGURE OUTPUT snowballs_client.cfg CONTENT "RootConfigFilename = \"snowballs_client_default.cfg\";\n")

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/snowballs_client_default.cfg ${CMAKE_CURRENT_BINARY_DIR}/snowballs_client.cfg DESTINATION ${SNOWBALLS_CONFIG_FILE} COMPONENT snowballsclient)
INSTALL(DIRECTORY data/
        DESTINATION ${SNOWBALLS_DATA_FILE}
	COMPONENT snowballsdata
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN ".gitkeep" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
