FILE(GLOB SRC *.cpp *.h *.rc *.rc2 res/*.rc2 res/*.ico)

SOURCE_GROUP("" FILES ${SRC})

ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(ryzom_mission_compiler_fe WIN32 ${SRC})

TARGET_LINK_LIBRARIES(ryzom_mission_compiler_fe nelmisc nelligo ryzom_mission_compiler_lib)

NL_DEFAULT_PROPS(ryzom_mission_compiler_fe "Ryzom, Tools, Misc: Mission Compiler Frontend")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler_fe)

IF(WITH_PCH)
  target_precompile_headers(ryzom_mission_compiler_fe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS ryzom_mission_compiler_fe RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
