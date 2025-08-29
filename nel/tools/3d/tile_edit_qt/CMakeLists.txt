FILE(GLOB TILE_EDIT_SRC *.cpp *.h)
FILE(GLOB PIC_SRC pic/*.c pic/*.cpp)
SET(TILE_EDIT_HDR items_edit_dlg.h tile_browser_dlg.h tile_edit_dlg.h tile_rotation_dlg.h tiles_model.h tile_widget.h)
SET(TILE_EDIT_UIS tile_widget_qt.ui tile_rotation_qt.ui tile_edit_qt.ui tile_browser_qt.ui items_edit_qt.ui)
SET(TILE_EDIT_RCS tile_edit_qt.qrc)


SOURCE_GROUP(QtResources FILES ${TILE_EDIT_UIS} ${TILE_EDIT_RCS})

ADD_EXECUTABLE(tile_edit_qt WIN32 ${TILE_EDIT_SRC} ${TILE_EDIT_HDR} ${TILE_EDIT_RCS} ${TILE_EDIT_UIS})

TARGET_INCLUDE_DIRECTORIES(tile_edit_qt PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(tile_edit_qt ${QT_LIBRARIES} nelmisc nel3d)
NL_DEFAULT_PROPS(tile_edit_qt "NeL, Tools, 3D: Qt Tile Editor")
NL_ADD_RUNTIME_FLAGS(tile_edit_qt)

IF(WITH_PCH)
  target_precompile_headers(tile_edit_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common.h)
ENDIF()

INSTALL(TARGETS tile_edit_qt RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)

