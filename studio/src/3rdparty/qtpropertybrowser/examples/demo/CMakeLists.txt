cmake_minimum_required(VERSION 2.9)
project(Demo)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(QtPropertyBrowser REQUIRED)

set(SOURCE_FILES main.cpp)
set(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/demo.qrc)
  
set(CMAKE_AUTORCC ON)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(Demo ${SOURCE_FILES} ${QRC_FILES})
target_link_libraries(Demo
    ${QTPROPERTYBROWSER_LIBRARY}
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
)

target_include_directories(Demo PUBLIC
    ${QTPROPERTYBROWSER_INCLUDE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
)
