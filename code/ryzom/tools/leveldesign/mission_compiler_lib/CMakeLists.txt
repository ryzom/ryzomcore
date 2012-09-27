FILE(GLOB LIBSRC mission_compiler.cpp
  mission_compiler.h
  step.h
  step_content.cpp
  steps.cpp
  variables.cpp)

INCLUDE_DIRECTORIES(${NEL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

NL_TARGET_LIB(ryzom_mission_compiler_lib ${LIBSRC})

TARGET_LINK_LIBRARIES(ryzom_mission_compiler_lib nelmisc nelligo)
NL_DEFAULT_PROPS(ryzom_mission_compiler_lib "Ryzom, Library: Mission Compiler")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler_lib)
NL_ADD_LIB_SUFFIX(ryzom_mission_compiler_lib)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_mission_compiler_lib LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)

FILE(GLOB EXESRC main.cpp)
  
ADD_EXECUTABLE(ryzom_mission_compiler ${EXESRC})

TARGET_LINK_LIBRARIES(ryzom_mission_compiler ryzom_mission_compiler_lib)
NL_DEFAULT_PROPS(ryzom_mission_compiler "Ryzom, Tools, Misc: Mission Compiler")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler)

INSTALL(TARGETS ryzom_mission_compiler RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
