FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheet_random_generator ${SRC})

TARGET_LINK_LIBRARIES(sheet_random_generator
						nelmisc
						nelgeorges)

NL_DEFAULT_PROPS(sheet_random_generator "Ryzom, Tools: Sheets Random Generator")
NL_ADD_RUNTIME_FLAGS(sheet_random_generator)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(sheet_random_generator ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
ENDIF()

INSTALL(TARGETS sheet_random_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
