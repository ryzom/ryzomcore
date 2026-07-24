FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

# The include-meshes context display shares the shape exporter's evaluation sources
# (compile-in clod reuse; no exporter source modified). lm_scene_build carries the
# scene light decode the paint-light setup rides on.
ADD_EXECUTABLE(zone_painter
	${SRCS}
	${HDRS}
	../pipeline_max_export_shape/scene_lib.cpp
	../pipeline_max_export_shape/scene_lib.h
	../pipeline_max_export_shape/mesh_eval.cpp
	../pipeline_max_export_shape/mesh_eval.h
	../pipeline_max_export_shape/material_build.cpp
	../pipeline_max_export_shape/material_build.h
	../pipeline_max_export_shape/mesh_build.cpp
	../pipeline_max_export_shape/mesh_build.h
	../pipeline_max_export_shape/lm_scene_build.cpp
	../pipeline_max_export_shape/lm_scene_build.h
	)

TARGET_LINK_LIBRARIES(zone_painter
	pipeline_max
	pipeline_max_export_common
	NeL::misc
	NeL::3d
	NeL::gui
	NeL::ligo
)

NL_DEFAULT_PROPS(zone_painter "NeL, Tools, 3D: Zone Painter")
NL_ADD_RUNTIME_FLAGS(zone_painter)

# Interface textures from a ryzomcore_graphics checkout (same NL_GRAPHICS_DIR as nl_sample_gui).
# Packed at build time with build_interface; no texture copies live in this code repo.
SET(NL_GRAPHICS_DIR "${CMAKE_SOURCE_DIR}/../ryzomcore_graphics" CACHE PATH
	"Path to a ryzomcore_graphics checkout (https://github.com/ryzom/ryzomcore_graphics) for sample/tool assets")

SET(ZONE_PAINTER_ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")

IF(CMAKE_CROSSCOMPILING)
	SET(NL_BUILD_INTERFACE_TOOL "NL_BUILD_INTERFACE_TOOL-NOTFOUND" CACHE FILEPATH
		"Path to a host build_interface executable (required when cross-compiling)")
ELSE()
	SET(NL_BUILD_INTERFACE_TOOL build_interface)
ENDIF()

# Color-picker palette lives in v3_fullscreen (same as nl_sample_gui). Stage into an
# extras dir packed into the atlas, and also as a loose file under assets/ so CCtrlColPick
# can sample pixels (getTextureColor) the way the GUI sample does.
SET(ZONE_PAINTER_EXTRA_TEXTURES
	"${NL_GRAPHICS_DIR}/interfaces/v3_fullscreen/color_palette.png")
SET(ZONE_PAINTER_EXTRAS_DIR "${CMAKE_CURRENT_BINARY_DIR}/extra_textures")

# Only wire the atlas pack when the graphics tree is actually present. CI
# configurations that don't stage ryzomcore_graphics still build the binary;
# the atlas is only needed to actually launch the editor with a UI.
IF(EXISTS "${NL_GRAPHICS_DIR}/interfaces/v3_fullscreen/color_palette.png")
	ADD_CUSTOM_COMMAND(
		OUTPUT "${ZONE_PAINTER_ASSETS_DIR}/zone_painter_atlas.png" "${ZONE_PAINTER_ASSETS_DIR}/zone_painter_atlas.txt"
		       "${ZONE_PAINTER_ASSETS_DIR}/color_palette.png"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${ZONE_PAINTER_ASSETS_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${ZONE_PAINTER_EXTRAS_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZONE_PAINTER_EXTRA_TEXTURES} "${ZONE_PAINTER_EXTRAS_DIR}"
		# loose file for pixel access (CCtrlColPick / getTextureColor)
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZONE_PAINTER_EXTRA_TEXTURES} "${ZONE_PAINTER_ASSETS_DIR}"
		COMMAND ${NL_BUILD_INTERFACE_TOOL} -k -f png "${ZONE_PAINTER_ASSETS_DIR}/zone_painter_atlas.png" "${NL_GRAPHICS_DIR}/interfaces/v3" "${ZONE_PAINTER_EXTRAS_DIR}"
		DEPENDS ${NL_BUILD_INTERFACE_TOOL} ${ZONE_PAINTER_EXTRA_TEXTURES}
		COMMENT "Packing zone_painter interface atlas from ${NL_GRAPHICS_DIR}/interfaces/v3 (+ color_palette)")

	ADD_CUSTOM_TARGET(zone_painter_assets DEPENDS
		"${ZONE_PAINTER_ASSETS_DIR}/zone_painter_atlas.png"
		"${ZONE_PAINTER_ASSETS_DIR}/zone_painter_atlas.txt"
		"${ZONE_PAINTER_ASSETS_DIR}/color_palette.png")
	ADD_DEPENDENCIES(zone_painter zone_painter_assets)
ELSE()
	MESSAGE(STATUS "zone_painter: NL_GRAPHICS_DIR (${NL_GRAPHICS_DIR}) not present; skipping atlas pack. The binary will build but has no UI textures at runtime.")
ENDIF()

TARGET_COMPILE_DEFINITIONS(zone_painter PRIVATE
	"ZONE_PAINTER_UI_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/ui\""
	"ZONE_PAINTER_ASSETS_DIR=\"${ZONE_PAINTER_ASSETS_DIR}\""
	"ZONE_PAINTER_FONT=\"${NL_GRAPHICS_DIR}/fonts/ingame.ttf\"")

INSTALL(TARGETS zone_painter RUNTIME DESTINATION bin COMPONENT toolsmisc)
