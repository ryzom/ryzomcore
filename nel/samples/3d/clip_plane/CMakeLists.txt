FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_clip_plane WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_clip_plane nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_clip_plane "NeL, Samples, 3D: Clip Plane")
NL_ADD_RUNTIME_FLAGS(nl_sample_clip_plane)

INSTALL(TARGETS nl_sample_clip_plane RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
