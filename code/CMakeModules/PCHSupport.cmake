# - Try to find precompiled headers support for GCC 3.4 and 4.x (and MSVC)
# Once done this will define:
#
# Variable:
#   PCHSupport_FOUND
#
#   ADD_PRECOMPILED_HEADER  _targetName _inputh _inputcpp
#   ADD_PRECOMPILED_HEADER_TO_TARGET _targetName _input _pch_output_to_use
#   ADD_NATIVE_PRECOMPILED_HEADER _targetName _inputh _inputcpp

IF(MSVC)
  SET(PCHSupport_FOUND TRUE)
ELSE()
  IF(CMAKE_COMPILER_IS_GNUCXX)
    EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
      ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
      OUTPUT_VARIABLE gcc_compiler_version)

    IF(gcc_compiler_version VERSION_LESS "4.2")
      SET(PCHSupport_FOUND FALSE)
    ELSE()
      SET(PCHSupport_FOUND TRUE)
    ENDIF()
  ELSE()
    # TODO: make tests for other compilers than GCC
    SET(PCHSupport_FOUND TRUE)
  ENDIF()
ENDIF()

MACRO(APPEND_DEFINITION _NAME _VAL)
  IF(CMAKE_VERSION VERSION_LESS "2.8.12")
    # don't support logical expressions, append definition
    LIST(APPEND ${_NAME} "-D${_VAL}")
  ELSE()
    # support logical expressions, use them
    LIST(APPEND ${_NAME} "$<$<BOOL:${_VAL}>:-D$<JOIN:${_VAL},-D>>")
  ENDIF()
ENDMACRO()

