FILE(GLOB SRC *.cpp *.h)

NL_TARGET_LIB(ryzom_aishare ${SRC})

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

IF(WITH_PCH)
  target_precompile_headers(ryzom_aishare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES)
  INSTALL(TARGETS ryzom_aishare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()
