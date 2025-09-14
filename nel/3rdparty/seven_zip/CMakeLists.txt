NL_TARGET_LIB(nel_sevenzip ${LIB_SRC})
add_project_library_alias(nel_sevenzip sevenzip)

# TARGET_LINK_LIBRARIES(nel_sevenzip ${PLATFORM_LINKFLAGS})
NL_DEFAULT_PROPS(nel_sevenzip "NeL, 3rd Party: Seven Zip")
NL_ADD_RUNTIME_FLAGS(nel_sevenzip)
NL_ADD_LIB_SUFFIX(nel_sevenzip)

FILE(GLOB LIB_SRC *.c)
LIST(REMOVE_ITEM LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/LzmaUtil.c)
target_sources(nel_sevenzip
        PRIVATE
        ${LIB_SRC}
)

FILE(GLOB LIB_HEADERS *.h)
target_sources(nel_sevenzip
        PUBLIC
        FILE_SET HEADERS
        FILES
        ${LIB_HEADERS}
)

TARGET_COMPILE_DEFINITIONS(nel_sevenzip PRIVATE _7ZIP_ST)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS nel_sevenzip
          EXPORT "${project_export_name}"
          LIBRARY DESTINATION ${NL_LIB_PREFIX}
          ARCHIVE DESTINATION ${NL_LIB_PREFIX}
          COMPONENT libraries
          FILE_SET HEADERS
  )
ENDIF()

IF(WITH_NEL_TOOLS)
  ADD_EXECUTABLE(nellzma ${CMAKE_CURRENT_SOURCE_DIR}/LzmaUtil.c)

  TARGET_LINK_LIBRARIES(nellzma NeL::sevenzip)
  NL_DEFAULT_PROPS(nellzma "NeL, 3rd Party: LZMA")
  NL_ADD_RUNTIME_FLAGS(nellzma)

  INSTALL(TARGETS nellzma RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools)
ENDIF()
