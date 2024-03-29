FILE(GLOB SRC *.cpp *.h *.def *.rc)

IF(WIN32)
  SET(NLDRV_AL_LIB "nel_drv_openal_win")
ELSE()
  SET(NLDRV_AL_LIB "nel_drv_openal")
ENDIF()

SOURCE_GROUP(efx FILES
        effect_al.cpp
        effect_al.h)

SOURCE_GROUP(sfx FILES
        buffer_al.cpp
        buffer_al.h
        listener_al.cpp
        listener_al.h
        music_channel_al.cpp
        music_channel_al.h
        source_al.cpp
        source_al.h)

SOURCE_GROUP(util FILES
        ext_al.cpp
        ext_al.h)

NL_TARGET_DRIVER(${NLDRV_AL_LIB} ${SRC})

IF(HUNTER_ENABLED)
  HUNTER_ADD_PACKAGE(OpenAL)
  FIND_PACKAGE(OpenAL CONFIG REQUIRED)
  SET(OPENAL_LIBRARY OpenAL::OpenAL)
  ADD_DEFINITIONS(-DAL_LIBTYPE_STATIC)
ELSE()
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR}/AL)
ENDIF()# hunter

TARGET_LINK_LIBRARIES(${NLDRV_AL_LIB} nelmisc nelsnd_lowlevel ${OPENAL_LIBRARY})
NL_DEFAULT_PROPS(${NLDRV_AL_LIB} "NeL, Driver, Sound: OpenAL")
NL_ADD_RUNTIME_FLAGS(${NLDRV_AL_LIB})
NL_ADD_LIB_SUFFIX(${NLDRV_AL_LIB})

IF(WIN32)
  # Find and include EFX-Util on Windows.
  FIND_PACKAGE(EFXUtil)
  IF(EFXUTIL_FOUND)
    INCLUDE_DIRECTORIES(${EFXUTIL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${NLDRV_AL_LIB} ${EFXUTIL_LIBRARY})
    ADD_DEFINITIONS(-DEFX_CREATIVE_AVAILABLE=1)
  ENDIF()
ENDIF()

IF(WITH_PCH)
  target_precompile_headers(${NLDRV_AL_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdopenal.h)
ENDIF()

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC_DRIVERS) OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS ${NLDRV_AL_LIB} RUNTIME DESTINATION ${NL_DRIVER_PREFIX} LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT driverssound)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS ${NLDRV_AL_LIB} RUNTIME DESTINATION maxplugin COMPONENT driverssound)
  ENDIF()
ENDIF()
