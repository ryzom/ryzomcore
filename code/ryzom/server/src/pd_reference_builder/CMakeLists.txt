FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(reference_builder_service ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(reference_builder_service	
						ryzom_gameshare
						pd_lib
						nelmisc 
						nelnet
						nelgeorges
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						)
						
						
NL_DEFAULT_PROPS(reference_builder_service "Ryzom, Services: Reference Builder Service (RBS)")
NL_ADD_RUNTIME_FLAGS(reference_builder_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS reference_builder_service RUNTIME DESTINATION sbin COMPONENT services)


