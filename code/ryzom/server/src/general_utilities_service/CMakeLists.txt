FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/Backup\ 1\ of\ stat_job_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/ec_event_chat.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_popups.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_popups.h
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_select_with_text.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gus_mfc_select_with_text.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_repository_emitter.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/rr_repository_reciever.cpp)

ADD_EXECUTABLE(general_utilities_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR}  ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(general_utilities_service	admin_modules 
						ryzom_gameshare 
						server_share
						ai_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${ZLIB_LIBRARIES} 
						${NELMISC_LIBRARY} 
						${NELNET_LIBRARY} 
						${NELGEORGES_LIBRARY} 
						${NELLIGO_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(general_utilities_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: General Utilities Service (GUS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS general_utilities_service RUNTIME DESTINATION sbin COMPONENT services)


