FILE(GLOB SRC *.cpp *.h)

ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(georges_exe WIN32 ${SRC} georges_exe.rc)

TARGET_LINK_LIBRARIES(georges_exe nelmisc nelgeorges georges_dll)

NL_DEFAULT_PROPS(georges_exe "Ryzom, Tools, Georges: Georges Exe")
NL_ADD_RUNTIME_FLAGS(georges_exe)

INSTALL(TARGETS georges_exe RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
