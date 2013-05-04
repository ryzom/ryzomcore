QT += core
QT += gui
QT += opengl

INCLUDEPATH += E:/projects/ryzom/bin/include

win32:LIBS += E:/projects/ryzom/bin/lib/nel3d_d.lib
win32:LIBS += E:/projects/ryzom/bin/lib/nelmisc_d.lib
win32:LIBS += E:/projects/ryzom/src/external_stlport/lib/libjpeg.lib
win32:LIBS += E:/projects/ryzom/src/external_stlport/lib/libpng.lib
win32:LIBS += user32.lib
win32:LIBS += gdi32.lib
win32:LIBS += advapi32.lib
win32:LIBS += shell32.lib
win32:LIBS += ole32.lib
win32:LIBS += opengl32.lib

SOURCES += \
    client_config_dialog.cpp \
    config.cpp \
    display_settings_advanced_widget.cpp \
    display_settings_details_widget.cpp \
    display_settings_widget.cpp \
    general_settings_widget.cpp \
    main.cpp \
    sound_settings_widget.cpp \
    system.cpp \
    sys_info_d3d_widget.cpp \
    sys_info_opengl_widget.cpp \
    sys_info_widget.cpp \


FORMS += \
    client_config_dialog.ui \
    display_settings_advanced_widget.ui \
    display_settings_details_widget.ui \
    display_settings_widget.ui \
    general_settings_widget.ui \
    sound_settings_widget.ui \
    sys_info_d3d_widget.ui \
    sys_info_opengl_widget.ui \
    sys_info_widget.ui \

RESOURCES += \
    resources.qrc

HEADERS += \
    client_config_dialog.h \
    config.h \
    display_settings_advanced_widget.h \
    display_settings_details_widget.h \
    display_settings_widget.h \
    general_settings_widget.h \
    sound_settings_widget.h \
    system.h \
    sys_info_d3d_widget.h \
    sys_info_opengl_widget.h \
    sys_info_widget.h \
    widget_base.h \

TRANSLATIONS = \
    ryzom_configuration_en.ts \
    ryzom_configuration_hu.ts

