FILE(GLOB GEORGES_EDITOR_SRC src/*.cpp src/*.h)

SET(GEORGES_EDITOR_RCS src/resources.qrc)

SOURCE_GROUP(QtResources FILES ${GEORGES_EDITOR_RCS})

ADD_EXECUTABLE(georges_editor_qt WIN32 MACOSX_BUNDLE ${GEORGES_EDITOR_SRC} ${GEORGES_EDITOR_RCS})

TARGET_INCLUDE_DIRECTORIES(georges_editor_qt
		PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(georges_editor_qt
		PRIVATE
		NeL::misc
		NeL::georges
		${QT_LIBRARIES}
		${QT_QTMAIN_LIBRARY})

NL_DEFAULT_PROPS(georges_editor_qt "NeL, Tools, Georges: Georges Editor Qt")
NL_ADD_RUNTIME_FLAGS(georges_editor_qt)

IF(WITH_PCH)
  target_precompile_headers(georges_editor_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/stdpch.h)
ENDIF()

INSTALL(TARGETS georges_editor_qt RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsgeorges)
