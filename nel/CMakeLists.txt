project(NeL VERSION ${RyzomCore_VERSION} LANGUAGES CXX C)

set(project_export_name "${PROJECT_NAME}-targets")

IF(WITH_STATIC_DRIVERS)
  add_compile_definitions(NL_STATIC)
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
  ELSEIF(EMSCRIPTEN)
    # Emscripten provides FreeType as a port via -sUSE_FREETYPE=1
    SET(FREETYPE_FOUND TRUE)
    ADD_LIBRARY(Freetype::Freetype INTERFACE IMPORTED)
    SET_TARGET_PROPERTIES(Freetype::Freetype PROPERTIES
      INTERFACE_COMPILE_OPTIONS "-sUSE_FREETYPE=1"
    )
  ELSE()
    FIND_PACKAGE(Freetype)
  ENDIF() #hunter

  IF(WITH_NEL_CEGUI)
    FIND_PACKAGE(CEGUI REQUIRED)
    # FindCEGUI does not stop on its own
    IF(NOT CEGUI_FOUND)
      MESSAGE(FATAL_ERROR "WITH_NEL_CEGUI needs CEGUI (the CEGUIBase library and its headers), which was not found: set CEGUI_DIR, or CEGUI_LIBRARY and CEGUI_INCLUDE_DIRS, or turn WITH_NEL_CEGUI off")
    ENDIF()
  ENDIF()
ENDIF()

IF(WITH_SOUND AND NOT HUNTER_ENABLED)
  FIND_PACKAGE(Ogg)
  FIND_PACKAGE(Vorbis)
  IF(WITH_FFMPEG)
    FIND_PACKAGE(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE)
  ENDIF()

  IF(WITH_DRIVER_OPENAL)
    FIND_PACKAGE(OpenAL REQUIRED)
  ENDIF()

  IF(WITH_DRIVER_FMOD)
    FIND_PACKAGE(FMOD REQUIRED)
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
    # the definitions the NeL headers need, which NeL::misc gives to CMake projects, for nel-config and the .pc files
    # (the build type of a single-configuration build: a multi-configuration generator has none to give here)
    STRING(TOUPPER "${CMAKE_BUILD_TYPE}" NL_PC_BUILD_TYPE)
    IF(NL_PC_BUILD_TYPE STREQUAL "DEBUG")
      SET(NL_PC_CFLAGS "-DNL_DEBUG")
    ELSE()
      SET(NL_PC_CFLAGS "-DNL_RELEASE")
    ENDIF()
    IF(WITH_SSE2)
      STRING(APPEND NL_PC_CFLAGS " -DNL_HAS_SSE2")
      IF(WITH_SSE3)
        STRING(APPEND NL_PC_CFLAGS " -DNL_HAS_SSE3")
      ENDIF()
    ENDIF()
    FOREACH(NL_PC_DEFINITION ${NL_PUBLIC_DEFINITIONS})
      STRING(APPEND NL_PC_CFLAGS " -D${NL_PC_DEFINITION}")
    ENDFOREACH()
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

set(project_namespace "${PROJECT_NAME}")
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
# the components are the NeL libraries this build has
set(package_components)
foreach(package_component 3d georges gui ligo logic misc net pacs sevenzip snd_lowlevel sound web)
  if(TARGET NeL::${package_component})
    list(APPEND package_components ${package_component})
  endif()
endforeach()
set(package_required_dependencies LibXml2) # LibXml2 for misc, which links it PUBLIC
if(FREETYPE_FOUND)
  set(package_3d_dependencies Freetype)
endif()
set(package_gui_dependencies CURL Lua Luabind)
set(package_sound_dependencies Vorbis)
set(package_web_dependencies CURL)


set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
write_basic_package_version_file(
        "${version_config}"
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file("../cmake/config.cmake.in"
        "${project_config}"
        INSTALL_DESTINATION "${config_install_dir}"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/internal/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file("../cmake/config-internal.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/internal/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${config_install_dir}"
)

IF(WITH_INSTALL_LIBRARIES)
  install(EXPORT "${project_export_name}"
          COMPONENT libraries
          NAMESPACE "${project_namespace}::"
          DESTINATION "${config_install_dir}"
  )

  install(FILES "${project_config}" "${version_config}"
          COMPONENT libraries
          DESTINATION "${config_install_dir}"
  )
ENDIF()
