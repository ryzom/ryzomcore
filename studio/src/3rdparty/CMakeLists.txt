ADD_SUBDIRECTORY(qtpropertybrowser)

SET(QtPropertyBrowser_LIBRARY "QtPropertyBrowser")

target_include_directories(QtPropertyBrowser
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/qtpropertybrowser/include>
        $<INSTALL_INTERFACE:include>
)
