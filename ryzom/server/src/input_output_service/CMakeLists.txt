FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log_inline.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ios_pd_inline.h)

# Part of the ios sheets library (Ryzom::iossheets), compiled once and shared
# with sheets_packer_shard.
LIST(REMOVE_ITEM SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/string_manager_sheet.cpp)

FIND_PATH(BOOST_UUID_INCLUDE_DIR boost/uuid/uuid_generators.hpp
	HINTS ${Boost_INCLUDE_DIR} ${BOOST_INCLUDEDIR} ${EXTERNAL_PATH}/include)
IF(NOT BOOST_UUID_INCLUDE_DIR)
	MESSAGE(FATAL_ERROR "Boost UUID headers are required by ryzom_ios_service")
ENDIF()

ADD_EXECUTABLE(ryzom_ios_service WIN32 ${SRC})
TARGET_INCLUDE_DIRECTORIES(ryzom_ios_service PRIVATE ${BOOST_UUID_INCLUDE_DIR})

TARGET_INCLUDE_DIRECTORIES(ryzom_ios_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

# chat_manager uses boost::uuid (header only)
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(ryzom_ios_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::iossheets
        Ryzom::pd
        Ryzom::gameshare
        Ryzom::servershare
        NeL::misc
        NeL::net
        NeL::pacs
        NeL::georges
        NeL::ligo
)

IF(WITH_MONGODB)
  TARGET_LINK_LIBRARIES(ryzom_ios_service PRIVATE ${MONGO_LIBRARIES})
  ADD_DEFINITIONS(${MONGO_DEFINITIONS})
ENDIF()

IF(WITH_MEMCACHED)
  TARGET_LINK_LIBRARIES(ryzom_ios_service PRIVATE ${MEMCACHED_LIBRARIES})
  ADD_DEFINITIONS(${MEMCACHED_DEFINITIONS})
ENDIF()

NL_DEFAULT_PROPS(ryzom_ios_service "Ryzom, Services: Input Output Service (IOS)")
NL_ADD_RUNTIME_FLAGS(ryzom_ios_service)

IF (WITH_PCH)
    target_precompile_headers(ryzom_ios_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_ios_service RUNTIME DESTINATION sbin COMPONENT services)


