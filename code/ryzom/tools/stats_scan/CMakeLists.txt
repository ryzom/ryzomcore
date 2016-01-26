FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(stats_scan WIN32 ${SRC})

TARGET_LINK_LIBRARIES(stats_scan ryzom_gameshare nelmisc nelnet nelligo nelgeorges)

NL_DEFAULT_PROPS(stats_scan "Ryzom, Tools, Misc: Stats Scan")
NL_ADD_RUNTIME_FLAGS(stats_scan)

INSTALL(TARGETS stats_scan RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
