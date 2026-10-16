cmake_minimum_required(VERSION 3.28)

project(Ryzom VERSION ${RyzomCore_VERSION} LANGUAGES CXX)

find_package(NeL ${RyzomCore_VERSION} REQUIRED COMPONENTS misc net ligo georges 3d gui sound sevenzip pacs)

#-----------------------------------------------------------------------------
#Platform specifics

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)

SET(RZ_SERVER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src)

if(WITH_RYZOM_XSLT)
  find_package(LibXslt REQUIRED COMPONENTS xsltproc)
  include(../cmake/add_module_interface.cmake)
endif()

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(client)

IF(WITH_RYZOM_SERVER OR WITH_RYZOM_TOOLS)
  # Need servershare for build packed collision tool
  # Need aishare for build wmap tool
  FIND_PACKAGE(MySQL)
  IF(MYSQL_FOUND)
    ADD_SUBDIRECTORY(server)
  ELSE ()
    message(WARNING Server dependencies are not build due to missing MySQL package)
  ENDIF()
ENDIF()

if (WITH_TESTING)
  add_subdirectory(tools/server/test_modules)
endif ()