# Set PCH_FLAGS for common flags, PCH_ARCH_XXX_FLAGS for specific archs flags and PCH_ARCHS for archs
MACRO(PCH_SET_COMPILE_FLAGS _target)
  SET(PCH_FLAGS)
  SET(PCH_ARCHS)
  SET(PCH_INCLUDES)

  # Append target for clang if defined
  IF(CMAKE_CXX_COMPILER_TARGET)
    LIST(APPEND PCH_FLAGS "--target=${CMAKE_CXX_COMPILER_TARGET}")
  ENDIF()

  IF(CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN)
    LIST(APPEND PCH_FLAGS "--gcc-toolchain=${CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN}")
  ENDIF()

  IF(CMAKE_SYSROOT)
    LIST(APPEND PCH_FLAGS "--sysroot=${CMAKE_SYSROOT}")
  ENDIF()

  IF(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES)
    FOREACH(item ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES})
      LIST(APPEND PCH_FLAGS "-isystem ${item}")
    ENDFOREACH()
  ENDIF()

  # C++ flags
  SET(_FLAGS ${CMAKE_CXX_FLAGS})
  SEPARATE_ARGUMENTS(_FLAGS)

  LIST(APPEND PCH_FLAGS ${_FLAGS})

  # C++ config flags
  STRING(TOUPPER "${CMAKE_BUILD_TYPE}" _UPPER_BUILD)

  SET(_FLAGS ${CMAKE_CXX_FLAGS_${_UPPER_BUILD}})
  SEPARATE_ARGUMENTS(_FLAGS)

  LIST(APPEND PCH_FLAGS ${_FLAGS})

  GET_TARGET_PROPERTY(_targetType ${_target} TYPE)

  SET(_USE_PIC OFF)

  IF(${_targetType} STREQUAL "SHARED_LIBRARY" OR ${_targetType} STREQUAL "MODULE_LIBRARY")
    SET(_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
    SEPARATE_ARGUMENTS(_FLAGS)
    LIST(APPEND PCH_FLAGS ${_FLAGS})
  ELSE()
    GET_TARGET_PROPERTY(_pic ${_target} POSITION_INDEPENDENT_CODE)
    IF(_pic)
      SET(_USE_PIC ON)
    ENDIF()
  ENDIF()

  GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
  FOREACH(item ${DIRINC})
    LIST(APPEND PCH_INCLUDES "${item}")
  ENDFOREACH()

  # NOTE: As cmake files (eg FindQT4) may now use generator expressions around their defines that evaluate
  #       to an empty string, wrap all "items" in an expression that outputs a -D IFF the generated
  #       expression is not empty.

  # Required for CMake 2.6
  SET(GLOBAL_DEFINITIONS)
  GET_DIRECTORY_PROPERTY(DEFINITIONS COMPILE_DEFINITIONS)
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_DIRECTORY_PROPERTY(DEFINITIONS COMPILE_DEFINITIONS_${_UPPER_BUILD})
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_DIRECTORY_PROPERTY(DEFINITIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_DIRECTORY_PROPERTY(DEFINITIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS_${_UPPER_BUILD})
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_TARGET_PROPERTY(oldProps ${_target} COMPILE_FLAGS)
  IF(oldProps)
    SET(_FLAGS ${oldProps})
    SEPARATE_ARGUMENTS(_FLAGS)
    LIST(APPEND PCH_FLAGS ${_FLAGS})
  ENDIF()

  GET_TARGET_PROPERTY(oldPropsBuild ${_target} COMPILE_FLAGS_${_UPPER_BUILD})
  IF(oldPropsBuild)
    SET(_FLAGS ${oldPropsBuild})
    SEPARATE_ARGUMENTS(_FLAGS)
    LIST(APPEND PCH_FLAGS ${_FLAGS})
  ENDIF()

  GET_TARGET_PROPERTY(DIRINC ${_target} INCLUDE_DIRECTORIES)
  IF(DIRINC)
    FOREACH(item ${DIRINC})
      LIST(APPEND PCH_INCLUDES "${item}")
    ENDFOREACH()
  ENDIF()

  GET_TARGET_PROPERTY(DEFINITIONS ${_target} COMPILE_DEFINITIONS)
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_TARGET_PROPERTY(DEFINITIONS ${_target} COMPILE_DEFINITIONS_${_UPPER_BUILD})
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
    ENDFOREACH()
  ENDIF()

  GET_TARGET_PROPERTY(OPTIONS ${_target} COMPILE_OPTIONS)
  IF(OPTIONS)
    SEPARATE_ARGUMENTS(OPTIONS)
    LIST(APPEND PCH_FLAGS ${OPTIONS})
  ENDIF()

  GET_TARGET_PROPERTY(OPTIONS ${_target} COMPILE_OPTIONS_${_UPPER_BUILD})
  IF(OPTIONS)
    SEPARATE_ARGUMENTS(OPTIONS)
    LIST(APPEND PCH_FLAGS ${OPTIONS})
  ENDIF()

  GET_TARGET_PROPERTY(_LIBS ${_target} INTERFACE_LINK_LIBRARIES)
  IF(_LIBS)
    FOREACH(_LIB ${_LIBS})
      IF(TARGET "${_LIB}")
        # use same include directories
        GET_TARGET_PROPERTY(_DIRS ${_LIB} INTERFACE_INCLUDE_DIRECTORIES)

        IF(_DIRS)
          FOREACH(item ${_DIRS})
            LIST(APPEND PCH_INCLUDES "${item}")
          ENDFOREACH()
        ENDIF()

        # use same compile definitions
        GET_TARGET_PROPERTY(_DEFINITIONS ${_LIB} INTERFACE_COMPILE_DEFINITIONS)

        IF(_DEFINITIONS)
          FOREACH(item ${_DEFINITIONS})
            APPEND_DEFINITION(GLOBAL_DEFINITIONS ${item})
          ENDFOREACH()
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()

  # Special Qt 5 cases
  IF(GLOBAL_DEFINITIONS MATCHES "QT_CORE_LIB")
    # Hack to define missing QT_NO_DEBUG with Qt 5.2
    IF(_UPPER_BUILD STREQUAL "RELEASE")
      LIST(APPEND GLOBAL_DEFINITIONS "-DQT_NO_DEBUG")
    ENDIF()

    # Qt5_POSITION_INDEPENDENT_CODE should be true if Qt was compiled with PIC
    IF(Qt5_POSITION_INDEPENDENT_CODE)
      SET(_USE_PIC ON)
    ENDIF()

    IF(_USE_PIC)
      LIST(APPEND PCH_FLAGS ${CMAKE_CXX_COMPILE_OPTIONS_PIC})
    ENDIF()
  ENDIF()

  IF(CMAKE_VERSION VERSION_LESS "3.3.0")
    GET_DIRECTORY_PROPERTY(_DIRECTORY_FLAGS DEFINITIONS)

    IF(_DIRECTORY_FLAGS)
      SEPARATE_ARGUMENTS(_DIRECTORY_FLAGS)
      FOREACH(item ${_DIRECTORY_FLAGS})
        LIST(APPEND PCH_FLAGS "${item}")
      ENDFOREACH()
    ENDIF()

    GET_DIRECTORY_PROPERTY(_DIRECTORY_DEFINITIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEFINITIONS)

    IF(_DIRECTORY_DEFINITIONS)
      SEPARATE_ARGUMENTS(_DIRECTORY_DEFINITIONS)
      FOREACH(item ${_DIRECTORY_DEFINITIONS})
        LIST(APPEND PCH_FLAGS "${item}")
      ENDFOREACH()
    ENDIF()
  ENDIF()

  IF(CMAKE_CXX11_EXTENSION_COMPILE_OPTION)
    LIST(APPEND PCH_FLAGS "${CMAKE_CXX11_EXTENSION_COMPILE_OPTION}")
  ENDIF()

  # Already in list form and items may contain non-leading spaces that should not be split on
  LIST(APPEND PCH_FLAGS "${GLOBAL_DEFINITIONS}")

  IF(WIN32)
    SET(SYSTEM_FLAG "[-/$]")
  ELSE()
    SET(SYSTEM_FLAG "[-$]")
  ENDIF()

  SET(_FINAL_FLAGS)
  SET(_PREVIOUS_FLAG)

  FOREACH(_FLAG ${PCH_FLAGS})
    # If parameter is really a flag (starts with -)
    IF(_FLAG MATCHES "^${SYSTEM_FLAG}")
      IF(_PREVIOUS_FLAG)
        # Append previous flag
        LIST(APPEND _FINAL_FLAGS ${_PREVIOUS_FLAG})
      ENDIF()

      SET(_PREVIOUS_FLAG ${_FLAG})
    ELSE()
      IF(_PREVIOUS_FLAG)
        # Append previous flag and its parameter
        # TODO: escape them only if there is an space
        LIST(APPEND _FINAL_FLAGS "${_PREVIOUS_FLAG} \"${_FLAG}\"")
        SET(_PREVIOUS_FLAG)
      ELSE()
        # Shouldn't happen
        MESSAGE(FATAL_ERROR "No previous flag before ${_FLAG}")
      ENDIF()
    ENDIF()
  ENDFOREACH()

  IF(_PREVIOUS_FLAG)
    LIST(APPEND _FINAL_FLAGS ${_PREVIOUS_FLAG})
  ENDIF()

  SET(PCH_FLAGS ${_FINAL_FLAGS})

  # Remove flags that don't work with PCH
  LIST(REMOVE_ITEM PCH_FLAGS "-Wa,--noexecstack")

  # Remove all empty parameters
  LIST(REMOVE_ITEM PCH_FLAGS "")

  # Remove duplicate parameters
  LIST(REMOVE_DUPLICATES PCH_FLAGS)

  # create a command-line string
  STRING(REGEX REPLACE ";" " " PCH_FLAGS "${PCH_FLAGS}")

  # and separate arguments
  SEPARATE_ARGUMENTS(PCH_FLAGS)
ENDMACRO()

MACRO(PCH_SET_COMPILE_COMMAND _inputcpp _compile_FLAGS _includes)
  IF(CMAKE_CXX_COMPILER_ARG1)
    # remove leading space in compiler argument
    STRING(REGEX REPLACE "^ +" "" pchsupport_compiler_cxx_arg1 ${CMAKE_CXX_COMPILER_ARG1})
  ELSE()
    SET(pchsupport_compiler_cxx_arg1 "")
  ENDIF()

  IF(MSVC)
    GET_INTERMEDIATE_PDB_FULLPATH(${_PCH_current_target} _PDB_FILE)

    SET(PCH_TEMP_CONTENT)

    FOREACH(_include ${_includes})
      SET(PCH_TEMP_CONTENT "${PCH_TEMP_CONTENT} -I \"${_include}\"")
    ENDFOREACH()

    SET(PCH_TEMP_FILE ${CMAKE_CURRENT_BINARY_DIR}/pch_command.txt)
    FILE(WRITE ${PCH_TEMP_FILE} "${PCH_TEMP_CONTENT}")

    SET(PCH_COMMAND ${CMAKE_CXX_COMPILER} /nologo @${PCH_TEMP_FILE} ${pchsupport_compiler_cxx_arg1} ${_compile_FLAGS} /Yc /Fp"${PCH_OUTPUT}" ${_inputcpp} /Fd"${_PDB_FILE}" /c /Fo"${PCH_OUTPUT}.obj")

    # Ninja PCH Support
    # http://public.kitware.com/pipermail/cmake-developers/2012-March/003653.html
    SET_SOURCE_FILES_PROPERTIES(${_inputcpp} PROPERTIES OBJECT_OUTPUTS "${PCH_OUTPUT}.obj")
  ELSE()
    SET(HEADER_FORMAT "c++-header")
    SET(_FLAGS)
    IF(APPLE)
      SET(HEADER_FORMAT "objective-${HEADER_FORMAT}")
      LIST(APPEND _FLAGS ${OBJC_FLAGS})
    ENDIF()

    FOREACH(_include ${_includes})
      LIST(APPEND _FLAGS -I "${_include}")
    ENDFOREACH()

    SET(PCH_COMMAND ${CMAKE_CXX_COMPILER} ${pchsupport_compiler_cxx_arg1} ${_compile_FLAGS} ${_FLAGS} -x ${HEADER_FORMAT} -o ${PCH_OUTPUT} -c ${PCH_INPUT})
  ENDIF()
ENDMACRO()

MACRO(PCH_SET_PRECOMPILED_HEADER_OUTPUT _targetName _input _arch _language)
  SET(_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${_targetName}_pch")

  IF(MSVC)
    FILE(MAKE_DIRECTORY ${_OUTPUT_DIR})
    GET_FILENAME_COMPONENT(_name ${_input} NAME_WE)
    SET(PCH_INPUT ${_input})
    SET(PCH_OUTPUT "${_OUTPUT_DIR}/${_name}.pch")
  ELSE()
    IF(NOT "${_arch}" STREQUAL "")
      SET(_OUTPUT_DIR "${_OUTPUT_DIR}_${_arch}")
    ENDIF()

    IF(NOT "${_language}" STREQUAL "")
      SET(_OUTPUT_DIR "${_OUTPUT_DIR}_${_language}")
    ENDIF()

    GET_FILENAME_COMPONENT(_name ${_input} NAME)

    # Copy .h to output dir
    SET(PCH_INPUT "${_OUTPUT_DIR}/${_name}")
    ADD_CUSTOM_COMMAND(OUTPUT ${PCH_INPUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_input} ${PCH_INPUT}
        DEPENDS ${_input}
        COMMENT "[${_targetName}] Update precompiled header - done"
    )

    IF(CLANG)
      SET(PCH_EXT "pth")
    ELSE()
      SET(PCH_EXT "gch")
    ENDIF()

    # For GCC and Clang, PCH needs to be in the same directory as .h
    SET(PCH_OUTPUT "${_OUTPUT_DIR}/${_name}.${PCH_EXT}")
  ENDIF()
ENDMACRO()

# Add common flags
MACRO(ADD_PRECOMPILED_HEADER_TO_TARGET _targetName)
  GET_TARGET_PROPERTY(oldProps ${_targetName} COMPILE_FLAGS)

  IF(${oldProps} MATCHES NOTFOUND)
    SET(oldProps "")
  ENDIF()

  IF(MSVC)
    SET(_target_cflags "${oldProps} /Yu\"${PCH_INPUT}\" /FI\"${PCH_INPUT}\" /Fp\"${PCH_OUTPUT}\"")
    # Ninja PCH Support
    # http://public.kitware.com/pipermail/cmake-developers/2012-March/003653.html
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES OBJECT_DEPENDS "${PCH_OUTPUT}")

    # NMAKE-VS2012 Error LNK2011 (NMAKE-VS2010 do not complain)
    # we need to link the pch.obj file, see http://msdn.microsoft.com/en-us/library/3ay26wa2(v=vs.110).aspx
    GET_TARGET_PROPERTY(_STATIC_LIBRARY_FLAGS ${_targetName} STATIC_LIBRARY_FLAGS)
    IF(NOT _STATIC_LIBRARY_FLAGS)
      SET(_STATIC_LIBRARY_FLAGS)
    ENDIF()
    SET(_STATIC_LIBRARY_FLAGS "${PCH_OUTPUT}.obj ${_STATIC_LIBRARY_FLAGS}")

    GET_TARGET_PROPERTY(_LINK_FLAGS ${_targetName} LINK_FLAGS)
    IF(NOT _LINK_FLAGS)
      SET(_LINK_FLAGS)
    ENDIF()
    SET(_LINK_FLAGS "${PCH_OUTPUT}.obj ${_LINK_FLAGS}")

    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES STATIC_LIBRARY_FLAGS ${_STATIC_LIBRARY_FLAGS} LINK_FLAGS ${_LINK_FLAGS})
  ELSE()
    # for use with distcc and gcc >4.0.1 if preprocessed files are accessible
    # on all remote machines set
    # PCH_ADDITIONAL_COMPILER_FLAGS to -fpch-preprocess
    SET(PCH_ADDITIONAL_COMPILER_FLAGS)
    LIST(LENGTH PCH_ARCHS PCH_ARCHS_COUNT)

    # If no arch is specified, create common flags
    IF(PCH_ARCHS_COUNT LESS 2)
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "-include ${PCH_INPUT} ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF()

    IF(APPLE)
      STRING(REPLACE ";" " " OBJC_FLAGS_STR "${OBJC_FLAGS}")
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "${OBJC_FLAGS_STR} -x objective-c++ ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF()

    IF(WITH_PCH_DEBUG)
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "-H ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF()

    SET(_target_cflags "${oldProps} ${PCH_ADDITIONAL_COMPILER_FLAGS} -Winvalid-pch")
  ENDIF()

  SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS ${_target_cflags})
