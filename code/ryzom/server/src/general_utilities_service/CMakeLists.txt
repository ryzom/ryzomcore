FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
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

ADD_EXECUTABLE(ryzom_general_utilities_service ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_general_utilities_service
						ryzom_adminmodules
						ryzom_gameshare 
						ryzom_servershare
						ryzom_aishare
						${LIBXML2_LIBRARIES} 
						${ZLIB_LIBRARIES} 
						nelmisc
						nelnet 
						nelgeorges} 
						nelligo)

NL_DEFAULT_PROPS(ryzom_general_utilities_service "Ryzom, Services: General Utilities Service (GUS)")
NL_ADD_RUNTIME_FLAGS(ryzom_general_utilities_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_general_utilities_service RUNTIME DESTINATION sbin COMPONENT services)
