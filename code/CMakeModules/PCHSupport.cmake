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
ELSE(MSVC)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
      ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
      OUTPUT_VARIABLE gcc_compiler_version)

    IF(gcc_compiler_version MATCHES "^4\\.1(\\.[0-9]+)?")
      SET(PCHSupport_FOUND FALSE)
    ELSEIF(gcc_compiler_version MATCHES "^4\\.[0-9]+(\\.[0-9]+)?")
      SET(PCHSupport_FOUND TRUE)
    ENDIF(gcc_compiler_version MATCHES "^4\\.1(\\.[0-9]+)?")
  ELSE(CMAKE_COMPILER_IS_GNUCXX)
    # TODO: make tests for other compilers than GCC
    SET(PCHSupport_FOUND TRUE)
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(MSVC)

# Set PCH_FLAGS for common flags, PCH_ARCH_XXX_FLAGS for specific archs flags and PCH_ARCHS for archs
MACRO(PCH_SET_COMPILE_FLAGS _target)
  SET(PCH_FLAGS)
  SET(PCH_ARCHS)

  SET(_FLAGS)
  LIST(APPEND _FLAGS ${CMAKE_CXX_FLAGS})

  STRING(TOUPPER "${CMAKE_BUILD_TYPE}" _UPPER_BUILD)
  LIST(APPEND _FLAGS " ${CMAKE_CXX_FLAGS_${_UPPER_BUILD}}")

  GET_TARGET_PROPERTY(_targetType ${_target} TYPE)

  IF(${_targetType} STREQUAL SHARED_LIBRARY OR ${_targetType} STREQUAL MODULE_LIBRARY)
    LIST(APPEND _FLAGS " ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
  ELSE(${_targetType} STREQUAL SHARED_LIBRARY OR ${_targetType} STREQUAL MODULE_LIBRARY)
    GET_TARGET_PROPERTY(_pic ${_target} POSITION_INDEPENDENT_CODE)
    IF(_pic)
      LIST(APPEND _FLAGS " ${CMAKE_CXX_COMPILE_OPTIONS_PIE}")
    ENDIF(_pic)
  ENDIF(${_targetType} STREQUAL SHARED_LIBRARY OR ${_targetType} STREQUAL MODULE_LIBRARY)

  GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
  FOREACH(item ${DIRINC})
    LIST(APPEND _FLAGS " -I\"${item}\"")
  ENDFOREACH(item)

  # Required for CMake 2.6
  SET(GLOBAL_DEFINITIONS)
  GET_DIRECTORY_PROPERTY(DEFINITIONS COMPILE_DEFINITIONS)
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      LIST(APPEND GLOBAL_DEFINITIONS " -D${item}")
    ENDFOREACH(item)
  ENDIF(DEFINITIONS)

  GET_DIRECTORY_PROPERTY(DEFINITIONS COMPILE_DEFINITIONS_${_UPPER_BUILD})
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      LIST(APPEND GLOBAL_DEFINITIONS " -D${item}")
    ENDFOREACH(item)
  ENDIF(DEFINITIONS)

  GET_TARGET_PROPERTY(oldProps ${_target} COMPILE_FLAGS)
  IF(oldProps)
    LIST(APPEND _FLAGS " ${oldProps}")
  ENDIF(oldProps)

  GET_TARGET_PROPERTY(oldPropsBuild ${_target} COMPILE_FLAGS_${_UPPER_BUILD})
  IF(oldPropsBuild)
    LIST(APPEND _FLAGS " ${oldPropsBuild}")
  ENDIF(oldPropsBuild)

  GET_TARGET_PROPERTY(DIRINC ${_target} INCLUDE_DIRECTORIES)
  IF(DIRINC)
    FOREACH(item ${DIRINC})
      LIST(APPEND _FLAGS " -I\"${item}\"")
    ENDFOREACH(item)
  ENDIF(DIRINC)

  GET_TARGET_PROPERTY(DEFINITIONS ${_target} COMPILE_DEFINITIONS)
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      LIST(APPEND GLOBAL_DEFINITIONS " -D${item}")
    ENDFOREACH(item)
  ENDIF(DEFINITIONS)

  GET_TARGET_PROPERTY(DEFINITIONS ${_target} COMPILE_DEFINITIONS_${_UPPER_BUILD})
  IF(DEFINITIONS)
    FOREACH(item ${DEFINITIONS})
      LIST(APPEND GLOBAL_DEFINITIONS " -D${item}")
    ENDFOREACH(item)
  ENDIF(DEFINITIONS)

  GET_DIRECTORY_PROPERTY(_directory_flags DEFINITIONS)
  GET_DIRECTORY_PROPERTY(_directory_definitions DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITIONS)
  LIST(APPEND _FLAGS " ${GLOBAL_DEFINITIONS}")
  LIST(APPEND _FLAGS " ${_directory_flags}")
  LIST(APPEND _FLAGS " ${_directory_definitions}")

  # Format definitions
  IF(MSVC)
    # Fix path with space
    SEPARATE_ARGUMENTS(_FLAGS UNIX_COMMAND "${_FLAGS}")
  ELSE(MSVC)
    STRING(REGEX REPLACE " +" " " _FLAGS ${_FLAGS})
    SEPARATE_ARGUMENTS(_FLAGS)
  ENDIF(MSVC)

  IF(CLANG)
    # Determining all architectures and get common flags
    SET(_ARCH_NEXT)
    SET(_XARCH_NEXT)
    FOREACH(item ${_FLAGS})
      IF(_ARCH_NEXT)
        LIST(FIND PCH_ARCHS ${item} ITEM_FOUND)
        IF(ITEM_FOUND EQUAL -1)
          LIST(APPEND PCH_ARCHS ${item})
          STRING(TOUPPER "${item}" _UPPER_ARCH)
          SET(PCH_ARCH_${_UPPER_ARCH}_FLAGS "-arch" ${item})
        ENDIF(ITEM_FOUND EQUAL -1)
        SET(_ARCH_NEXT OFF)
      ELSEIF(_XARCH_NEXT)
        SET(_XARCH_NEXT OFF)
      ELSE(_ARCH_NEXT)
        IF(item MATCHES "^-arch")
          SET(_ARCH_NEXT ON)
        ELSEIF(item MATCHES "^-Xarch_")
          STRING(REGEX REPLACE "-Xarch_([a-z0-9_]+)" "\\1" item ${item})
          LIST(FIND PCH_ARCHS ${item} ITEM_FOUND)
          IF(ITEM_FOUND EQUAL -1)
            LIST(APPEND PCH_ARCHS ${item})
            STRING(TOUPPER "${item}" _UPPER_ARCH)
            SET(PCH_ARCH_${_UPPER_ARCH}_FLAGS "-arch" ${item})
          ENDIF(ITEM_FOUND EQUAL -1)
          SET(_XARCH_NEXT ON)
        ELSE(item MATCHES "^-arch")
          LIST(APPEND PCH_FLAGS ${item})
        ENDIF(item MATCHES "^-arch")
      ENDIF(_ARCH_NEXT)
    ENDFOREACH(item)

    # Get architcture specific flags
    SET(_XARCH_NEXT)
    FOREACH(item ${_FLAGS})
      IF(_XARCH_NEXT)
        STRING(TOUPPER "${_XARCH_NEXT}" _UPPER_XARCH)
        LIST(APPEND PCH_ARCH_${_UPPER_XARCH}_FLAGS ${item})
        SET(_XARCH_NEXT OFF)
      ELSE(_XARCH_NEXT)
        IF(item MATCHES "^-Xarch_")
          STRING(SUBSTRING "${item}" 7 -1 _XARCH_NEXT)
        ENDIF(item MATCHES "^-Xarch_")
      ENDIF(_XARCH_NEXT)
    ENDFOREACH(item)

    # Remove duplicated architectures
    IF(_ARCHS AND PCH_ARCHS)
      LIST(REMOVE_DUPLICATES PCH_ARCHS)
    ENDIF(_ARCHS AND PCH_ARCHS)
  ELSE(CLANG)
    SET(PCH_FLAGS ${_FLAGS})
  ENDIF(CLANG)

  IF(PCH_FLAGS)
    LIST(REMOVE_DUPLICATES PCH_FLAGS)
  ENDIF(PCH_FLAGS)
ENDMACRO(PCH_SET_COMPILE_FLAGS)

MACRO(GET_PDB_FILENAME _out_filename _target)
  # determine output directory based on target type
  GET_TARGET_PROPERTY(_targetType ${_target} TYPE)
  IF(${_targetType} STREQUAL EXECUTABLE)
    SET(_targetOutput ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  ELSEIF(${_targetType} STREQUAL STATIC_LIBRARY)
    SET(_targetOutput ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  ELSE(${_targetType} STREQUAL EXECUTABLE)
    SET(_targetOutput ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  ENDIF(${_targetType} STREQUAL EXECUTABLE)

  # determine target postfix
  STRING(TOUPPER "${CMAKE_BUILD_TYPE}_POSTFIX" _postfix_var_name)
  GET_TARGET_PROPERTY(_targetPostfix ${_target} ${_postfix_var_name})
  IF(${_targetPostfix} MATCHES NOTFOUND)
    SET(_targetPostfix "")
  ENDIF(${_targetPostfix} MATCHES NOTFOUND)

  SET(${_out_filename} "${_targetOutput}/${_target}${_targetPostfix}.pdb")
ENDMACRO(GET_PDB_FILENAME)

MACRO(PCH_SET_COMPILE_COMMAND _inputcpp _compile_FLAGS)
  IF(CMAKE_CXX_COMPILER_ARG1)
    # remove leading space in compiler argument
    STRING(REGEX REPLACE "^ +" "" pchsupport_compiler_cxx_arg1 ${CMAKE_CXX_COMPILER_ARG1})
  ELSE(CMAKE_CXX_COMPILER_ARG1)
    SET(pchsupport_compiler_cxx_arg1 "")
  ENDIF(CMAKE_CXX_COMPILER_ARG1)

  IF(MSVC)
    GET_PDB_FILENAME(PDB_FILE ${_PCH_current_target})
    SET(PCH_COMMAND ${CMAKE_CXX_COMPILER} ${pchsupport_compiler_cxx_arg1} ${_compile_FLAGS} /Yc /Fp"${PCH_OUTPUT}" ${_inputcpp} /Fd"${PDB_FILE}" /c /Fo"${PCH_OUTPUT}.obj")
    # Ninja PCH Support
    # http://public.kitware.com/pipermail/cmake-developers/2012-March/003653.html
    SET_SOURCE_FILES_PROPERTIES(${_inputcpp} PROPERTIES OBJECT_OUTPUTS "${PCH_OUTPUT}.obj")
  ELSE(MSVC)
    SET(HEADER_FORMAT "c++-header")
    SET(_FLAGS "")
    IF(APPLE)
      SET(HEADER_FORMAT "objective-${HEADER_FORMAT}")
      SET(_FLAGS -fobjc-abi-version=2 -fobjc-legacy-dispatch)
    ENDIF(APPLE)
    SET(PCH_COMMAND ${CMAKE_CXX_COMPILER} ${pchsupport_compiler_cxx_arg1} ${_compile_FLAGS} ${_FLAGS} -x ${HEADER_FORMAT} -o ${PCH_OUTPUT} -c ${PCH_INPUT})
  ENDIF(MSVC)
ENDMACRO(PCH_SET_COMPILE_COMMAND)

MACRO(PCH_SET_PRECOMPILED_HEADER_OUTPUT _targetName _input _arch _language)
  SET(_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${_targetName}_pch")

  IF(MSVC)
    FILE(MAKE_DIRECTORY ${_OUTPUT_DIR})
    GET_FILENAME_COMPONENT(_name ${_input} NAME_WE)
    SET(PCH_INPUT ${_input})
    SET(PCH_OUTPUT "${_OUTPUT_DIR}/${_name}.pch")
  ELSE(MSVC)
    IF(NOT "${_arch}" STREQUAL "")
      SET(_OUTPUT_DIR "${_OUTPUT_DIR}_${_arch}")
    ENDIF(NOT "${_arch}" STREQUAL "")

    IF(NOT "${_language}" STREQUAL "")
      SET(_OUTPUT_DIR "${_OUTPUT_DIR}_${_language}")
    ENDIF(NOT "${_language}" STREQUAL "")

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
    ELSE(CLANG)
      SET(PCH_EXT "gch")
    ENDIF(CLANG)

    # For GCC and Clang, PCH needs to be in the same directory as .h
    SET(PCH_OUTPUT "${_OUTPUT_DIR}/${_name}.${PCH_EXT}")
  ENDIF(MSVC)
ENDMACRO(PCH_SET_PRECOMPILED_HEADER_OUTPUT)

# Add common flags
MACRO(ADD_PRECOMPILED_HEADER_TO_TARGET _targetName)
  GET_TARGET_PROPERTY(oldProps ${_targetName} COMPILE_FLAGS)

  IF(${oldProps} MATCHES NOTFOUND)
    SET(oldProps "")
  ENDIF(${oldProps} MATCHES NOTFOUND)

  IF(MSVC)
    SET(_target_cflags "${oldProps} /Yu\"${PCH_INPUT}\" /FI\"${PCH_INPUT}\" /Fp\"${PCH_OUTPUT}\"")
    # Ninja PCH Support
    # http://public.kitware.com/pipermail/cmake-developers/2012-March/003653.html
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES OBJECT_DEPENDS "${PCH_OUTPUT}")
    
    # NMAKE-VS2012 Error LNK2011 (NMAKE-VS2010 do not complain)
    # we need to link the pch.obj file, see http://msdn.microsoft.com/en-us/library/3ay26wa2(v=vs.110).aspx
    GET_TARGET_PROPERTY(DEPS ${_targetName} LINK_LIBRARIES)
    IF(NOT DEPS)
      SET(DEPS)
    ENDIF()
    LIST(INSERT DEPS 0 "${PCH_OUTPUT}.obj")
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES LINK_LIBRARIES "${DEPS}")
  ELSE(MSVC)
    # for use with distcc and gcc >4.0.1 if preprocessed files are accessible
    # on all remote machines set
    # PCH_ADDITIONAL_COMPILER_FLAGS to -fpch-preprocess
    SET(PCH_ADDITIONAL_COMPILER_FLAGS)
    LIST(LENGTH PCH_ARCHS PCH_ARCHS_COUNT)

    # If no arch is specified, create common flags
    IF(PCH_ARCHS_COUNT LESS 2)
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "-include ${PCH_INPUT} ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF(PCH_ARCHS_COUNT LESS 2)

    IF(APPLE)
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "-fobjc-abi-version=2 -fobjc-legacy-dispatch -x objective-c++ ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF(APPLE)
    
    IF(WITH_PCH_DEBUG)
      SET(PCH_ADDITIONAL_COMPILER_FLAGS "-H ${PCH_ADDITIONAL_COMPILER_FLAGS}")
    ENDIF(WITH_PCH_DEBUG)

    SET(_target_cflags "${oldProps} ${PCH_ADDITIONAL_COMPILER_FLAGS} -Winvalid-pch")
  ENDIF(MSVC)

  SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS ${_target_cflags})
ENDMACRO(ADD_PRECOMPILED_HEADER_TO_TARGET)

# Add specific flags for an arch
MACRO(ADD_PRECOMPILED_HEADER_TO_TARGET_ARCH _targetName _arch)
  LIST(LENGTH PCH_ARCHS PCH_ARCHS_COUNT)

  IF(PCH_ARCHS_COUNT GREATER 1)
    GET_TARGET_PROPERTY(_FLAGS ${_targetName} COMPILE_FLAGS)

    IF(${_FLAGS} MATCHES NOTFOUND)
      SET(_FLAGS "")
    ENDIF(${_FLAGS} MATCHES NOTFOUND)

    SET(_FLAGS "${_FLAGS} -Xarch_${_arch} -include${PCH_INPUT}")

    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS ${_FLAGS})
  ENDIF(PCH_ARCHS_COUNT GREATER 1)
ENDMACRO(ADD_PRECOMPILED_HEADER_TO_TARGET_ARCH)

MACRO(PCH_CREATE_TARGET _targetName _targetNamePCH)
  ADD_CUSTOM_COMMAND(OUTPUT ${PCH_OUTPUT} COMMAND ${PCH_COMMAND} COMMENT "Generating ${_targetNamePCH}" DEPENDS ${PCH_INPUT})
  ADD_CUSTOM_TARGET(${_targetNamePCH} DEPENDS ${PCH_INPUT} ${PCH_OUTPUT})
  ADD_DEPENDENCIES(${_targetName} ${_targetNamePCH})
ENDMACRO(PCH_CREATE_TARGET _targetName _inputh _inputcpp)

MACRO(ADD_PRECOMPILED_HEADER _targetName _inputh _inputcpp)
  SET(_PCH_current_target ${_targetName})

  IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(FATAL_ERROR
      "This is the ADD_PRECOMPILED_HEADER macro. "
      "You must set CMAKE_BUILD_TYPE!"
    )
  ENDIF(NOT CMAKE_BUILD_TYPE)

  PCH_SET_COMPILE_FLAGS(${_targetName})

  IF(PCH_ARCHS)
    SET(PCH_OUTPUTS)
    FOREACH(_ARCH ${PCH_ARCHS})
      STRING(TOUPPER "${_ARCH}" _UPPER_ARCH)

      PCH_SET_PRECOMPILED_HEADER_OUTPUT(${_targetName} ${_inputh} ${_ARCH} "")
      LIST(APPEND PCH_OUTPUTS ${PCH_OUTPUT})

      PCH_SET_COMPILE_COMMAND(${_inputcpp} "${PCH_ARCH_${_UPPER_ARCH}_FLAGS};${PCH_FLAGS}")
      PCH_CREATE_TARGET(${_targetName} ${_targetName}_pch_${_ARCH})

      ADD_PRECOMPILED_HEADER_TO_TARGET_ARCH(${_targetName} ${_ARCH})
    ENDFOREACH(_ARCH)
  ELSE(PCH_ARCHS)
    PCH_SET_PRECOMPILED_HEADER_OUTPUT(${_targetName} ${_inputh} "" "")
    LIST(APPEND PCH_OUTPUTS ${PCH_OUTPUT})

    PCH_SET_COMPILE_COMMAND(${_inputcpp} "${PCH_FLAGS}")
    PCH_CREATE_TARGET(${_targetName} ${_targetName}_pch)
  ENDIF(PCH_ARCHS)

  ADD_PRECOMPILED_HEADER_TO_TARGET(${_targetName})

  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PCH_OUTPUTS}")
