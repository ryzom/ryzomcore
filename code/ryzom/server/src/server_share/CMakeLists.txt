FILE(GLOB SRC *.cpp *.h)

FILE(GLOB R2 
	r2_*.h r2_*.cpp)

LIST(REMOVE_ITEM SRC R2)

SOURCE_GROUP("" FILES ${SRC})
SOURCE_GROUP("R2" FILES ${R2})

NL_TARGET_LIB(ryzom_servershare ${SRC} ${R2})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_servershare
						nelmisc
						nelpacs
						nelgeorges
						nelnet
						ryzom_gameshare
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARIES})

NL_DEFAULT_PROPS(ryzom_servershare "Ryzom, Library: Service Shared")
NL_ADD_RUNTIME_FLAGS(ryzom_servershare)
NL_ADD_LIB_SUFFIX(ryzom_servershare)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_servershare ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_servershare LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
