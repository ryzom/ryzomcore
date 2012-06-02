FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/re_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/spt_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/spt_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/spm_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/spm_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/spa_module_itf.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/spa_module_itf.h
			${CMAKE_CURRENT_SOURCE_DIR}/re_repository_emitter.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/re_repository_emitter.h
			${CMAKE_CURRENT_SOURCE_DIR}/rr_repository_reciever.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/rr_repository_reciever.h
			${CMAKE_CURRENT_SOURCE_DIR}/stdin_monitor_thread.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/stdin_monitor_thread.h)

ADD_EXECUTABLE(ryzom_patchman_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_patchman_service
					ryzom_adminmodules
					ryzom_gameshare 
					ryzom_servershare
					${LIBXML2_LIBRARIES} 
					${MYSQL_LIBRARIES} 
					${ZLIB_LIBRARIES}
					nelmisc
					nelnet
					nelpacs
					nelgeorges
					nelligo)

NL_DEFAULT_PROPS(ryzom_patchman_service "Ryzom, Services: Patch Manager Service (PMS)")
NL_ADD_RUNTIME_FLAGS(ryzom_patchman_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_patchman_service RUNTIME DESTINATION sbin COMPONENT services)
