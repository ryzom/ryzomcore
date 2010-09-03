ADD_EXECUTABLE(nl_sample_ct_ai_service WIN32 ai_service.cpp)

ADD_EXECUTABLE(nl_sample_ct_gd_service WIN32 gd_service.cpp)

ADD_DEFINITIONS(-DNL_CT_CFG="\\"${NL_SHARE_PREFIX}/nl_sample_class_transport/\\"")

TARGET_LINK_LIBRARIES(nl_sample_ct_ai_service ${PLATFORM_LINKFLAGS} nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ct_ai_service "NeL, Samples, Net, Class Transport: AI Service")
NL_ADD_RUNTIME_FLAGS(nl_sample_ct_ai_service)

TARGET_LINK_LIBRARIES(nl_sample_ct_gd_service ${PLATFORM_LINKFLAGS} nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_ct_gd_service "NeL, Samples, Net, Class Transport: GD Service")
NL_ADD_RUNTIME_FLAGS(nl_sample_ct_gd_service)

INSTALL(TARGETS nl_sample_ct_ai_service nl_sample_ct_gd_service RUNTIME DESTINATION bin COMPONENT samplesnet)
INSTALL(FILES ai_service.cfg gd_service.cfg DESTINATION share/nel/nl_sample_class_transport COMPONENT samplesnet)
