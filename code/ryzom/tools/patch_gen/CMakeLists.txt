SET(MAIN_SRC patch_gen_common.cpp patch_gen_main.cpp patch_gen_main.h)
SET(SERVICE_SRC patch_gen_common.cpp patch_gen_service.cpp patch_gen_service.h)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(patch_gen ${MAIN_SRC})
TARGET_LINK_LIBRARIES(patch_gen ryzom_gameshare ${PLATFORM_LINKFLAGS} ${LIBXML2_LIBRARIES} nelmisc nelnet nelligo nelgeorges)

ADD_EXECUTABLE(patch_gen_service ${SERVICE_SRC})
TARGET_LINK_LIBRARIES(patch_gen_service ryzom_gameshare ${PLATFORM_LINKFLAGS} ${LIBXML2_LIBRARIES} nelmisc nelnet nelligo nelgeogres)

NL_DEFAULT_PROPS(patch_gen_service "Ryzom, Tools: Patch Generator")
NL_ADD_RUNTIME_FLAGS(patch_gen_service)

NL_DEFAULT_PROPS(patch_gen "Ryzom, Tools: Patch Generator Service")
NL_ADD_RUNTIME_FLAGS(patch_gen)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS patch_gen RUNTIME DESTINATION bin COMPONENT tools)
INSTALL(TARGETS patch_gen_service RUNTIME DESTINATION bin COMPONENT tools)
