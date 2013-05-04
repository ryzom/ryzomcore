FILE(GLOB SRC *.cpp *.h)

ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(ryzom_mission_compiler_fe WIN32 ${SRC} mission_compiler_fe.rc)

TARGET_LINK_LIBRARIES(ryzom_mission_compiler_fe nelmisc nelligo ryzom_mission_compiler_lib)

NL_DEFAULT_PROPS(ryzom_mission_compiler_fe "Ryzom, Tools, Misc: Mission Compiler Frontend")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler_fe)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_mission_compiler_fe ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_mission_compiler_fe RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
