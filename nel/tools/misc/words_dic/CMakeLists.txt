FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(words_dic WIN32 ${SRC} words_dic.rc)

TARGET_LINK_LIBRARIES(words_dic nelmisc)
NL_DEFAULT_PROPS(words_dic "NeL, Tools, Misc: words_dic")
NL_ADD_RUNTIME_FLAGS(words_dic)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(words_dic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS words_dic RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
