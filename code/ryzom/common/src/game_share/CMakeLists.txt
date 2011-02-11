FILE(GLOB SRC *.cpp time_weather_season/*.cpp)
FILE(GLOB PRIV_H *.h time_weather_season/*.h)

SOURCE_GROUP(headers FILES ${PRIV_H})

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.cpp)
LIST(REMOVE_ITEM PRIV_H ${CMAKE_CURRENT_SOURCE_DIR}/enum_template.h)

NL_TARGET_LIB(ryzom_gameshare ${PRIV_H} ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ryzom_gameshare nelmisc nelnet nelligo nelgeorges ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
NL_DEFAULT_PROPS(ryzom_gameshare "Ryzom, Library: Game Share")
NL_ADD_RUNTIME_FLAGS(ryzom_gameshare)
NL_ADD_LIB_SUFFIX(ryzom_gameshare)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_gameshare ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_gameshare LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)

