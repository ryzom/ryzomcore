FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheets_packer	${SRC} 
				${CMAKE_SOURCE_DIR}/client/src/continent_manager_build.cpp
				${CMAKE_SOURCE_DIR}/client/src/continent_manager_build.h
				${CMAKE_SOURCE_DIR}/client/src/sheet_manager.cpp
				${CMAKE_SOURCE_DIR}/client/src/sheet_manager.h)

INCLUDE_DIRECTORIES(	${LIBXML2_INCLUDE_DIR} 
			${NEL_INCLUDE_DIR} 
			${CMAKE_SOURCE_DIR}/client/src)
TARGET_LINK_LIBRARIES(sheets_packer	client_sheets
					game_share
					${NELMISC_LIBRARY}
					${NELGEORGES_LIBRARY}
					${NELNET_LIBRARY}
					${NELLIGO_LIBRARY}
					${PLATFORM_LINKFLAGS}
					${LIBXML2_LIBRARIES} )
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS sheets_packer RUNTIME DESTINATION bin COMPONENT tools)
