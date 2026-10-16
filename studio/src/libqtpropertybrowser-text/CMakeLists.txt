FIND_PACKAGE(QtPropertyBrowser REQUIRED)

set(CMAKE_AUTOMOC ON)

set(SRC
		libqtpropertybrowser-text/QtTextEditorFactory.cpp
		libqtpropertybrowser-text/QtTextEditWidget.cpp
		libqtpropertybrowser-text/QtTextPropertyManager.cpp
)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(QtPropertyBrowserText ${SRC})

ADD_LIBRARY(QtPropertyBrowser::Text ALIAS QtPropertyBrowserText)

TARGET_LINK_LIBRARIES(QtPropertyBrowserText ${QT_LIBRARIES} ${QtPropertyBrowser_LIBRARY})

TARGET_COMPILE_DEFINITIONS(QtPropertyBrowserText PRIVATE cxx_std_17)

INCLUDE_DIRECTORIES(
		${CMAKE_CURRENT_SOURCE_DIR}
		${QT_INCLUDES}
		${QtPropertyBrowser_INCLUDE_DIR}
)

TARGET_INCLUDE_DIRECTORIES(QtPropertyBrowserText
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
