FIND_PACKAGE(OpenGL REQUIRED)

FILE(GLOB SRC src/*.cpp src/*.h resources/*.rc)

FILE(GLOB CLIENT_CONFIG_UIS ui/*.ui)
FILE(GLOB CLIENT_CONFIG_BASE_TRANS translations/qtbase_*.ts)
FILE(GLOB CLIENT_CONFIG_TRANS translations/ryzom_configuration_*.ts)

CONFIGURE_FILE(translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY)

#  uncomment this line if you want to update original translations
#  QT5_CREATE_TRANSLATION(CLIENT_CONFIG_QM ${CLIENT_CONFIG_UIS} ${SRC} ${CLIENT_CONFIG_TRANS} OPTIONS -I ${CMAKE_CURRENT_SOURCE_DIR})
QT5_ADD_TRANSLATION(CLIENT_CONFIG_QM ${CLIENT_CONFIG_TRANS} ${CLIENT_CONFIG_BASE_TRANS})

SOURCE_GROUP("Resources" FILES ${CLIENT_CONFIG_RCS})
SOURCE_GROUP("Forms" FILES ${CLIENT_CONFIG_UIS})

ADD_EXECUTABLE(ryzom_configuration_qt WIN32)

target_sources(ryzom_configuration_qt
        PRIVATE
        resources/resources.qrc
        resources/client_config_qt.rc
        ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
        ${CLIENT_CONFIG_QM}
        ${CLIENT_CONFIG_UIS}
        src/client_config_dialog.cpp
        src/config.cpp
        src/display_settings_advanced_widget.cpp
        src/display_settings_details_widget.cpp
        src/display_settings_widget.cpp
        src/general_settings_widget.cpp
        src/main.cpp
        src/sound_settings_widget.cpp
        src/stdpch.cpp
        src/sys_info_d3d_widget.cpp
        src/sys_info_opengl_widget.cpp
        src/sys_info_widget.cpp
        src/system.cpp
        src/widget_base.h
)

TARGET_INCLUDE_DIRECTORIES(ryzom_configuration_qt
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET ryzom_configuration_qt PROPERTY AUTOUIC_SEARCH_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

IF (WITH_RYZOM_STEAM)
    TARGET_COMPILE_DEFINITIONS(ryzom_configuration_qt PRIVATE RZ_USE_STEAM)
ENDIF ()

NL_DEFAULT_PROPS(ryzom_configuration_qt "Ryzom, Tools: Ryzom Configuration Qt")
NL_ADD_RUNTIME_FLAGS(ryzom_configuration_qt)
NL_ADD_LIB_SUFFIX(ryzom_configuration_qt)
TARGET_LINK_LIBRARIES(ryzom_configuration_qt
        PRIVATE
        NeL::misc
        NeL::3d
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::Widgets
        OpenGL::GL
)

IF (WITH_PCH)
    target_precompile_headers(ryzom_configuration_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_configuration_qt RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client)
