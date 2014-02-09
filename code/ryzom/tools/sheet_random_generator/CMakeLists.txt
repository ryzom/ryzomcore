FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheet_random_generator ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(sheet_random_generator
						${LIBXML2_LIBRARIES}
						nelmisc
						nelgeorges)

NL_DEFAULT_PROPS(sheet_random_generator "Ryzom, Tools: Sheets Random Generator")
NL_ADD_RUNTIME_FLAGS(sheet_random_generator)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(sheet_random_generator ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS sheet_random_generator RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
