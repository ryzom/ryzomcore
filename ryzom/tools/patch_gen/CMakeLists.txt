INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/nel/3rdparty/seven_zip)

SET(MAIN_SRC patch_gen_common.cpp patch_gen_main.cpp patch_gen_main.h patch_gen.rc)
SET(SERVICE_SRC patch_gen_common.cpp patch_gen_service.cpp patch_gen_service.h patch_gen_service.rc)

ADD_EXECUTABLE(patch_gen ${MAIN_SRC} ${RECS})
TARGET_LINK_LIBRARIES(patch_gen nel_sevenzip ryzom_gameshare nelmisc nelnet nelligo nelgeorges)
NL_DEFAULT_PROPS(patch_gen "Ryzom, Tools: Patch Generator")
NL_ADD_RUNTIME_FLAGS(patch_gen)

ADD_EXECUTABLE(patch_gen_service WIN32 ${SERVICE_SRC} ${RECS})
TARGET_LINK_LIBRARIES(patch_gen_service nel_sevenzip ryzom_gameshare nelmisc nelnet nelligo nelgeorges)
NL_DEFAULT_PROPS(patch_gen_service "Ryzom, Tools: Patch Generator Service")
NL_ADD_RUNTIME_FLAGS(patch_gen_service)

INSTALL(TARGETS patch_gen RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
INSTALL(TARGETS patch_gen_service RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
