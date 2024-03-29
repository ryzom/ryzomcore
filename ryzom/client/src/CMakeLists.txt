IF(WITH_RYZOM_TOOLS OR WITH_RYZOM_CLIENT)
  # Need clientsheets lib for sheets packer tool
  ADD_SUBDIRECTORY(client_sheets)
ENDIF()

IF(WITH_RYZOM_CLIENT)
  IF(NOT WITH_GUI)
    MESSAGE(FATAL_ERROR "The client cannot be built without the NeL GUI Library (WITH_GUI)")
  ENDIF()

  # Patch should never be enabled on Steam
  IF(WITH_RYZOM_STEAM)
    ADD_DEFINITIONS(-DRZ_USE_STEAM)
    FIND_PACKAGE(Steam)
  ELSEIF(WITH_RYZOM_PATCH)
    ADD_DEFINITIONS(-DRZ_USE_PATCH)

    IF(WITH_RYZOM_CUSTOM_PATCH_SERVER)
      ADD_DEFINITIONS(-DRZ_USE_CUSTOM_PATCH_SERVER)
    ENDIF()
  ENDIF()

  FILE(GLOB CFG ../*.cfg)
  FILE(GLOB SRC *.cpp *.h motion/*.cpp motion/*.h client.rc *.manifest)
  FILE(GLOB SRC_INTERFACE interface_v3/*.h interface_v3/*.cpp)
  FILE(GLOB SRC_MODE motion/modes/*.cpp motion/modes/*.h)
  FILE(GLOB SRC_R2 r2/*.h r2/*.cpp r2/dmc/*.h r2/dmc/*.cpp)

  # Filter out the source files not actually compiled.
  LIST(REMOVE_ITEM SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/animated_scene_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animated_scene_object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animation_fx_sheet.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animation_fx_sheet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d_notes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d_notes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rosace_functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rosace_functions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scene_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene_parser.h)
  LIST(REMOVE_ITEM SRC_MODE
    ${CMAKE_CURRENT_SOURCE_DIR}/motion/modes/free_head.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/motion/modes/free_head.h)
  LIST(REMOVE_ITEM SRC_INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/lua_dll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/lua_dll.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/view_bitmap_progress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface_v3/view_bitmap_progress.h)

  SOURCE_GROUP("" FILES ${SRC})
  SOURCE_GROUP("__cfg" FILES ${CFG})
  SOURCE_GROUP("_interface_v3" FILES ${SRC_INTERFACE})
  SOURCE_GROUP("_motion_modes" FILES ${SRC_MODE})
  SOURCE_GROUP("_r2" FILES ${SRC_R2})

  # VS grouping for convenience
  FILE(GLOB RZCLIENT_NETWORK
    net*.cpp net*.h
    cdb*.cpp cdb*.h
    conn*.cpp conn*.h
    *_decoder.cpp *_decoder.h
    login.cpp login.h
    login_*.cpp login_*.h
    ping.cpp ping.h
    perm*.cpp perm*.h
    session_*.cpp session_*.h
    steam_client.cpp steam_client.h
    string_manager_client.cpp string_manager_client.h
    *_connection.cpp *_connection.h
    *download*.cpp *download*.h
  )
  SOURCE_GROUP("network" FILES ${RZCLIENT_NETWORK})
  FILE(GLOB RZCLIENT_GLOBALS
    *_database_*.cpp *_database_*.h
    global.cpp global.h
  )
  SOURCE_GROUP("globals" FILES ${RZCLIENT_GLOBALS})
  FILE(GLOB RZCLIENT_ANIMATION
    animation.cpp animation.h
    animation_*.cpp animation_*.h
  )
  SOURCE_GROUP("animation" FILES ${RZCLIENT_ANIMATION})
  FILE(GLOB RZCLIENT_FX
    fx*.cpp fx*.h
    *_fx*.cpp *_fx*.h
    projectile_*.cpp projectile_*.h
  )
  SOURCE_GROUP("fx" FILES ${RZCLIENT_FX})
  FILE(GLOB RZCLIENT_WEATHER
    sky.cpp sky.h
    sky_*.cpp sky_*.h
    weather.cpp weather.h
    weather_*.cpp weather_*.h
    precipitation.cpp precipitation.h
    precipitation_*.cpp precipitation_*.h
    light_cycle_*.cpp light_cycle_*.h
  )
  SOURCE_GROUP("weather" FILES ${RZCLIENT_WEATHER})
  FILE(GLOB RZCLIENT_OUTPOSTS
    outpost.cpp outpost.h
    outpost_*.cpp outpost_*.h
  )
  SOURCE_GROUP("outposts" FILES ${RZCLIENT_OUTPOSTS})
  FILE(GLOB RZCLIENT_DECALS
    decal.cpp decal.h
    decal*.cpp decal_*.h
  )
  SOURCE_GROUP("decals" FILES ${RZCLIENT_DECALS})

  # on Mac, create a .App Bundle
  if(APPLE)
    # We could force another build version for the bundle
    IF(NOT RYZOM_BUILD)
      SET(RYZOM_BUILD ${RYZOM_VERSION_SHORT})
    ENDIF()

    SET(MACOSX_BUNDLE_INFO_STRING "Ryzom Core 4")
    SET(MACOSX_BUNDLE_ICON_FILE "ryzom.icns")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "dev.ryzom.core4")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${RYZOM_VERSION})
    SET(MACOSX_BUNDLE_BUNDLE_NAME "Ryzom Core 4")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${RYZOM_VERSION_SHORT})
    SET(MACOSX_BUNDLE_BUNDLE_VERSION ${RYZOM_BUILD})
    SET(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})
    # RYZOM_OUTPUT_DIR == build.xcode/Debug/Ryzom Core4.app/Contents/MacOS
    SET(RYZOM_OUTPUT_DIR $<TARGET_FILE_DIR:ryzom_client>/../..)
    SET(RYZOM_CONTENTS_DIR ${RYZOM_OUTPUT_DIR}/Contents)
    SET(RYZOM_RESOURCES_DIR ${RYZOM_CONTENTS_DIR}/Resources)
    SET(MAC_RESOURCES_DIR ${CMAKE_SOURCE_DIR}/ryzom/client/macosx)
  ENDIF()

  ADD_EXECUTABLE(ryzom_client WIN32 MACOSX_BUNDLE ${SRC} ${SRC_INTERFACE} ${SRC_MODE} ${SRC_R2})

  IF(APPLE)
    SET_TARGET_PROPERTIES(ryzom_client PROPERTIES OUTPUT_NAME ${MACOSX_BUNDLE_BUNDLE_NAME})
    SET_TARGET_PROPERTIES(ryzom_client PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${MAC_RESOURCES_DIR}/Info.plist)
    ADD_CUSTOM_COMMAND(TARGET ryzom_client PRE_BUILD COMMAND mkdir -p ${RYZOM_RESOURCES_DIR})
    ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp ARGS -p ${MAC_RESOURCES_DIR}/PkgInfo ${RYZOM_CONTENTS_DIR})
    ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp ARGS -p ${MAC_RESOURCES_DIR}/ryzom.icns ${RYZOM_RESOURCES_DIR})
    ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp ARGS -p ${CMAKE_SOURCE_DIR}/ryzom/client/client_default.cfg ${RYZOM_RESOURCES_DIR})

    # remove any present installscript_osx.vdf before signing
    ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND rm -f ${RYZOM_OUTPUT_DIR}/installscript_osx.vdf)

    IF(RYZOM_DATA_DIR)
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp ARGS -RpX ${RYZOM_DATA_DIR} ${RYZOM_RESOURCES_DIR})
    ENDIF()

    IF(WITH_RYZOM_SANDBOX)
      SET(ENTITLEMENTS --entitlements ${MAC_RESOURCES_DIR}/ryzom.entitlements)
    ELSE()
      SET(ENTITLEMENTS)
    ENDIF()

    IF(TARGET crash_report)
      ADD_DEPENDENCIES(ryzom_client crash_report)

      # copy crash_report to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p $<TARGET_FILE:crash_report> ${RYZOM_CONTENTS_DIR}/MacOS/CrashReport)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign ${ENTITLEMENTS} -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/CrashReport" COMMENT "Signing CrashReport executable...")
      ENDIF()
    ENDIF()

    IF(TARGET ryzom_client_patcher)
      ADD_DEPENDENCIES(ryzom_client ryzom_client_patcher)

      # copy ryzom_client_patcher to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p $<TARGET_FILE:ryzom_client_patcher> ${RYZOM_CONTENTS_DIR}/MacOS/RyzomClientPatcher)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign ${ENTITLEMENTS} -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/RyzomClientPatcher" COMMENT "Signing RyzomClientPatcher executable...")
      ENDIF()
    ENDIF()

    IF(TARGET ryzom_configuration_qt)
      ADD_DEPENDENCIES(ryzom_client ryzom_configuration_qt)

      # copy ryzom_configuration_qt to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p $<TARGET_FILE:ryzom_configuration_qt> ${RYZOM_CONTENTS_DIR}/MacOS/RyzomConfiguration)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign ${ENTITLEMENTS} -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/RyzomConfiguration" COMMENT "Signing RyzomConfiguration executable...")
      ENDIF()
    ENDIF()

    IF(WITH_RYZOM_STEAM AND STEAM_RUNTIME)
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p ${STEAM_RUNTIME} ${RYZOM_CONTENTS_DIR}/MacOS)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign --no-strict -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/${STEAM_RUNTIMENAME}" COMMENT "Signing Steam client runtime...")
      ENDIF()
    ENDIF()

    IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign ${ENTITLEMENTS} -fs "${APPLE_CERTIFICATE}" "${RYZOM_OUTPUT_DIR}" COMMENT "Signing Ryzom bundle...")
    ENDIF()
  ENDIF()

  INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
        ${LUABIND_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
  )

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/nel/3rdparty)

  IF(STEAM_FOUND)
    INCLUDE_DIRECTORIES(${STEAM_INCLUDE_DIRS})
  ENDIF()

  TARGET_LINK_LIBRARIES(ryzom_client
              nelmisc
              nelnet
              nelligo
              nelgeorges
              nel3d
              nelgui
              nelsound
              nel_sevenzip
              ryzom_clientsheets
              ryzom_gameshare
              nelpacs
              ${LUA_LIBRARIES}
              ${LUABIND_LIBRARIES}
              ${CURL_LIBRARIES}
              ${OPENSSL_LIBRARIES}
  )

  IF(NOT APPLE AND NOT WIN32)
    TARGET_LINK_LIBRARIES(ryzom_client ${X11_LIBRARIES})
  ENDIF()

  IF(APPLE)
    TARGET_LINK_LIBRARIES(ryzom_client ${FOUNDATION_FRAMEWORK})
  ENDIF()

  IF(WITH_MSQUIC)
    INCLUDE_DIRECTORIES(${msquic_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(ryzom_client ${msquic_LIBRARIES})
  ENDIF()

  ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${CURL_DEFINITIONS} ${LUABIND_DEFINITIONS})

  NL_DEFAULT_PROPS(ryzom_client "Ryzom, Client: Ryzom Core 4")
  NL_ADD_RUNTIME_FLAGS(ryzom_client)

  IF(FINAL_VERSION)
    NL_ADD_LIB_SUFFIX(ryzom_client)
  ELSE()
    IF(WIN32)
      SET_TARGET_PROPERTIES(ryzom_client PROPERTIES DEBUG_POSTFIX "_dev_d" RELEASE_POSTFIX "_dev_r")
    ELSEIF(NOT APPLE)
      SET_TARGET_PROPERTIES(ryzom_client PROPERTIES DEBUG_POSTFIX "_dev" RELEASE_POSTFIX "_dev")
    ENDIF()
  ENDIF()

  IF(WITH_RYZOM_CLIENT_UAC)
    IF(CMAKE_GENERATOR MATCHES "Visual Studio")
      SET_PROPERTY(TARGET ryzom_client PROPERTY LINK_FLAGS_RELEASE "/level='requireAdministrator'")
    ELSEIF(CMAKE_GENERATOR MATCHES "NMake Makefiles")
      SET_PROPERTY(TARGET ryzom_client PROPERTY LINK_FLAGS_RELEASE "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
    ENDIF()
  ENDIF()

  IF(WITH_PCH AND (NOT MINGW OR NOT WITH_SYMBOLS))
    target_precompile_headers(ryzom_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
  ENDIF()

  INSTALL(TARGETS ryzom_client RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client BUNDLE DESTINATION /Applications)
ENDIF()
