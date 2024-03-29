MACRO(NL_CONFIGURE_CHECKS)
  INCLUDE(GetRevision)

  # 3D drivers
  IF(WITH_DRIVER_OPENGL)
    SET(NL_OPENGL_AVAILABLE 1)
  ENDIF()

  IF(WITH_DRIVER_OPENGLES)
    SET(NL_OPENGLES_AVAILABLE 1)
  ENDIF()

  IF(WITH_DRIVER_DIRECT3D)
    SET(NL_DIRECT3D_AVAILABLE 1)
  ENDIF()

  # sound drivers
  IF(WITH_DRIVER_FMOD)
    SET(NL_FMOD_AVAILABLE 1)
  ENDIF()

  IF(WITH_DRIVER_OPENAL)
    SET(NL_OPENAL_AVAILABLE 1)
  ENDIF()

  IF(WITH_DRIVER_DSOUND)
    SET(NL_DSOUND_AVAILABLE 1)
  ENDIF()

  IF(WITH_DRIVER_XAUDIO2)
    SET(NL_XAUDIO2_AVAILABLE 1)
  ENDIF()

  IF(WITH_MSQUIC)
    SET(NL_MSQUIC_AVAILABLE 1)
  ENDIF()

  IF(NOT RYZOM_VERSION_MAJOR)
    SET(RYZOM_VERSION_MAJOR ${NL_VERSION_MAJOR})
    SET(RYZOM_VERSION_MINOR ${NL_VERSION_MINOR})
    SET(RYZOM_VERSION_PATCH ${NL_VERSION_PATCH})
  ENDIF()

  IF(DESCRIBE)
    SET(NL_VERSION "${DESCRIBE}")
  ELSE()
    SET(NL_VERSION "${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}.${NL_VERSION_PATCH}.${REVISION}")
  ENDIF()
  SET(NL_VERSION_RC "${NL_VERSION_MAJOR},${NL_VERSION_MINOR},${NL_VERSION_PATCH},${REVISION}")
  SET(NL_PRODUCT_VERSION "${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}.${NL_VERSION_PATCH}")

  SET(RYZOM_VERSION_SHORT "${RYZOM_VERSION_MAJOR}.${RYZOM_VERSION_MINOR}.${RYZOM_VERSION_PATCH}")
  IF(DESCRIBE)
    SET(RYZOM_VERSION "${DESCRIBE}")
  ELSE()
    SET(RYZOM_VERSION "${RYZOM_VERSION_SHORT}.${REVISION}")
  ENDIF()
  SET(RYZOM_VERSION_RC "${RYZOM_VERSION_MAJOR},${RYZOM_VERSION_MINOR},${RYZOM_VERSION_PATCH},${REVISION}")
  SET(RYZOM_PRODUCT_VERSION "${RYZOM_VERSION_MAJOR}.${RYZOM_VERSION_MINOR}.${RYZOM_VERSION_PATCH}")
  NOW(BUILD_DATE)

  SET(COPYRIGHT "Copyright (C) ${YEAR} ${AUTHOR}")

  IF(NOT RYZOM_CLIENT_ICON)
    SET(RYZOM_CLIENT_ICON "ryzom_client")
  ENDIF()

  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
  ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ENDMACRO()
