FIND_PACKAGE(QtPropertyBrowser REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

FILE(GLOB SRC libqtpropertybrowser/*.cpp)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(qtpropertybrowser ${SRC})

ADD_LIBRARY(qtpropertybrowser::qtpropertybrowser ALIAS qtpropertybrowser)

TARGET_LINK_LIBRARIES(qtpropertybrowser ${QT_LIBRARIES} ${QtPropertyBrowser_LIBRARY})

INCLUDE_DIRECTORIES(
		${CMAKE_CURRENT_SOURCE_DIR}
		${QT_INCLUDES}
		${QtPropertyBrowser_INCLUDE_DIR}
)

TARGET_INCLUDE_DIRECTORIES(qtpropertybrowser
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
