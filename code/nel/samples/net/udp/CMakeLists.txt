FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_udpclient client.cpp graph.cpp graph.h simlag.cpp simlag.h)

ADD_EXECUTABLE(nl_sample_udpserver WIN32 bench_service.cpp receive_task.cpp receive_task.h)

ADD_DEFINITIONS(-DUDP_DIR="\\"${NL_SHARE_PREFIX}/nl_sample_udp/\\"")

IF(WITH_3D)
  ADD_DEFINITIONS(-DUSE_3D)
ENDIF(WITH_3D)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nl_sample_udpclient ${PLATFORM_LINKFLAGS} nelmisc nelnet nel3d)
TARGET_LINK_LIBRARIES(nl_sample_udpserver ${PLATFORM_LINKFLAGS} nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_udpclient "NeL, Samples, Net, UDP: UDP Client")
NL_DEFAULT_PROPS(nl_sample_udpserver "NeL, Samples, Net, UDP: UDP Server")
NL_ADD_RUNTIME_FLAGS(nl_sample_udpclient)
NL_ADD_RUNTIME_FLAGS(nl_sample_udpserver)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nl_sample_udpclient nl_sample_udpserver RUNTIME DESTINATION bin COMPONENT samplesnet)
INSTALL(FILES bench_service.cfg client.cfg readme.txt DESTINATION share/nel/nl_sample_udp COMPONENT samplesnet)

IF(WITH_3D)
  INSTALL(FILES n019003l.pfb DESTINATION share/nel/nl_sample_udp COMPONENT samplesnet)
ENDIF(WITH_3D)
