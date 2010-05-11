ADD_SUBDIRECTORY(seven_zip)

# These are Windows/MFC apps
IF(WIN32)
  ADD_SUBDIRECTORY(bug_report)
ENDIF(WIN32)

ADD_SUBDIRECTORY(client_sheets)

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
IF(UNIX)
  INCLUDE(${CMAKE_BINARY_DIR})
ENDIF(UNIX)

ADD_EXECUTABLE(client ${SRC})

INCLUDE_DIRECTORIES(	${LIBXML2_INCLUDE_DIR} 
			${NEL_INCLUDE_DIR} 
			${LUA_INCLUDE_DIR} 
			${LIBWWW_INCLUDE_DIR} 
			${CURL_INCLUDE_DIRS} 
			${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(client 	${PLATFORM_LINKFLAGS} 
				${LIBXML2_LIBRARIES} 
				${NELMISC_LIBRARY} 
				game_share 
				${NELNET_LIBRARY} 
				${NELLIGO_LIBRARY} 
				${NELGEORGES_LIBRARY} 
				${LUA_LIBRARIES} 
				${CURL_LIBRARIES} 
				${NELSOUND_LIBRARY} 
				${NELSNDDRV_LIBRARY}
				client_sheets 
				${NELPACS_LIBRARY} 
				${LIBWWW_LIBRARY} 
				${Boost_LIBRARIES} 
				${X11_LIBRARIES}
				seven_zip
				luabind)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS client RUNTIME DESTINATION bin COMPONENT client)
