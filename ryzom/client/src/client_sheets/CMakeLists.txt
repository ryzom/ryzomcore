FILE(GLOB SRC *.cpp)
FILE(GLOB PRIV_H *.h)

SOURCE_GROUP(headers FILES ${PRIV_H} ${PUB_H})

NL_TARGET_LIB(ryzom_clientsheets ${SRC} ${PRIV_H})

TARGET_LINK_LIBRARIES(ryzom_clientsheets ryzom_gameshare nelmisc)
NL_DEFAULT_PROPS(ryzom_clientsheets "Ryzom, Library: Client Sheets")
NL_ADD_LIB_SUFFIX(ryzom_clientsheets)

IF(WITH_PCH)
  target_precompile_headers(ryzom_clientsheets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_clientsheets LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()
