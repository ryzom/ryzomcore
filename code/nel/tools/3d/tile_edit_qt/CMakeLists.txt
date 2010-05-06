INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR}/tools/3d/tile_edit_qt/ )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/tools/3d/tile_edit_qt/ )
INCLUDE( ${QT_USE_FILE} )

FILE(GLOB TILE_EDIT_SRC *.cpp pic/*.c pic/*.cpp)
SET(TILE_EDIT_HDR items_edit_dlg.h tile_browser_dlg.h tile_edit_dlg.h tile_rotation_dlg.h tiles_model.h tile_widget.h)
SET(TILE_EDIT_UIS tile_widget_qt.ui tile_rotation_qt.ui tile_edit_qt.ui tile_browser_qt.ui items_edit_qt.ui)
SET(TILE_EDIT_RCS tile_edit_qt.qrc)


SET( QT_USE_QT3SUPPORT TRUE)
SET( QT_USE_QTXML TRUE)

QT4_ADD_RESOURCES( TILE_EDIT_RC_SRCS ${TILE_EDIT_RCS} )
QT4_WRAP_UI( TILE_EDIT_UI_HDRS ${TILE_EDIT_UIS} )
QT4_WRAP_CPP( TILE_EDIT_MOC_SRCS ${TILE_EDIT_HDR})

ADD_EXECUTABLE(tile_edit_qt ${TILE_EDIT_SRC} ${TILE_EDIT_MOC_SRCS} ${TILE_EDIT_RC_SRCS} ${TILE_EDIT_UI_HDRS})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${QT_INCLUDES})
TARGET_LINK_LIBRARIES(tile_edit_qt ${LIBXML2_LIBRARIES} ${QT_LIBRARIES} ${PLATFORM_LINKFLAGS} nelmisc nel3d)
SET_TARGET_PROPERTIES(tile_edit_qt PROPERTIES PROJECT_LABEL "Tools, 3D: Qt Tile Editor")
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${QT_DEFINITIONS})

INSTALL(TARGETS tile_edit_qt RUNTIME DESTINATION bin COMPONENT tools3d)

