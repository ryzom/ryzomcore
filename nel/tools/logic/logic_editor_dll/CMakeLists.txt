FILE(GLOB SRC *.cpp *.h res/logic_editor.rc2)

ENABLE_LANGUAGE(RC)

ADD_DEFINITIONS(${MFC_DEFINITIONS})
SET(CMAKE_MFC_FLAG 2) 
ADD_LIBRARY(logic_editor_dll SHARED ${SRC})

TARGET_LINK_LIBRARIES(logic_editor_dll nelmisc nellogic nelnet)
NL_DEFAULT_PROPS(logic_editor_dll "NeL, Tools, Logic: Logic Editor DLL")
NL_ADD_RUNTIME_FLAGS(logic_editor_dll)
NL_ADD_LIB_SUFFIX(logic_editor_dll)

IF(WITH_PCH)
  target_precompile_headers(logic_editor_dll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS logic_editor_dll LIBRARY DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT toolsmisc)
