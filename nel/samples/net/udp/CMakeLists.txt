FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_udpclient client.cpp graph.cpp graph.h simlag.cpp simlag.h)

ADD_EXECUTABLE(nl_sample_udpserver WIN32 bench_service.cpp receive_task.cpp receive_task.h)

SET(UDP_DIR "UDP_DIR=\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_udp/\"")
TARGET_COMPILE_DEFINITIONS(nl_sample_udpclient PRIVATE ${UDP_DIR})
TARGET_COMPILE_DEFINITIONS(nl_sample_udpserver PRIVATE ${UDP_DIR})

TARGET_LINK_LIBRARIES(nl_sample_udpclient nelmisc nelnet)
TARGET_LINK_LIBRARIES(nl_sample_udpserver nelmisc nelnet)

IF(WITH_3D)
  ADD_DEFINITIONS(-DUSE_3D)
  TARGET_LINK_LIBRARIES(nl_sample_udpclient nel3d)
ENDIF()

NL_DEFAULT_PROPS(nl_sample_udpclient "NeL, Samples, Net, UDP: UDP Client")
NL_DEFAULT_PROPS(nl_sample_udpserver "NeL, Samples, Net, UDP: UDP Server")
NL_ADD_RUNTIME_FLAGS(nl_sample_udpclient)
NL_ADD_RUNTIME_FLAGS(nl_sample_udpserver)

INSTALL(TARGETS nl_sample_udpclient nl_sample_udpserver RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesnet)
INSTALL(FILES bench_service.cfg client.cfg readme.txt DESTINATION ${NL_SHARE_PREFIX}/nl_sample_udp COMPONENT samplesnet)

IF(WITH_3D)
  INSTALL(FILES n019003l.pfb DESTINATION ${NL_SHARE_PREFIX}/nl_sample_udp COMPONENT samplesnet)
ENDIF()
