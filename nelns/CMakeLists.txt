cmake_minimum_required(VERSION 3.28)

project(NeLNS VERSION ${RyzomCore_VERSION} LANGUAGES CXX)

find_package(NeL ${RyzomCore_VERSION} REQUIRED COMPONENTS misc net)

FIND_PACKAGE(MySQL)

IF(WITH_NELNS_SERVER)
	ADD_SUBDIRECTORY(admin_executor_service)
	ADD_SUBDIRECTORY(admin_service)
	ADD_SUBDIRECTORY(naming_service)
	ADD_SUBDIRECTORY(login_service)
	ADD_SUBDIRECTORY(welcome_service)
ENDIF()

IF(WITH_NELNS_LOGIN_SYSTEM)
	ADD_SUBDIRECTORY(login_system)
ENDIF()
