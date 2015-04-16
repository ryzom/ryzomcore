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

IF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   SET(MYSQL_FOUND TRUE)

ELSE()

  FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
      PATH_SUFFIXES mysql
      /usr/include/mysql
      /usr/local/include/mysql
      /opt/local/include/mysql5/mysql
      $ENV{ProgramFiles}/MySQL/*/include
      $ENV{SystemDrive}/MySQL/*/include)

  IF(WIN32 AND MSVC)
    FIND_LIBRARY(MYSQL_LIBRARY_RELEASE NAMES libmysql mysqlclient
      PATHS
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib/opt)

    FIND_LIBRARY(MYSQL_LIBRARY_DEBUG NAMES libmysqld mysqlclientd
      PATHS
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib/opt)
  ELSE()
    FIND_LIBRARY(MYSQL_LIBRARY_RELEASE NAMES mysqlclient
      PATHS
      /usr/lib
      /usr/local/lib
      /usr/lib/mysql
      /usr/local/lib/mysql
      /opt/local/lib/mysql5/mysql
      )

    FIND_LIBRARY(MYSQL_LIBRARY_DEBUG NAMES mysqlclientd
      PATHS
      /usr/lib
      /usr/local/lib
      /usr/lib/mysql
      /usr/local/lib/mysql
      /opt/local/lib/mysql5/mysql
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
        SET(MYSQL_LIBRARIES ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES})
      ENDIF()
    ENDIF()
  ENDIF()

  IF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    SET(MYSQL_FOUND TRUE)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
  ELSE()
    SET(MYSQL_FOUND FALSE)
    MESSAGE(STATUS "MySQL not found.")
  ENDIF()

  MARK_AS_ADVANCED(MYSQL_LIBRARY_RELEASE MYSQL_LIBRARY_DEBUG)

ENDIF()
