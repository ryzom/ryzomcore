FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_shard_unifier_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_shard_unifier_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_shard_unifier_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::gameshare
        Ryzom::servershare
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_shard_unifier_service "Ryzom, Services: Shard Unifier Service (SUS)")
NL_ADD_RUNTIME_FLAGS(ryzom_shard_unifier_service)

IF (MSVC)
    SET_TARGET_PROPERTIES(ryzom_shard_unifier_service PROPERTIES COMPILE_FLAGS "/bigobj")
ENDIF ()

IF (WITH_PCH)
    target_precompile_headers(ryzom_shard_unifier_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_shard_unifier_service RUNTIME DESTINATION sbin COMPONENT services)
