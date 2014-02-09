ADD_EXECUTABLE(nl_sample_ls_client client.cpp)

ADD_EXECUTABLE(nl_sample_ls_fes WIN32 frontend_service.cpp)

ADD_DEFINITIONS(-DNL_LS_CFG="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_login_system/\\"")

TARGET_LINK_LIBRARIES(nl_sample_ls_client nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ls_client "NeL, Samples, Net, Login Service: LS Client")
NL_ADD_RUNTIME_FLAGS(nl_sample_ls_client)

TARGET_LINK_LIBRARIES(nl_sample_ls_fes nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ls_fes "NeL, Samples, Net, Login Service: LS Frontend")
NL_ADD_RUNTIME_FLAGS(nl_sample_ls_fes)

INSTALL(TARGETS nl_sample_ls_client nl_sample_ls_fes RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesnet)
INSTALL(FILES frontend_service.cfg client.cfg DESTINATION ${NL_SHARE_PREFIX}/nl_sample_login_system COMPONENT samplesnet)
