TEMPLATE = lib
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += plugin
# Input
HEADERS += ovqt_sheet_builder.h \
           ../../extension_system/iplugin.h \
           ../../extension_system/iplugin_manager.h \
           ../../extension_system/plugin_spec.h \
    sheetbuilderdialog.h \
    sheetbuilder.h \
    sheetbuilderconfgdialog.h
SOURCES += ovqt_sheet_builder.cpp \
           ../../extension_system/plugin_spec.cpp \
    sheetbuilderdialog.cpp \
    sheetbuilderconfgdialog.cpp
