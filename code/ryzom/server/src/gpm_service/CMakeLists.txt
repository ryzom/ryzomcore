FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_gpm_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_gpm_service
					ryzom_adminmodules
					ryzom_gameshare
					ryzom_servershare
					${LIBXML2_LIBRARIES}
					nelmisc 
					nelnet
					nelpacs
					nelgeorges
					nelligo)

NL_DEFAULT_PROPS(ryzom_gpm_service "Ryzom, Services: Global Position Manager Service (GPMS)")
NL_ADD_RUNTIME_FLAGS(ryzom_gpm_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_gpm_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_gpm_service RUNTIME DESTINATION sbin COMPONENT services)
