FILE(GLOB LIBSRC mission_compiler.cpp
  mission_compiler.h
  step.h
  step_content.cpp
  steps.cpp
  variables.cpp)

NL_TARGET_LIB(ryzom_mission_compiler_lib ${LIBSRC})
add_library(Ryzom::mission_compiler_lib ALIAS ryzom_mission_compiler_lib)

target_sources(ryzom_mission_compiler_lib
        PUBLIC
        FILE_SET HEADERS
        FILES
        mission_compiler.h
        PRIVATE
        step_content.cpp
        steps.cpp
        variables.cpp
)


TARGET_INCLUDE_DIRECTORIES(ryzom_mission_compiler_lib PRIVATE ${NEL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_mission_compiler_lib
        PUBLIC
        NeL::misc
        NeL::ligo
)
NL_DEFAULT_PROPS(ryzom_mission_compiler_lib "Ryzom, Library: Mission Compiler")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler_lib)
NL_ADD_LIB_SUFFIX(ryzom_mission_compiler_lib)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_mission_compiler_lib LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()

FILE(GLOB EXESRC main.cpp)
  
ADD_EXECUTABLE(ryzom_mission_compiler ${EXESRC})

TARGET_LINK_LIBRARIES(ryzom_mission_compiler PRIVATE Ryzom::mission_compiler_lib)
NL_DEFAULT_PROPS(ryzom_mission_compiler "Ryzom, Tools, Misc: Mission Compiler")
NL_ADD_RUNTIME_FLAGS(ryzom_mission_compiler)

INSTALL(TARGETS ryzom_mission_compiler RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
