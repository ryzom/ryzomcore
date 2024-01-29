cmake_minimum_required(VERSION 2.9)
project(QtPropertyBrowser)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtbuttonpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qteditorfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtgroupboxpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowserutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertymanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qttreepropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtvariantproperty.cpp
)
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtbuttonpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qteditorfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtgroupboxpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowserutils_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertymanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qttreepropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtvariantproperty.h
)

set(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qtpropertybrowser.qrc)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})

set(CMAKE_AUTORCC ON)

option(BUILD_SHARED_LIBS "Build shared library." ON)
option(BUILD_EXAMPLES "Build examples." OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_library(QtPropertyBrowser ${HEADER_FILES} ${SOURCE_FILES} ${QRC_FILES})

target_include_directories(QtPropertyBrowser PUBLIC
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
)

target_link_libraries(QtPropertyBrowser PUBLIC
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
)

set_target_properties(QtPropertyBrowser PROPERTIES AUTOMOC TRUE)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/demo)
endif()

include(GenerateExportHeader)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
generate_export_header(QtPropertyBrowser)

install(TARGETS QtPropertyBrowser
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/QtPropertyBrowser)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQtPropertyBrowser.cmake DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtpropertybrowser_export.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/QtPropertyBrowser)


option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
if(BUILD_DOCUMENTATION)
    include(doxygen/doxygen.cmake)
endif()
