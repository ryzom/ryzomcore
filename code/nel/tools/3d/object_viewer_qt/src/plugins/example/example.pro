TEMPLATE = lib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += plugin
# Input
HEADERS += plugin1.h \
           ../../extension_system/iplugin.h \
           ../../extension_system/iplugin_manager.h \
           ../../extension_system/plugin_spec.h
SOURCES += plugin1.cpp \
           ../../extension_system/plugin_spec.cpp
