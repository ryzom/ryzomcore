FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_planar_reflection WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_planar_reflection nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_planar_reflection "NeL, Samples, 3D: Planar Reflection")
NL_ADD_RUNTIME_FLAGS(nl_sample_planar_reflection)

INSTALL(TARGETS nl_sample_planar_reflection RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
