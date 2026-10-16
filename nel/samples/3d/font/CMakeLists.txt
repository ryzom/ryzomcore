FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_font WIN32 ${SRC})

# TODO: hunter TARGET_LINK_LIBRARIES(nl_sample_font PRIVATE nelmisc nel3d)
IF(EMSCRIPTEN)
  TARGET_COMPILE_DEFINITIONS(nl_sample_font PRIVATE "FONT_DIR=\"/share/nl_sample_font/\"")
  TARGET_LINK_LIBRARIES(nl_sample_font PRIVATE
    -Wl,--whole-archive NeL::misc NeL::3d nel_drv_opengles3 -Wl,--no-whole-archive
  )
  SET_TARGET_PROPERTIES(nl_sample_font PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "-s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s FULL_ES3=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY -Wl,--no-gc-sections -Wl,--export=__main_argc_argv --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/beteckna.ttf@/share/nl_sample_font/beteckna.ttf -sUSE_FREETYPE=1"
  )
ELSE()
  TARGET_COMPILE_DEFINITIONS(nl_sample_font PRIVATE "FONT_DIR=\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_font/\"")
  TARGET_LINK_LIBRARIES(nl_sample_font PRIVATE NeL::misc NeL::3d)
ENDIF()
NL_DEFAULT_PROPS(nl_sample_font "NeL, Samples, 3D: Font")
NL_ADD_RUNTIME_FLAGS(nl_sample_font)

INSTALL(TARGETS nl_sample_font RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
INSTALL(FILES beteckna.ttf DESTINATION ${NL_SHARE_PREFIX}/nl_sample_font COMPONENT samples3d)
