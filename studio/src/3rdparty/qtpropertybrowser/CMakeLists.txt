INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDES})
INCLUDE(${QT_USE_FILE})

FILE(GLOB SRC *.cpp *.h)

SET(QT_PROPERTY_EDITOR_HDR	qtpropertybrowser.h
				qtpropertymanager.h
				qteditorfactory.h
				qtvariantproperty.h
				qttreepropertybrowser.h
				qtbuttonpropertybrowser.h
				qtgroupboxpropertybrowser.h
				qtpropertybrowserutils_p.h)

SET(QT_PROPERTY_EDITOR_RCS	qtpropertybrowser.qrc)

SET(QT_USE_QTGUI TRUE)

QT4_ADD_RESOURCES(QT_PROPERTY_EDITOR_RC_SRCS ${QT_PROPERTY_EDITOR_RCS})
QT4_WRAP_CPP(QT_PROPERTY_EDITOR_MOC_SRC ${QT_PROPERTY_EDITOR_HDR})

SOURCE_GROUP(QtResources FILES ${QT_PROPERTY_EDITOR_RCS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${QT_PROPERTY_EDITOR_MOC_SRC})
SOURCE_GROUP("Qt Property Editor Source" FILES ${SRC})

qt4_generate_moc(qtpropertymanager.cpp ${CMAKE_CURRENT_BINARY_DIR}/qtpropertymanager.moc)
qt4_generate_moc(qteditorfactory.cpp ${CMAKE_CURRENT_BINARY_DIR}/qteditorfactory.moc)
qt4_generate_moc(qttreepropertybrowser.cpp ${CMAKE_CURRENT_BINARY_DIR}/qttreepropertybrowser.moc)

# Need to remove these so that they are not linked as they are inline included.
LIST(REMOVE_ITEM QT_PROPERTY_EDITOR_MOC_SRC	${CMAKE_CURRENT_BINARY_DIR}/moc_qtbuttonpropertybrowser.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qteditorfactory.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qtgroupboxpropertybrowser.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertybrowser.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertymanager.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qttreepropertybrowser.cxx
						${CMAKE_CURRENT_BINARY_DIR}/moc_qtvariantproperty.cxx)

# We need to add new depencencies on removed files because we need them to be still generated
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qtbuttonpropertybrowser.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qtbuttonpropertybrowser.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qteditorfactory.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qteditorfactory.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qtgroupboxpropertybrowser.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qtgroupboxpropertybrowser.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qtpropertybrowser.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertybrowser.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qtpropertymanager.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertymanager.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qttreepropertybrowser.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qttreepropertybrowser.cxx)
SET_PROPERTY(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/qtvariantproperty.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_qtvariantproperty.cxx)

#set(
#  qtpropertyeditor_HEADERS_ONLY_MOC
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertybrowser.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertymanager.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qteditorfactory.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtvariantproperty.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qttreepropertybrowser.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtbuttonpropertybrowser.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtgroupboxpropertybrowser.cpp
#  ${CMAKE_CURRENT_BINARY_DIR}/qtpropertymanager.moc
#  ${CMAKE_CURRENT_BINARY_DIR}/qteditorfactory.moc
#  ${CMAKE_CURRENT_BINARY_DIR}/qttreepropertybrowser.moc
#)
#
#set_source_files_properties(
#  ${qtpropertyeditor_HEADERS_ONLY_MOC}
#  PROPERTIES
#    HEADER_FILE_ONLY true
#)
#

#set(
#  qtpropertyeditor_HEADERS_MOC
#  ${CMAKE_CURRENT_BINARY_DIR}/moc_qtpropertybrowserutils_p.cpp
#)

#set(
#  qtpropertyeditor_MOC
#  ${qtpropertyeditor_HEADERS_MOC}
#  ${qtpropertyeditor_HEADERS_ONLY_MOC}
#)

ADD_LIBRARY(qt_property_browser SHARED	${SRC}
					${QT_PROPERTY_EDITOR_MOC_SRC}
					${QT_PROPERTY_EDITOR_RC_SRCS}
					${CMAKE_CURRENT_BINARY_DIR}/qtpropertymanager.moc
					${CMAKE_CURRENT_BINARY_DIR}/qttreepropertybrowser.moc
					${CMAKE_CURRENT_BINARY_DIR}/qteditorfactory.moc)

TARGET_LINK_LIBRARIES(qt_property_browser ${QT_LIBRARIES})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_DLL)
ADD_DEFINITIONS(-DQT_QTPROPERTYBROWSER_EXPORT)
ADD_DEFINITIONS(-DQT_PLUGIN)
#ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

NL_DEFAULT_PROPS(qt_property_browser "3rdParty: Qt Property Browser 2.5")
NL_ADD_RUNTIME_FLAGS(qt_property_browser)
NL_ADD_LIB_SUFFIX(qt_property_browser)

INSTALL(TARGETS qt_property_browser LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
