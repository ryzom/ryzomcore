# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
find_package(PkgConfig QUIET)
pkg_check_modules(PC_MYSQL QUIET libmysql)
pkg_check_modules(PC_MARIADB QUIET libmariadb)
IF(APPLE)
  # MySQL's own module name, which Homebrew's mysql links into its pkg-config path; the Linux
  # packages, which ship it too, are found by the search below as before
  pkg_check_modules(PC_MYSQLCLIENT QUIET mysqlclient)
ENDIF()

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   SET(MYSQL_FOUND TRUE)
elseif (PC_MYSQL_FOUND)
  SET(MYSQL_FOUND TRUE)
  set(MYSQL_INCLUDE_DIR ${PC_MYSQL_INCLUDE_DIRS})
  SET(MYSQL_LIBRARIES ${PC_MYSQL_LINK_LIBRARIES})
elseif (PC_MARIADB_FOUND)
  SET(MYSQL_FOUND TRUE)
  set(MYSQL_INCLUDE_DIR ${PC_MARIADB_INCLUDE_DIRS})
  SET(MYSQL_LIBRARIES ${PC_MARIADB_LINK_LIBRARIES})
elseif (PC_MYSQLCLIENT_FOUND)
  SET(MYSQL_FOUND TRUE)
  set(MYSQL_INCLUDE_DIR ${PC_MYSQLCLIENT_INCLUDE_DIRS})
  SET(MYSQL_LIBRARIES ${PC_MYSQLCLIENT_LINK_LIBRARIES})
else()

  FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
      PATH_SUFFIXES mysql mariadb
      PATHS
      /usr/include/mysql
      /usr/include/mariadb
      /usr/local/include/mysql
      /usr/local/include/mariadb
      /opt/local/include/mysql5/mysql
      /opt/local/include/mysql55/mysql
      /opt/local/include/mysql51/mysql
      /usr/local/mysql/include
      /opt/homebrew/opt/mysql-client/include
      /usr/local/opt/mysql-client/include
      $ENV{ProgramFiles}/MySQL/*/include
      $ENV{SystemDrive}/MySQL/*/include)

  IF(WIN32 AND MSVC)
    FIND_LIBRARY(MYSQL_LIBRARY_RELEASE NAMES libmysql mysqlclient libmariadb mariadbclient
      PATHS
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib/opt)
    FIND_LIBRARY(MYSQL_LIBRARY_DEBUG NAMES libmysqld mysqlclientd libmariadb mariadbclient
      PATHS
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib/opt)
  ELSE()
    FIND_LIBRARY(MYSQL_LIBRARY_RELEASE NAMES mysqlclient mariadbclient
      PATHS
      /usr/lib
      /usr/local/lib
      /usr/lib/mariadb
      /usr/lib/mysql
      /usr/local/lib/mysql
      /usr/local/lib/mariadb
      /opt/local/lib/mysql5/mysql
      /opt/local/lib/mysql55/mysql
      /opt/local/lib/mysql51/mysql
      /usr/local/mysql/lib
      /opt/homebrew/opt/mysql-client/lib
      /usr/local/opt/mysql-client/lib
      )

    FIND_LIBRARY(MYSQL_LIBRARY_DEBUG NAMES mysqlclientd mariadbclientd
      PATHS
      /usr/lib
      /usr/local/lib
      /usr/lib/mysql
      /usr/local/lib/mysql
      /opt/local/lib/mysql5/mysql
      /opt/local/lib/mysql55/mysql
      /opt/local/lib/mysql51/mysql
      /usr/local/mysql/lib
      /opt/homebrew/opt/mysql-client/lib
      /usr/local/opt/mysql-client/lib
      )
  ENDIF()

  IF(MYSQL_INCLUDE_DIR)
    IF(MYSQL_LIBRARY_RELEASE)
      IF(MYSQL_LIBRARY_DEBUG)
        SET(MYSQL_LIBRARIES optimized ${MYSQL_LIBRARY_RELEASE} debug ${MYSQL_LIBRARY_DEBUG})
      ELSE()
	    SET(MYSQL_LIBRARIES ${MYSQL_LIBRARY_RELEASE})
      ENDIF()
      FIND_PACKAGE(OpenSSL)
      IF(OPENSSL_FOUND)
        IF(WIN32)
          SET(OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} Crypt32.lib)
        ENDIF()
        SET(MYSQL_LIBRARIES ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES})
      ENDIF()
    ENDIF()
  ENDIF()

  IF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    SET(MYSQL_FOUND TRUE)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
    IF (MYSQL_LIBRARIES MATCHES "libmariadb" OR MYSQL_LIBRARIES MATCHES "mariadbclient")
      SET(MARIADB_FOUND TRUE)
      MESSAGE(STATUS "Found MariaDB.")
    ENDIF()
  ELSE()
    SET(MYSQL_FOUND FALSE)
    MESSAGE(STATUS "MySQL not found.")
  ENDIF()

  MARK_AS_ADVANCED(MYSQL_LIBRARY_RELEASE MYSQL_LIBRARY_DEBUG)

ENDIF()

# alias target to get usage requirements automatically
if(MYSQL_FOUND AND NOT TARGET MySQL::MySQL)
    set(MySQL_FOUND TRUE)
    add_library(MySQL::MySQL INTERFACE IMPORTED)
    set_target_properties(
            MySQL::MySQL
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
    )
    target_link_libraries(MySQL::MySQL INTERFACE ${MYSQL_LIBRARIES})
endif()