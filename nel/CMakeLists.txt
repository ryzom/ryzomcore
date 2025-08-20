IF (COMMAND cmake_policy)
  IF (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
  ENDIF ()
ENDIF ()

project(NeL VERSION ${RyzomCore_VERSION} LANGUAGES CXX C)

set(project_export_name "${PROJECT_NAME}-targets")
include(../cmake/add_project_library_alias.cmake)

IF(WITH_STATIC_DRIVERS)
  ADD_DEFINITIONS(-DNL_STATIC)
ENDIF()

IF(WIN32)
  # On Win32 we can also build the MAX plugins.
  IF(WITH_NEL_MAXPLUGIN)
    FIND_PACKAGE(3dsMaxSDK)
  ENDIF()
ENDIF()

IF(WITH_3D)
  IF(HUNTER_ENABLED)
    HUNTER_ADD_PACKAGE(freetype)
    FIND_PACKAGE(freetype CONFIG REQUIRED)
    SET(FREETYPE_LIBRARIES freetype::freetype)
  ELSE()
    FIND_PACKAGE(Freetype)
  ENDIF() #hunter

  IF(WITH_NEL_CEGUI)
    FIND_PACKAGE(CEGUI)
  ENDIF()
ENDIF()

IF(WITH_SOUND AND NOT HUNTER_ENABLED)
  FIND_PACKAGE(Ogg)
  FIND_PACKAGE(Vorbis)
  IF(WITH_FFMPEG)
    FIND_PACKAGE(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE)
  ENDIF()

  IF(WITH_DRIVER_OPENAL)
    FIND_PACKAGE(OpenAL)
  ENDIF()

  IF(WITH_DRIVER_FMOD)
    FIND_PACKAGE(FMOD)
  ENDIF()
ENDIF()

IF(WITH_GTK)
  FIND_PACKAGE(GTK2)
ENDIF()

IF(WITH_INSTALL_LIBRARIES)
  IF(UNIX)
    SET(prefix ${CMAKE_INSTALL_PREFIX})
    SET(exec_prefix ${NL_BIN_ABSOLUTE_PREFIX})
    SET(libdir ${NL_LIB_ABSOLUTE_PREFIX})
    SET(includedir ${CMAKE_INSTALL_PREFIX}/include)
    SET(enable_ligo ${WITH_LIGO})
    SET(enable_logic ${WITH_LOGIC})
    SET(enable_georges ${WITH_GEORGES})
    SET(enable_net ${WITH_NET})
    SET(enable_3d ${WITH_3D})
    SET(enable_pacs ${WITH_PACS})
    SET(enable_sound ${WITH_SOUND})
    CONFIGURE_FILE(nel-config.in ${CMAKE_CURRENT_BINARY_DIR}/nel-config)

    INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/nel-config DESTINATION ${NL_BIN_PREFIX})
  ENDIF()

  ADD_SUBDIRECTORY(include)
ENDIF()

ADD_SUBDIRECTORY(3rdparty)
ADD_SUBDIRECTORY(src)

IF(WITH_NEL_SAMPLES)
  ADD_SUBDIRECTORY(samples)
ENDIF()

ADD_SUBDIRECTORY(tools)


# packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

IF(WITH_INSTALL_LIBRARIES)
  set(project_namespace "${PROJECT_NAME}")
  set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
  set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
  set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
  set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
  set(package_components 3d georges gui ligo misc net pacs sevenzip sound snd_lowlevel) # TODO append in components to have actual list of build targets
  set(package_required_dependencies CURL)

  install(EXPORT "${project_export_name}"
          COMPONENT libraries
          NAMESPACE "${project_namespace}::"
          DESTINATION "${config_install_dir}"
  )

  write_basic_package_version_file(
          "${version_config}" COMPATIBILITY SameMajorVersion
  )

  configure_package_config_file("../cmake/config.cmake.in"
          "${project_config}"
          INSTALL_DESTINATION "${config_install_dir}"
  )

  install(FILES "${project_config}" "${version_config}"
          COMPONENT libraries
          DESTINATION "${config_install_dir}"
  )
ENDIF()
