FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheet_random_generator ${SRC})

TARGET_LINK_LIBRARIES(sheet_random_generator
						nelmisc
						nelgeorges)

NL_DEFAULT_PROPS(sheet_random_generator "Ryzom, Tools: Sheets Random Generator")
NL_ADD_RUNTIME_FLAGS(sheet_random_generator)

IF(WITH_PCH)
  target_precompile_headers(sheet_random_generator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS sheet_random_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
