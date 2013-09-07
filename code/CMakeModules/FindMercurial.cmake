# - Extract information from a subversion working copy
# The module defines the following variables:
#  Mercurial_HG_EXECUTABLE - path to hg command line client
#  Mercurial_VERSION_HG - version of hg command line client
#  Mercurial_FOUND - true if the command line client was found
#  MERCURIAL_FOUND - same as Mercurial_FOUND, set for compatiblity reasons
#
# The minimum required version of Mercurial can be specified using the
# standard syntax, e.g. FIND_PACKAGE(Mercurial 1.4)
#
# If the command line client executable is found two macros are defined:
#  Mercurial_WC_INFO(<dir> <var-prefix>)
#  Mercurial_WC_LOG(<dir> <var-prefix>)
# Mercurial_WC_INFO extracts information of a subversion working copy at
# a given location. This macro defines the following variables:
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO - output of command `hg info <dir>'
# Mercurial_WC_LOG retrieves the log message of the base revision of a
# subversion working copy at a given location. This macro defines the
# variable:
#  <var-prefix>_LAST_CHANGED_LOG - last log of base revision
# Example usage:
#  FIND_PACKAGE(Mercurial)
#  IF(MERCURIAL_FOUND)
#    Mercurial_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#    Mercurial_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
#  ENDIF(MERCURIAL_FOUND)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Tristan Carel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PROGRAM(Mercurial_HG_EXECUTABLE hg
  DOC "mercurial command line client"
  HINTS /opt/local/bin)
MARK_AS_ADVANCED(Mercurial_HG_EXECUTABLE)

IF(Mercurial_HG_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${Mercurial_HG_EXECUTABLE} --version
    OUTPUT_VARIABLE Mercurial_VERSION_HG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
	
  STRING(REGEX REPLACE ".*version ([\\.0-9]+).*"
    "\\1" Mercurial_VERSION_HG "${Mercurial_VERSION_HG}")

  MACRO(Mercurial_WC_INFO dir prefix)
    EXECUTE_PROCESS(COMMAND ${Mercurial_HG_EXECUTABLE} tip --template "{rev};{node};{tags};{author}"
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE Mercurial_hg_info_error
      RESULT_VARIABLE Mercurial_hg_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Mercurial_hg_info_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Mercurial_HG_EXECUTABLE} tip\" failed with output:\n${Mercurial_hg_info_error}")
    ELSE(NOT ${Mercurial_hg_info_result} EQUAL 0)
      LIST(LENGTH ${prefix}_WC_INFO _COUNT)
      IF(_COUNT EQUAL 4)
        LIST(GET ${prefix}_WC_INFO 0 ${prefix}_WC_REVISION)
        LIST(GET ${prefix}_WC_INFO 1 ${prefix}_WC_CHANGESET)
        LIST(GET ${prefix}_WC_INFO 2 ${prefix}_WC_BRANCH)
        LIST(GET ${prefix}_WC_INFO 3 ${prefix}_WC_LAST_CHANGED_AUTHOR)
      ELSE(_COUNT EQUAL 4)
        MESSAGE(STATUS "Bad output from HG")
        SET(${prefix}_WC_REVISION "unknown")
        SET(${prefix}_WC_CHANGESET "unknown")
        SET(${prefix}_WC_BRANCH "unknown")
      ENDIF(_COUNT EQUAL 4)
    ENDIF(NOT ${Mercurial_hg_info_result} EQUAL 0)

  ENDMACRO(Mercurial_WC_INFO)

  MACRO(Mercurial_WC_LOG dir prefix)
    # This macro can block if the certificate is not signed:
    # hg ask you to accept the certificate and wait for your answer
    # This macro requires a hg server network access (Internet most of the time)
    # and can also be slow since it access the hg server
    EXECUTE_PROCESS(COMMAND
      ${Mercurial_HG_EXECUTABLE} --non-interactive log -r BASE ${dir}
      OUTPUT_VARIABLE ${prefix}_LAST_CHANGED_LOG
      ERROR_VARIABLE Mercurial_hg_log_error
      RESULT_VARIABLE Mercurial_hg_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Mercurial_hg_log_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Mercurial_HG_EXECUTABLE} log -r BASE ${dir}\" failed with output:\n${Mercurial_hg_log_error}")
    ENDIF(NOT ${Mercurial_hg_log_result} EQUAL 0)
  ENDMACRO(Mercurial_WC_LOG)
ENDIF(Mercurial_HG_EXECUTABLE)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Mercurial DEFAULT_MSG Mercurial_HG_EXECUTABLE)