ENDMACRO()

# Add specific flags for an arch
MACRO(ADD_PRECOMPILED_HEADER_TO_TARGET_ARCH _targetName _arch)
  LIST(LENGTH PCH_ARCHS PCH_ARCHS_COUNT)

  IF(PCH_ARCHS_COUNT GREATER 1)
    GET_TARGET_PROPERTY(_FLAGS ${_targetName} COMPILE_FLAGS)

    IF(${_FLAGS} MATCHES NOTFOUND)
      SET(_FLAGS "")
    ENDIF()

    SET(_FLAGS "${_FLAGS} -Xarch_${_arch} -include${PCH_INPUT}")

    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS ${_FLAGS})
  ENDIF()
ENDMACRO()

MACRO(PCH_CREATE_TARGET _targetName _targetNamePCH)
  ADD_CUSTOM_COMMAND(OUTPUT ${PCH_OUTPUT} COMMAND ${PCH_COMMAND} COMMENT "Generating ${_targetNamePCH}" DEPENDS ${PCH_INPUT})
  ADD_CUSTOM_TARGET(${_targetNamePCH} DEPENDS ${PCH_INPUT} ${PCH_OUTPUT})
  ADD_DEPENDENCIES(${_targetName} ${_targetNamePCH})
ENDMACRO()

