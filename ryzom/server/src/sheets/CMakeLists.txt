# Sheet-definition code shared between the shard services and the
# sheets_packer_shard tool, compiled once into real libraries. Historically
# the tool recompiled service translation units with NO_EGS_VARS/NO_AI_COMP
# preprocessor forks, which silently broke the tool link whenever the service
# code drifted. Service-global dependencies are link-time seams instead: each
# consumer provides the few symbols documented at their declarations (see
# writeDirectory in egs_sheets.cpp), so a missing definition is a link error,
# never a behavior change.
#
# These libraries are deliberately STATIC: the seam symbols are resolved from
# the linking executable, which a shared library on Linux would only see with
# -rdynamic.

SET(EGS_DIR ${RZ_SERVER_SRC_DIR}/entities_game_service)

FILE(GLOB EGS_SHEETS_SRC ${EGS_DIR}/egs_sheets/*.cpp ${EGS_DIR}/egs_sheets/*.h)
LIST(APPEND EGS_SHEETS_SRC
	${EGS_DIR}/entity_structure/resists.cpp
	${EGS_DIR}/entity_structure/resists.h)

ADD_LIBRARY(ryzom_egssheets STATIC ${EGS_SHEETS_SRC})
add_library(Ryzom::egssheets ALIAS ryzom_egssheets)

SOURCE_GROUP("egs" FILES ${EGS_SHEETS_SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_egssheets PUBLIC ${RZ_SERVER_SRC_DIR} ${EGS_DIR})

TARGET_LINK_LIBRARIES(ryzom_egssheets
		PUBLIC
		Ryzom::gameshare
		Ryzom::servershare
		Ryzom::aishare
		NeL::misc
		NeL::net
		NeL::georges
)

NL_DEFAULT_PROPS(ryzom_egssheets "Ryzom, Library: EGS Sheets")

IF(MSVC)
	SET_TARGET_PROPERTIES(ryzom_egssheets PROPERTIES COMPILE_FLAGS "/bigobj")
ENDIF()

IF(WITH_PCH AND NOT MINGW)
	target_precompile_headers(ryzom_egssheets PRIVATE ${EGS_DIR}/stdpch.h)
ENDIF()

SET(AIS_DIR ${RZ_SERVER_SRC_DIR}/ai_service)

SET(AIS_SHEETS_SRC
	${AIS_DIR}/sheets.cpp
	${AIS_DIR}/sheets.h
	${AIS_DIR}/commands_mlf.cpp)

ADD_LIBRARY(ryzom_aissheets STATIC ${AIS_SHEETS_SRC})
add_library(Ryzom::aissheets ALIAS ryzom_aissheets)

SOURCE_GROUP("ais" FILES ${AIS_SHEETS_SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_aissheets PUBLIC ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_aissheets
		PUBLIC
		Ryzom::gameshare
		Ryzom::servershare
		Ryzom::aishare
		NeL::misc
		NeL::net
		NeL::georges
)

NL_DEFAULT_PROPS(ryzom_aissheets "Ryzom, Library: AIS Sheets")

IF(WITH_PCH AND NOT MINGW)
	target_precompile_headers(ryzom_aissheets PRIVATE ${AIS_DIR}/stdpch.h)
ENDIF()

SET(GPM_DIR ${RZ_SERVER_SRC_DIR}/gpm_service)

SET(GPM_SHEETS_SRC
	${GPM_DIR}/sheets.cpp
	${GPM_DIR}/sheets.h)

ADD_LIBRARY(ryzom_gpmsheets STATIC ${GPM_SHEETS_SRC})
add_library(Ryzom::gpmsheets ALIAS ryzom_gpmsheets)

SOURCE_GROUP("gpm" FILES ${GPM_SHEETS_SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_gpmsheets PUBLIC ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_gpmsheets
		PUBLIC
		Ryzom::gameshare
		NeL::misc
		NeL::net
		NeL::georges
)

NL_DEFAULT_PROPS(ryzom_gpmsheets "Ryzom, Library: GPMS Sheets")

IF(WITH_PCH AND NOT MINGW)
	target_precompile_headers(ryzom_gpmsheets PRIVATE ${GPM_DIR}/stdpch.h)
ENDIF()

SET(IOS_DIR ${RZ_SERVER_SRC_DIR}/input_output_service)

SET(IOS_SHEETS_SRC
	${IOS_DIR}/string_manager_sheet.cpp
	${IOS_DIR}/string_manager.h)

ADD_LIBRARY(ryzom_iossheets STATIC ${IOS_SHEETS_SRC})
add_library(Ryzom::iossheets ALIAS ryzom_iossheets)

SOURCE_GROUP("ios" FILES ${IOS_SHEETS_SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_iossheets PUBLIC ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_iossheets
		PUBLIC
		Ryzom::gameshare
		NeL::misc
		NeL::net
		NeL::georges
)

NL_DEFAULT_PROPS(ryzom_iossheets "Ryzom, Library: IOS Sheets")

IF(WITH_PCH AND NOT MINGW)
	target_precompile_headers(ryzom_iossheets PRIVATE ${IOS_DIR}/stdpch.h)
ENDIF()
