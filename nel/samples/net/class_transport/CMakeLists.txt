ADD_EXECUTABLE(nl_sample_ct_ai_service WIN32 ai_service.cpp)

ADD_EXECUTABLE(nl_sample_ct_gd_service WIN32 gd_service.cpp)

ADD_DEFINITIONS(-DNL_CT_CFG="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_class_transport/\\"")

TARGET_LINK_LIBRARIES(nl_sample_ct_ai_service nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ct_ai_service "NeL, Samples, Net, Class Transport: AI Service")
NL_ADD_RUNTIME_FLAGS(nl_sample_ct_ai_service)

TARGET_LINK_LIBRARIES(nl_sample_ct_gd_service nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ct_gd_service "NeL, Samples, Net, Class Transport: GD Service")
NL_ADD_RUNTIME_FLAGS(nl_sample_ct_gd_service)

INSTALL(TARGETS nl_sample_ct_ai_service nl_sample_ct_gd_service RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesnet)
INSTALL(FILES ai_service.cfg gd_service.cfg DESTINATION ${NL_SHARE_PREFIX}/nl_sample_class_transport COMPONENT samplesnet)