ENDMACRO(ADD_PRECOMPILED_HEADER)

MACRO(ADD_NATIVE_PRECOMPILED_HEADER _targetName _inputh _inputcpp)
  IF(NOT PCHSupport_FOUND)
    MESSAGE(STATUS "PCH disabled because compiler doesn't support them")
    RETURN()
  ENDIF(NOT PCHSupport_FOUND)

  # 0 => creating a new target for PCH, works for all makefiles
  # 1 => setting PCH for VC++ project, works for VC++ projects
  # 2 => setting PCH for XCode project, works for XCode projects
  IF(CMAKE_GENERATOR MATCHES "Visual Studio")
    SET(PCH_METHOD 1)
  ELSEIF(CMAKE_GENERATOR MATCHES "Xcode")
    SET(PCH_METHOD 2)
  ELSE(CMAKE_GENERATOR MATCHES "Visual Studio")
    SET(PCH_METHOD 0)
  ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio")

  IF(PCH_METHOD EQUAL 1)
    # Auto include the precompile (useful for moc processing, since the use of
    # precompiled is specified at the target level
    # and I don't want to specifiy /F- for each moc/res/ui generated files (using Qt)

    GET_TARGET_PROPERTY(oldProps ${_targetName} COMPILE_FLAGS)
    IF(${oldProps} MATCHES NOTFOUND)
      SET(oldProps "")
    ENDIF(${oldProps} MATCHES NOTFOUND)

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
  ELSE(PCH_METHOD EQUAL 1)
    #Fallback to the "old" precompiled suppport
    ADD_PRECOMPILED_HEADER(${_targetName} ${_inputh} ${_inputcpp})
  ENDIF(PCH_METHOD EQUAL 1)

  IF(TARGET ${_targetName}_static)
    ADD_NATIVE_PRECOMPILED_HEADER(${_targetName}_static ${_inputh} ${_inputcpp})
  ENDIF(TARGET ${_targetName}_static)
ENDMACRO(ADD_NATIVE_PRECOMPILED_HEADER)
