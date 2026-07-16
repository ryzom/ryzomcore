FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_ppl WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_ppl NeL::misc NeL::3d)
NL_DEFAULT_PROPS(nl_sample_ppl "NeL, Samples, 3D: Per-Pixel Lighting")
NL_ADD_RUNTIME_FLAGS(nl_sample_ppl)

IF(WITH_STATIC_DRIVERS)
  NL_ADD_STATIC_VID_DRIVERS(nl_sample_ppl)
ENDIF()

IF(EMSCRIPTEN)
  SET_TARGET_PROPERTIES(nl_sample_ppl PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ASYNCIFY -Wl,--no-gc-sections -Wl,--export=__main_argc_argv")

  TARGET_LINK_OPTIONS(nl_sample_ppl PRIVATE
    "LINKER:--whole-archive"
    $<TARGET_FILE:nelmisc>
    $<TARGET_FILE:nel3d>
    $<TARGET_FILE:nel_drv_opengles3>
    "LINKER:--no-whole-archive")
ENDIF()

INSTALL(TARGETS nl_sample_ppl RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
