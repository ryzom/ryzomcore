FILE(GLOB SRC *.cpp *.h)

NL_TARGET_LIB(ryzom_servershare ${SRC})

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

INSTALL(TARGETS ryzom_servershare LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
