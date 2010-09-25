#-----------------------------------------------------------------------------^M
# Set CMake 2.6 Policies.
IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)


#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_PREFIX_PATHS()

#-----------------------------------------------------------------------------
# Override default options
OPTION(BUILD_CLIENT "Build the Snowballs Client" ON)
OPTION(ENABLE_SOUND "Enable sound in the Snowballs Client" OFF)

OPTION(BUILD_SERVER "Build the Snowballs Servers" ON)
SET(SNOWBALLS_CONFIG_FILE "${NL_ETC_PREFIX}/snowballs" CACHE FILEPATH "Snowballs config file location")
SET(SNOWBALLS_DATA_FILE "${NL_SHARE_PREFIX}/games/snowballs" CACHE FILEPATH "Snowballs data file location")
SET(SNOWBALLS_LOG_FILE "${NL_LOG_PREFIX}/snowballs" CACHE FILEPATH "Snowballs log file location")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${SNOWBALLS_LOG_FILE})")

#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

NL_SETUP_BUILD()

FIND_PACKAGE(NeL COMPONENTS nelmisc nelgeorges nel3d nelnet nelpacs REQUIRED)
FIND_PACKAGE(NeLNS COMPONENTS naming_service login_service welcome_service nel_launcher_qt)

###
# An example of finding NeL 3D and CEGUI Renderer.
###
IF(BUILD_CLIENT)
  FIND_PACKAGE(FreeType)
  FIND_PACKAGE(Jpeg)
  
  IF(ENABLE_SOUND)
    FIND_PACKAGE(NeL COMPONENTS nelsound)
  ENDIF(ENABLE_SOUND)
ENDIF(BUILD_CLIENT)

IF(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake)
  SET(WINSOCK2_LIB ws2_32.lib)
ENDIF(WIN32)

NL_SETUP_BUILD_FLAGS()

ADD_DEFINITIONS(-DSNOWBALLS_CONFIG="${SNOWBALLS_CONFIG_FILE}/" -DSNOWBALLS_STATE="${SNOWBALLS_LOG_FILE}/" -DSNOWBALLS_LOGS="${SNOWBALLS_LOG_FILE}/")

IF(BUILD_CLIENT)
  ADD_SUBDIRECTORY(client)
ENDIF(BUILD_CLIENT)

IF(BUILD_SERVER)
  ADD_SUBDIRECTORY(server)
ENDIF(BUILD_SERVER)

# packaging information
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NeL MMORPG Framework - Snowballs Demo")
SET(CPACK_PACKAGE_VENDOR "NeL")
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
SET(CPACK_PACKAGE_VERSION_MAJOR "${SB_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SB_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SB_VERSION_PATCH}")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};Snowballs;ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES "snowballs-${SB_VERSION}" "snowballs")

# NSIS Specific Packing Setup
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Snowballs")
SET(CPACK_NSIS_MODIFY_PATH "ON")
SET(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/resources/nevraxpill.ico)
SET(CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/resources/nevraxpill.ico)
SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/resources\\\\nel.bmp)
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Snowballs")
SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\dev.ryzom.com\\\\projects\\\\nel")
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\dev.ryzom.com\\\\projects\\\\nel")
SET(CPACK_NSIS_CONTACT "matt.raykowski@gmail.com")  

## Source Packages
SET(CPACK_PACKAGE_FILE_NAME "snowballs-${SB_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "snowballs-${SB_VERSION}")
IF(WIN32)
  SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSE(WIN32)
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_SOURCE_GENERATOR "TGZ")
ENDIF(WIN32)
set(CPACK_SOURCE_IGNORE_FILES
        "~$"
        "\\\\.cvsignore$"
        "^${CMAKE_SOURCE_DIR}.*/CVS/"
        "^${CMAKE_SOURCE_DIR}.*/\\\\.svn/"
        "^${CMAKE_SOURCE_DIR}/debian/"
        "^${CMAKE_SOURCE_DIR}/old/")
IF(WIN32)
  IF(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    # Install "Debug" specific stuff here.
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    message(status "install debug libraries.")
  ELSE(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    # Install "Release" Specific stuff here.
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
  ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  
  IF(BUILD_CLIENT)
	# Needed to find dependencies for nel_launcher_qt.
	FIND_PACKAGE(Qt4 REQUIRED)
	
    # Install Qt dependencies for the launcher.
    INCLUDE(${QT_USE_FILE})
    INSTALL(FILES 
        "${QT_LIBRARY_DIR}/QtGuid4.dll"        
        "${QT_LIBRARY_DIR}/QtXmld4.dll"
        "${QT_LIBRARY_DIR}/QtCored4.dll"
        DESTINATION bin)
        
     # Install the Qt Launcher.
     INSTALL(FILES ${NELNS_NEL_LAUNCHER_BIN} DESTINATION bin COMPONENT snowballsclient)
     
     # Install the sound/video drivers
     INSTALL(FILES ${NELDRVOPENGL_DLL} ${NELDRVDIRECT3D_DLL} DESTINATION bin COMPONENT snowballsclient)
  ENDIF(BUILD_CLIENT)
  
  IF(BUILD_SERVER)
    INSTALL(FILES 
		${NELNS_NAMING_SERVICE_BIN} 
		${NELNS_NAMING_SERVICE_CFG} 
		${NELNS_LOGIN_SERVICE_BIN} 
		${NELNS_LOGIN_SERVICE_CFG}
		${NELNS_WELCOME_SERVICE_BIN} 
		${NELNS_WELCOME_SERVICE_CFG}
		DESTINATION sbin 
		COMPONENT services)
  ENDIF(BUILD_SERVER)
  
  INCLUDE(InstallRequiredSystemLibraries)
ENDIF(WIN32)

INCLUDE(CPack)

cpack_add_install_type(Full DISPLAY_NAME "Full Install")
cpack_add_component_group(Client 
  EXPANDED 
  DESCRPTION "Snowballs Client")
cpack_add_component_group(Server 
  EXPANDED 
  DESCRPTION "Snowballs Services")
cpack_add_component(snowballsclient
  DISPLAY_NAME "Snowballs Client Runtime"
  DESCRIPTION "Hello World Application"
  GROUP Application
  INSTALL_TYPES Full)
cpack_add_component(Unspecified
  DISPLAY_NAME "Application Runtime Libraries"
  DESCRIPTION "Hello World Application Libraries"
  GROUP Client
  INSTALL_TYPES Full)
cpack_add_component(snowballsdata
  DISPLAY_NAME "Snowballs Application Data"
  DESCRIPTION "Snowballs Application Data"
  GROUP Client
  INSTALL_TYPES Full) 
cpack_add_component(services
  DISPLAY_NAME "Snowballs Services"
  DESCRIPTION "Snowballs Services"
  GROUP Server
  INSTALL_TYPES Full) 