MACRO(ADD_PRECOMPILED_HEADER _targetName _inputh _inputcpp)
  SET(_PCH_current_target ${_targetName})

  IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(FATAL_ERROR
      "This is the ADD_PRECOMPILED_HEADER macro. "
      "You must set CMAKE_BUILD_TYPE!"
    )
  ENDIF()

  PCH_SET_COMPILE_FLAGS(${_targetName})

  IF(PCH_ARCHS)
    SET(PCH_OUTPUTS)
    FOREACH(_ARCH ${PCH_ARCHS})
      STRING(TOUPPER "${_ARCH}" _UPPER_ARCH)

      PCH_SET_PRECOMPILED_HEADER_OUTPUT(${_targetName} ${_inputh} ${_ARCH} "")
      LIST(APPEND PCH_OUTPUTS ${PCH_OUTPUT})

      PCH_SET_COMPILE_COMMAND(${_inputcpp} "${PCH_ARCH_${_UPPER_ARCH}_FLAGS};${PCH_FLAGS}" "${PCH_INCLUDES}")
      PCH_CREATE_TARGET(${_targetName} ${_targetName}_pch_${_ARCH})

      ADD_PRECOMPILED_HEADER_TO_TARGET_ARCH(${_targetName} ${_ARCH})
    ENDFOREACH()
  ELSE()
    PCH_SET_PRECOMPILED_HEADER_OUTPUT(${_targetName} ${_inputh} "" "")
    LIST(APPEND PCH_OUTPUTS ${PCH_OUTPUT})

    PCH_SET_COMPILE_COMMAND(${_inputcpp} "${PCH_FLAGS}" "${PCH_INCLUDES}")
    PCH_CREATE_TARGET(${_targetName} ${_targetName}_pch)
  ENDIF()

  ADD_PRECOMPILED_HEADER_TO_TARGET(${_targetName})

  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PCH_OUTPUTS}")
