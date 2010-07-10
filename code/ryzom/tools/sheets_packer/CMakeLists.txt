FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(sheets_packer	${SRC} 
				${CMAKE_SOURCE_DIR}/client/src/continent_manager_build.cpp
				${CMAKE_SOURCE_DIR}/client/src/continent_manager_build.h
				${CMAKE_SOURCE_DIR}/client/src/sheet_manager.cpp
				${CMAKE_SOURCE_DIR}/client/src/sheet_manager.h)

INCLUDE_DIRECTORIES(	${LIBXML2_INCLUDE_DIR} 
			${NEL_INCLUDE_DIR} 
			${CMAKE_SOURCE_DIR}/client/src)
TARGET_LINK_LIBRARIES(sheets_packer	ryzom_clientsheets
					ryzom_gameshare
					${NELMISC_LIBRARY}
					${NELGEORGES_LIBRARY}
					${NELNET_LIBRARY}
					${NELLIGO_LIBRARY}
					${PLATFORM_LINKFLAGS}
					${LIBXML2_LIBRARIES} )
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(sheets_packer ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS sheets_packer RUNTIME DESTINATION bin COMPONENT tools)
