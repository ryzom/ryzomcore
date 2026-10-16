FILE(GLOB SRC *.cpp)

# console app: the sample defines main(), not WinMain — the WIN32 keyword made
# MSVC links look for a GUI entry point that doesn't exist
ADD_EXECUTABLE(nl_sample_gui ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_gui NeL::misc NeL::3d NeL::gui)
NL_DEFAULT_PROPS(nl_sample_gui "NeL, Samples: GUI")
NL_ADD_RUNTIME_FLAGS(nl_sample_gui)

IF(WITH_STATIC_DRIVERS)
  NL_ADD_STATIC_VID_DRIVERS(nl_sample_gui)
ENDIF()

# Interface textures come from a checkout of the public ryzomcore_graphics
# repository; no texture copies live in this code repo. The interface atlas is
# packed on the fly at build time with the build_interface tool.
SET(NL_GRAPHICS_DIR "${CMAKE_SOURCE_DIR}/../ryzomcore_graphics" CACHE PATH
  "Path to a ryzomcore_graphics checkout (https://github.com/ryzom/ryzomcore_graphics) for sample assets")

SET(GUI_SAMPLE_ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")

IF(CMAKE_CROSSCOMPILING)
  # build_interface cannot run from this build tree; point at a host build
  SET(NL_BUILD_INTERFACE_TOOL "NL_BUILD_INTERFACE_TOOL-NOTFOUND" CACHE FILEPATH
    "Path to a host build_interface executable (required when cross-compiling)")
ELSE()
  SET(NL_BUILD_INTERFACE_TOOL build_interface)
ENDIF()

# A few textures used by the showcase live outside interfaces/v3 (e.g. the
# color picker palette in v3_fullscreen). Stage those single files into an
# extras directory that is packed into the atlas alongside the v3 set.
SET(GUI_SAMPLE_EXTRA_TEXTURES
  "${NL_GRAPHICS_DIR}/interfaces/v3_fullscreen/color_palette.png")
SET(GUI_SAMPLE_EXTRAS_DIR "${CMAKE_CURRENT_BINARY_DIR}/extra_textures")

# Only wire the atlas pack when the graphics tree is actually present.
# Desktop CI configurations that don't stage ryzomcore_graphics still build
# the binary; the atlas is only needed to actually launch the sample.
IF(EXISTS "${NL_GRAPHICS_DIR}/interfaces/v3_fullscreen/color_palette.png" AND NL_BUILD_INTERFACE_TOOL)
  ADD_CUSTOM_COMMAND(
    OUTPUT "${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.png" "${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.txt"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GUI_SAMPLE_ASSETS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GUI_SAMPLE_EXTRAS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GUI_SAMPLE_EXTRA_TEXTURES} "${GUI_SAMPLE_EXTRAS_DIR}"
    # the color picker reads its palette as a loose file (it needs pixel access)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GUI_SAMPLE_EXTRA_TEXTURES} "${GUI_SAMPLE_ASSETS_DIR}"
    COMMAND ${NL_BUILD_INTERFACE_TOOL} -k -f png "${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.png" "${NL_GRAPHICS_DIR}/interfaces/v3" "${GUI_SAMPLE_EXTRAS_DIR}"
    DEPENDS ${NL_BUILD_INTERFACE_TOOL}
    COMMENT "Packing GUI sample interface atlas from ${NL_GRAPHICS_DIR}/interfaces/v3 (+ staged extras)")

  ADD_CUSTOM_TARGET(nl_sample_gui_assets DEPENDS
    "${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.png"
    "${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.txt")
  ADD_DEPENDENCIES(nl_sample_gui nl_sample_gui_assets)
ELSE()
  MESSAGE(STATUS "nl_sample_gui: NL_GRAPHICS_DIR (${NL_GRAPHICS_DIR}) not present; skipping atlas pack. The binary will build but has no UI textures at runtime.")
ENDIF()

IF(EMSCRIPTEN)
  TARGET_COMPILE_DEFINITIONS(nl_sample_gui PRIVATE
    "GUI_SAMPLE_DATA_DIR=\"/share/nl_sample_gui/data\""
    "GUI_SAMPLE_ASSETS_DIR=\"/share/nl_sample_gui/assets\""
    "GUI_SAMPLE_FONT=\"/share/nl_sample_gui/ingame.ttf\"")

  SET(GUI_PRELOAD_FLAGS "")
  STRING(APPEND GUI_PRELOAD_FLAGS " --preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/data@/share/nl_sample_gui/data\"")
  STRING(APPEND GUI_PRELOAD_FLAGS " --preload-file \"${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.png@/share/nl_sample_gui/assets/gui_sample_atlas.png\"")
  STRING(APPEND GUI_PRELOAD_FLAGS " --preload-file \"${GUI_SAMPLE_ASSETS_DIR}/gui_sample_atlas.txt@/share/nl_sample_gui/assets/gui_sample_atlas.txt\"")
  # loose files read outside the atlas (color picker palette)
  STRING(APPEND GUI_PRELOAD_FLAGS " --preload-file \"${GUI_SAMPLE_ASSETS_DIR}/color_palette.png@/share/nl_sample_gui/assets/color_palette.png\"")
  STRING(APPEND GUI_PRELOAD_FLAGS " --preload-file \"${NL_GRAPHICS_DIR}/fonts/ingame.ttf@/share/nl_sample_gui/ingame.ttf\"")

  SET_TARGET_PROPERTIES(nl_sample_gui PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ASYNCIFY -Wl,--no-gc-sections -Wl,--export=__main_argc_argv -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_FREETYPE=1${GUI_PRELOAD_FLAGS}")

  # Force inclusion of all NeL symbols from static archives so the driver factory works
  TARGET_LINK_OPTIONS(nl_sample_gui PRIVATE
    "LINKER:--whole-archive"
    $<TARGET_FILE:nelmisc>
    $<TARGET_FILE:nel3d>
    $<TARGET_FILE:nelgui>
    $<TARGET_FILE:nel_drv_opengles3>
    "LINKER:--no-whole-archive")
ELSE()
  TARGET_COMPILE_DEFINITIONS(nl_sample_gui PRIVATE
    "GUI_SAMPLE_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\""
    "GUI_SAMPLE_ASSETS_DIR=\"${GUI_SAMPLE_ASSETS_DIR}\""
    "GUI_SAMPLE_FONT=\"${NL_GRAPHICS_DIR}/fonts/ingame.ttf\"")
ENDIF()

INSTALL(TARGETS nl_sample_gui RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesgui)
