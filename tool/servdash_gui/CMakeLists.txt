# servdash_gui: the service dashboard as a Qt application, for development shards.
# Builds in the Ryzom Core tree with WITH_QT5 or WITH_QT6, and on its own:
#   cmake -S tool/servdash_gui -B build_gui -DCMAKE_PREFIX_PATH=<qt install>
IF(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
  PROJECT(servdash_gui CXX)
  IF(WIN32)
    ENABLE_LANGUAGE(RC)
  ENDIF()
  SET(CMAKE_CXX_STANDARD 17)
  SET(CMAKE_CXX_STANDARD_REQUIRED ON)
  SET(CMAKE_AUTOMOC ON)
  FIND_PACKAGE(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
  FIND_PACKAGE(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)
  SET(SERVDASH_GUI_STANDALONE ON)
ENDIF()

ADD_EXECUTABLE(servdash_gui WIN32)
TARGET_SOURCES(servdash_gui
  PRIVATE
  src/main.cpp
  src/console_widget.cpp
  src/console_widget.h
  src/dashboard.cpp
  src/dashboard.h
  src/dashboard_config.cpp
  src/dashboard_config.h
  src/icons.cpp
  src/icons.h
  src/launch_ctrl.cpp
  src/launch_ctrl.h
  src/main_window.cpp
  src/main_window.h
  src/process_entry.cpp
  src/process_entry.h
  src/process_row.cpp
  src/process_row.h
  src/shard_lock.cpp
  src/shard_lock.h
  src/view_settings.cpp
  src/view_settings.h
)
TARGET_LINK_LIBRARIES(servdash_gui PRIVATE Qt::Core Qt::Gui Qt::Widgets Qt::Network)
IF(WIN32)
  TARGET_LINK_LIBRARIES(servdash_gui PRIVATE kernel32)
  TARGET_SOURCES(servdash_gui PRIVATE servdash_gui.rc)
ENDIF()

IF(SERVDASH_GUI_STANDALONE)
  TARGET_COMPILE_FEATURES(servdash_gui PRIVATE cxx_std_17)
ELSE()
  NL_DEFAULT_PROPS(servdash_gui "Ryzom, Tools: Service Dashboard Qt")
  NL_ADD_RUNTIME_FLAGS(servdash_gui)
  INSTALL(TARGETS servdash_gui RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools)
ENDIF()
