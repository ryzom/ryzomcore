#-----------------------------------------------------------------------------
# Override default options
OPTION(BUILD_SERVICES		"Build NeLNS Services"				ON )
OPTION(BUILD_LOGIN_SYSTEM	"Build NeLNS Login System Tools"	ON )
OPTION(BUILD_RYZOM_LAUNCHER	"Build Ryzom Launcher"				OFF)

#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(MySQL)
FIND_PACKAGE(CURL)

IF(WITH_QT)
  FIND_PACKAGE(Qt4 REQUIRED)
ENDIF(WITH_QT)

SET(ETCDIR "${NL_ETC_PREFIX}/nelns")
SET(PKGDIR "${NL_SHARE_PREFIX}/nelns")
SET(LOGDIR "${NL_LOG_PREFIX}/nelns")
ADD_DEFINITIONS(-DNELNS_CONFIG="\\"${ETCDIR}\\"" -DNELNS_STATE="\\"${PKGDIR}\\"" -DNELNS_LOGS="\\"${LOGDIR}\\"")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${LOGDIR})")

IF(BUILD_SERVICES)
	ADD_SUBDIRECTORY(admin_executor_service)
	ADD_SUBDIRECTORY(admin_service)
	ADD_SUBDIRECTORY(naming_service)
	ADD_SUBDIRECTORY(login_service)
	ADD_SUBDIRECTORY(welcome_service)
ENDIF(BUILD_SERVICES)

IF(BUILD_LOGIN_SYSTEM)
	ADD_SUBDIRECTORY(login_system)
ENDIF(BUILD_LOGIN_SYSTEM)
