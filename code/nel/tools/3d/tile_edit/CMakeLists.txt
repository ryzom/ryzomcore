FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

FILE(GLOB SRC2 cpu.cpp DllEntry.cpp Popup.* thread_win32.* TileCtrl.* TileList.* TileView.*)
LIST(REMOVE_ITEM SRC ${SRC2})

#ENABLE_LANGUAGE(RC)

ADD_EXECUTABLE(tile_edit WIN32 ${SRC} tile_edit_exe.rc)

TARGET_LINK_LIBRARIES(tile_edit nelmisc nel3d)
NL_DEFAULT_PROPS(tile_edit "NeL, Tools, 3D: Tile Editor")
NL_ADD_RUNTIME_FLAGS(tile_edit)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(tile_edit ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp)
ENDIF()

INSTALL(TARGETS tile_edit RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
