FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(multi_cd_setup_fix WIN32 ${SRC} multi_cd_setup_fix.rc)

#TARGET_LINK_LIBRARIES(multi_cd_setup_fix ${PLATFORM_LINKFLAGS})
NL_DEFAULT_PROPS(multi_cd_setup_fix "NeL, Tools, Misc: multi_cd_setup_fix")
NL_ADD_RUNTIME_FLAGS(multi_cd_setup_fix)

INSTALL(TARGETS multi_cd_setup_fix RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
