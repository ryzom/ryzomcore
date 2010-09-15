FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ai_build_wmap ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ai_build_wmap
					ryzom_gameshare
					ryzom_servershare
					ryzom_aishare
					${PLATFORM_LINKFLAGS} 
					${LIBXML2_LIBRARIES} 
					nelmisc)

NL_DEFAULT_PROPS(ai_build_wmap "Ryzom, Services: Entity View Service (EVS)")
NL_ADD_RUNTIME_FLAGS(ai_build_wmap)
					
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ai_build_wmap RUNTIME DESTINATION bin COMPONENT tools)
