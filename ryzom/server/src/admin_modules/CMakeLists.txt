FILE(GLOB SRC *.cpp *.h)

NL_TARGET_LIB(ryzom_adminmodules ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_adminmodules nelmisc nelnet)
NL_DEFAULT_PROPS(ryzom_adminmodules "Ryzom, Library: Service Admin Modules")
NL_ADD_RUNTIME_FLAGS(ryzom_adminmodules)
NL_ADD_LIB_SUFFIX(ryzom_adminmodules)

IF(WITH_PCH)
  target_precompile_headers(ryzom_adminmodules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_adminmodules LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()