ENDMACRO()

MACRO(ADD_NATIVE_PRECOMPILED_HEADER _targetName _inputh _inputcpp)
  IF(PCHSupport_FOUND)
    # 0 => creating a new target for PCH, works for all makefiles
    # 1 => setting PCH for VC++ project, works for VC++ projects
    # 2 => setting PCH for XCode project, works for XCode projects
    IF(CMAKE_GENERATOR MATCHES "Visual Studio")
      SET(PCH_METHOD 1)
    ELSEIF(CMAKE_GENERATOR MATCHES "Xcode")
      SET(PCH_METHOD 2)
    ELSE()
      SET(PCH_METHOD 0)
    ENDIF()

    IF(PCH_METHOD EQUAL 1)
      # Auto include the precompile (useful for moc processing, since the use of
      # precompiled is specified at the target level
      # and I don't want to specifiy /F- for each moc/res/ui generated files (using Qt)

      GET_TARGET_PROPERTY(oldProps ${_targetName} COMPILE_FLAGS)
      IF(${oldProps} MATCHES NOTFOUND)
        SET(oldProps "")
      ENDIF()

      SET(newProperties "${oldProps} /Yu\"${_inputh}\" /FI\"${_inputh}\"")
      SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS "${newProperties}")

      #also inlude ${oldProps} to have the same compile options
      SET_SOURCE_FILES_PROPERTIES(${_inputcpp} PROPERTIES COMPILE_FLAGS "${oldProps} /Yc\"${_inputh}\"")
    ELSEIF(PCH_METHOD EQUAL 2)
      # For Xcode, cmake needs my patch to process
      # GCC_PREFIX_HEADER and GCC_PRECOMPILE_PREFIX_HEADER as target properties

      # When buiding out of the tree, precompiled may not be located
      # Use full path instead.
      GET_FILENAME_COMPONENT(fullPath ${_inputh} ABSOLUTE)

      SET_TARGET_PROPERTIES(${_targetName} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${fullPath}")
      SET_TARGET_PROPERTIES(${_targetName} PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    ELSE()
      #Fallback to the "old" precompiled suppport
      IF(CMAKE_OSX_ARCHITECTURES AND TARGETS_COUNT GREATER 1)
        FOREACH(_ARCH ${CMAKE_OSX_ARCHITECTURES})
          ADD_PRECOMPILED_HEADER(${_targetName}_${_ARCH} ${_inputh} ${_inputcpp})
        ENDFOREACH()
      ELSE()
        ADD_PRECOMPILED_HEADER(${_targetName} ${_inputh} ${_inputcpp})
      ENDIF()
    ENDIF()

    IF(TARGET ${_targetName}_static)
      ADD_NATIVE_PRECOMPILED_HEADER(${_targetName}_static ${_inputh} ${_inputcpp})
    ENDIF()
  ELSE()
    MESSAGE(STATUS "PCH disabled because compiler doesn't support them")
  ENDIF()
ENDMACRO()
