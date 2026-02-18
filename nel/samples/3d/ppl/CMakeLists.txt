FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_ppl WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_ppl nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_ppl "NeL, Samples, 3D: Per-Pixel Lighting")
NL_ADD_RUNTIME_FLAGS(nl_sample_ppl)

INSTALL(TARGETS nl_sample_ppl RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
