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

# The headless Ring scenario chain: a console tool that runs the editor's Lua on scenario files, and tests
# that run it on scenarios of the tree. It is part of the test tree and runs the scripts on the Lua the
# interface is built with, so it needs both; asking for it without them is an error, not a silent skip.
option(WITH_RYZOM_R2_SCENARIO_CHAIN "Build the headless Ring scenario chain tool and its tests" OFF)
if (WITH_RYZOM_R2_SCENARIO_CHAIN)
  if (NOT WITH_TESTING)
    message(FATAL_ERROR "WITH_RYZOM_R2_SCENARIO_CHAIN needs WITH_TESTING: the tool is built with the tests")
  endif ()
  if (NOT TARGET Lua::Lua)
    message(FATAL_ERROR "WITH_RYZOM_R2_SCENARIO_CHAIN needs Lua, which is only looked for with WITH_GUI")
  endif ()
  if (NOT (WITH_LUA53 OR WITH_LUA54 OR WITH_LUA55))
    message(FATAL_ERROR "WITH_RYZOM_R2_SCENARIO_CHAIN needs Lua 5.3 or later: WITH_LUA53, WITH_LUA54 or WITH_LUA55")
  endif ()
  if (NOT TARGET Ryzom::gameshare)
    message(FATAL_ERROR "WITH_RYZOM_R2_SCENARIO_CHAIN needs the game_share library (ryzom/common)")
  endif ()
endif ()

if (WITH_TESTING)
  add_subdirectory(tools/server/test_modules)
endif ()
