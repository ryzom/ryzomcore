FILE(GLOB SRC *.cpp *.h)

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

NL_TARGET_DRIVER(nel_drv_openal ${SRC})

INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_drv_openal ${OPENAL_LIBRARY} nelmisc nelsnd_lowlevel)
NL_DEFAULT_PROPS(nel_drv_openal "NeL, Driver, Sound: OpenAL")
NL_ADD_LIB_SUFFIX(nel_drv_openal)
NL_ADD_RUNTIME_FLAGS(nel_drv_openal)

IF(WIN32)
  # Find and include EFX-Util on Windows.
  FIND_PACKAGE(EFXUtil)
  INCLUDE_DIRECTORIES(${EFXUTIL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(nel_drv_openal ${EFXUTIL_LIBRARY})
ENDIF(WIN32)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_drv_openal ${CMAKE_CURRENT_SOURCE_DIR}/stdopenal.h ${CMAKE_CURRENT_SOURCE_DIR}/stdopenal.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nel_drv_openal RUNTIME DESTINATION bin LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION lib COMPONENT driverssound)
IF(WITH_MAXPLUGIN)
  INSTALL(TARGETS nel_drv_openal RUNTIME DESTINATION maxplugin COMPONENT driverssound)
ENDIF(WITH_MAXPLUGIN)
