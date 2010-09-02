ADD_EXECUTABLE(nl_sample_ls_client client.cpp)

ADD_EXECUTABLE(nl_sample_ls_fes frontend_service.cpp)

ADD_DEFINITIONS(-DNL_LS_CFG="\\"${NL_SHARE_PREFIX}/nl_sample_login_system/\\"")

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nl_sample_ls_client ${PLATFORM_LINKFLAGS} nelmisc nelnet)
TARGET_LINK_LIBRARIES(nl_sample_ls_fes ${PLATFORM_LINKFLAGS} nelmisc nelnet)
IF(WIN32)
  SET_TARGET_PROPERTIES(nl_sample_ls_client PROPERTIES 
    LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
    PROJECT_LABEL "NeL, Samples, Net, Login Service: LS Client")
  SET_TARGET_PROPERTIES(nl_sample_ls_fes PROPERTIES 
    LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    PROJECT_LABEL "NeL, Samples, Net, Login Service: LS Frontend")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nl_sample_ls_client nl_sample_ls_fes RUNTIME DESTINATION bin COMPONENT samplesnet)
INSTALL(FILES frontend_service.cfg client.cfg DESTINATION share/nel/nl_sample_login_system COMPONENT samplesnet)
