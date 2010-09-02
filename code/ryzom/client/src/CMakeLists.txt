# These are Windows/MFC apps
IF(WIN32)
#  ADD_SUBDIRECTORY(bug_report)
  SET(SEVENZIP_LIBRARY "ryzom_sevenzip")
ENDIF(WIN32)

ADD_SUBDIRECTORY(client_sheets)
ADD_SUBDIRECTORY(seven_zip)

FILE(GLOB SRC *.cpp *.h motion/*.cpp motion/*.h motion/modes/*.cpp motion/modes/*.h r2/*.h r2/*.cpp r2/dmc/*.h r2/dmc/*.cpp interface_v3/*.h interface_v3/*.cpp)

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC
	${CMAKE_CURRENT_SOURCE_DIR}/animated_scene_object.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/animated_scene_object.h
	${CMAKE_CURRENT_SOURCE_DIR}/animation_fx_sheet.h
	${CMAKE_CURRENT_SOURCE_DIR}/animation_fx_sheet.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/3d_notes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/3d_notes.h
	${CMAKE_CURRENT_SOURCE_DIR}/rosace_functions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rosace_functions.h
	${CMAKE_CURRENT_SOURCE_DIR}/scene_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/scene_parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/motion/modes/free_head.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/motion/modes/free_head.h
	${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/lua_dll.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/lua_dll.h
	${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/view_bitmap_progress.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/view_bitmap_progress.h)

# On Unix we'll need to include config.h
#IF(UNIX)
#  INCLUDE(${CMAKE_BINARY_DIR})
#ENDIF(UNIX)

# on Mac, create a .App Bundle
if(APPLE)
  SET(MACOSX_BUNDLE_INFO_STRING "Ryzom Core Client")
  SET(MACOSX_BUNDLE_ICON_FILE "")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "")
  SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${NL_VERSION})
  SET(MACOSX_BUNDLE_BUNDLE_NAME "Ryzom Core Client")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.8")
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
  SET(MACOSX_BUNDLE_COPYRIGHT "Winchgate and The Ryzom Core Community")

  ADD_EXECUTABLE(ryzom_client MACOSX_BUNDLE ${SRC})

  # TODO: in release mode, cmake could copy all the dylibs into the .app
  # bundle for redistribution... should some part of cpack handle that?
  # ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD
  #   # make frameworks directory in app bundle
  #   COMMAND ${CMAKE_COMMAND} -E make_directory
  #     ${CMAKE_CURRENT_BINARY_DIR}/ryzom_client.app/Contents/Frameworks
  #   # copy framework into app bundle
  #   COMMAND ${CMAKE_COMMAND} -E copy ${SOME_LIBRARY}
  #     ${CMAKE_CURRENT_BINARY_DIR}/ryzom_client.app/Contents/Frameworks
  #   # ...
  #   # install_name_tool the lib pathes

ELSE(APPLE)
  ADD_EXECUTABLE(ryzom_client ${SRC})
ENDIF(APPLE)

INCLUDE_DIRECTORIES(
			${LIBXML2_INCLUDE_DIR}
			${LUA_INCLUDE_DIR}
			${LIBWWW_INCLUDE_DIR}
			${CURL_INCLUDE_DIRS}
			${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_client 	${PLATFORM_LINKFLAGS}
				nelmisc
				nelnet
				nelligo
				nelgeorges
				nel3d
				nelsound
				nelsnd_lowlevel
				ryzom_clientsheets
				ryzom_gameshare
				nelpacs
				${LUA_LIBRARIES}
				${CURL_LIBRARIES}
				${LIBWWW_LIBRARY}
				${SEVENZIP_LIBRARY}
				${LIBXML2_LIBRARIES}
				luabind # TODO: find luabind and expat cleanly using a find script
				expat)

IF(NOT APPLE AND NOT WIN32)
	TARGET_LINK_LIBRARIES(ryzom_client ${X11_LIBRARIES})
ENDIF(NOT APPLE AND NOT WIN32)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

NL_DEFAULT_PROPS(ryzom_client "Ryzom, Client: Ryzom Core Client")
NL_ADD_RUNTIME_FLAGS(ryzom_client)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_client ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_client RUNTIME DESTINATION games COMPONENT client BUNDLE DESTINATION /Applications)
