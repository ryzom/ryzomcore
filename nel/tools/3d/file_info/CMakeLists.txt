FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(file_info ${SRC})

TARGET_LINK_LIBRARIES(file_info nel3d nelmisc)
NL_DEFAULT_PROPS(file_info "NeL, Tools, 3D: file_info")
NL_ADD_RUNTIME_FLAGS(file_info)

INSTALL(TARGETS file_info RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
