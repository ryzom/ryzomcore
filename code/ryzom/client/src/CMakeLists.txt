# Need clientsheets lib for sheets packer tool
ADD_SUBDIRECTORY(client_sheets)

IF(WITH_RYZOM_CLIENT)
  # These are Windows/MFC apps
  SET(SEVENZIP_LIBRARY "ryzom_sevenzip")

  ADD_SUBDIRECTORY(seven_zip)

  IF(WITH_RYZOM_PATCH)
    ADD_DEFINITIONS(-DRZ_USE_PATCH)

    IF(WITH_RYZOM_CUSTOM_PATCH_SERVER)
      ADD_DEFINITIONS(-DRZ_USE_CUSTOM_PATCH_SERVER)
    ENDIF()
  ENDIF()

  FILE(GLOB CFG ../*.cfg)
  FILE(GLOB SRC *.cpp *.h motion/*.cpp motion/*.h client.rc)
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
  SOURCE_GROUP("cfg" FILES ${CFG})
  SOURCE_GROUP("interface_v3" FILES ${SRC_INTERFACE})
  SOURCE_GROUP("mode" FILES ${SRC_MODE})
  SOURCE_GROUP("r2" FILES ${SRC_R2})

  # on Mac, create a .App Bundle
  if(APPLE)
    SET(MACOSX_BUNDLE_INFO_STRING "Ryzom Core Client")
    SET(MACOSX_BUNDLE_ICON_FILE "ryzom.icns")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${RYZOM_VERSION})
    SET(MACOSX_BUNDLE_BUNDLE_NAME "Ryzom Core Client")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${RYZOM_VERSION})
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    SET(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})
    SET(RYZOM_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MACOSX_BUNDLE_BUNDLE_NAME}.app)
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

    IF(TARGET crash_report)
      ADD_DEPENDENCIES(ryzom_client crash_report)

      # copy crash_report to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crash_report ${RYZOM_CONTENTS_DIR}/MacOS/CrashReport)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/CrashReport" COMMENT "Signing CrashReport executable...")
      ENDIF()
    ENDIF()

    IF(TARGET ryzom_client_patcher)
      ADD_DEPENDENCIES(ryzom_client ryzom_client_patcher)

      # copy ryzom_client_patcher to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ryzom_client_patcher ${RYZOM_CONTENTS_DIR}/MacOS/RyzomClientPatcher)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/RyzomClientPatcher" COMMENT "Signing RyzomClientPatcher executable...")
      ENDIF()
    ENDIF()

    IF(TARGET ryzom_configuration_qt)
      ADD_DEPENDENCIES(ryzom_client ryzom_configuration_qt)

      # copy ryzom_configuration_qt to bundle
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND cp -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ryzom_configuration_qt ${RYZOM_CONTENTS_DIR}/MacOS/RyzomConfiguration)

      IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
        ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign -fs "${APPLE_CERTIFICATE}" "${RYZOM_CONTENTS_DIR}/MacOS/RyzomConfiguration" COMMENT "Signing RyzomConfiguration executable...")
      ENDIF()
    ENDIF()

    IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
      ADD_CUSTOM_COMMAND(TARGET ryzom_client POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign -fs "${APPLE_CERTIFICATE}" "${RYZOM_OUTPUT_DIR}" COMMENT "Signing Ryzom bundle...")
    ENDIF()
  ENDIF()

  INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBXML2_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
        ${LUABIND_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIR}
  )

  TARGET_LINK_LIBRARIES(ryzom_client
              nelmisc
              nelnet
              nelligo
              nelgeorges
              nel3d
              nelgui
              nelsound
              ryzom_clientsheets
              ryzom_gameshare
              nelpacs
              ${LIBXML2_LIBRARIES}
              ${LUA_LIBRARIES}
              ${LUABIND_LIBRARIES}
              ${CURL_LIBRARIES}
              ${SEVENZIP_LIBRARY}
  )

  IF(NOT APPLE AND NOT WIN32)
    TARGET_LINK_LIBRARIES(ryzom_client ${X11_LIBRARIES})
  ENDIF()

  IF(APPLE)
    FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
    TARGET_LINK_LIBRARIES(ryzom_client ${FOUNDATION_LIBRARY})
  ENDIF()

  ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${CURL_DEFINITIONS} ${LUABIND_DEFINITIONS})

  NL_DEFAULT_PROPS(ryzom_client "Ryzom, Client: Ryzom Core Client")
  NL_ADD_RUNTIME_FLAGS(ryzom_client)

  IF(FINAL_VERSION)
    NL_ADD_LIB_SUFFIX(ryzom_client)
  ELSE()
    IF(WIN32)
      SET_TARGET_PROPERTIES(ryzom_client PROPERTIES DEBUG_POSTFIX "_dev_d" RELEASE_POSTFIX "_dev_r")
    ELSE()
      SET_TARGET_PROPERTIES(ryzom_client PROPERTIES POSTFIX "_dev")
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
    ADD_NATIVE_PRECOMPILED_HEADER(ryzom_client ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
  ENDIF()

  INSTALL(TARGETS ryzom_client RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client BUNDLE DESTINATION /Applications)
ENDIF()
