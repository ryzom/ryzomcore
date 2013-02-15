FILE(GLOB SRC *.cpp *.h)

NL_TARGET_LIB(ryzom_aishare ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_aishare
						ryzom_gameshare
						ryzom_servershare
						nelmisc
						nelnet
						nelgeorges
						nelpacs)

NL_DEFAULT_PROPS(ryzom_aishare "Ryzom, Library: AI Shared")
NL_ADD_RUNTIME_FLAGS(ryzom_aishare)
NL_ADD_LIB_SUFFIX(ryzom_aishare)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_aishare ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_aishare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
