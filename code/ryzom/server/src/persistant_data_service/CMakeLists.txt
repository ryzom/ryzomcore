FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(persistant_data_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(persistant_data_service	admin_modules 
						ryzom_gameshare
						pd_lib
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${NELMISC_LIBRARY} 
						${NELNET_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(persistant_data_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Persistent Data Service (PDS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS persistant_data_service RUNTIME DESTINATION sbin COMPONENT services)


