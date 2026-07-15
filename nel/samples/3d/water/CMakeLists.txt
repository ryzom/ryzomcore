FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_water WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_water NeL::misc NeL::3d)
NL_DEFAULT_PROPS(nl_sample_water "NeL, Samples, 3D: Water")
NL_ADD_RUNTIME_FLAGS(nl_sample_water)

IF(WITH_STATIC_DRIVERS)
  NL_ADD_STATIC_VID_DRIVERS(nl_sample_water)
ENDIF()

IF(EMSCRIPTEN)
  # Assets come from a checkout of the public ryzomcore_graphics repository;
  # no asset copies live in this code repo. Only the files listed below are
  # packed into the .data preload bundle.
  SET(NL_GRAPHICS_DIR "${CMAKE_SOURCE_DIR}/../ryzomcore_graphics" CACHE PATH
    "Path to a ryzomcore_graphics checkout (https://github.com/ryzom/ryzomcore_graphics) for sample assets")

  SET(WATER_SAMPLE_ASSETS
    "landscape/water/shapes/waterbassina01.shape"
    "landscape/water/meshes/lacustre/waterenvmap.png"
    "landscape/water/meshes/lacustre/waterunderenvmap.png"
    "landscape/water/meshes/lacustre/waterdisplace.png"
    "landscape/water/meshes/lacustre/waterbump.png"
    "landscape/water/meshes/lacustre/waterdiffuse.png")

  SET(WATER_PRELOAD_FLAGS "")
  FOREACH(asset ${WATER_SAMPLE_ASSETS})
    IF(EXISTS "${NL_GRAPHICS_DIR}/${asset}")
      GET_FILENAME_COMPONENT(assetName "${asset}" NAME)
      STRING(APPEND WATER_PRELOAD_FLAGS " --preload-file \"${NL_GRAPHICS_DIR}/${asset}@/share/nl_sample_water/${assetName}\"")
    ELSE()
      MESSAGE(WARNING "nl_sample_water: missing asset ${NL_GRAPHICS_DIR}/${asset}; the demo will run without it (set NL_GRAPHICS_DIR)")
    ENDIF()
  ENDFOREACH()

  # HUD font from the font sample (part of the code repo, not a graphics asset)
  STRING(APPEND WATER_PRELOAD_FLAGS " --preload-file \"${CMAKE_SOURCE_DIR}/nel/samples/3d/font/beteckna.ttf@/share/nl_sample_water/beteckna.ttf\"")

  SET_TARGET_PROPERTIES(nl_sample_water PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ASYNCIFY -Wl,--no-gc-sections -Wl,--export=__main_argc_argv -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_FREETYPE=1${WATER_PRELOAD_FLAGS}")

  # Force inclusion of all NeL symbols from static archives so the driver factory works
  TARGET_LINK_OPTIONS(nl_sample_water PRIVATE
    "LINKER:--whole-archive"
    $<TARGET_FILE:nelmisc>
    $<TARGET_FILE:nel3d>
    $<TARGET_FILE:nel_drv_opengles3>
    "LINKER:--no-whole-archive")
ENDIF()

INSTALL(TARGETS nl_sample_water RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
