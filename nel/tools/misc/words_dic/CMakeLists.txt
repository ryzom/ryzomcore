FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(words_dic WIN32 ${SRC} words_dic.rc)
SET_TARGET_PROPERTIES(words_dic PROPERTIES NL_MFC_UNICODE_APP ON) # unicode MFC GUI: pre-VS2010 links need the explicit wWinMain entry (see NL_DEFAULT_PROPS)

TARGET_LINK_LIBRARIES(words_dic PRIVATE NeL::misc)
NL_DEFAULT_PROPS(words_dic "NeL, Tools, Misc: words_dic")
NL_ADD_RUNTIME_FLAGS(words_dic)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(words_dic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS words_dic RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
