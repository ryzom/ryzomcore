FILE(GLOB SRC ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.cpp ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/egs_sheets/*.h)

LIST(APPEND SRC
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/entity_structure/resists.h
)

NL_TARGET_LIB(ryzom_egs_sheets ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_egs_sheets PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/common/src ${CMAKE_SOURCE_DIR}/ryzom/server/src ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service)

TARGET_LINK_LIBRARIES(ryzom_egs_sheets
        PUBLIC
        ryzom_gameshare
        ryzom_servershare
        ryzom_aishare
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_egs_sheets "Ryzom, Library: EGS Sheets")
NL_ADD_RUNTIME_FLAGS(ryzom_egs_sheets)
NL_ADD_LIB_SUFFIX(ryzom_egs_sheets)

IF(MSVC)
    SET_TARGET_PROPERTIES(ryzom_egs_sheets PROPERTIES COMPILE_FLAGS "/bigobj")
ENDIF()

IF(WITH_PCH)
    target_precompile_headers(ryzom_egs_sheets PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/server/src/entities_game_service/stdpch.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS ryzom_egs_sheets LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()
