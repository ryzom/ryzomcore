INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${NEL_INCLUDE_DIR} ${QT_INCLUDES})
INCLUDE( ${QT_USE_FILE} )

FILE( GLOB SRC *.cpp *.h )

SET( CLIENT_CONFIG_HDR
                      client_config_dialog.h
                      display_settings_advanced_widget.h
                      display_settings_details_widget.h
                      display_settings_widget.h
                      general_settings_widget.h
                      sound_settings_widget.h
                      sys_info_d3d_widget.h
                      sys_info_opengl_widget.h
                      sys_info_widget.h
                      widget_base.h
)

SET( CLIENT_CONFIG_UIS
                      client_config_dialog.ui
                      display_settings_advanced_widget.ui
                      display_settings_details_widget.ui
                      display_settings_widget.ui
                      general_settings_widget.ui
                      sound_settings_widget.ui
                      sys_info_d3d_widget.ui
                      sys_info_opengl_widget.ui
                      sys_info_widget.ui
)

SET( CLIENT_CONFIG_TRANS
                     translations/ryzom_configuration_en.ts
                     translations/ryzom_configuration_hu.ts
)

CONFIGURE_FILE( translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY )
SET( CLIENT_CONFIG_RCS resources.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc )
QT4_ADD_TRANSLATION( CLIENT_CONFIG_QM ${CLIENT_CONFIG_TRANS} )
QT4_ADD_RESOURCES( CLIENT_CONFIG_RC_SRCS ${CLIENT_CONFIG_RCS} )
QT4_WRAP_CPP( CLIENT_CONFIG_MOC_SRC ${CLIENT_CONFIG_HDR} )
QT4_WRAP_UI( CLIENT_CONFIG_UI_HDRS ${CLIENT_CONFIG_UIS} )
SOURCE_GROUP( "Resources" FILES ${CLIENT_CONFIG_RCS} )
SOURCE_GROUP( "Forms" FILES ${CLIENT_CONFIG_UIS} )
SOURCE_GROUP( "Generated Files" FILES ${CLIENT_CONFIG_UI_HDRS} ${CLIENT_CONFIG_MOC_SRC} )
SOURCE_GROUP( "Translation Files" FILES ${CLIENT_CONFIG_TRANS} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} )
ADD_EXECUTABLE( ryzom_configuration_qt WIN32 MACOSX_BUNDLE ${SRC} ${CLIENT_CONFIG_MOC_SRC} ${CLIENT_CONFIG_UI_HDRS} ${CLIENT_CONFIG_RC_SRCS} ${CLIENT_CONFIG_TRANS} )
NL_DEFAULT_PROPS( ryzom_configuration_qt "Ryzom, Tools: Ryzom Configuration Qt" )
NL_ADD_RUNTIME_FLAGS( ryzom_configuration_qt )
NL_ADD_LIB_SUFFIX( ryzom_configuration_qt )
TARGET_LINK_LIBRARIES( ryzom_configuration_qt nelmisc nel3d ${QT_LIBRARIES})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_configuration_qt ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_configuration_qt RUNTIME DESTINATION games COMPONENT client BUNDLE DESTINATION /Applications)

