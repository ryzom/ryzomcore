FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(logic_editor WIN32 ${SRC})

TARGET_LINK_LIBRARIES(logic_editor nelmisc logic_editor_dll)
NL_DEFAULT_PROPS(logic_editor "NeL, Tools, Logic: Logic Editor")
NL_ADD_RUNTIME_FLAGS(logic_editor)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(logic_editor ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS logic_editor RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
