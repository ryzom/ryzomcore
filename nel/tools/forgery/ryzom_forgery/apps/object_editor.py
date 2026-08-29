"""Ryzom Forgery object editor: browse, inspect and edit .shape files.

3D display is supported for CMesh, CMeshMRM (finest LOD) and CMeshMultiLod
(slot 0, whose geometry is itself a CMesh/CMeshMRM). Other shape types
(skeleton, water, flare, particles, ...) show their properties only, no 3D
render yet.
"""

import io
import shutil
import subprocess
import threading
from datetime import datetime
from math import ceil, cos, pi, radians, sin
from pathlib import Path

import numpy

from panda3d.core import (
	AlphaTestAttrib, ClockObject, ColorBlendAttrib, Geom, GeomNode, GeomTriangles, GeomVertexData,
	GeomVertexFormat, GeomVertexWriter, InternalName, LineSegs, Material as PandaMaterial, NodePath, PNMImage, Point3,
	Quat, Texture as PandaTexture, TextureStage, TransformState, TransparencyAttrib, Vec3,
)

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx, portable_file_dialogs as pfd

from ryzom_forgery.app import _AVAILABLE_FONTS, ForgeryApp
from ryzom_forgery.camera import ObjectManipulator, OrbitCamera
from ryzom_forgery import dds_export
from ryzom_forgery.explorer import ExplorerItem
from ryzom_forgery.export_dialog import ExportDialog
from ryzom_forgery.import_dialog import ImportDialog
from ryzom_forgery.import_watcher import ImportWatcher
from ryzom_forgery.material_docs import load_material_docs
from ryzom_forgery.navcube import NavigationCube
from ryzom_forgery import panoply
from ryzom_forgery import panoply_bake
from ryzom_forgery import panoply_colorize
from ryzom_forgery import panoply_config
from ryzom_forgery import panoply_live
from ryzom_forgery import panoply_texture
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.search_paths_dialog import SearchPathsDialog
from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.shape_geometry import (
	IDENTITY_QUAT, compose_uv_matrix, decompose_uv_matrix, finest_skinned_lod, iter_render_passes,
	load_panda_texture, resolve_texture_ref, rgba_to_color, shape_bbox, shape_geom, solid_color_texture,
	texture_to_pnm_image, uv_matrix_to_panda_mat4,
)
from ryzom_forgery.shape_import import texture_search_dirs_for
from ryzom_forgery.tex_dds_sync import TexDdsSyncWatcher
from ryzom_forgery.workspace_setup_dialog import WorkspaceSetupDialog, _truncate_path_to_width
from ryzom_forgery.workspace_sync import WorkspaceSyncWatcher, SYNCED_SUBDIRS
from ryzom_forgery.workspace_watch import WorkspaceWatcher
from ryzom_forgery.workspaces import SUBDIRS as WORKSPACE_SUBDIRS, ensure_structure, reveal_in_system_file_manager

from pynel.ryzom_animation import (
	AnimationParseError, animation_duration, evaluate_all_bone_world_matrices, parse_animation,
)
from pynel.ryzom_shape import (
	Matrix, MeshMRMSkinned, Quaternion, Rgba, ShapeFile, ShapeParseError, ShapeWriteError, SkeletonShape, Texture,
	WindTreeParams, parse_shape, save_shape,
)
from pynel.ryzom_skin import bone_skin_matrices_for_mesh
from pynel import repository_paths

# Shape types pynel's save_shape() can actually write back out -- matches
# ryzom_shape.py's _SHAPE_CLASS_NAMES, the Save UI only shows for these.
_WRITABLE_SHAPE_TYPES = {"Mesh", "MeshMRM", "MeshMRMSkinned", "MeshMultiLod"}

# Toggleable scale-reference shapes shown alongside whatever's loaded, for an
# at-a-glance sense of scale -- a 1x1x1 cube and the shortest/tallest playable
# character shapes, kept in this repo (not the Ryzom data tree) since they're
# tool fixtures, not game assets.
_REFERENCE_EXAMPLES_DIR = Path(__file__).resolve().parent.parent / "examples"
_REFERENCE_SHAPES = [
	("Cube (1x1x1)", "ge_mission_1_caisse.shape"),
	("Smallest character", "npc_dummy_short.shape"),
	("Tallest character", "npc_dummy_tall.shape"),
]
_REFERENCE_GAP = 1.5  # meters between reference objects (and the main shape), beyond their own bbox width

# Icon for each toggle in the top-left viewport button bar (see
# _draw_reference_shapes_toggles()) -- these read as "important, chunky
# toggles" next to the smaller bottom-left _draw_viewport_toggles() bar, so
# they're drawn with app.large_icon_font (2x _ICON_FONT_SIZE, see app.py's
# _load_icon_font()) instead of the normal merged-into-text-font icon glyphs.
_REFERENCE_ICONS = {"Cube (1x1x1)": fa_icons.ICON_FA_CUBE, "Smallest character": fa_icons.ICON_FA_CHILD,
                     "Tallest character": fa_icons.ICON_FA_MALE}

_OVERWRITE_POPUP_ID = "Overwrite shape?"
_REPLACE_MATCH_POPUP_ID = "Match materials"
_IMPORT_CONFLICT_POPUP_ID = "Shape open, about to auto-update"
_RESTORE_SCAN_POPUP_ID = "Scanning assets"
_BAKE_PROGRESS_POPUP_ID = "Baking Panoply variants"

_STATUS_HINT_COLOR = (1.0, 0.6, 0.15, 1.0)  # orange, for material_options.md hints shown in the status bar
_TEXTURE_NORMAL_COLOR = (1.0, 1.0, 1.0, 1.0)
_TEXTURE_IN_WORKSPACE_COLOR = (0.4, 1.0, 0.4, 1.0)  # green -- this texture reference already lives in the active workspace's tex/
# Same 3 extensions the texture browse dialog offers (_start_texture_browse()).
_WORKSPACE_TEXTURE_EXTENSIONS = {".tga", ".dds", ".png"}

# draw_panel()'s tab bar (_push_tab_color()).
_TAB_COLOR_TEXTURES = (0.565, 0.933, 0.565, 1.0)  # lightgreen
_TAB_COLOR_MATERIALS = (0.878, 1.0, 1.0, 1.0)  # lightcyan
_TAB_COLOR_ALL_PROPERTIES = (0.5, 0.5, 0.5, 1.0)  # gray
_TAB_COLOR_SETTINGS = (0.8, 0.75, 0.15, 1.0)  # yellow

_SYNC_NOW_COLOR = (0.85, 0.55, 0.15, 1.0)  # orange -- _draw_workspace_sync_settings()'s catch-up button

# _draw_import_conflict_popup()'s 4 choice buttons, color-coded by how
# "safe" each one is with the in-memory edits (green: nothing lost, orange:
# both kept but in 2 files, pink: current edits discarded, yellow: no-op).
_IMPORT_CONFLICT_SAVE_COLOR = (0.85, 0.55, 0.15, 1.0)  # orange
_IMPORT_CONFLICT_DISCARD_COLOR = (0.85, 0.35, 0.55, 1.0)  # pink
_IMPORT_CONFLICT_BACKUP_COLOR = (0.35, 0.75, 0.35, 1.0)  # green
_IMPORT_CONFLICT_CANCEL_COLOR = (0.8, 0.75, 0.15, 1.0)  # yellow

# ImportWatcher outcome messages on the sysinfo status bar (see
# _flush_pending_import_status()) -- same red as self.shape_error elsewhere
# in this file, same green as _IMPORT_CONFLICT_BACKUP_COLOR above.
_IMPORT_STATUS_ERROR_COLOR = (1.0, 0.4, 0.4, 1.0)  # red
_IMPORT_STATUS_SUCCESS_COLOR = (0.35, 0.75, 0.35, 1.0)  # green

# _draw_bottom_bar()'s Save/Export/Quit buttons -- blue for the export
# buttons (not pink, to stay visually distinct from Quit's pink).
_SAVE_BUTTON_COLOR = (0.6, 0.85, 0.65, 1.0)  # pastel green
_EXPORT_AS_BUTTON_COLOR = (0.65, 0.8, 0.95, 1.0)  # light blue
_QUICK_EXPORT_BUTTON_COLOR = (0.4, 0.65, 0.9, 1.0)  # blue -- workspace-only quick export
_QUIT_BUTTON_COLOR = (0.9, 0.55, 0.7, 1.0)  # pink

_COLOR_POPUP_ID = "material-color-picker"
_DIFFUSE_COLOR_POPUP_ID = "material-diffuse-picker"

# CMaterial flag/enum values (nel/include/nel/3d/material.h), needed to render
# translucent materials (e.g. glass) correctly instead of opaque.
_IDRV_MAT_ZWRITE = 0x00000004
_IDRV_MAT_BLEND = 0x00000080
_IDRV_MAT_DOUBLE_SIDED = 0x00000100
_IDRV_MAT_ALPHA_TEST = 0x00000200
_IDRV_MAT_TEX_ADDR = 0x00000400
# Per-stage "this stage's tex_user_mat is enabled" bit -- material.cpp's
# CMaterial::enableUserTexMat()/getFlags(): stage 0 is bit 0x00100000.
_IDRV_MAT_USER_TEX_MAT_STAGE0 = 0x00100000

# CMaterial::TBlend (material.h) -> panda3d.core.ColorBlendAttrib.Operand, in
# TBlend's own declaration order so the enum's int value is the list index.
_TBLEND_TO_PANDA_OPERAND = [
	ColorBlendAttrib.O_one,
	ColorBlendAttrib.O_zero,
	ColorBlendAttrib.O_incoming_alpha,
	ColorBlendAttrib.O_one_minus_incoming_alpha,
	ColorBlendAttrib.O_incoming_color,
	ColorBlendAttrib.O_one_minus_incoming_color,
	ColorBlendAttrib.O_constant_color,
	ColorBlendAttrib.O_one_minus_constant_color,
	ColorBlendAttrib.O_constant_alpha,
	ColorBlendAttrib.O_one_minus_constant_alpha,
]

# CMaterial::TBlend's own member names (material.h), same order as
# _TBLEND_TO_PANDA_OPERAND above -- for the Src/Dst Blend combo boxes in the
# Materials tab's Transparency section.
_TBLEND_NAMES = [
	"one", "zero", "srcalpha", "invsrcalpha", "srccolor", "invsrccolor",
	"blendConstantColor", "blendConstantInvColor", "blendConstantAlpha", "blendConstantInvAlpha",
]

# _draw_transform_row()'s per-property drag_float() speed/range -- position
# in meters (no hard limit needed for a 3D scene), rotation in degrees (a
# full turn either way), scale as a multiplier (must stay positive).
_TRANSFORM_DRAG_PARAMS = {
	"position": (0.005, -10000.0, 10000.0),
	"rotation": (0.5, -360.0, 360.0),
	"scale": (0.005, 0.001, 1000.0),
}

# Texture sampling params (docs/material_options.md's "Filtrage et
# répétition des textures" section) -- see texture.h's TWrapMode/TMagFilter/
# TMinFilter for the enum values these index into.
_TEXTURE_WRAP_NAMES = ["Repeat", "Clamp"]
_TEXTURE_MAG_FILTER_NAMES = ["Nearest (pixelated)", "Linear (smooth)"]
_TEXTURE_MIN_FILTER_NAMES = [
	"Nearest, no mipmap", "Nearest, mipmap nearest", "Nearest, mipmap linear",
	"Linear, no mipmap", "Linear, mipmap nearest", "Linear, mipmap linear (smoothest)",
]
# Engine construction defaults (texture.h) -- shown in the combo for a
# freshly imported texture's None fields (no real value captured yet) until
# the user actually picks something, at which point a concrete value is
# written and the None/heuristic-fallback stops applying (see
# shape_geometry.py's load_panda_texture()).
_TEXTURE_WRAP_DEFAULT = 0  # Repeat
_TEXTURE_MAG_FILTER_DEFAULT = 1  # Linear
_TEXTURE_MIN_FILTER_DEFAULT = 5  # LinearMipMapLinear

# Src/Dst Blend presets (docs/material_options.md's "Mélange" section
# promises these two shortcuts, pre-filling the raw src/dst combos with the
# most common values -- classic alpha blending vs. additive/glow effects).
_TBLEND_PRESET_ALPHA = (2, 3)  # srcalpha, invsrcalpha
_TBLEND_PRESET_ADDITIVE = (0, 0)  # one, one

# Opaque backdrop drawn behind texture preview thumbnails/hover-zooms (see
# ObjectEditorApp._draw_image_opaque_bg()) -- most textures are mostly
# transparent (e.g. a shape cut from a square atlas), which otherwise blends
# into the panel's own background and makes the preview hard to read.
_PREVIEW_BG_COLOR = (0.0, 0.0, 0.0, 1.0)

# Multi Bitmap slot index -> (quality label, ecosystem label, season label),
# the three known Georges/engine conventions documented in
# docs/material_options.md (item_map.typ's map_variant, _creature_texture.typ,
# and EGSPD::CSeason in ryzom/common/src/game_share/season.h) -- which one
# actually applies depends on the shape, so all three are always shown together.
_MULTI_BITMAP_SLOT_LABELS = [
	("Low Quality", "Forest", "Spring"),  # "Forest": labelled "none" in _creature_texture.typ, but means Forest in practice
	("Medium Quality", "Lacustre", "Summer"),
	("High Quality", "Desert", "Autumn"),
	("Super Quality", "Jungle", "Winter"),
	("XL Quality", "Primr", None),
	("Suprem Quality", "goo", None),
	("Divine Quality", None, None),
	("Obiwan Quality", None, None),
]


_ACTIVE_COLOR = (0.26, 0.59, 0.98, 0.8)  # blue -- default "on" highlight
_LOCKED_COLOR = (0.45, 0.45, 0.45, 0.8)  # grey -- "on" highlight for a lock toggle specifically
_COMPATIBLE_COLOR = (0.35, 0.85, 0.35, 1.0)  # green -- "this .skel matches the loaded shape's bones"


def _icon_button(icon, tooltip, active=False, square=False, large_font=None, active_color=_ACTIVE_COLOR, disabled=False):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory. `active` highlights it (toggle-button style, in
	`active_color`) when the feature it controls is currently on. `square`
	sizes it to the current font's frame height on both axes, so a row of
	these all matches -- without it, imgui.button()'s default auto-size
	makes each button only as wide as its own glyph, which visibly varies
	between Font Awesome icons. `large_font`, if given, is an (ImFont, size)
	pair (app.large_icon_font, app.large_icon_font_size) pushed around just
	the button glyph itself -- NOT the tooltip below, which needs the normal
	text font: that font is icon-glyphs-only, so a tooltip drawn under it
	renders as blank/invisible text instead of readable words."""
	if active:
		imgui.push_style_color(imgui.Col_.button.value, active_color)
	if large_font is not None:
		imgui.push_font(*large_font)
	imgui.begin_disabled(disabled)
	size = (imgui.get_frame_height(), imgui.get_frame_height()) if square else (0, 0)
	clicked = imgui.button(icon, size)
	imgui.end_disabled()
	if large_font is not None:
		imgui.pop_font()
	if active:
		imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


def _center_next_widget(width):
	"""Shifts the cursor so a `width`-wide widget drawn right after this call
	lands horizontally centered in the current window's content region --
	e.g. _draw_import_conflict_popup()'s buttons, each a different width
	(auto-sized to its own label) but all centered under each other. A no-op
	if `width` is already at least as wide as the available space."""
	avail = imgui.get_content_region_avail().x
	if avail > width:
		imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + (avail - width) / 2)


def _colored_button(label, color):
	"""A plain text button (unlike _icon_button above) tinted `color`, with
	lighter/darker hover/active variants derived from it -- see
	_draw_import_conflict_popup()'s 4 choices, color-coded by how "safe"
	each one is."""
	r, g, b, a = color
	imgui.push_style_color(imgui.Col_.button.value, color)
	imgui.push_style_color(imgui.Col_.button_hovered.value, (min(r + 0.1, 1.0), min(g + 0.1, 1.0), min(b + 0.1, 1.0), a))
	imgui.push_style_color(imgui.Col_.button_active.value, (max(r - 0.1, 0.0), max(g - 0.1, 0.0), max(b - 0.1, 0.0), a))
	imgui.push_style_color(imgui.Col_.text.value, (0.0, 0.0, 0.0, 1.0))
	clicked = imgui.button(label)
	imgui.pop_style_color(4)
	return clicked


def _push_tab_color(color):
	"""Tints one panel tab (Textures/Materials/All Properties/Settings --
	see draw_panel()) so each is visually distinct at a glance instead of
	every tab looking alike. Same lighter/darker-variant idea as
	_colored_button() above, just for Col_.tab* instead of Col_.button*: the
	unselected tab itself a bit darker than `color`, hover a bit lighter,
	the selected/active tab exactly `color`."""
	r, g, b, a = color
	imgui.push_style_color(imgui.Col_.tab.value, (max(r - 0.15, 0.0), max(g - 0.15, 0.0), max(b - 0.15, 0.0), a))
	imgui.push_style_color(imgui.Col_.tab_hovered.value, (min(r + 0.1, 1.0), min(g + 0.1, 1.0), min(b + 0.1, 1.0), a))
	imgui.push_style_color(imgui.Col_.tab_selected.value, color)


def _pop_tab_color():
	imgui.pop_style_color(3)


def _begin_tab_item_with_icon(icon, label, flags=0):
	"""Same as imgui.begin_tab_item_simple(label), just icon-only (no
	visible text -- `label` only lives in the hidden ##id part and a hover
	tooltip) with the icon glyph itself forced black, readable against the
	light background colors _push_tab_color() gives each tab (see
	draw_panel()'s tab bar). The black text push/pop is scoped to only the
	begin_tab_item_simple call itself (the tab header, drawn immediately
	regardless of whether it's the active tab) -- popped before the
	tooltip, so that stays whatever color tooltips normally are, and before
	a tab's own content too, so that isn't forced black along with it.

	`flags` (added 2026-08-29) forwards to begin_tab_item_simple as-is --
	needed so ObjectEditorApp._consume_settings_tab_flags() can force the
	Settings tab selected for a pending request_settings_attention()."""
	imgui.push_style_color(imgui.Col_.text.value, (0.0, 0.0, 0.0, 1.0))
	opened = imgui.begin_tab_item_simple(f"{icon}##{label}", flags)
	imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(label)
	return opened


# Viewport helper toggles (floor grid, world/pivot axes) drawn bottom-left of
# the 3D viewport -- see ObjectEditorApp._draw_viewport_toggles(). Grid squares
# are 1x1m, matching how shapes are actually scaled in-game. Both the grid's
# footprint and the axes' length are re-derived from the loaded shape's own
# bounding box each time geometry is (re)built (see _rebuild_viewport_helpers()),
# so they scale to cover whatever object is on screen instead of a fixed size
# that's too small for a building and absurdly oversized for a trinket. The
# constants below are only the fallback used before any shape is loaded.
_GRID_SQUARES = 10
_GRID_STEP = 1.0
_GRID_COLOR = (0.5, 0.5, 0.5, 1.0)
_GRID_MARGIN_SQUARES = 2  # extra squares of margin around the bbox footprint
_MIN_GRID_SQUARES = 4

# World axes use the conventional red/green/blue X/Y/Z scheme (matches
# navcube.py's own gizmo); pivot axes deliberately use a different palette so
# both can be shown together without one being mistaken for the other.
_WORLD_AXIS_COLORS = {"x": (0.95, 0.25, 0.25, 1.0), "y": (0.3, 0.85, 0.3, 1.0), "z": (0.3, 0.55, 1.0, 1.0)}
_PIVOT_AXIS_COLORS = {"x": (0.95, 0.2, 0.85, 1.0), "y": (0.95, 0.85, 0.15, 1.0), "z": (0.2, 0.9, 0.9, 1.0)}
_AXIS_VECTORS = {"x": (1.0, 0.0, 0.0), "y": (0.0, 1.0, 0.0), "z": (0.0, 0.0, 1.0)}
_AXIS_LENGTH = 3.0  # fallback, before any shape is loaded
_AXIS_MARGIN_FACTOR = 1.2  # how far axes extend past the bbox radius

_VIEWPORT_TOGGLE_MARGIN_PX = 10
_OBJECT_TRANSPARENCY_ALPHA = 0.5


def _build_grid_geom(center_x, center_y, z, squares):
	"""LineSegs floor grid on the world XY plane at height `z`, centered on
	(center_x, center_y): `squares` x `squares` squares, _GRID_STEP meters each."""
	half = squares * _GRID_STEP / 2.0
	lines = LineSegs("floor-grid")
	lines.set_color(*_GRID_COLOR)
	for i in range(squares + 1):
		coord = -half + i * _GRID_STEP
		lines.move_to(center_x + coord, center_y - half, z)
		lines.draw_to(center_x + coord, center_y + half, z)
		lines.move_to(center_x - half, center_y + coord, z)
		lines.draw_to(center_x + half, center_y + coord, z)
	return lines.create()


def _build_axes_geom(colors, length):
	"""LineSegs X/Y/Z axes through the origin, each spanning +/-length along
	its axis, colored per `colors` (one of _WORLD_AXIS_COLORS/_PIVOT_AXIS_COLORS)."""
	lines = LineSegs("axes")
	for axis, vector in _AXIS_VECTORS.items():
		lines.set_color(*colors[axis])
		lines.move_to(-vector[0] * length, -vector[1] * length, -vector[2] * length)
		lines.draw_to(vector[0] * length, vector[1] * length, vector[2] * length)
	return lines.create()


class _WindState:
	"""Per-loaded-shape data for the live wind preview (see
	ObjectEditorApp._update_wind()) -- built once in _rebuild_geometry() from
	the shape's WindTreeParams + PrimaryColor vertex channel, then read every
	frame by the animation task. `vdata` (the shared GeomVertexData every
	render pass's Geom points at, see _build_vertex_data()) is filled in by
	the caller once it's built."""

	def __init__(self, params, base_positions, factors, idx2, idx3):
		self.params = params
		self.base_positions = base_positions  # (N,3) float32
		self.factors = factors  # (N,3) float32, level-0/1/2 blend weight per vertex
		self.idx2 = idx2  # (N,) int, which of the 4 level-1 phase branches each vertex follows
		self.idx3 = idx3  # (N,) int, same for level-2
		self.current_time = [0.0, 0.0, 0.0]  # per level, wrapped to [0, 1), see _update_wind()
		self.vdata = None
		# vdata's array-0 layout (floats per row / the "vertex" column's start
		# offset within a row) -- fixed for the shape's lifetime once vdata is
		# built, so computed once and cached, unlike the numpy view itself
		# (see _update_wind(), that must be re-acquired every frame).
		self.vertex_stride = None
		self.vertex_pos_offset = None


def _build_wind_state(wind_params, vertex_buffer):
	"""Precomputes the per-vertex data _update_wind() needs every frame, from
	the shape's own PrimaryColor channel -- encodes, per vertex, its level-0/
	1/2 wind blend weight (R) and which of the 4 level-1/level-2 phase
	branches it swings with (G/B), exactly like the engine's own
	wind_tree_vp.glsl decodes it (`vprimaryColor.xxx*3 + (0,-1,-2)` clamped,
	and `vprimaryColor.yz*3.99` truncated to an index) -- see
	nel/src/3d/meshvp_wind_tree.cpp for the reference implementation this
	whole preview is ported from. None if the shape has no PrimaryColor data
	to decode (WindTreeParams present but no per-vertex assignment yet)."""
	positions = vertex_buffer.channels.get("Position")
	colors = vertex_buffer.channels.get("PrimaryColor")
	if not positions or not colors:
		return None

	base_positions = numpy.array(positions, dtype=numpy.float32)
	rgb = numpy.array([(c.r, c.g, c.b) for c in colors], dtype=numpy.float32) / 255.0
	r, g, b = rgb[:, 0], rgb[:, 1], rgb[:, 2]
	factors = numpy.clip(numpy.stack([r * 3.0, r * 3.0 - 1.0, r * 3.0 - 2.0], axis=1), 0.0, 1.0).astype(numpy.float32)
	idx2 = numpy.minimum((g * 3.99).astype(numpy.int32), 3)
	idx3 = numpy.minimum((b * 3.99).astype(numpy.int32), 3)
	return _WindState(wind_params, base_positions, factors, idx2, idx3)


class _SkinState:
	"""Per-loaded-shape data for live CMeshMRMSkinned re-skinning (see
	ObjectEditorApp._update_skin_preview()) -- built once in _rebuild_geometry()
	from the finest lod's resolved PackedVertex list, then read every frame by
	the animation task. `vdata` is filled in by the caller once it's built,
	same as _WindState."""

	def __init__(self, geom, skeleton, bone_names, local_positions, local_normals, bone_indices, weights):
		self.geom = geom
		self.skeleton = skeleton
		self.bone_names = bone_names  # geom.bones_name, same order/indices as bone_indices
		self.local_positions = local_positions  # (N,4) float32, homogeneous (w=1)
		self.local_normals = local_normals  # (N,4) float32, homogeneous (w=0, no translation)
		self.bone_indices = bone_indices  # (N,4) int32, indices into bone_names
		self.weights = weights  # (N,4) float32, already /255
		self.vdata = None
		self.vertex_stride = None
		self.vertex_pos_offset = None
		self.vertex_normal_offset = None


def _build_skin_state(geom, skeleton):
	"""Precomputes the static per-vertex tables _update_skin_preview() blends
	every frame, from the finest lod's geomorph-resolved PackedVertex list
	(see finest_skinned_lod()) -- None if the geom has no lods at all."""
	resolved_lod = finest_skinned_lod(geom)
	if resolved_lod is None:
		return None
	_lod, resolved = resolved_lod

	n = len(resolved)
	local_positions = numpy.empty((n, 4), dtype=numpy.float32)
	local_normals = numpy.empty((n, 4), dtype=numpy.float32)
	bone_indices = numpy.empty((n, 4), dtype=numpy.int32)
	weights = numpy.empty((n, 4), dtype=numpy.float32)
	local_positions[:, 3] = 1.0
	local_normals[:, 3] = 0.0
	for i, v in enumerate(resolved):
		local_positions[i, :3] = v.decompact_pos(geom.decompact_scale)
		local_normals[i, :3] = v.decompact_normal()
		bone_indices[i] = v.matrices
		weights[i] = [w / 255.0 for w in v.weights]

	return _SkinState(geom, skeleton, list(geom.bones_name), local_positions, local_normals, bone_indices, weights)


def _multi_bitmap_slot_label(index):
	if 0 <= index < len(_MULTI_BITMAP_SLOT_LABELS):
		quality, ecosystem, season = _MULTI_BITMAP_SLOT_LABELS[index]
		labels = " / ".join(label for label in (quality, ecosystem, season) if label)
		return labels or str(index)
	return str(index)


# How far outside [0, 1] a UV has to land before it's treated as genuine tiling
# intent (see _uvs_need_repeat()) rather than float imprecision from an .fbx/
# .dae/.obj import (assimp/Blender routinely overshoot by a thousandth or so) --
# real tiling content overshoots by at least a whole unit (e.g. V up to 2.0 on
# ooc_summer_raceline.shape), nowhere close to this margin.
_UV_REPEAT_MARGIN = 0.05


def _uvs_need_repeat(texcoords):
	"""Whether `texcoords` (a shape's whole TexCoord0 channel) actually relies
	on the texture wrapping/repeating (see load_panda_texture()'s `repeat`
	param) -- Panda3D's own default wrap mode is clamp, correct for the
	common case of a single, non-tiling texture per material, but wrong for
	shapes that deliberately go outside [0, 1] for tiling. Real, native NeL
	shapes do this often (repeating a wall/floor texture); imported meshes'
	UVs staying just barely outside [0, 1] (e.g. 1.0008) is float noise, not
	tiling intent, and switching those to repeat too visibly shifts the
	texture instead (a fraction of a pixel wrapping becomes a full, visible
	seam once the *whole* texture wraps around the boundary)."""
	if not texcoords:
		return False
	return any(
		u < -_UV_REPEAT_MARGIN or u > 1.0 + _UV_REPEAT_MARGIN or v < -_UV_REPEAT_MARGIN or v > 1.0 + _UV_REPEAT_MARGIN
		for u, v in texcoords)


def _build_vertex_data(vertex_buffer, dynamic=False):
	"""Builds the GeomVertexData for `vertex_buffer` -- shared by every render
	pass's Geom (all of a mesh's passes index into the exact same vertex
	array, see iter_render_passes()), so it's built once per _rebuild_geometry()
	call rather than once per pass. `dynamic`, when the loaded shape has wind
	animation data (see _update_wind()), marks the vertex column UH_dynamic so
	Panda3D doesn't assume the position data is upload-once-and-forget."""
	positions = vertex_buffer.channels.get("Position")
	normals = vertex_buffer.channels.get("Normal")
	texcoords = vertex_buffer.channels.get("TexCoord0")

	if normals and texcoords:
		vformat = GeomVertexFormat.get_v3n3t2()
	elif normals:
		vformat = GeomVertexFormat.get_v3n3()
	elif texcoords:
		vformat = GeomVertexFormat.get_v3t2()
	else:
		vformat = GeomVertexFormat.get_v3()

	vdata = GeomVertexData("shape", vformat, Geom.UH_dynamic if dynamic else Geom.UH_static)
	vdata.set_num_rows(vertex_buffer.num_verts)

	vertex_writer = GeomVertexWriter(vdata, "vertex")
	for p in positions:
		vertex_writer.add_data3(p[0], p[1], p[2])

	if normals:
		normal_writer = GeomVertexWriter(vdata, "normal")
		for n in normals:
			normal_writer.add_data3(n[0], n[1], n[2])

	if texcoords:
		uv_writer = GeomVertexWriter(vdata, "texcoord")
		for uv in texcoords:
			# NeL .shape files always store texture V with the opposite
			# origin from Panda3D; flip here (once, for every shape,
			# regardless of source -- shape_import.py's importers convert
			# .obj/.dae/.fbx's own native V convention into this same NeL one
			# at import time, precisely so this flip can stay simple and
			# unconditional and every *saved* .shape file is correct on its
			# own, not just correct-looking in Forgery for as long as it
			# remembers where a shape came from).
			#
			# Deliberately NOT wrapped to [0, 1) here, even though some
			# shapes' UVs go beyond that range (e.g. a whole render pass at V
			# in [1, 2) on ooc_summer_raceline.shape) -- reducing each vertex
			# mod 1 independently breaks any triangle whose UVs straddle an
			# integer boundary (routine for tiling textures, e.g. a
			# cylindrical trunk wrapping around): one corner jumps to the far
			# side of the texture while the others don't, stretching the
			# whole image across that triangle. The out-of-range values are
			# left as-is; the texture's own wrap mode (see
			# load_panda_texture()) is what needs to repeat correctly instead.
			uv_writer.add_data2(uv[0], 1.0 - uv[1])

	return vdata


def _build_geom(vdata, indices):
	"""Builds one render pass's Geom (just its triangle indices) against an
	already-built, shared vdata (see _build_vertex_data())."""
	triangles = GeomTriangles(Geom.UH_static)
	for i in range(0, len(indices), 3):
		triangles.add_vertices(indices[i], indices[i + 1], indices[i + 2])
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	return geom


APP_INFO = {
	"id": "object_editor",
	"name": "Patina",
	"subtitle": "Object Editor",
	"description": "Browse, inspect and edit .shape files.",
}


class ObjectEditorApp(ForgeryApp):
	def __init__(self):
		# The Explorer starts out wherever the highest-priority configured
		# search path points (there's no separate "data root" concept --
		# see ryzom_forgery.search_paths_dialog's own module docstring),
		# falling back to the user's home folder the very first time
		# there's nothing configured yet.
		configured_dirs = app_settings.load().search_paths
		explorer_root = Path(configured_dirs[0].path) if configured_dirs else Path.home()
		ForgeryApp.__init__(self, explorer_root=explorer_root, title="Ryzom Forgery - Patina",
		                     explorer_default_filter="*.shape")

		self.material_docs = load_material_docs()

		self._texture_cache = {}
		# {name: mtime} for _texture_cache entries NOT tracked by
		# self._panoply_texture_signatures -- see _update_texture_freshness().
		self._texture_freshness_mtimes = {}
		self._preview_texture_refs = {}  # texture name -> imgui.ImTextureRef, for thumbnail/tooltip previews in the UI
		self._color_texture_refs = {}  # (r,g,b,a) rounded -> imgui.ImTextureRef, for plain-color material swatches

		# Ctrl+drag (ObjectManipulator) rotates/moves this pivot instead of
		# the camera -- kept separate from model_root itself since that gets
		# torn down and rebuilt on every shape load/replace, but the pivot
		# (and whatever rotation/position the user set up on it) shouldn't be.
		self._object_pivot = self.render.attach_new_node("object-pivot")
		self.model_root = self._object_pivot.attach_new_node("shape-root")

		# Viewport helper toggles (see _draw_viewport_toggles()) -- built once
		# and just shown/hidden, except _object_transparent which has to be
		# re-applied after every _rebuild_geometry() since model_root itself
		# gets torn down and recreated then.
		self._grid_visible = False
		self._world_axes_visible = False
		self._pivot_axes_visible = False
		self._object_transparent = False
		self._viewport_toggle_size = (10.0, 10.0)
		self._transform_panel_size = (10.0, 10.0)

		# Real geometry (sized to the loaded shape's bbox) is built by
		# _rebuild_viewport_helpers(), called below and again from
		# _rebuild_geometry() -- these placeholders just give it something to
		# remove_node() the first time.
		self._grid_np = self.render.attach_new_node("floor-grid-placeholder")
		self._world_axes_np = self.render.attach_new_node("world-axes-placeholder")
		self._pivot_axes_np = self._object_pivot.attach_new_node("pivot-axes-placeholder")
		self._rebuild_viewport_helpers(None)

		self.shape_file = None
		self.shape_error = None
		self._reference_root = self.render.attach_new_node("reference-root")
		self._reference_shapes = {}  # label -> ShapeFile or None (failed to load), lazily parsed once
		self._reference_active = set()  # labels currently shown
		self._reference_placement = {}  # label -> "origin"/"pivot"; missing/"auto" = default side-by-side layout
		self._reference_transparent = set()  # labels currently shown at 50% transparency
		self._material_node_paths = {}  # material_id -> list[NodePath], to re-apply a material live after an edit
		self._multi_bitmap_expanded = set()  # slot indices currently expanded in the Multi Bitmap editor
		self._material_expanded = set()  # material_ids currently expanded in the Materials tab's property editor
		self._material_section_expanded = set()  # (material_id, section_key) pairs currently expanded within that
		self._multi_bitmap_hint_shown = False  # whether the status bar currently shows one of our doc hints
		self._material_hint_shown = False  # same, for the material property editor's own doc hints
		self._texture_browse_dialogs = {}  # key -> (in-flight portable_file_dialogs.open_file, on_result callback)
		self._material_override_colors = {}  # material_id -> (r,g,b,a), a manual flat-color override for that material
		# material_id -> {offset_u, offset_v, scale_u, scale_v, rotation, mirror_u, mirror_v} for
		# _draw_material_texture_transform_section() -- tracked here instead of re-derived from
		# Material.tex_user_mat[0] every frame, since decompose_uv_matrix() can't represent an
		# independent per-axis mirror without it bleeding into rotation (a mirrored-only axis and a
		# 180-degree-rotated one look identical to its matrix decomposition).
		self._tex_transform_ui_state = {}
		self._panoply_selection = {}  # axis (panoply.AXES) -> value, shape-wide, render-only, never touches shape data
		self._panoply_selection_defaulted = False  # see _draw_global_panoply_section()'s auto-default-once logic
		# Not reset on shape reload (unlike _panoply_selection/_texture_cache
		# above) -- keyed by source mtimes (see LiveColorizeCache), so a live
		# recompute stays valid across shapes/reloads as long as the base
		# texture + masks it read from haven't actually changed on disk.
		self._live_panoply_cache = panoply_live.LiveColorizeCache()
		# {resolved variant name: (base_name, dims)} for _update_texture_freshness()
		# to keep re-checking, and {resolved variant name: signature} for it to
		# compare against -- both reset on shape reload below, together with
		# _texture_cache itself (see _reset_shape_state()).
		self._panoply_texture_sources = {}
		self._panoply_texture_signatures = {}
		self._texture_freshness_last_check = 0.0

		self._shape_source_path = None  # Path this shape was loaded from on disk, or None if loaded from inside a .bnp -- Save no longer depends on this (always targets the active workspace instead), only _texture_search_dirs/dialog-default-folder purposes still do
		self._shape_source_bnp_path = None  # set alongside _shape_source_path when the shape lives inside a .bnp -- see _save_session_state()
		self._shape_source_name = None  # original file name, kept even when _shape_source_path is None -- also Save's destination filename within the workspace's shapes/ folder
		self._texture_search_dirs = []  # extra folders (see shape_geometry.load_panda_texture's search_dirs) to fall back to for this shape's textures -- an imported mesh's own folder, tex/textures/data subfolders included
		self._texture_needs_repeat = False  # see _uvs_need_repeat() -- whether the loaded shape's own UVs rely on texture tiling
		self._save_overwrite_confirmed = False  # session-scoped: asked once, no more Save confirmations after that
		self._confirm_overwrite_open = False
		self._pending_save_path = None  # workspace destination shown/used by the overwrite-confirmation popup
		self._save_status = ""
		self._restore_scan_popup_open = False  # see _restore_session_state()/_draw_restore_scan_popup()
		self._panoply_cfg_changed = False  # set from WorkspaceWatcher's background thread, see _on_panoply_cfg_settled()/_update_texture_freshness()
		self._bake_progress = None  # dict or None -- see _start_panoply_bake()/_draw_bake_progress_popup()

		self._replace_pending_mesh = None  # imported Mesh awaiting material-count matching (mismatched-count "Replace")
		self._replace_mapping = []  # per pending-mesh-material index: target existing material index, or None = "add as new"
		self._replace_match_popup_opened = False  # whether open_popup() has been called yet for the current _replace_pending_mesh

		# Overrides which action a plain left-click drag performs, in either
		# OrbitCamera (no Ctrl) or ObjectManipulator (Ctrl) -- None means the
		# normal left/middle/right split; "move"/"rotate"/"scale" forces
		# left-click alone to do that one, in whichever of the two contexts
		# is currently active. Set by clicking navcube.py's status icon.
		self.forced_drag_mode = None

		# Overrides whether a drag targets the camera (OrbitCamera) or the
		# object (ObjectManipulator) -- None means the normal Ctrl-held
		# split; "camera"/"object" forces every drag to that one regardless
		# of Ctrl. Set by clicking navcube.py's mode icon.
		self.target_mode = None

		# Per-row lock state for the Position/Rotation/Scale panel (see
		# _draw_transform_panel()) -- "pivot": locked routes edits to
		# model_root's own local transform instead of _object_pivot's (see
		# _transform_node()), so the object moves within a pivot that stays
		# put, instead of taking the pivot along with it (the default).
		# "x"/"y"/"z": locked means that axis never changes, from either the
		# text field or a Ctrl+drag (ObjectManipulator reads this too).
		self.transform_locks = {
			prop: {"pivot": False, "x": False, "y": False, "z": False}
			for prop in ("position", "rotation", "scale")
		}

		self.orbit_camera = OrbitCamera(self, distance=10.0)
		self.object_manipulator = ObjectManipulator(self, self._object_pivot, self.orbit_camera)
		self.nav_cube = NavigationCube(self, self.orbit_camera, self._object_pivot)

		# Wind preview (see _rebuild_geometry()/_update_wind()) -- viewer-only
		# controls, never saved to the shape (mirrors the engine's own
		# CScene::setGlobalWindPower/Direction, a scene setting, not a
		# per-shape one). self._wind_state is None whenever the loaded shape
		# has no WindTreeParams+PrimaryColor data to animate.
		self._wind_state = None
		self._wind_power = 1.0
		self._wind_direction_deg = 0.0
		self._wind_animate = True
		self._wind_panel_size = (10.0, 10.0)
		self.taskMgr.add(self._update_wind, "object-editor-wind")
		self.taskMgr.add(self._update_texture_freshness, "object-editor-texture-freshness")
		self.export_dialog = ExportDialog()
		self.import_dialog = ImportDialog(on_new_shape=self._on_import_new_shape, on_replace=self._on_import_replace)
		self.search_paths_dialog = SearchPathsDialog()
		# Wexplorer (Explorer.pinned_folders, see _on_active_workspace_changed())
		# covers the whole active workspace already watched by search_paths_dialog
		# -- piggybacks on that same watch instead of a 3rd Observer, so any
		# on-disk change (this app's own writes included: Save, auto-export...)
		# refreshes the Wexplorer's listing without a manual Refresh click.
		self.search_paths_dialog.on_workspace_changed = self.explorer.refresh
		self.workspace_setup_dialog = WorkspaceSetupDialog()
		# import_watcher/workspace_sync/tex_dds_sync each keep their own
		# conversion/sync logic and state, but share a single filesystem
		# watch (self.workspace_watch, one Observer/one debounce handler for
		# the whole active workspace) instead of each owning their own --
		# see workspace_watch.py's module docstring for why (previously 3
		# near-identical dedicated Observers, consolidated 2026-08-27).
		self.import_watcher = ImportWatcher(
			is_shape_open=self._is_shape_open_at, on_open_shape_conflict=self._on_open_shape_conflict,
			on_status=self._on_import_status)
		self.workspace_sync = WorkspaceSyncWatcher()
		self.tex_dds_sync = TexDdsSyncWatcher(on_status=self._on_import_status)
		self.workspace_watch = WorkspaceWatcher()
		self.workspace_watch.register("imports", self.import_watcher.handle_settled)
		self.workspace_watch.register("tex", self.tex_dds_sync.handle_settled)
		self.workspace_watch.register("panoply.cfg", self._on_panoply_cfg_settled)
		for _synced_subdir in SYNCED_SUBDIRS:
			self.workspace_watch.register(_synced_subdir, self.workspace_sync.handle_settled)
		self._workspace_sync_folder_dialog = None  # active portable_file_dialogs.select_folder, or None
		self._repository_paths_dialog = None  # active portable_file_dialogs.select_folder, or None
		self._repository_paths_dialog_repo = None  # which pynel.repository_paths.REPOSITORIES entry _repository_paths_dialog is for
		# Set from ImportWatcher's own background thread (see
		# _on_open_shape_conflict()); drawn once per frame from draw_panel()
		# (_draw_import_conflict_popup()) since it needs imgui -- main-thread-only.
		self._pending_import_conflict = None  # (source_path, target_path) or None
		self._import_conflict_popup_opened = False
		# Same cross-thread-safe queuing as _pending_import_conflict above --
		# set from ImportWatcher's background thread (_on_import_status()),
		# drained once per frame in draw_ui() since sysinfo.set_status() needs
		# imgui state that's main-thread-only.
		self._pending_import_status = None  # (message, is_error) or None
		# Active workspace's folder always searched first (see the
		# Workspaces chantier in `.todo/forgery-object-editor.md`) -- synced
		# on every change via this callback, and once right away here so a
		# workspace already active from a previous session is in effect
		# before the first scan below.
		self.workspace_setup_dialog.on_active_workspace_changed = self._on_active_workspace_changed
		self._on_active_workspace_changed(self.workspace_setup_dialog.active_workspace_dir)
		# Kicked off immediately (background thread, see ensure_scanned()) so
		# the very first shape loaded already has a populated texture/skel/
		# anim/panoply index, instead of possibly rendering with missing
		# textures while a scan started only on that first load is still
		# catching up.
		self.search_paths_dialog.ensure_scanned()
		# Skinning preview (see _update_skin_preview_time()/_draw_bone_preview_controls()):
		# the skeleton/animation driving a loaded CMeshMRMSkinned's own skin
		# (see _update_skin_preview() below). Independent of the loaded
		# .shape (a skeleton/animation can be picked before or after loading
		# the shape they're meant to drive).
		self._bone_preview_skeleton = None
		self._bone_preview_skeleton_name = ""
		self._bone_preview_animation = None
		self._bone_preview_animation_name = ""
		self._bone_preview_animation_duration = 0.0
		self._bone_preview_time = 0.0
		self._bone_preview_playing = True
		self._bone_preview_panel_size = (10.0, 10.0)
		self._skeleton_file_dialog = None  # in-flight portable_file_dialogs.open_file, for the Skinning preview's own "load a .skel" icon button
		self._animation_file_dialog = None  # same, for its "load a .anim" icon button
		self._image_editor_dialog = None  # same, for the Settings tab's image editor executable picker
		self._text_editor_dialog = None  # same, for the Settings tab's text editor executable picker
		self.taskMgr.add(self._update_skin_preview_time, "object-editor-skin-preview-time")
		# Live re-skinning of a loaded CMeshMRMSkinned (see _build_skin_state()/
		# _update_skin_preview()) -- reuses the same skeleton/animation state
		# above, whenever the *main* shape itself is skinned (a character/
		# creature body), independent of the bone-attach preview's own chosen
		# attach bone. self._skin_state is None whenever the loaded shape isn't
		# CMeshMRMSkinned, or no skeleton is loaded yet.
		self._skin_state = None
		self.taskMgr.add(self._update_skin_preview, "object-editor-skin-preview")
		self.commands.register_for_extension(
			".skel", "Load as bone-preview skeleton", self._on_load_skeleton_command)
		self.commands.register_for_extension(
			".anim", "Load as bone-preview animation", self._on_load_animation_command)

		self.commands.register_for_extension(".shape", "Load in viewer", self._on_load_command)
		self.explorer.extra_toolbar = self._draw_import_toolbar_button
		self.explorer.extra_header = self.workspace_setup_dialog.draw_active_workspace_row

		# Last, once every piece of state _load_shape()/_reset_shape_state()
		# touches actually exists -- reopens wherever the Explorer/loaded
		# shape were when the previous session quit (see
		# _save_session_state()/_restore_session_state()).
		self._restore_session_state()

	def on_selection_changed(self, items):
		# Selecting alone no longer auto-loads anything -- .shape used to,
		# .skel/.anim never did, which read as inconsistent. Right-click ->
		# "Load in viewer"/"Load as bone-preview skeleton"/"...animation" is
		# now the one way to load any of them, matching across all three.
		print(f"[object_editor] selection changed: {[item.name for item in items]}")

	def _on_active_workspace_changed(self, workspace_dir):
		"""Fans the active-workspace change out to every folder-scoped watcher
		this app owns -- called by WorkspaceSetupDialog (see __init__)."""
		if workspace_dir is not None:
			# Backfills any SUBDIRS missing on disk (a folder introduced in a
			# later Forgery version, or removed by hand) -- idempotent.
			# WorkspaceSetupDialog.set_active_workspace() already does this
			# too, but only when a workspace is picked/created interactively;
			# this covers every other path into this method, in particular
			# __init__ resuming a workspace already active from a previous
			# session (read straight off active_workspace_dir, bypassing
			# set_active_workspace() entirely).
			ensure_structure(workspace_dir)
		self.search_paths_dialog.set_workspace_dir(workspace_dir)
		panoply_config.set_workspace_dir(workspace_dir)
		self.import_watcher.set_workspace_dir(workspace_dir)
		self.workspace_sync.set_workspace_dir(workspace_dir)
		self.tex_dds_sync.set_workspace_dir(workspace_dir)
		self.workspace_watch.set_workspace_dir(workspace_dir)
		workspace_name = self.workspace_setup_dialog.active_workspace_name
		sync_folders = app_settings.load().workspace_sync_folders
		self.workspace_sync.set_sync_folder(sync_folders.get(workspace_name) if workspace_name else None)
		# Explorer.pinned_folders (see explorer.py): each workspace subfolder
		# (tex/shapes/anims/skels/exports/imports) as its own expandable tree
		# section above Favorites, so the workspace's own content stays
		# browsable/loadable-from no matter where the Explorer is currently
		# navigated to.
		if workspace_dir is None:
			self.explorer.pinned_folders = []
		else:
			self.explorer.pinned_folders = [(subdir, workspace_dir / subdir) for subdir in WORKSPACE_SUBDIRS]

	def _is_shape_open_at(self, target_path):
		"""ImportWatcher's `is_shape_open` hook -- runs off its own background
		thread, but only ever reads a couple of plain attributes (no
		imgui/Panda3D calls), same as other cross-thread reads in this app."""
		return self.shape_file is not None and self._workspace_shape_save_path() == target_path

	def _on_open_shape_conflict(self, source_path, target_path):
		"""ImportWatcher's `on_open_shape_conflict` hook -- runs off its own
		background thread, so only queues state; _draw_import_conflict_popup()
		(called once per frame from draw_panel()) does the actual imgui/disk
		work on the main thread."""
		self._pending_import_conflict = (source_path, target_path)

	def _on_import_status(self, message, is_error):
		"""ImportWatcher's `on_status` hook -- runs off its own background
		thread, so only queues state; _flush_pending_import_status() (called
		once per frame from draw_panel()) does the actual imgui work on the
		main thread, same reasoning as _on_open_shape_conflict() above."""
		self._pending_import_status = (message, is_error)

	def _flush_pending_import_status(self):
		"""Surfaces the last ImportWatcher outcome (queued by
		_on_import_status()) on the sysinfo status bar -- red for a failure,
		green for a successful auto-export/update/backup-and-reexport, same
		colors as _IMPORT_CONFLICT_DISCARD_COLOR/_IMPORT_CONFLICT_BACKUP_COLOR
		use elsewhere in this file for the same "bad"/"safe" meaning."""
		if self._pending_import_status is None:
			return
		message, is_error = self._pending_import_status
		self._pending_import_status = None
		color = _IMPORT_STATUS_ERROR_COLOR if is_error else _IMPORT_STATUS_SUCCESS_COLOR
		self.sysinfo.set_status(message, color=color)

	def _reload_shape_value_from_disk(self, target_path):
		"""Pulls target_path's just-rewritten geometry/materials back into the
		viewport without resetting editor-only state (material override
		colors, expanded panels, camera...) or re-seeding the pivot rotation
		-- same minimal-disruption approach as _replace_geometry(), since
		conceptually this *is* a replace (this exact shape, new geometry from
		an import), just triggered by the auto-export watcher instead of the
		manual Import -> Replace flow. Does strip the shape's own previous
		default_rot_quat back out of the live pivot, though -- see the
		comment below, same reasoning as _replace_geometry()."""
		old_base = getattr(self.shape_file.value, "base", None)
		old_rot = old_base.default_rot_quat if old_base is not None else IDENTITY_QUAT
		shape_file = parse_shape(target_path.read_bytes())
		self.shape_file.value = shape_file.value
		if old_rot != IDENTITY_QUAT:
			# Same reasoning as _replace_geometry(): the just-reloaded
			# geometry already has old_rot baked into its vertices (see
			# import_watcher.py's update_existing_shape()), so strip it back
			# out of the live pivot to avoid applying it twice.
			old_quat = Quat(old_rot.w, old_rot.x, old_rot.y, old_rot.z)
			self._object_pivot.set_quat(old_quat.conjugate() * self._object_pivot.get_quat())
		self._rebuild_geometry()

	def _has_unsaved_changes_at(self, target_path):
		"""True if the in-memory shape currently open differs from what's on
		disk at `target_path` -- lets _draw_import_conflict_popup() only
		actually ask something when there's a real conflict, instead of on
		every single open-shape auto-update. Rather than tracking every
		individual edit throughout the whole editor (a much bigger, more
		error-prone undertaking -- easy to miss a spot that mutates
		self.shape_file without going through it), serializes the in-memory
		shape to bytes (save_shape() accepts any BinaryIO, so this never
		touches disk) and compares against a fresh read of the file -- exact
		and always in sync with reality, at the cost of one extra
		parse+serialize per conflict check (cheap next to the geometry
		import itself, and only ever runs when the target is the shape
		that's actually open)."""
		buffer = io.BytesIO()
		save_shape(buffer, self.shape_file)
		try:
			on_disk = target_path.read_bytes()
		except OSError:
			return True
		return buffer.getvalue() != on_disk

	def _apply_import_conflict_update(self, source_path, target_path):
		"""Common tail of every _draw_import_conflict_popup() choice: routes
		through ImportWatcher._update_existing_target() -- same update /
		material-count-mismatch-backup-and-reexport / error handling and
		sysinfo-bar reporting as the automatic (shape-not-open) path, so
		there's a single outcome-reporting story regardless of whether the
		target happened to be open in the viewport. Refreshes the viewport
		from disk only if target_path actually ended up rewritten."""
		if self.import_watcher._update_existing_target(source_path, target_path):
			self._reload_shape_value_from_disk(target_path)

	def _draw_import_conflict_popup(self):
		"""The shape currently open in the viewport is also the auto-export
		watcher's target for a just-changed import/ source file -- if the
		in-memory shape has actually diverged from what's on disk (see
		_has_unsaved_changes_at()), overwriting the file could silently
		discard that in-progress work, so this asks how to proceed instead of
		auto-updating (see the chantier discussion). When there's nothing
		unsaved, this behaves exactly like the "not currently open" case --
		no popup, straight to the update."""
		if self._pending_import_conflict is None:
			self._import_conflict_popup_opened = False
			return

		if not self._import_conflict_popup_opened:
			source_path, target_path = self._pending_import_conflict
			if not self._has_unsaved_changes_at(target_path):
				self._pending_import_conflict = None
				self._apply_import_conflict_update(source_path, target_path)
				return
			imgui.open_popup(_IMPORT_CONFLICT_POPUP_ID)
			self._import_conflict_popup_opened = True

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_IMPORT_CONFLICT_POPUP_ID, None, flags)
		if not opened:
			return

		source_path, target_path = self._pending_import_conflict
		imgui.text(f"{target_path.name} is open in the viewport and about to be updated\n"
		           f"from the changed import {source_path.name}.")
		imgui.text_wrapped("This shape has unsaved changes in Patina -- choose how to proceed.")
		imgui.separator()

		save_label = f"Save this shape then import {source_path.name}"
		discard_label = f"Import {source_path.name} without saving"
		backup_label = "Save a copy of this shape"
		cancel_label = "Cancel (leave the on-disk file untouched)"
		frame_padding_x2 = imgui.get_style().frame_padding.x * 2

		_center_next_widget(imgui.calc_text_size(save_label).x + frame_padding_x2)
		if _colored_button(save_label, _IMPORT_CONFLICT_SAVE_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			self._write_shape(target_path)
			self._apply_import_conflict_update(source_path, target_path)
		_center_next_widget(imgui.calc_text_size(discard_label).x + frame_padding_x2)
		if _colored_button(discard_label, _IMPORT_CONFLICT_DISCARD_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			self._apply_import_conflict_update(source_path, target_path)
		_center_next_widget(imgui.calc_text_size(backup_label).x + frame_padding_x2)
		if _colored_button(backup_label, _IMPORT_CONFLICT_BACKUP_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
			backup_path = target_path.with_name(f"{target_path.stem}_backup_{timestamp}{target_path.suffix}")
			self._write_shape(backup_path)
			self._apply_import_conflict_update(source_path, target_path)
		imgui.separator()
		_center_next_widget(imgui.calc_text_size(cancel_label).x + frame_padding_x2)
		if _colored_button(cancel_label, _IMPORT_CONFLICT_CANCEL_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _on_load_command(self, items):
		if items:
			self._load_shape(items[0])

	def _on_load_skeleton_command(self, items):
		if not items:
			return
		item = items[0]
		self._load_skeleton_bytes(item.read_bytes(), item.name)

	def _load_skeleton_bytes(self, data, name):
		"""Parses `data` as a .skel, applying it as the Skinning preview's
		skeleton if valid -- shared by the Explorer's right-click "Load as
		bone-preview skeleton" command and the Skinning preview's own "load
		from disk" icon button (_poll_skeleton_file_dialog())."""
		try:
			shape_file = parse_shape(data)
		except ShapeParseError as exc:
			print(f"[object_editor] cannot parse skeleton {name}: {exc}")
			return
		if not isinstance(shape_file.value, SkeletonShape):
			print(f"[object_editor] {name} is not a CSkeletonShape")
			return
		self._apply_bone_preview_skeleton(shape_file.value, name)

	def _apply_bone_preview_skeleton(self, skeleton, name):
		"""Shared by the Explorer's right-click "Load as bone-preview
		skeleton" command and the Skinning preview's own Skeleton combo
		(see _draw_bone_preview_controls())."""
		self._bone_preview_skeleton = skeleton
		self._bone_preview_skeleton_name = name
		# The main shape might itself be CMeshMRMSkinned and was rendering its
		# rigid bind-pose fallback for lack of a skeleton (see
		# _rebuild_geometry()) -- now that one just got picked, rebuild so it
		# renders skinned instead.
		if self.shape_file is not None:
			self._rebuild_geometry()

	def _on_load_animation_command(self, items):
		if not items:
			return
		item = items[0]
		self._apply_bone_preview_animation_bytes(item.read_bytes(), item.name)

	def _apply_bone_preview_animation_bytes(self, data, name):
		"""Parses `data` as a .anim, applying it as the Skinning preview's
		animation if valid -- shared by the Explorer's right-click "Load as
		bone-preview animation" command, the Animation combo, and the "load
		from disk" icon button (_poll_animation_file_dialog())."""
		try:
			anim = parse_animation(data)
		except AnimationParseError as exc:
			print(f"[object_editor] cannot parse animation {name}: {exc}")
			return
		self._apply_bone_preview_animation(anim, name)

	def _apply_bone_preview_animation(self, anim, name):
		self._bone_preview_animation = anim
		self._bone_preview_animation_name = name
		self._bone_preview_animation_duration = animation_duration(anim)
		self._bone_preview_time = 0.0

	def _unload_bone_preview(self):
		self._bone_preview_skeleton = None
		self._bone_preview_skeleton_name = ""
		self._bone_preview_animation = None
		self._bone_preview_animation_name = ""
		self._bone_preview_animation_duration = 0.0
		self._bone_preview_time = 0.0
		if self.shape_file is not None:
			self._rebuild_geometry()

	def _bone_world_matrices_for(self, names):
		"""{bone name: 4x4 world matrix} for every name in `names` that's
		actually in the loaded skeleton, at the current preview time/
		animation -- {} if no skeleton is loaded. Computes the *whole*
		skeleton's world matrices in one pass (pynel's
		evaluate_all_bone_world_matrices(), O(bone count) instead of
		evaluate_bone_world_matrix() once per name, which independently
		re-walks each bone's own ancestor chain -- called every frame for a
		loaded skinned shape's re-skin (_update_skin_preview()), where that
		redundant work was the dominant per-frame cost past a handful of
		bones)."""
		skeleton = self._bone_preview_skeleton
		if skeleton is None:
			return {}
		all_matrices = evaluate_all_bone_world_matrices(skeleton, self._bone_preview_animation, self._bone_preview_time)
		return {name: all_matrices[name] for name in names if name in skeleton.bone_map}

	def _update_skin_preview(self, task):
		"""Per-frame: re-skins the loaded shape's geometry in place (see
		_build_skin_state()) whenever it's a CMeshMRMSkinned with a skeleton
		loaded -- vectorized (numpy), unlike pynel.ryzom_skin's plain-Python
		skin_vertex()/skin_mesh() (fine for one-off/CLI use, far too slow
		called per-vertex every frame for a real character mesh -- see
		_update_wind()'s own note on exactly this same tradeoff). No-op
		otherwise, same pattern as _update_wind()/_update_skin_preview_time()."""
		state = self._skin_state
		if state is None or state.vdata is None:
			return task.cont

		bone_world_matrices = self._bone_world_matrices_for(state.bone_names)
		bone_skin_matrices = bone_skin_matrices_for_mesh(state.geom, state.skeleton, bone_world_matrices)
		matrices = numpy.array(bone_skin_matrices, dtype=numpy.float32)  # (B,4,4)

		# state.bone_indices: (N,4) int, one of the B bones per influence slot
		# -- gathers each vertex's own 4 candidate matrices in one shot.
		gathered = matrices[state.bone_indices]  # (N,4,4,4): (vertex, slot, row, col)
		transformed_pos = numpy.einsum("nsij,nj->nsi", gathered, state.local_positions)
		weighted_pos = (transformed_pos[:, :, :3] * state.weights[:, :, None]).sum(axis=1)

		transformed_normal = numpy.einsum("nsij,nj->nsi", gathered, state.local_normals)
		weighted_normal = (transformed_normal[:, :, :3] * state.weights[:, :, None]).sum(axis=1)
		lengths = numpy.linalg.norm(weighted_normal, axis=1, keepdims=True)
		lengths[lengths == 0] = 1.0
		weighted_normal /= lengths

		# Same buffer-protocol/modification-stamp technique as _update_wind()
		# -- see its own docstring for why a per-row GeomVertexRewriter loop
		# isn't used instead.
		array_data = state.vdata.modify_array(0)
		if state.vertex_pos_offset is None:
			array_format = array_data.get_array_format()
			state.vertex_stride = array_format.get_stride() // 4
			state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4
			state.vertex_normal_offset = array_format.get_column(InternalName.get_normal()).get_start() // 4

		view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
		view[:, state.vertex_pos_offset:state.vertex_pos_offset + 3] = weighted_pos
		view[:, state.vertex_normal_offset:state.vertex_normal_offset + 3] = weighted_normal
		return task.cont

	def _update_skin_preview_time(self, task):
		"""Per-frame: advances self._bone_preview_time while playing -- the
		clock driving the Skinning preview's animation (_update_skin_preview()
		re-skins the loaded shape's geometry against it every frame)."""
		if self._bone_preview_playing and self._bone_preview_animation_duration > 0:
			dt = ClockObject.get_global_clock().get_dt()
			self._bone_preview_time = (self._bone_preview_time + dt) % self._bone_preview_animation_duration
		return task.cont

	def _draw_bone_preview_controls(self):
		"""Floating "Skinning preview" window: picks the skeleton/animation
		driving the loaded shape's own skinning, if it's a CMeshMRMSkinned
		(see _update_skin_preview()). Shown as soon as a shape is loaded (not
		gated on a skeleton being picked yet) so the Skeleton combo below is
		reachable even before any skeleton is chosen. Positioned the same way
		as _draw_wind_controls(), stacked right below it (both top-right of
		the viewport, flush against the panel)."""
		if self.shape_file is None:
			return

		is_skinned = isinstance(self.shape_file.value, MeshMRMSkinned)
		shape_bones = set(shape_geom(self.shape_file.value).bones_name) if is_skinned else set()

		display_width = imgui.get_io().display_size.x
		win_w, win_h = self._bone_preview_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		if self._wind_state is not None:
			y += self._wind_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("Skinning preview", flags=flags):
			# Every .skel found by the last scan (see SearchPathsDialog.reload(),
			# triggered by the reload icon below), sorted first/highlighted
			# green when it's actually compatible with the loaded shape's own
			# skinning bones -- picking an incompatible one is still allowed,
			# same as the engine itself doesn't refuse an incompatible bind,
			# just remaps missing bones to the root (CSkeletonModel::remapSkinBones).
			scanned_names = self.search_paths_dialog.scanned_skeleton_names()
			compatible_names = set(self.search_paths_dialog.compatible_for(shape_bones)) if is_skinned else set()
			# Compatible ones first (still sorted among themselves), then the
			# rest -- the match everyone actually wants shouldn't be buried
			# alphabetically among dozens of irrelevant skeletons.
			ordered_names = sorted(compatible_names) + sorted(name for name in scanned_names if name not in compatible_names)
			imgui.set_next_item_width(220)
			preview = self._bone_preview_skeleton_name or "(choose a skeleton)"
			if imgui.begin_combo("##skeleton-combo", preview):
				for name in ordered_names:
					compatible = name in compatible_names
					if compatible:
						imgui.push_style_color(imgui.Col_.text.value, _COMPATIBLE_COLOR)
					clicked, _ = imgui.selectable(name, name == self._bone_preview_skeleton_name)
					if compatible:
						imgui.pop_style_color()
					if clicked:
						self._apply_bone_preview_skeleton(self.search_paths_dialog.skeleton_for(name), name)
				imgui.end_combo()
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##skeleton-file", "Load a skeleton from disk..."):
				self._skeleton_file_dialog = pfd.open_file("Choose a .skel file", "", ["Ryzom skeleton", "*.skel"])
			imgui.same_line()
			if _icon_button(
					fa_icons.ICON_FA_SYNC, "Rescan the configured search paths",
					disabled=self.search_paths_dialog.scanning):
				self.search_paths_dialog.reload()
			if self.search_paths_dialog.scanning:
				imgui.text_disabled("Scanning search paths...")

			# Same idea as the Skeleton combo above, but the other way around
			# (see SearchPathsDialog.compatible_animations_for()): an
			# animation is compatible when ITS OWN tracked bones are all
			# present in the *currently chosen* skeleton -- so this list is
			# empty until a skeleton is picked.
			animation_names = self.search_paths_dialog.scanned_animation_names()
			compatible_animations = set(self.search_paths_dialog.compatible_animations_for(self._bone_preview_skeleton))
			ordered_animations = (
				sorted(compatible_animations)
				+ sorted(name for name in animation_names if name not in compatible_animations))
			imgui.set_next_item_width(220)
			preview = self._bone_preview_animation_name or "(none, bind pose)"
			if imgui.begin_combo("##animation-combo", preview):
				for name in ordered_animations:
					compatible = name in compatible_animations
					if compatible:
						imgui.push_style_color(imgui.Col_.text.value, _COMPATIBLE_COLOR)
					clicked, _ = imgui.selectable(name, name == self._bone_preview_animation_name)
					if compatible:
						imgui.pop_style_color()
					if clicked:
						self._apply_bone_preview_animation(self.search_paths_dialog.animation_for(name), name)
				imgui.end_combo()
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##animation-file", "Load an animation from disk..."):
				self._animation_file_dialog = pfd.open_file("Choose a .anim file", "", ["Ryzom animation", "*.anim"])

			if self._bone_preview_animation is not None:
				icon = fa_icons.ICON_FA_PAUSE if self._bone_preview_playing else fa_icons.ICON_FA_PLAY
				if _icon_button(icon, "Play/pause"):
					self._bone_preview_playing = not self._bone_preview_playing
				imgui.same_line()
				imgui.set_next_item_width(160)
				changed, new_time = imgui.slider_float(
					"##bone-preview-time", self._bone_preview_time,
					0.0, max(self._bone_preview_animation_duration, 0.001), "%.2f s")
				if changed:
					self._bone_preview_time = new_time

			if _icon_button(fa_icons.ICON_FA_TIMES, "Unload skeleton/animation, stop preview"):
				self._unload_bone_preview()
			self._bone_preview_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_import_toolbar_button(self):
		if _icon_button(fa_icons.ICON_FA_UPLOAD, "Import mesh (.obj/.dae/.fbx/.gltf/.glb)..."):
			self.import_dialog.open(self.shape_file is not None)

	def _on_import_new_shape(self, mesh, source_path):
		# Only CMesh can be built from scratch this way (see
		# ryzom_forgery/shape_import.py's module docstring) -- matches
		# shape_importer.py's CLI equivalent.
		self._reset_shape_state()
		if source_path is not None:
			self._shape_source_name = source_path.stem + ".shape"
			# The imported file's own folder (see load_panda_texture()'s
			# search_dirs) -- an .fbx/.dae/.obj's textures routinely sit right
			# next to it (or in a tex/textures/data subfolder, or an .fbx's
			# own <name>.fbm sibling) rather than anywhere in the Ryzom asset
			# root this app otherwise searches.
			self._texture_search_dirs = texture_search_dirs_for(source_path)
		self._display_shape(ShapeFile(type_name="Mesh", value=mesh))

	def _on_import_replace(self, mesh, source_path):
		if self.shape_file is None:
			return
		if self.shape_file.type_name != "Mesh":
			self._save_status = (
				f"Can't replace geometry: the current shape is a {self.shape_file.type_name!r}, "
				f"only a plain Mesh's geometry can be swapped this way.")
			return

		if source_path is not None:
			# Any material the mismatched-count remap path below adds from
			# the imported mesh (see _replace_geometry()) carries that mesh's
			# own texture names -- same reasoning as _on_import_new_shape().
			self._texture_search_dirs.extend(texture_search_dirs_for(source_path))

		current_materials = self.shape_file.value.materials
		if len(mesh.materials) == len(current_materials):
			# Same convention the .obj/.dae exporter itself used to number
			# materials in the first place -- indices already line up.
			self._replace_geometry(mesh, index_map=None)
		else:
			# open_popup() isn't called here: this runs while still nested
			# inside ImportDialog's own "Import mesh" popup (not yet closed
			# for the rest of this frame), and opening one popup from inside
			# another that hasn't ended yet silently failed to register.
			# _draw_replace_match_popup() opens it itself, next frame,
			# from draw_panel()'s top level instead.
			self._replace_pending_mesh = mesh
			self._replace_mapping = [None] * len(mesh.materials)

	def _replace_geometry(self, mesh, index_map):
		"""Swaps the current Mesh's geometry (vertex buffer, matrix
		blocks/render passes, bbox...) for `mesh.geom`, leaving
		self.shape_file.value's materials (and whatever editing was done to
		them) untouched. `index_map`, if given, remaps each render pass's
		material_id from `mesh`'s own material indices to the current
		shape's (used for the mismatched-material-count case, after
		_draw_replace_match_popup() resolved a mapping)."""
		if index_map is not None:
			for matrix_block in mesh.geom.matrix_blocks:
				for rdr_pass in matrix_block.rdr_passes:
					rdr_pass.material_id = index_map[rdr_pass.material_id]

		self.shape_file.value.geom = mesh.geom
		# See import_watcher.py's update_existing_shape() for why: an
		# imported mesh's source file already has the shape's own (previous)
		# default_rot_quat baked into its vertices by shape_export.py's
		# export_shape(), so the new geometry already represents the final
		# orientation -- reset to identity to avoid rotating it again.
		base = getattr(self.shape_file.value, "base", None)
		if base is not None:
			old_rot = base.default_rot_quat
			if old_rot != IDENTITY_QUAT:
				# _object_pivot was seeded with this same old_rot on the
				# original _display_shape() (plus, potentially, further
				# Ctrl+drag deltas right-multiplied onto it since -- see
				# ObjectManipulator._rotate()). Left-multiplying by its
				# conjugate strips exactly that seeded rotation back out,
				# leaving only the user's own manual deltas -- otherwise the
				# pivot would still apply old_rot on top of geometry that
				# now already has it baked in, throwing the model out of
				# sync with the camera framing computed from its new bbox.
				old_quat = Quat(old_rot.w, old_rot.x, old_rot.y, old_rot.z)
				self._object_pivot.set_quat(old_quat.conjugate() * self._object_pivot.get_quat())
			base.default_rot_quat = IDENTITY_QUAT
		self._rebuild_geometry()
		self._save_status = "Geometry replaced."

	def _describe_current_material(self, index):
		material = self.shape_file.value.materials[index]
		texture = material.textures[0] if material.textures else None
		name = texture.file_name if texture is not None and texture.file_name else None
		return f"{index}: {name}" if name else f"{index}: (no texture)"

	def _draw_replace_match_popup(self):
		"""Mismatched material counts between the imported mesh and the
		current shape: lets the user match each imported material to an
		existing one (keeping its edits) or add it as a new material,
		before _replace_geometry() actually swaps the geometry in."""
		if self._replace_pending_mesh is None:
			self._replace_match_popup_opened = False
			return

		if not self._replace_match_popup_opened:
			imgui.open_popup(_REPLACE_MATCH_POPUP_ID)
			self._replace_match_popup_opened = True

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_REPLACE_MATCH_POPUP_ID, None, flags)
		if not opened:
			return

		mesh = self._replace_pending_mesh
		current_materials = self.shape_file.value.materials
		imgui.text(f"The imported mesh has {len(mesh.materials)} material(s), "
		           f"the current shape has {len(current_materials)} -- match each one:")
		imgui.separator()

		for i, material in enumerate(mesh.materials):
			texture = material.textures[0] if material.textures else None
			label = texture.file_name if texture is not None and texture.file_name else f"material {i}"
			imgui.text(label)
			imgui.same_line()
			imgui.push_id(f"replace-match-{i}")

			target = self._replace_mapping[i]
			preview = "Add as new material" if target is None else self._describe_current_material(target)
			imgui.set_next_item_width(220)
			if imgui.begin_combo("##target", preview):
				clicked, _ = imgui.selectable("Add as new material", target is None)
				if clicked:
					self._replace_mapping[i] = None
				for j in range(len(current_materials)):
					clicked, _ = imgui.selectable(self._describe_current_material(j), target == j)
					if clicked:
						self._replace_mapping[i] = j
				imgui.end_combo()

			imgui.pop_id()

		imgui.separator()
		if imgui.button("Replace"):
			self._confirm_replace_matching()
			imgui.close_current_popup()
		imgui.same_line()
		if imgui.button("Cancel"):
			self._replace_pending_mesh = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _confirm_replace_matching(self):
		mesh, self._replace_pending_mesh = self._replace_pending_mesh, None
		current_materials = self.shape_file.value.materials

		index_map = {}
		for i, target in enumerate(self._replace_mapping):
			if target is None:
				current_materials.append(mesh.materials[i])
				index_map[i] = len(current_materials) - 1
			else:
				index_map[i] = target

		self._replace_geometry(mesh, index_map)

	def _load_shape(self, item):
		self._reset_shape_state()
		self._shape_source_path = item.path if item.bnp_path is None else None
		self._shape_source_bnp_path = item.bnp_path
		self._shape_source_name = item.name
		if self._shape_source_path is not None:
			self._texture_search_dirs = [self._shape_source_path.parent]

		try:
			shape_file = parse_shape(item.read_bytes())
		except ShapeParseError as exc:
			self.shape_file = None
			self.shape_error = str(exc)
			return

		self._display_shape(shape_file)

	def _save_session_state(self):
		"""Persists where the Explorer is browsing and which shape is
		loaded (see settings.py's last_folder/last_bnp/last_shape_* fields)
		so the next launch can pick up right where this one left off --
		called right before quitting (see _draw_bottom_bar()'s Quit
		button). Best-effort: this is a nice-to-have, not worth failing
		Quit over."""
		try:
			fresh = app_settings.load()
			fresh.last_folder = str(self.explorer.root)
			fresh.last_bnp = str(self.explorer._current_bnp) if self.explorer._current_bnp is not None else None
			shape_path = self._shape_source_bnp_path if self._shape_source_bnp_path is not None else self._shape_source_path
			fresh.last_shape_path = str(shape_path) if shape_path is not None else None
			fresh.last_shape_bnp = str(self._shape_source_bnp_path) if self._shape_source_bnp_path is not None else None
			fresh.last_shape_name = self._shape_source_name
			app_settings.save(fresh)
		except OSError as exc:
			print(f"[object_editor] could not save session state: {exc}")

	def _restore_session_state(self):
		"""Reopens whatever the Explorer/loaded shape were at the end of
		the previous session (see _save_session_state()) -- called once
		from __init__(), after self.explorer/search_paths_dialog are set
		up. Best-effort: a folder/shape that's since moved or vanished is
		silently skipped, same as active_workspace_path()'s own handling
		of a vanished workspace, not worth an error dialog over."""
		settings = app_settings.load()
		if settings.last_folder and Path(settings.last_folder).is_dir():
			self.explorer._navigate_to(Path(settings.last_folder))
			if settings.last_bnp and Path(settings.last_bnp).is_file():
				self.explorer._enter_bnp(Path(settings.last_bnp))

		if not (settings.last_shape_name and settings.last_shape_path):
			return
		if settings.last_shape_bnp:
			bnp_path = Path(settings.last_shape_bnp)
			if bnp_path.is_file():
				self._load_shape(ExplorerItem(path=bnp_path, name=settings.last_shape_name, bnp_path=bnp_path))
		else:
			shape_path = Path(settings.last_shape_path)
			if shape_path.is_file():
				self._load_shape(ExplorerItem(path=shape_path, name=settings.last_shape_name))

		# The shape displays right away, but its textures are resolved via
		# self.search_paths_dialog.find_texture() -- backed by the index
		# ensure_scanned() (called just before this in __init__) built.
		# has_scanned_data (not just scanning) is what actually matters
		# here: a cache-hit startup already published a real (if possibly
		# slightly stale) index synchronously, so the still-running
		# background refresh scan has nothing left for this popup to wait
		# on. Only a genuinely cold start (no cache yet, index still empty)
		# needs to show the shape untextured with a popup saying so, and
		# refresh every material once that first scan completes (see
		# _draw_restore_scan_popup()).
		if self.shape_file is not None and self.search_paths_dialog.scanning \
				and not self.search_paths_dialog.has_scanned_data:
			self._restore_scan_popup_open = True

	def _draw_restore_scan_popup(self):
		"""Drawn every frame from draw_panel() while a shape was just
		restored (see _restore_session_state()) before the search-path scan
		it needs for textures had finished -- closes itself and refreshes
		every material the moment the scan completes."""
		if not self._restore_scan_popup_open:
			return
		if not imgui.is_popup_open(_RESTORE_SCAN_POPUP_ID):
			imgui.open_popup(_RESTORE_SCAN_POPUP_ID)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_RESTORE_SCAN_POPUP_ID, None, flags)
		if opened:
			imgui.text("Scanning assets for textures...")
			if not self.search_paths_dialog.scanning:
				# load_panda_texture() unconditionally caches its result, a
				# miss (None) included -- the very first resolution attempt
				# (right after the shape was restored, well before the scan
				# populated the index) would otherwise poison every texture
				# name as "not found" forever, so _reapply_all_materials()
				# alone would just keep re-serving those same stale misses.
				self._texture_cache.clear()
				self._reapply_all_materials()
				imgui.close_current_popup()
				self._restore_scan_popup_open = False
			imgui.end_popup()

	def _reset_shape_state(self):
		"""Common reset before showing any shape in the viewer -- a `.shape`
		opened from the Explorer, or a mesh imported as a brand-new shape."""
		self.model_root.remove_node()
		self.model_root = self._object_pivot.attach_new_node("shape-root")
		self.shape_error = None
		self._material_node_paths = {}
		self._multi_bitmap_expanded = set()
		self._material_expanded = set()
		self._material_section_expanded = set()
		self._texture_browse_dialogs = {}
		self._material_override_colors = {}
		self._tex_transform_ui_state = {}
		self._panoply_selection = {}
		self._panoply_selection_defaulted = False
		self._shape_source_path = None
		self._shape_source_bnp_path = None  # set alongside _shape_source_path when the shape lives inside a .bnp -- see _save_session_state()
		self._shape_source_name = None
		self._texture_search_dirs = []
		self._texture_needs_repeat = False
		self._save_status = ""
		# Both keyed by texture *name* only, not by which shape/search_dirs
		# resolved it -- two different shapes can use the same texture name
		# for genuinely different files (different _texture_search_dirs
		# fallbacks) or the same file with different wrap-mode needs (see
		# _uvs_need_repeat()), which only gets applied the first time a name
		# is loaded (load_panda_texture()'s cache). Carrying either cache
		# over to a newly loaded shape risked reusing another shape's
		# resolved texture/wrap mode outright.
		self._texture_cache = {}
		self._texture_freshness_mtimes = {}
		self._preview_texture_refs = {}
		# Same reasoning as _texture_cache just above -- also keyed by
		# resolved texture name only.
		self._panoply_texture_sources = {}
		self._panoply_texture_signatures = {}

	def _display_shape(self, shape_file):
		"""Renders an already-parsed/-built ShapeFile. Assumes
		_reset_shape_state() was already called (separate so a
		replace-geometry flow can skip resetting the editing state that's
		meant to survive it, e.g. material overrides)."""
		self.shape_file = shape_file

		# CMeshBase::DefaultRotQuat is what the engine actually rotates the
		# object by at instance creation (nel/src/3d/mesh_base.cpp) -- not
		# just an editor default. Seeding _object_pivot with it up front, on
		# a fresh load, means the shape (and the navcube gizmo's inner cube,
		# which mirrors _object_pivot) already shows its real in-game tilt
		# instead of looking un-rotated until you happen to notice
		# default_rot_quat in the All Properties tab. A geometry-only
		# replace (_replace_geometry(), which calls _rebuild_geometry()
		# directly and never this method) deliberately does NOT re-seed --
		# it must preserve whatever the user already set up via Ctrl+drag.
		base = getattr(shape_file.value, "base", None)
		if base is not None:
			rot = base.default_rot_quat
			self._object_pivot.set_quat(Quat(rot.w, rot.x, rot.y, rot.z))

		# Baseline for the gizmo's Ctrl+Reset (see reset_object_rotation()) --
		# whatever the object's rotation is right after loading, until a save
		# makes the current Ctrl+drag rotation the new baseline instead.
		self._object_pivot_base_quat = Quat(self._object_pivot.get_quat())

		self._rebuild_geometry()
		self._auto_select_multi_bitmap_slot()

	def _auto_select_multi_bitmap_slot(self):
		"""If the shape's own stored Multi Bitmap selection (a CTextureMultiFile's
		_CurrSelectedTexture, faithfully preserved as-is by pynel -- see
		ryzom_shape.py) points at a slot that's empty for every material
		(common for creatures/props only exported with a subset of the
		quality/ecosystem/season variants filled in -- e.g. fo_carnitree.shape,
		whose slot 0 is empty across all 5 materials), the shape renders
		blank/white by default. Auto-switches to the first slot that actually
		has a texture in at least one material instead -- the real client
		picks its slot dynamically (graphics quality/ecosystem/season) rather
		than trusting whatever was last saved as "current", so there's no
		single "correct" slot to fall back to anyway; this just finds
		*something* to show rather than nothing."""
		entries = self._multi_bitmap_entries()
		if not entries:
			return
		representative = entries[0][1]
		current_index = representative.selected_index
		if current_index is not None and any(
				current_index < len(texture.file_names) and texture.file_names[current_index]
				for _material_id, texture in entries):
			return  # already resolves to something for at least one material

		slot_count = max(len(_MULTI_BITMAP_SLOT_LABELS), max(len(t.file_names) for _, t in entries))
		for index in range(slot_count):
			if any(index < len(texture.file_names) and texture.file_names[index] for _material_id, texture in entries):
				self._select_multi_bitmap_slot(entries, index)
				return

	def _rebuild_viewport_helpers(self, bbox):
		"""(Re)builds the floor grid + world/pivot axes geometry, sized and
		centered to `bbox` (the just-loaded shape's bounding box, or None
		before any shape is loaded / for a shape with no renderable geometry --
		falls back to a fixed default size then). Called from __init__ and
		from _rebuild_geometry() every time the shape (and so its bbox)
		changes, so these always scale to cover whatever's on screen instead
		of a fixed size that's tiny for a building and huge for a trinket."""
		if bbox is not None:
			radius = max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1)
			grid_squares = max(
				_MIN_GRID_SQUARES,
				ceil(max(bbox.half_size.x, bbox.half_size.y) * 2 / _GRID_STEP) + _GRID_MARGIN_SQUARES)
			# bbox is in model_root's own local space (pre-pivot-rotation) --
			# same reasoning as _rebuild_geometry()'s camera framing: must go
			# through model_root's current transform to land in world space.
			local_center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
			world_center = self.render.get_relative_point(self.model_root, local_center)
			grid_center_x, grid_center_y = world_center.x, world_center.y
			# Fixed at world Z=0 (not the bbox's own bottom) so it reads as an
			# actual ground reference -- whether the object floats above it or
			# sinks below it is exactly the thing this grid is meant to show.
			grid_z = 0.0
			axis_length = radius * _AXIS_MARGIN_FACTOR
		else:
			grid_squares = _GRID_SQUARES
			grid_center_x = grid_center_y = grid_z = 0.0
			axis_length = _AXIS_LENGTH

		self._grid_np.remove_node()
		self._grid_np = self.render.attach_new_node(
			_build_grid_geom(grid_center_x, grid_center_y, grid_z, grid_squares))
		self._grid_np.set_light_off()
		if not self._grid_visible:
			self._grid_np.hide()

		self._world_axes_np.remove_node()
		self._world_axes_np = self.render.attach_new_node(_build_axes_geom(_WORLD_AXIS_COLORS, axis_length))
		self._world_axes_np.set_light_off()
		if not self._world_axes_visible:
			self._world_axes_np.hide()

		self._pivot_axes_np.remove_node()
		self._pivot_axes_np = self._object_pivot.attach_new_node(_build_axes_geom(_PIVOT_AXIS_COLORS, axis_length))
		self._pivot_axes_np.set_light_off()
		if not self._pivot_axes_visible:
			self._pivot_axes_np.hide()

	def _toggle_grid(self):
		self._grid_visible = not self._grid_visible
		(self._grid_np.show if self._grid_visible else self._grid_np.hide)()

	def _toggle_world_axes(self):
		self._world_axes_visible = not self._world_axes_visible
		(self._world_axes_np.show if self._world_axes_visible else self._world_axes_np.hide)()

	def _toggle_pivot_axes(self):
		self._pivot_axes_visible = not self._pivot_axes_visible
		(self._pivot_axes_np.show if self._pivot_axes_visible else self._pivot_axes_np.hide)()

	def _apply_object_transparency(self):
		if self._object_transparent:
			self.model_root.set_transparency(TransparencyAttrib.M_alpha)
			self.model_root.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
		else:
			self.model_root.clear_transparency()
			self.model_root.clear_color_scale()

	def _toggle_object_transparency(self):
		self._object_transparent = not self._object_transparent
		self._apply_object_transparency()

	def _update_wind(self, task):
		"""Per-frame wind animation, ported from nel/src/3d/meshvp_wind_tree.cpp
		(CMeshVPWindTree::setupPerMesh()/setupPerInstanceConstants()) and its
		wind_tree_vp.glsl vertex decode -- see _build_wind_state()'s docstring.
		No-op whenever the loaded shape has nothing to animate, or the user
		paused it (self._wind_animate)."""
		state = self._wind_state
		if state is None or not self._wind_animate or state.vdata is None:
			return task.cont

		dt = ClockObject.get_global_clock().get_dt()
		params = state.params

		# Wind direction is a WORLD-space vector (matches the engine: the same
		# global wind blows every instance the same way, see
		# CScene::getGlobalWindDirection()) -- converted to the object's own
		# local space the same way setupPerInstanceConstants() does, via the
		# inverse of the pivot's current *rotation only* (a pure direction,
		# translation doesn't apply), so Ctrl-dragging the object to face a
		# different way doesn't change how the wind reads relative to it.
		direction_rad = radians(self._wind_direction_deg)
		wind_dir_world = Vec3(cos(direction_rad), sin(direction_rad), 0.0)
		pivot_quat = self._object_pivot.get_quat(self.render)
		inv_pivot_quat = Quat(pivot_quat)
		inv_pivot_quat.invert_in_place()
		wind_dir_local = inv_pivot_quat.xform(wind_dir_world)

		def speed_cos(t):
			return cos(2.0 * pi * t)

		max_delta = []
		for i in range(3):
			state.current_time[i] = (
				state.current_time[i] + dt * (params.frequency[i] + params.frequency_wind_factor[i] * self._wind_power)
			) % 1.0
			max_delta.append(Vec3(
				wind_dir_local.x * params.power_xy[i] * self._wind_power,
				wind_dir_local.y * params.power_xy[i] * self._wind_power,
				params.power_z[i] * self._wind_power,
			))

		f0 = speed_cos(state.current_time[0]) + params.bias[0]
		wind_l1 = numpy.array([max_delta[0].x, max_delta[0].y, max_delta[0].z], dtype=numpy.float32) * f0

		wind_l2 = numpy.empty((4, 3), dtype=numpy.float32)
		wind_l3 = numpy.empty((4, 3), dtype=numpy.float32)
		for k in range(4):
			f2 = speed_cos(state.current_time[1] + k * 0.25) + params.bias[1]
			wind_l2[k] = (max_delta[1].x * f2, max_delta[1].y * f2, max_delta[1].z * f2)
			f3 = speed_cos(state.current_time[2] + k * 0.25) + params.bias[2]
			wind_l3[k] = (max_delta[2].x * f3, max_delta[2].y * f3, max_delta[2].z * f3)

		offset = (state.factors[:, 0:1] * wind_l1
		          + wind_l2[state.idx2] * state.factors[:, 1:2]
		          + wind_l3[state.idx3] * state.factors[:, 2:3])
		new_positions = state.base_positions + offset

		# A per-row GeomVertexRewriter loop (tried first) means one
		# Python-level set_row()/set_data3() call pair per vertex, every
		# frame -- fine for a handful of vertices, but a real prop-sized mesh
		# (e.g. 28k verts on ooc_summer_raceline.shape) dropped this from
		# 60fps to ~38fps, all in Python call overhead rather than actual
		# work. GeomVertexArrayData supports the buffer protocol directly
		# (confirmed writable via modify_array()), so a numpy view onto it
		# lets every vertex's position be written in one vectorized C-level
		# assignment instead -- but modify_array() itself (not just writing
		# into the buffer it returns) is what bumps the array's modification
		# stamp Panda3D's GSG checks to know the GPU-side vertex buffer needs
		# re-uploading; caching the numpy view across frames and only writing
		# into the same buffer (tried first) silently stopped animating
		# anything after the first frame, since nothing told the renderer the
		# data had changed. modify_array() itself is cheap (no data copy), so
		# it's called fresh every frame -- only the row layout (stride/offset)
		# is cached, since that can't change for this vdata's lifetime.
		array_data = state.vdata.modify_array(0)
		if state.vertex_pos_offset is None:
			array_format = array_data.get_array_format()
			state.vertex_stride = array_format.get_stride() // 4
			state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4

		view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
		pos_offset = state.vertex_pos_offset
		view[:, pos_offset:pos_offset + 3] = new_positions

		return task.cont

	def _draw_wind_controls(self):
		"""Viewer-only wind preview controls (strength/direction/play-pause),
		see _update_wind() -- only shown when the loaded shape actually has
		wind data to animate (self._wind_state), since there's nothing to
		preview otherwise. Not a shape property, so it's a floating window
		top-right of the 3D viewport rather than part of the side panel,
		positioned flush against the panel's left edge (same auto-size
		tracking pattern as _draw_viewport_toggles)."""
		if self._wind_state is None:
			return

		display_width = imgui.get_io().display_size.x
		win_w, win_h = self._wind_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("Wind preview", flags=flags):
			_, self._wind_animate = imgui.checkbox("Animate", self._wind_animate)
			imgui.set_next_item_width(160)
			_, self._wind_power = imgui.slider_float("Strength", self._wind_power, 0.0, 1.0)
			imgui.set_next_item_width(160)
			_, self._wind_direction_deg = imgui.slider_float("Direction", self._wind_direction_deg, 0.0, 360.0, "%.0f deg")
			self._wind_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_viewport_toggles(self):
		"""Small floating icon-button bar bottom-left of the 3D viewport (same
		positioning pattern as navcube.py's draw_controls()): floor grid,
		world axes, object-pivot axes, 50% object transparency."""
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return

		width, height = self._viewport_toggle_size
		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##viewport-toggles", flags=flags):
			if _icon_button(fa_icons.ICON_FA_TABLE, "Floor grid (1m squares, sized to the object)",
			                self._grid_visible, square=True, large_font=large_font):
				self._toggle_grid()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_COMPASS, "World X/Y/Z axes",
			                self._world_axes_visible, square=True, large_font=large_font):
				self._toggle_world_axes()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, "Object pivot X/Y/Z axes",
			                self._pivot_axes_visible, square=True, large_font=large_font):
				self._toggle_pivot_axes()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_ADJUST, "50% object transparency",
			                self._object_transparent, square=True, large_font=large_font):
				self._toggle_object_transparency()
			self._viewport_toggle_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _transform_node(self, prop):
		"""The NodePath position/rotation/scale editing for `prop`
		("position"/"rotation"/"scale") currently applies to -- _object_pivot
		by default (moves the object along with it, the existing Ctrl+drag
		behavior), or model_root itself when that row's pivot lock is on
		(see _draw_transform_panel()): edits then change the object's own
		local offset within the pivot, so the pivot's world transform stays
		put instead of moving with it."""
		return self.model_root if self.transform_locks[prop]["pivot"] else self._object_pivot

	def _get_transform_values(self, prop):
		"""Current (x, y, z) for `prop`, from whichever node _transform_node()
		says currently owns it. Rotation is Euler degrees around each world
		axis (NodePath.get_hpr()'s heading/pitch/roll are Z/X/Y respectively),
		not the raw quaternion -- matches the panel's X/Y/Z fields."""
		return self._node_transform_values(self._transform_node(prop), prop)

	def _node_transform_values(self, node, prop):
		"""Like _get_transform_values(), but for an explicit `node` instead
		of whichever one _transform_node() currently owns `prop` -- reset
		needs both _object_pivot's and model_root's own current values
		regardless of which one is presently the active pivot."""
		if prop == "position":
			v = node.get_pos()
			return (v.x, v.y, v.z)
		if prop == "rotation":
			h, p, r = node.get_hpr()
			return (p, r, h)
		v = node.get_scale()
		return (v.x, v.y, v.z)

	def _set_node_transform(self, node, prop, values):
		"""Sets `node`'s `prop` to `values` -- same (x, y, z)/(p, r, h)
		order as _node_transform_values()."""
		if prop == "position":
			node.set_pos(*values)
		elif prop == "rotation":
			p, r, h = values
			node.set_hpr(h, p, r)
		else:
			node.set_scale(*values)

	def _quat_to_prh(self, quat):
		"""(pitch, roll, heading) for `quat` -- same order _node_transform_values()
		uses for rotation. Panda's NodePath.get_hpr() has no quaternion
		equivalent that's proven safe here, so this goes through a scratch,
		unattached, never-rendered NodePath instead."""
		scratch = NodePath("scratch")
		scratch.set_quat(quat)
		h, p, r = scratch.get_hpr()
		return (p, r, h)

	def _reset_node_transform(self, node, prop, target):
		"""Resets `node`'s `prop` to `target` (same order as
		_node_transform_values()), except any axis currently locked
		(self.transform_locks[prop]), which keeps its current value instead
		-- the same per-axis lock _set_transform_axis() already enforces
		for manual edits and camera.py's Ctrl-drag."""
		locks = self.transform_locks[prop]
		current = self._node_transform_values(node, prop)
		values = [current[i] if locks["xyz"[i]] else target[i] for i in range(3)]
		self._set_node_transform(node, prop, values)

	def _set_transform_axis(self, prop, axis_index, value):
		"""Sets one axis (0=X, 1=Y, 2=Z) of `prop` to `value`, on whichever
		node _transform_node() currently owns it -- a no-op if that axis is
		locked (see _draw_transform_panel()); ObjectManipulator (camera.py)
		enforces the same lock for Ctrl+drag."""
		if self.transform_locks[prop]["xyz"[axis_index]]:
			return
		node = self._transform_node(prop)
		if prop == "position":
			values = list(node.get_pos())
			values[axis_index] = value
			node.set_pos(*values)
		elif prop == "rotation":
			h, p, r = node.get_hpr()
			values = [p, r, h]
			values[axis_index] = value
			p, r, h = values
			node.set_hpr(h, p, r)
		else:
			values = list(node.get_scale())
			values[axis_index] = value
			node.set_scale(*values)

	def _reset_transform(self, prop):
		"""Resets `prop` to its default -- position/scale to identity,
		rotation to the shape's own baseline (_object_pivot_base_quat, see
		_display_shape()) -- on BOTH _object_pivot and model_root, regardless
		of which one that row's pivot lock currently targets, so the result
		always reads as a clean reset rather than only clearing whichever of
		the two happens to be locked right now. Any axis currently locked
		(see _draw_transform_panel()) keeps its current value instead of
		being reset, same as a manual edit on that axis would be a no-op.
		Used by reset_object_transform() (the gizmo's Home button, a
		deliberately total reset); the panel's own per-row Reset button uses
		_reset_transform_row() instead, which only touches the currently
		selected reference frame."""
		if prop == "position":
			pivot_target = model_target = (0, 0, 0)
		elif prop == "rotation":
			pivot_target = self._quat_to_prh(self._object_pivot_base_quat)
			model_target = (0, 0, 0)  # Quat()'s default constructor is the identity rotation
		else:
			pivot_target = model_target = (1, 1, 1)
		self._reset_node_transform(self._object_pivot, prop, pivot_target)
		self._reset_node_transform(self.model_root, prop, model_target)

	def _reset_transform_row(self, prop):
		"""Resets `prop` to its default -- position/scale to identity,
		rotation to the shape's own baseline (_object_pivot_base_quat) if
		that's the currently selected node, identity otherwise -- only on
		whichever node _transform_node() currently owns (pivot-locked or
		not), leaving the other reference frame untouched. Any axis
		currently locked keeps its current value instead, same as
		_reset_transform(). Bound to the panel's own per-row Reset button;
		see _reset_transform() for the gizmo's Home button, which resets
		both frames at once instead."""
		node = self._transform_node(prop)
		if prop == "position":
			target = (0, 0, 0)
		elif prop == "rotation":
			is_pivot = node is self._object_pivot
			target = self._quat_to_prh(self._object_pivot_base_quat) if is_pivot else (0, 0, 0)
		else:
			target = (1, 1, 1)
		self._reset_node_transform(node, prop, target)

	def reset_object_transform(self):
		"""Resets position, rotation, and scale together -- bound to the
		gizmo's Home button while the object is targeted (see
		navcube.py's draw_controls())."""
		for prop in ("position", "rotation", "scale"):
			self._reset_transform(prop)

	def _draw_transform_panel(self):
		"""Position/Rotation/Scale editor, floating in the viewport flush
		against the left edge of the navcube gizmo's own pixel rect, its
		bottom edge aligned with _draw_viewport_toggles()'s bottom-left icon
		bar (same y formula, own height). One row per property: a pivot-lock
		toggle, X/Y/Z axis-lock toggles, X/Y/Z value fields (live,
		see _get_transform_values()/_set_transform_axis()), and a
		row reset button (_reset_transform()). Hidden until a shape (with a
		pivot worth editing) is loaded."""
		if self.shape_file is None:
			return
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return
		left, _right, _top, _bottom = self.nav_cube._panel_px
		width, height = self._transform_panel_size
		x = left - _VIEWPORT_TOGGLE_MARGIN_PX - width
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("##transform-panel", flags=flags):
			self._draw_transform_row("position", "Pos", "%.3f")
			self._draw_transform_row("rotation", "Rot", "%.2f")
			self._draw_transform_row("scale", "Scl", "%.3f")
			self._transform_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_transform_row(self, prop, label, value_format):
		"""One _draw_transform_panel() row -- see its own docstring. `label`
		is the row's own text label (not editable); `value_format` sets the
		X/Y/Z fields' display precision (position/scale: float32 is good to
		~7 significant digits, well past what's ever meaningful in meters or
		a scale multiplier here -- 3 decimals; rotation: 2 decimals, since a
		float32 quaternion's angular resolution is far finer than 0.01
		degrees already)."""
		imgui.push_id(f"xform-{prop}")
		locks = self.transform_locks[prop]

		imgui.text(label)
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_ANCHOR,
		                "Lock pivot: edits move the object within a fixed pivot, instead of moving the pivot itself",
		                locks["pivot"], square=True, active_color=_LOCKED_COLOR):
			locks["pivot"] = not locks["pivot"]

		values = list(self._get_transform_values(prop))
		for axis_index, axis_name in enumerate("XYZ"):
			axis_locked = locks[axis_name.lower()]
			imgui.same_line()
			if _icon_button(axis_name, f"Lock {axis_name} (never changes, drag or typed)",
			                axis_locked, square=True, active_color=_LOCKED_COLOR):
				locks[axis_name.lower()] = not axis_locked

			imgui.same_line()
			imgui.set_next_item_width(70)
			imgui.begin_disabled(axis_locked)
			v_speed, v_min, v_max = _TRANSFORM_DRAG_PARAMS[prop]
			changed, new_value = imgui.drag_float(
				f"##{prop}-{axis_name}", values[axis_index], v_speed=v_speed, v_min=v_min, v_max=v_max,
				format=value_format)
			imgui.end_disabled()
			if changed:
				self._set_transform_axis(prop, axis_index, new_value)

		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_UNDO, f"Reset {label.lower()} (current reference frame only)", square=True):
			self._reset_transform_row(prop)

		imgui.pop_id()

	def _rebuild_geometry(self):
		"""(Re)builds the 3D render passes + camera framing from
		self.shape_file.value -- shared by _display_shape() and by replacing
		a Mesh's geometry in place (which intentionally does NOT go through
		_reset_shape_state(), so material edits survive it)."""
		self.model_root.remove_node()
		self.model_root = self._object_pivot.attach_new_node("shape-root")
		self._apply_object_transparency()
		self.shape_error = None
		self._material_node_paths = {}
		self._wind_state = None
		self._skin_state = None
		self._texture_needs_repeat = False

		geom_value = shape_geom(self.shape_file.value)
		# CMeshMRMSkinnedGeom has no vertex_program field at all (no wind
		# animation support for skinned characters/creatures in NeL).
		wind_params = getattr(geom_value, "vertex_program", None)
		wind_params = wind_params if isinstance(wind_params, WindTreeParams) else None

		# CMeshMRMSkinned reuses the Skinning preview's own loaded skeleton/
		# animation (see _bone_world_matrices_for()), independent of any
		# attach bone chosen there. Without one loaded yet,
		# iter_render_passes() below still renders -- its own rigid bind-pose
		# fallback, matching the real client's behavior for an unskinned
		# CMeshMRMSkinned instance.
		is_skinned = isinstance(self.shape_file.value, MeshMRMSkinned)
		skeleton = self._bone_preview_skeleton if is_skinned else None
		bone_world_matrices = self._bone_world_matrices_for(geom_value.bones_name) if skeleton is not None else None
		if is_skinned and skeleton is not None:
			self._skin_state = _build_skin_state(geom_value, skeleton)

		vdata = None
		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(
				self.shape_file.value, skeleton=skeleton, bone_world_matrices=bone_world_matrices):
			if not indices:
				continue
			print(f"[object_editor] pass {pass_count}: material={material_id} "
			      f"verts={vertex_buffer.num_verts} tris={len(indices) // 3} "
			      f"channels={list(vertex_buffer.channels.keys())}")
			if vdata is None:
				# Built once, not per-pass: every pass indexes into the exact
				# same vertex array (see iter_render_passes()), and the wind/
				# skin preview needs a single vdata to update per frame rather
				# than one per pass.
				vdata = _build_vertex_data(vertex_buffer, dynamic=wind_params is not None or self._skin_state is not None)
				if wind_params is not None:
					self._wind_state = _build_wind_state(wind_params, vertex_buffer)
				self._texture_needs_repeat = _uvs_need_repeat(vertex_buffer.channels.get("TexCoord0"))
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"pass-{pass_count}")
			geom_node.add_geom(geom)
			node_path = self.model_root.attach_new_node(geom_node)
			self._material_node_paths.setdefault(material_id, []).append(node_path)
			self._apply_material(node_path, material_id)
			pass_count += 1

		if self._wind_state is not None:
			self._wind_state.vdata = vdata
		if self._skin_state is not None:
			self._skin_state.vdata = vdata

		if pass_count == 0:
			self.shape_error = f"No renderable 3D geometry for shape type {self.shape_file.type_name!r}"

		bbox = shape_bbox(self.shape_file.value)
		if bbox is not None:
			local_center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
			# shape_bbox() is in model_root's own local space (pre-pivot-
			# rotation) -- OrbitCamera.frame() expects a world/render-space
			# target, so this must go through model_root's current transform
			# (which includes _object_pivot's rotation, e.g. a seeded
			# default_rot_quat) rather than being used as-is.
			center = self.render.get_relative_point(self.model_root, local_center)
			radius = max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1)
			self.orbit_camera.frame(center, radius * 5.65)
		self._rebuild_viewport_helpers(bbox)

		self._rebuild_reference_shapes()

	def _get_reference_shape(self, label):
		"""Lazily parses (once) and caches a reference shape by label."""
		if label in self._reference_shapes:
			return self._reference_shapes[label]
		filename = dict(_REFERENCE_SHAPES)[label]
		path = _REFERENCE_EXAMPLES_DIR / filename
		try:
			shape_file = parse_shape(path.read_bytes())
		except (OSError, ShapeParseError) as exc:
			print(f"[object_editor] failed to load reference shape {label!r} ({path}): {exc}")
			shape_file = None
		self._reference_shapes[label] = shape_file
		return shape_file

	def _toggle_reference_shape(self, label):
		if label in self._reference_active:
			self._reference_active.discard(label)
		elif self._get_reference_shape(label) is not None:
			self._reference_active.add(label)
		self._rebuild_reference_shapes()

	def _set_reference_placement(self, label, placement):
		"""Clicking the already-active placement button turns it back off
		(back to the default side-by-side layout) instead of being stuck on."""
		current = self._reference_placement.get(label, "auto")
		self._reference_placement[label] = "auto" if current == placement else placement
		self._rebuild_reference_shapes()

	def _toggle_reference_transparency(self, label):
		if label in self._reference_transparent:
			self._reference_transparent.discard(label)
		else:
			self._reference_transparent.add(label)
		self._rebuild_reference_shapes()

	def _build_reference_geometry(self, shape_value, parent_node_path):
		materials = getattr(shape_value, "materials", None)
		vdata = None
		for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
			if not indices:
				continue
			if vdata is None:
				# Built once, not per-pass -- see _build_vertex_data()'s docstring.
				vdata = _build_vertex_data(vertex_buffer)
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"ref-pass-{material_id}")
			geom_node.add_geom(geom)
			node_path = parent_node_path.attach_new_node(geom_node)
			material = materials[material_id] if materials and material_id < len(materials) else None
			self._apply_material_common(node_path, material)
			self._apply_material_texture(node_path, material, None)

	def _rebuild_reference_shapes(self):
		"""Lines up the currently-active reference shapes side by side, just
		past the main shape's own bbox (bottoms/centers aligned on X, so
		comparing sizes doesn't need any camera repositioning) -- unless a
		shape has its own placement mode set (see _set_reference_placement()),
		in which case it's placed at the world origin or the main object's
		pivot instead, and skipped when advancing the side-by-side cursor."""
		self._reference_root.remove_node()
		self._reference_root = self.render.attach_new_node("reference-root")

		main_bbox = shape_bbox(self.shape_file.value) if self.shape_file is not None else None
		cursor_x = (main_bbox.center.x + main_bbox.half_size.x + _REFERENCE_GAP) if main_bbox is not None else 0.0

		for label, _ in _REFERENCE_SHAPES:
			if label not in self._reference_active:
				continue
			shape_file = self._reference_shapes.get(label)
			if shape_file is None:
				continue
			bbox = shape_bbox(shape_file.value)
			node_path = self._reference_root.attach_new_node(label)
			placement = self._reference_placement.get(label, "auto")
			if placement == "origin":
				node_path.set_pos(self.render, 0, 0, 0)
			elif placement == "pivot":
				node_path.set_pos(self.render, self._object_pivot.get_pos(self.render))
			else:
				if bbox is not None:
					node_path.set_x(cursor_x + bbox.half_size.x - bbox.center.x)
			self._build_reference_geometry(shape_file.value, node_path)
			if label in self._reference_transparent:
				node_path.set_transparency(TransparencyAttrib.M_alpha)
				node_path.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
			if placement == "auto" and bbox is not None:
				cursor_x += bbox.half_size.x * 2 + _REFERENCE_GAP

	def _draw_reference_shapes_toggles(self):
		"""Top-left viewport bar for the 3 scale-reference toggles (Cube /
		shortest / tallest character) -- square icon buttons at 2x
		_draw_viewport_toggles()'s icon size (app.large_icon_font), since
		these are a more prominent, deliberately-reached-for control. Once a
		reference shape is active, 3 more square buttons appear stacked
		vertically right below its toggle (placement: origin/pivot, plus a
		transparency toggle) -- each shape's column is its own imgui group so
		everything stays aligned regardless of how many buttons the column
		has."""
		display_size = imgui.get_io().display_size
		if display_size.y <= 0:
			return

		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##reference-shapes-toggles", flags=flags):
			for i, (label, _) in enumerate(_REFERENCE_SHAPES):
				if i > 0:
					imgui.same_line()
				imgui.push_id(f"ref-shape-{label}")
				with imgui_ctx.begin_group():
					if _icon_button(_REFERENCE_ICONS[label], label, label in self._reference_active,
					                square=True, large_font=large_font):
						self._toggle_reference_shape(label)
					if label in self._reference_active:
						placement = self._reference_placement.get(label, "auto")
						if _icon_button(fa_icons.ICON_FA_DOT_CIRCLE, "Place at 0,0,0",
						                placement == "origin", square=True, large_font=large_font):
							self._set_reference_placement(label, "origin")
						if _icon_button(fa_icons.ICON_FA_ANCHOR, "Place on the object's pivot",
						                placement == "pivot", square=True, large_font=large_font):
							self._set_reference_placement(label, "pivot")
						if _icon_button(fa_icons.ICON_FA_ADJUST, "50% transparent",
						                label in self._reference_transparent, square=True, large_font=large_font):
							self._toggle_reference_transparency(label)
				imgui.pop_id()

	@staticmethod
	def _apply_material_common(node_path, material):
		"""Clears any render state possibly left over from a previous
		material, then applies the parts that don't depend on
		material_id/override-color bookkeeping: two-sided, depth-write.
		Shared by _apply_material() (the current shape's own, editable
		materials) and reference-shape rendering (_build_reference_geometry()),
		which has no material_id/override-color concept of its own."""
		node_path.clear_texture()
		node_path.clear_color()
		node_path.clear_material()
		node_path.clear_light()
		node_path.clear_transparency()
		node_path.clear_attrib(ColorBlendAttrib)
		node_path.clear_attrib(AlphaTestAttrib)
		node_path.clear_depth_write()

		# Follow the shape's own CMaterial::DOUBLE_SIDED flag rather than
		# always forcing two-sided rendering: some materials (e.g. additive
		# glass) look wrong when both faces of the geometry render, since
		# their blend stacks each overlapping face's contribution.
		node_path.set_two_sided(bool(material.flags & _IDRV_MAT_DOUBLE_SIDED) if material is not None else True)

		# Follow CMaterial::ZWRITE too: blended materials (e.g. glass) often
		# have it off on purpose, so they don't occlude whatever's behind
		# them in the depth buffer -- leaving depth write on (Panda's
		# default) made such materials intermittently hide unrelated
		# geometry depending on draw order.
		node_path.set_depth_write(bool(material.flags & _IDRV_MAT_ZWRITE) if material is not None else True)

	def _panoply_dims_for(self, base_name):
		"""Shape-wide panoply axis selection (_panoply_selection, see
		_draw_global_panoply_section()) narrowed down to just the axes
		`base_name` actually has a variant for, per panoply_files.txt --
		{} if nothing is selected, or this texture has no variants at all
		(e.g. a buckle/weapon that doesn't change color). Shared by
		_resolve_panoply_texture_name() (what file name to try loading) and
		_ensure_live_panoply_texture() (what to try computing live if that
		file turns out to be missing/stale)."""
		if not self._panoply_selection or not base_name:
			return {}
		available = self.search_paths_dialog.panoply_variants_for(base_name)
		return {
			axis: value for axis, value in self._panoply_selection.items()
			if value in available.get(axis, ())
		}

	def _resolve_panoply_texture_name(self, base_name):
		"""`base_name` unchanged, unless _panoply_dims_for() finds one or
		more applicable axis picks -- only ever affects what gets loaded for
		rendering/preview, the shape's own material data (base_name itself)
		is never modified. When the resulting variant is missing or stale on
		disk (older than the base texture or a mask it was built from --
		panoply_live.is_baked_stale()), and every mask this selection needs
		is actually available, pre-computes it live in memory
		(_ensure_live_panoply_texture()) and inserts it into
		self._texture_cache under the resolved name -- every caller of
		load_panda_texture(resolved_name, cache=self._texture_cache, ...)
		(3D render, thumbnails, popups) then picks it up transparently
		through that cache's own name lookup, no special case of their own
		needed. Never writes to disk. Also records `base_name`/`dims` under
		the resolved name in self._panoply_texture_sources, so
		_update_texture_freshness() can keep re-checking this combination
		later even though this method itself only runs again when the
		material actually gets re-applied."""
		dims = self._panoply_dims_for(base_name)
		if not dims:
			return base_name
		resolved_name = panoply.variant_file_name(base_name, **dims)
		self._panoply_texture_sources[resolved_name] = (base_name, dims)
		self._ensure_live_panoply_texture(base_name, resolved_name, dims)
		return resolved_name

	def _resolve_panoply_refs(self, base_name, dims):
		"""Every search_paths.FoundEntry a live recompute of this
		(base_name, dims) combination would need or want to know about: the
		base texture (None if it can't be resolved at all), each dims
		axis's mask (only the ones that actually resolve -- missing ones
		are just absent from the dict, not None entries), and whatever's
		already on disk for the baked variant name itself (None if
		nothing's there). Shared by _ensure_live_panoply_texture() (decides
		whether/how to (re)compute) and _update_texture_freshness() (decides
		whether a previously-resolved combination needs re-checking) so
		both always resolve names exactly the same way."""
		finder = self.search_paths_dialog.find_texture
		base_ref = resolve_texture_ref(base_name, self._texture_search_dirs, finder)
		stem = Path(base_name).stem
		mask_refs = {}
		for axis in dims:
			mask_ref = resolve_texture_ref(f"{stem}_{axis}.tga", self._texture_search_dirs, finder)
			if mask_ref is not None:
				mask_refs[axis] = mask_ref
		resolved_name = panoply.variant_file_name(base_name, **dims)
		baked_ref = resolve_texture_ref(resolved_name, self._texture_search_dirs, finder)
		return base_ref, mask_refs, baked_ref

	@staticmethod
	def _panoply_freshness_signature(base_ref, mask_refs, baked_ref):
		"""(base mtime, sorted mask mtimes, baked mtime-or-None) for a
		resolved combination's current sources -- not "is this stale"
		(panoply_live.is_baked_stale() already answers that), but "has
		anything actually changed since I last looked", which is what
		_update_texture_freshness() needs to avoid re-evicting/recomputing
		an unstale-but-not-baked live combination every single tick."""
		base_mtime = base_ref.cache_stat()[0] if base_ref is not None else None
		mask_mtimes = tuple(sorted((axis, ref.cache_stat()[0]) for axis, ref in mask_refs.items()))
		baked_mtime = baked_ref.cache_stat()[0] if baked_ref is not None else None
		return base_mtime, mask_mtimes, baked_mtime

	def _ensure_live_panoply_texture(self, base_name, resolved_name, dims):
		"""Best-effort: if `resolved_name` isn't already cached and the
		already-baked file for it turns out to be missing/stale
		(panoply_live.is_baked_stale()), recolors the base texture live in
		memory (panoply_colorize.py, parameters from panoply_config.py) and
		pre-inserts the result into self._texture_cache -- see
		_resolve_panoply_texture_name()'s docstring for why that's enough
		for every caller to pick it up. A no-op whenever the base texture or
		any mask this selection needs can't be resolved, or this texture's
		race doesn't actually have one of the selected axes (e.g. no "eyes"
		for zorai) -- falls through to whatever load_panda_texture() itself
		finds on disk instead, same as if this method didn't exist. Records
		the freshness signature seen at this point either way (even when
		the baked file was fine as-is), so _update_texture_freshness() has
		a baseline to compare later ticks against."""
		if resolved_name in self._texture_cache:
			return
		base_ref, mask_refs, baked_ref = self._resolve_panoply_refs(base_name, dims)
		if base_ref is None or set(mask_refs) != set(dims):
			return  # missing the base texture or a mask one of the selected axes needs -- can't compute this live

		self._panoply_texture_signatures[resolved_name] = self._panoply_freshness_signature(base_ref, mask_refs, baked_ref)
		if not panoply_live.is_baked_stale(baked_ref, base_ref, mask_refs):
			return  # the baked file on disk is fine as-is -- load_panda_texture() will load it normally

		stem = Path(base_name).stem
		cache_key = panoply_live.LiveColorizeCache.make_key(base_name, dims, base_ref, mask_refs)
		result_rgba = self._live_panoply_cache.get(cache_key)
		if result_rgba is None:
			base_rgba = panoply_texture.ref_to_rgba_array(base_ref)
			if base_rgba is None:
				return
			race = panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())
			axis_masks = []
			for axis in panoply.AXES:
				if axis not in dims:
					continue
				params = panoply_config.get_color_params(axis, panoply.color_id_for(axis, dims[axis]), race)
				if params is None:
					return
				mask_rgba = panoply_texture.ref_to_rgba_array(mask_refs[axis])
				if mask_rgba is None:
					return
				axis_masks.append((mask_rgba[..., 0], params))  # mask weight lives in the red channel
			result_rgba = panoply_colorize.colorize(base_rgba, axis_masks)
			self._live_panoply_cache.set(cache_key, result_rgba)

		self._texture_cache[resolved_name] = panoply_texture.rgba_array_to_texture(result_rgba)

	_TEXTURE_FRESHNESS_CHECK_INTERVAL = 1.0  # seconds

	def _update_texture_freshness(self, task):
		"""Runs about once a second, two checks in one pass:

		1. Every Panoply-affected texture name currently in play
		(self._panoply_texture_sources), re-resolved against its current
		on-disk sources and compared to the signature recorded when it was
		last resolved (_panoply_freshness_signature()) -- on a mismatch (a
		base texture, mask, or baked file was added/edited/removed since),
		evicts it from self._texture_cache, which forces
		_resolve_panoply_texture_name()/_ensure_live_panoply_texture() to
		run again for it on the next material re-apply below.

		2. Every other (non-Panoply) name already in self._texture_cache --
		a plain material texture, or whichever Multi Bitmap slot is
		currently selected (see _pad_multi_bitmap_slots()/_apply_material_
		texture() -- only the selected slot's name is ever actually loaded/
		cached, so this needs no Multi-Bitmap-specific handling) -- re-
		resolved and compared by mtime against self._texture_freshness_
		mtimes (seeded in _apply_material_texture() at load time, not here,
		so the very first tick after a texture is first cached never sees a
		spurious "changed"). Deliberately restricted to files inside the
		active workspace (FoundEntry.fs_path, skipping .bnp entries and
		anything outside it) -- textures from other search paths (mods, the
		Ryzom install, etc.) are left alone, so the workspace stays the one
		place edits get picked up live, same idea as the auto-export
		imports/ watcher only ever touching the active workspace.

		Either check evicting anything forces every material to re-apply
		once, at the end. This is Forgery's whole answer to "I edited a
		texture in Gimp, now what" -- picked automatically, with no manual
		reload action: see .todo/forgery-object-editor.md's Phase A Step 5
		for why a manual "Reload" button was decided against once the
		Panoply half of this existed."""
		if task.time - self._texture_freshness_last_check < self._TEXTURE_FRESHNESS_CHECK_INTERVAL:
			return task.cont
		self._texture_freshness_last_check = task.time

		changed = False
		if self._panoply_cfg_changed:
			# The workspace's panoply.cfg (or its absence) changed -- see
			# _on_panoply_cfg_settled(). Every already-rendered Panoply
			# texture was colorized with whatever parameters were active at
			# the time, and neither _texture_cache nor _live_panoply_cache's
			# keys depend on those parameters (see LiveColorizeCache.make_key()),
			# so nothing else would ever notice this changed -- evict every
			# Panoply-tracked entry and drop the whole live-colorize cache,
			# forcing a full recompute against the new colors below.
			self._panoply_cfg_changed = False
			for resolved_name in list(self._panoply_texture_sources):
				self._texture_cache.pop(resolved_name, None)
			self._panoply_texture_signatures.clear()
			self._live_panoply_cache.clear()
			changed = True

		for resolved_name, (base_name, dims) in list(self._panoply_texture_sources.items()):
			if resolved_name not in self._texture_cache:
				continue
			base_ref, mask_refs, baked_ref = self._resolve_panoply_refs(base_name, dims)
			if base_ref is None or set(mask_refs) != set(dims):
				continue
			signature = self._panoply_freshness_signature(base_ref, mask_refs, baked_ref)
			if signature != self._panoply_texture_signatures.get(resolved_name):
				del self._texture_cache[resolved_name]
				self._panoply_texture_signatures.pop(resolved_name, None)
				changed = True

		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is not None:
			finder = self.search_paths_dialog.find_texture
			for name in list(self._texture_freshness_mtimes):
				if name not in self._texture_cache or name in self._panoply_texture_sources:
					self._texture_freshness_mtimes.pop(name, None)
					continue
				ref = resolve_texture_ref(name, self._texture_search_dirs, finder)
				if ref is None or ref.fs_path is None or not ref.fs_path.is_relative_to(workspace_dir):
					continue
				try:
					mtime = ref.cache_stat()[0]
				except OSError:
					# Resolved a moment ago (resolve_texture_ref() above), but
					# gone by the time we stat it -- deleted/renamed between
					# the two, e.g. externally or via a workspace file-manager
					# action. Leave the cached texture as the last-known-good
					# one instead of crashing the whole render loop over it.
					continue
				if mtime != self._texture_freshness_mtimes[name]:
					del self._texture_cache[name]
					self._texture_freshness_mtimes.pop(name, None)
					changed = True

		if changed:
			self._reapply_all_materials()
		return task.cont

	def _shape_texture_names(self):
		"""Every distinct, currently-active texture base name across the
		whole shape's materials (slot 0 -- for a Multi Bitmap material,
		texture.file_name is whichever quality/season/ecosystem set is
		presently selected, same one _apply_material_texture() renders) --
		the set _draw_global_panoply_section() checks against
		panoply_files.txt to decide which axis buttons to offer."""
		materials = getattr(self.shape_file.value, "materials", None) or []
		names = set()
		for material in materials:
			texture = material.textures[0] if material.textures else None
			if texture is not None and texture.file_name:
				names.add(texture.file_name)
		return names

	_PANOPLY_AXIS_LABELS = {"skin": "Skin", "user": "User color", "hair": "Hair", "eyes": "Eyes"}

	def _copy_panoply_cfg_to_workspace(self):
		"""Copies the bundled default panoply.cfg (panoply_config.py) as-is
		into the active workspace's root, as an editable starting point --
		once there, panoply_config._resolve_cfg_path() always prefers it
		over the bundled default from then on (both for the live-preview
		colors and any real bake -- see _bake_panoply_real()). Picked from
		the gear icon next to "Panoply:" in _draw_global_panoply_section()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		dest = panoply_config.workspace_cfg_path(workspace_dir)
		try:
			shutil.copyfile(panoply_config.bundled_cfg_path(), dest)
		except OSError as exc:
			self._save_status = f"Could not copy panoply.cfg: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Copied panoply.cfg to workspace ({dest})"
		print(f"[object_editor] {self._save_status}")

	def _on_panoply_cfg_settled(self, path):
		"""Registered onto the shared WorkspaceWatcher for the workspace
		root's own panoply.cfg -- a root-level file's relative_to(workspace_dir)
		has a single part, which WorkspaceWatcher._dispatch() treats as its
		own "subdir" key, so this fires whenever <workspace>/panoply.cfg
		itself settles (created, edited, or deleted), same mechanism as the
		"tex"/"imports" subfolder registrations just needs no subfolder here.
		Runs off that watcher's background thread -- only sets a flag,
		_update_texture_freshness() (main thread, ~once a second) does the
		actual cache eviction + material re-apply on the next tick. Without
		this, an edited workspace panoply.cfg would never be picked up by
		the live preview: panoply_config.py's own cache invalidates itself
		lazily by mtime, but nothing else would know to recompute/re-cache
		the already-rendered Panoply textures for the new colors."""
		self._panoply_cfg_changed = True

	def _draw_global_panoply_section(self):
		"""Ryzom's pre-baked texture recolors (see panoply.py), shape-wide:
		each axis's pick (skin tone, user/craft color, hair color, eye color)
		applies to every one of the shape's textures at once, independently
		of the other axes -- skin is a race skin-tone difference (Fyros
		tanned, Matis pale, Tryker in between, Zorai blue), user color is an
		item's craft color, both meant to be uniform across a whole equipped
		piece, not picked per texture; hair/eyes are the same idea for a
		head/face shape. A texture with no mask for a given axis (e.g. an
		armor piece has no hair/eyes masks, a hairstyle has no user masks)
		just keeps rendering its own base name for that axis -- expected, not
		a missing-data problem (see _resolve_panoply_texture_name()). Shown
		once, at the top of the Textures and Materials tabs alike. Purely a
		render override -- never edits the shape's own material data.

		Buttons offered on each axis are the union across every texture the
		shape actually uses right now (no single texture's own list is
		authoritative for what to show, since different textures can support
		different axes/values). No real texture on disk is ever the base
		name alone with just one axis picked (e.g. "..._FY.tga" doesn't
		exist, only "..._FY_U1.tga" does -- panoply_maker's masks are always
		baked together whenever more than one applies to the same texture),
		so the first click on an empty selection backfills every other
		available axis with its first value too, instead of resolving to a
		texture that isn't actually on disk (rendering as blank/white)."""
		texture_names = self._shape_texture_names()
		available = {axis: set() for axis in panoply.AXES}
		for name in texture_names:
			for axis, values in self.search_paths_dialog.panoply_variants_for(name).items():
				available[axis].update(values)

		imgui.text("Panoply:")
		imgui.same_line()
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		has_override = workspace_dir is not None and panoply_config.workspace_cfg_path(workspace_dir).is_file()
		if has_override:
			editor_path = app_settings.load().text_editor_path
			if _icon_button(fa_icons.ICON_FA_EDIT, "Edit this workspace's panoply.cfg in the configured text editor"):
				if not editor_path:
					self.request_settings_attention("Tools", "text_editor_path")
				else:
					subprocess.Popen([editor_path, str(panoply_config.workspace_cfg_path(workspace_dir))])
		else:
			tooltip = "Copy the bundled panoply.cfg into this workspace, to edit its colors"
			if _icon_button(fa_icons.ICON_FA_COG, tooltip, disabled=workspace_dir is None):
				self._copy_panoply_cfg_to_workspace()
		imgui.same_line()
		if _icon_button(
			fa_icons.ICON_FA_FIRE, "Bake real Panoply variants for every texture of this shape (writes tex/ + build/)",
			disabled=workspace_dir is None,
		):
			self._bake_panoply_real_all()
		if not any(available.values()):
			imgui.same_line()
			imgui.text_disabled("no variants detected for this shape's textures")
			imgui.separator()
			return

		axis_values = {}
		for axis in panoply.AXES:
			values = panoply.RACES if axis == "skin" else sorted(available[axis])
			axis_values[axis] = [value for value in values if value in available[axis]]

		# Auto-default once per shape (2026-08-29): without this, a freshly
		# loaded shape with real variants (_panoply_selection still empty,
		# per _reset_shape_state()) rendered blank/white until the user
		# clicked a button themselves -- same backfill-with-first-value
		# logic as an empty-selection click below, just applied proactively
		# the first time real variants are detected instead of waiting for
		# a click. _panoply_selection_defaulted (not "is _panoply_selection
		# empty") is what gates this, so it fires exactly once per shape --
		# a later click on "skin" that deliberately clears back to the
		# shape's own base texture (see below) must stay cleared, not get
		# re-defaulted right back on the next frame.
		if not self._panoply_selection_defaulted and any(axis_values.values()):
			for axis, values in axis_values.items():
				if values:
					self._panoply_selection[axis] = values[0]
			self._panoply_selection_defaulted = True
			self._reapply_all_materials()

		for axis in panoply.AXES:
			values = axis_values[axis]
			if not values:
				continue
			selected = self._panoply_selection.get(axis)
			imgui.text(f"{self._PANOPLY_AXIS_LABELS[axis]}:")
			for value in values:
				imgui.same_line()
				active = value == selected
				label = value if axis == "skin" else f"{axis[0]}{value}"
				if _icon_button(label, f"{self._PANOPLY_AXIS_LABELS[axis]} {value!r}", active=active):
					if active:
						# Only "skin" can be turned back off (falling back to
						# the shape's own base texture, its meaningful "no
						# override" state) -- user/hair/eyes never had a
						# "none" state to begin with (no real texture on disk
						# for e.g. just "..._U1.tga" alone, always paired with
						# a skin), so clicking their already-active button is
						# a no-op, not a way to leave the shape half-resolved.
						if axis == "skin":
							self._panoply_selection.clear()
						else:
							continue
					else:
						was_empty = not self._panoply_selection
						self._panoply_selection[axis] = value
						if was_empty:
							for other_axis, other_values in axis_values.items():
								if other_axis != axis and other_values:
									self._panoply_selection[other_axis] = other_values[0]
					self._reapply_all_materials()
		imgui.separator()

	def _create_panoply_mask(self, base_texture_name, axis):
		"""Creates a new mask file for `axis` -- solid black (0 weight
		everywhere, no effect until painted over), at the same pixel
		dimensions as `base_texture_name`'s own texture -- directly in the
		active workspace's masks/ folder. Picked from the "+" button in
		_draw_panoply_masks_for()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		base_panda_texture = load_panda_texture(
			base_texture_name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
			finder=self.search_paths_dialog.find_texture)
		width = base_panda_texture.get_x_size() if base_panda_texture is not None else 256
		height = base_panda_texture.get_y_size() if base_panda_texture is not None else 256
		image = PNMImage(width, height)
		image.fill(0, 0, 0)
		stem = Path(base_texture_name).stem
		dest = workspace_dir / "masks" / f"{stem}_{axis}.tga"
		try:
			dest.parent.mkdir(parents=True, exist_ok=True)
			image.write(str(dest))
		except OSError as exc:
			self._save_status = f"Could not create mask {dest.name}: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Created mask {dest.name}"
		print(f"[object_editor] {self._save_status}")

	def _bake_panoply_real(self, base_texture_name, on_variant=None):
		"""Real offline Panoply bake (panoply_bake.py -- the exact,
		non-live-preview port of panoply_maker.cpp, see
		/repos/project-todos/ryzom-core/panoply-runtime-tint.md) for one
		base texture: writes every color variant into the active workspace's
		tex/, and an updated .hlsinfo/panoply_files.txt/characters.hlsbank
		into its build/ (started from, never overwriting, the real
		production files under ryzom-data's final_bnps/characters_maps_hr/,
		see panoply_bake.bake_and_write()'s docstring). Distinct from the
		approximate live-preview already driving the axis pickers in
		_draw_global_panoply_section() -- this produces the real baked files
		panoply_maker.exe would, not just a render override. Restricted to
		plain on-disk files (FoundEntry.fs_path) -- a base texture or mask
		living only inside a .bnp can't be baked from here.

		Runs on _start_panoply_bake()'s background thread (2026-08-29) --
		never touches imgui or Settings-attention (the active-workspace/
		ryzom-data checks live in _start_panoply_bake() instead, on the main
		thread, before the thread is even started -- neither imgui calls nor
		request_settings_attention() are safe off the main thread).
		`on_variant`, if given, is forwarded straight to
		panoply_bake.bake_and_write() for live progress reporting. Returns
		the list of written texture paths (empty if nothing was written)."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		entry = self.search_paths_dialog.find_texture(base_texture_name)
		if entry is None or entry.fs_path is None:
			self._save_status = f"Can't bake {base_texture_name}: not a plain file on disk"
			print(f"[object_editor] {self._save_status}")
			return []

		stem = Path(base_texture_name).stem
		race = panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())
		axes = panoply_bake.axes_for_source(stem, race)

		def _mask_loader(mask_ext):
			mask_entry = self.search_paths_dialog.find_texture(f"{stem}_{mask_ext}.tga")
			if mask_entry is None or mask_entry.fs_path is None:
				return None
			return panoply_bake.load_mask_luminance(mask_entry.fs_path)

		data_root = repository_paths.get("ryzom-data")
		hlsbank_source = data_root / "final_bnps" / "characters_maps_hr" / "characters.hlsbank"
		panoply_files_source = data_root / "final_bnps" / "characters_maps_hr" / "panoply_files.txt"

		try:
			base_rgba = dds_export.load_rgba(str(entry.fs_path))
			written = panoply_bake.bake_and_write(
				stem, base_rgba, axes, _mask_loader, workspace_dir / "tex", workspace_dir / "build",
				hlsbank_source, panoply_files_source, on_variant=on_variant,
			)
		except (OSError, RuntimeError) as exc:
			self._save_status = f"Bake failed for {base_texture_name}: {exc}"
			print(f"[object_editor] {self._save_status}")
			return []

		return written

	def _panoply_texture_has_mask(self, base_texture_name):
		"""Same "has at least one resolvable mask" check as
		_draw_panoply_masks_for()'s own mask_names list -- shared by
		_bake_panoply_real_all() below and that method's fire button."""
		stem = Path(base_texture_name).stem
		return any(self.search_paths_dialog.find_texture(f"{stem}_{axis}.tga") is not None for axis in panoply.AXES)

	def _bake_panoply_real_all(self):
		"""Starts a background bake (see _start_panoply_bake()) of every
		texture of the currently loaded shape that has at least one
		resolvable Panoply mask -- the "bake all" counterpart to the
		per-texture fire button in _draw_panoply_masks_for(), shown next to
		the gear/edit button in _draw_global_panoply_section() since baking
		is naturally a shape-wide action from the user's point of view, even
		though _bake_panoply_real() itself only ever handles one base
		texture at a time (each texture can have its own distinct set of
		masks)."""
		names = [name for name in self._shape_texture_names() if self._panoply_texture_has_mask(name)]
		self._start_panoply_bake(names)

	def _start_panoply_bake(self, texture_names):
		"""Starts a background thread that bakes every name in
		`texture_names` (in order) via _bake_panoply_real(), updating
		self._bake_progress as it goes so _draw_bake_progress_popup() can
		render live feedback (current texture/variant, an overall progress
		bar) -- baking is CPU-bound (numpy) and slow enough (~1s/variant on
		the real machine, 2026-08-29 cross-validation) that running it on
		the main thread would freeze the whole UI for however long the bake
		takes. Call from the main thread only: the checks below do imgui/
		request_settings_attention calls, which aren't safe off the main
		thread -- only _run_panoply_bake() (pure numpy/file I/O) actually
		runs in the background thread."""
		if self._bake_progress is not None and not self._bake_progress["done"]:
			return  # a bake is already running
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		if not repository_paths.is_valid("ryzom-data"):
			self.request_settings_attention("Paths", "ryzom-data")
			return
		if not texture_names:
			return

		self._bake_progress = {
			"texture_index": 0, "texture_total": len(texture_names), "texture_name": texture_names[0],
			"variant_suffix": "", "variant_index": 0, "variant_total": 0,
			"done": False, "error": None,
		}
		thread = threading.Thread(target=self._run_panoply_bake, args=(list(texture_names),), daemon=True)
		thread.start()

	def _run_panoply_bake(self, texture_names):
		"""Background-thread body for _start_panoply_bake() above -- never
		touches imgui or Settings-attention, only self._bake_progress (plain
		dict field writes, safe enough under the GIL for this simple
		main-thread-polls-it use, no lock needed) and _bake_panoply_real()'s
		own numpy/file I/O."""
		total_written = 0
		for index, name in enumerate(texture_names):
			progress = self._bake_progress
			progress["texture_index"] = index
			progress["texture_name"] = name
			progress["variant_suffix"] = ""
			progress["variant_index"] = 0
			progress["variant_total"] = 0

			def _on_variant(suffix, variant_index, variant_total, progress=progress):
				progress["variant_suffix"] = suffix
				progress["variant_index"] = variant_index
				progress["variant_total"] = variant_total

			written = self._bake_panoply_real(name, on_variant=_on_variant)
			total_written += len(written)

		self._bake_progress["done"] = True
		self._save_status = f"Baked {total_written} variant(s) across {len(texture_names)} texture(s) to tex/ + build/"
		print(f"[object_editor] {self._save_status}")

	def _draw_bake_progress_popup(self):
		"""Modal, opened/kept open for the whole lifetime of a background
		bake (see _start_panoply_bake()) -- shows the current texture/
		variant being baked and an overall progress bar, closes itself once
		done (or on error, after the user acknowledges it). Read-only view
		of self._bake_progress, which the background thread updates -- no
		imgui calls happen on that thread, only here on the main one."""
		progress = self._bake_progress
		if progress is None:
			return
		if not imgui.is_popup_open(_BAKE_PROGRESS_POPUP_ID):
			imgui.open_popup(_BAKE_PROGRESS_POPUP_ID)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_BAKE_PROGRESS_POPUP_ID, None, flags)
		if not opened:
			return

		if progress["error"] is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), f"Bake failed: {progress['error']}")
			if imgui.button("OK"):
				self._bake_progress = None
				imgui.close_current_popup()
			imgui.end_popup()
			return

		imgui.text(f"Texture {progress['texture_index'] + 1}/{progress['texture_total']}: {progress['texture_name']}")
		if progress["variant_total"]:
			imgui.text(f"Variant {progress['variant_index'] + 1}/{progress['variant_total']}: {progress['variant_suffix']}")
		else:
			imgui.text("Variant: (starting...)")
		texture_total = max(progress["texture_total"], 1)
		texture_fraction = progress["texture_index"] / texture_total
		variant_fraction = ((progress["variant_index"] + 1) / progress["variant_total"] / texture_total
		                     if progress["variant_total"] else 0.0)
		imgui.progress_bar(min(1.0, texture_fraction + variant_fraction), (300, 0))
		if progress["done"]:
			if imgui.button("OK"):
				self._bake_progress = None
				imgui.close_current_popup()
		imgui.end_popup()

	def _draw_panoply_masks_for(self, base_texture_name):
		"""Below a material's own texture row: thumbnails of whichever
		grayscale masks (mask weight in the red channel) already resolve for
		it -- e.g. for "tr_hom_armor00_epaule_c1.tga",
		"tr_hom_armor00_epaule_c1_skin.png"/"..._user.png" sitting in a
		sibling "mask/" folder next to the base texture, resolved through
		the same search-paths lookup as any other texture (so wherever the
		user's search paths actually cover that "mask/" subfolder) --
		regardless of whether panoply_files.txt happens to list this texture
		at all. A "+" button offers to create any axis (panoply.AXES) that
		doesn't have a mask yet, directly in the workspace -- the way a
		plain, not-yet-panoplied texture gets its first mask."""
		if not base_texture_name:
			return
		stem = Path(base_texture_name).stem
		mask_names = []
		missing_axes = []
		for axis in panoply.AXES:
			mask_name = f"{stem}_{axis}.tga"
			if self.search_paths_dialog.find_texture(mask_name) is not None:
				mask_names.append(mask_name)
			else:
				missing_axes.append(axis)
		imgui.text_disabled("Masks:")
		for mask_name in mask_names:
			imgui.same_line()
			imgui.push_id(mask_name)
			self._draw_texture_preview_static(mask_name, subdir="masks")
			imgui.same_line()
			# No material field to rewrite on copy (unlike a real texture
			# reference) -- a mask is only ever derived from the base
			# texture's own name + axis, never stored anywhere editable, so
			# on_copied is a no-op; the copy alone is enough for the next
			# frame's find_texture() to resolve to it (the active
			# workspace's own folders are already in the search paths).
			self._draw_texture_copy_button(mask_name, lambda new_name: None, subdir="masks")
			imgui.pop_id()

		if mask_names:
			imgui.same_line()
			disabled = self.workspace_setup_dialog.active_workspace_dir is None
			if _icon_button(fa_icons.ICON_FA_FIRE, "Bake real Panoply variants (writes tex/ + tex/hlsinfo/)", disabled=disabled):
				self._start_panoply_bake([base_texture_name])

		if missing_axes:
			imgui.same_line()
			imgui.push_id(f"add-mask-{stem}")
			disabled = self.workspace_setup_dialog.active_workspace_dir is None
			if _icon_button(fa_icons.ICON_FA_PLUS, "Add a mask for a missing axis", disabled=disabled):
				imgui.open_popup("add-mask-popup")
			if imgui.begin_popup("add-mask-popup"):
				for axis in missing_axes:
					clicked, _ = imgui.selectable(self._PANOPLY_AXIS_LABELS[axis], False)
					if clicked:
						self._create_panoply_mask(base_texture_name, axis)
						imgui.close_current_popup()
				imgui.end_popup()
			imgui.pop_id()

	def _apply_material_texture(self, node_path, material, material_id):
		"""Material color/blend/alpha-test/texture -- the part of rendering
		a material that _apply_material_common() doesn't cover. Assumes the
		caller already handled clearing and any color override."""
		if material is None:
			return

		panda_material = PandaMaterial()
		panda_material.set_diffuse(rgba_to_color(material.diffuse))
		panda_material.set_ambient(rgba_to_color(material.ambient))
		panda_material.set_emission(rgba_to_color(material.emissive))
		panda_material.set_specular(rgba_to_color(material.specular))
		panda_material.set_shininess(material.shininess)
		panda_material.set_twoside(True)
		node_path.set_material(panda_material)

		# BLEND and ALPHA_TEST are independent GL states in the real engine
		# (driver_opengl_material.cpp: enableBlend()/enableAlphaTest() are two
		# separate `if` blocks, each driven only by its own flag) -- both can
		# be active together (alpha test discards fully-transparent texels
		# outright, blend still applies a soft fade to what's left). This was
		# wrongly an `if`/`elif` here, silently dropping alpha test whenever
		# blend was also on.
		if material.flags & _IDRV_MAT_BLEND:
			src_op = _TBLEND_TO_PANDA_OPERAND[material.src_blend]
			dst_op = _TBLEND_TO_PANDA_OPERAND[material.dst_blend]
			node_path.set_attrib(ColorBlendAttrib.make(ColorBlendAttrib.M_add, src_op, dst_op))
		if material.flags & _IDRV_MAT_ALPHA_TEST:
			# Cutout transparency (e.g. foliage): discard texels below the
			# threshold outright rather than blending, matching the engine's
			# own glAlphaFunc(GL_GREATER, threshold) (driver_opengl_material.cpp).
			node_path.set_attrib(AlphaTestAttrib.make(AlphaTestAttrib.M_greater, material.alpha_test_threshold))

		texture = material.textures[0] if material.textures else None
		if texture is not None and texture.file_name:
			resolved_name = self._resolve_panoply_texture_name(texture.file_name)
			# A panoply override changes which file gets loaded, never the
			# shape's own material data (texture.file_name) -- see
			# _resolve_panoply_texture_name()'s own note. The texture cache
			# is keyed by the resolved name, so base and each variant are
			# cached independently.
			panda_texture = load_panda_texture(
				resolved_name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
				repeat=self._texture_needs_repeat, finder=self.search_paths_dialog.find_texture,
				wrap_s=texture.wrap_s, wrap_t=texture.wrap_t,
				min_filter=texture.min_filter, mag_filter=texture.mag_filter,
				load_grayscale_as_alpha=texture.load_grayscale_as_alpha)
			if panda_texture is not None:
				node_path.set_texture(panda_texture)
				# Seeds _update_texture_freshness()'s baseline right away, so
				# its very first tick after this load has something to
				# compare against instead of treating "no baseline yet" as a
				# change. Panoply-tracked names get their own signature from
				# _ensure_live_panoply_texture() instead -- skip those here.
				if resolved_name not in self._panoply_texture_sources and resolved_name not in self._texture_freshness_mtimes:
					ref = resolve_texture_ref(resolved_name, self._texture_search_dirs, self.search_paths_dialog.find_texture)
					if ref is not None:
						try:
							self._texture_freshness_mtimes[resolved_name] = ref.cache_stat()[0]
						except OSError:
							pass  # gone by the time we stat it -- see _update_texture_freshness()'s own guard

		tex_user_mat = None
		if material is not None and material.flags & _IDRV_MAT_TEX_ADDR and material.flags & _IDRV_MAT_USER_TEX_MAT_STAGE0:
			tex_user_mat = material.tex_user_mat.get(0)
		if tex_user_mat is not None:
			node_path.set_tex_transform(
				TextureStage.get_default(),
				TransformState.make_mat(uv_matrix_to_panda_mat4(tex_user_mat)))
		else:
			node_path.clear_tex_transform()

	def _apply_material(self, node_path, material_id):
		materials = getattr(self.shape_file.value, "materials", None)
		material = materials[material_id] if materials and material_id < len(materials) else None
		self._apply_material_common(node_path, material)

		override_color = self._material_override_colors.get(material_id)
		if override_color is not None:
			# A manual color override (see the material color picker button)
			# replaces the texture entirely, so the material reads unambiguously
			# as "flagged this color" rather than a tinted texture. Lighting and
			# the Material (diffuse/ambient/...) must also be disabled here: a
			# lit NodePath computes its shaded color from its Material, not from
			# set_color(), so leaving either on would still show the old texture's
			# material tint instead of the flat override color.
			node_path.set_texture_off(1)
			node_path.set_material_off(1)
			node_path.set_light_off(1)
			node_path.set_color(override_color, 1)
			return

		self._apply_material_texture(node_path, material, material_id)

	def _reapply_material(self, material_id):
		"""Re-runs _apply_material on every NodePath using this material, e.g.
		after an in-place edit of the parsed Material (texture set change...)."""
		for node_path in self._material_node_paths.get(material_id, []):
			self._apply_material(node_path, material_id)

	def _reapply_all_materials(self):
		"""_reapply_material() for every material -- a shape-wide change (the
		panoply race/user-color pick) affects any material whose texture has
		a variant for it, not just one."""
		for material_id in self._material_node_paths:
			self._reapply_material(material_id)

	def _set_material_override_color(self, material_id, color):
		"""Sets (or, if color is None, clears) a material's manual flat-color
		override -- the "no color" option in its color picker button."""
		if color is None:
			self._material_override_colors.pop(material_id, None)
		else:
			self._material_override_colors[material_id] = tuple(color)
		self._reapply_material(material_id)

	def _get_preview_texture_ref(self, name):
		"""Resolves a texture by name (like _apply_material does for
		rendering) into an imgui.ImTextureRef for UI thumbnails/tooltips,
		cached. Returns None if there's no name or it can't be decoded."""
		if not name:
			return None
		tex_ref = self._preview_texture_refs.get(name)
		if tex_ref is not None:
			return tex_ref
		panda_texture = load_panda_texture(
			name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
			repeat=self._texture_needs_repeat, finder=self.search_paths_dialog.find_texture)
		if panda_texture is None:
			return None
		# p3dimgui's loadTexture() mutates whatever Texture it's given
		# in-place (backend.py: texture.store(pnm); pnm.flip(...); texture.load(pnm)
		# -- flips it vertically and reloads it into the SAME object, for
		# ImGui's own OpenGL-vs-Panda row-order convention). `panda_texture`
		# here is the exact same cached object _apply_material_texture() put
		# on the live 3D material (self._texture_cache is shared) -- passing
		# it straight to loadTexture() would silently flip the 3D-rendered
		# texture too, the first time this shape's preview thumbnail/tooltip
		# is ever drawn. A throwaway copy absorbs the mutation instead.
		#
		# That copy is also where alpha gets dropped: most of these textures
		# are mostly transparent everywhere (e.g. a shape cut from a square
		# atlas), and alpha blending genuinely hides the real color underneath
		# low-alpha pixels -- no backdrop color can undo that (see
		# _draw_image_opaque_bg(), still used as a fallback for whatever's
		# left translucent). Safe to do on this copy specifically now that
		# it's confirmed independent of the live 3D material's own texture
		# (see above) -- doing this to the shared one would have made the
		# actual 3D-rendered object opaque too.
		#
		# Done via a PNMImage round-trip (store() -> remove_alpha() ->
		# load()), not Texture.set_format(F_rgb) directly: set_format() only
		# relabels the already-stored ram image's component count, it doesn't
		# actually repack the pixel bytes from 4 to 3 per pixel -- every pixel
		# after the first then reads shifted by one channel, which looked
		# exactly like a scrambled checkerboard for any texture that actually
		# had an alpha channel. store() decodes through the real pixel data
		# (works for a compressed source too), giving a real RGB buffer to
		# reload from.
		preview_texture = panda_texture.make_copy()
		pnm_image = PNMImage()
		preview_texture.store(pnm_image)
		if pnm_image.has_alpha():
			pnm_image.remove_alpha()
			preview_texture = PandaTexture()
			preview_texture.load(pnm_image)
		tex_ref = self.imgui.loadTexture(preview_texture)
		self._preview_texture_refs[name] = tex_ref
		return tex_ref

	@staticmethod
	def _draw_image_opaque_bg(tex_ref, size):
		"""imgui.image() (unlike image_button()) has no bg_col param -- draws
		an opaque black rect behind it by hand instead, so a texture with
		transparency (routinely most of the image, e.g. a banner cut out of a
		square atlas) doesn't blend into the panel's own dark background and
		become hard to actually see. Same reasoning as the bg_col passed to
		image_button() everywhere else in this file."""
		pos = imgui.get_cursor_screen_pos()
		width, height = size
		imgui.get_window_draw_list().add_rect_filled(
			pos, (pos.x + width, pos.y + height), imgui.get_color_u32(_PREVIEW_BG_COLOR))
		imgui.image(tex_ref, size)

	def _draw_thumbnail_button(self, str_id, tex_ref, tooltip, size=24):
		"""Common image_button + hover-zoom-tooltip widget: shared by real
		texture thumbnails and solid-color swatches, so a plain material
		color gets the exact same button chrome/hover style as a real
		texture instead of a hand-approximated look-alike. Both the thumbnail
		and the hover preview get an opaque white backdrop (see
		_draw_image_opaque_bg()) -- textures are routinely mostly transparent
		(e.g. a shape cut out of a square atlas), which otherwise blends into
		the panel and makes the preview hard to read for no benefit here."""
		clicked = imgui.image_button(str_id, tex_ref, (size, size), bg_col=_PREVIEW_BG_COLOR)
		if imgui.is_item_hovered():
			if imgui.begin_tooltip():
				self._draw_image_opaque_bg(tex_ref, (192, 192))
				if tooltip:
					imgui.text(tooltip)
				imgui.end_tooltip()
		return clicked

	def _draw_texture_preview_button(self, str_id, name, size=24):
		"""An image_button thumbnail for texture `name` (a plain color-button
		placeholder if there's no name or it can't be resolved), with a
		bigger preview shown in a tooltip on hover. Returns True if clicked."""
		tex_ref = self._get_preview_texture_ref(name)
		if tex_ref is None:
			return imgui.color_button(str_id, (0.5, 0.5, 0.5, 0.4), 0, (size, size))
		return self._draw_thumbnail_button(str_id, tex_ref, name, size)

	def _draw_texture_preview_static(self, name, size=24, badge=None, subdir="tex"):
		"""Same thumbnail button (size, frame, hover-zoom tooltip) as
		_draw_texture_preview_button, but a plain reveal-in-file-manager
		action instead of whatever a real texture-picker button would do
		(the text field/browse icon next to it are the actual editing
		controls for that) -- disabled only when there's no real file to
		reveal (unresolvable, or living inside a `.bnp`, which has no
		files of its own on disk). A plain imgui.image() looked visibly
		smaller: image_button adds the usual button frame padding around
		the image that a bare image() doesn't get, and disabling it
		(rather than swapping widget types) keeps that same size.

		`badge` (e.g. a material index), if given, is overlaid directly on
		the thumbnail -- see _draw_preview_badge() -- instead of a separate
		imgui.text() label, which would cost horizontal space this panel
		doesn't have to spare. `subdir` ("tex" or "masks", see
		_is_texture_in_workspace()) decides which workspace folder
		"already in the workspace" is checked against, for the green
		border (see _draw_preview_workspace_border())."""
		tex_ref = self._get_preview_texture_ref(name)
		if tex_ref is None:
			imgui.dummy((size, size))
		else:
			resolved = self._resolve_texture(name)
			can_reveal = resolved is not None and resolved.fs_path is not None
			imgui.begin_disabled(not can_reveal)
			if imgui.image_button("##preview", tex_ref, (size, size), bg_col=_PREVIEW_BG_COLOR) and can_reveal:
				reveal_in_system_file_manager(resolved.fs_path)
			imgui.end_disabled()
			if self._is_texture_in_workspace(name, subdir):
				self._draw_preview_workspace_border()
			if imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
				if imgui.begin_tooltip():
					self._draw_image_opaque_bg(tex_ref, (192, 192))
					imgui.text(name)
					if can_reveal:
						imgui.text_disabled("Click to reveal in file manager")
					imgui.end_tooltip()
		if badge is not None:
			self._draw_preview_badge(badge)

	@staticmethod
	def _draw_preview_badge(text, scale=0.7):
		"""Overlays `text` (e.g. "3") in the bottom-right corner of
		whichever preview button/thumbnail/color-swatch was just drawn, at
		`scale`x the normal font size -- takes no extra horizontal layout
		space, unlike a separate imgui.text() next to it, which is the
		point: these rows are already tight on width. A 1px black shadow
		keeps it legible over a bright/busy thumbnail.

		add_text()'s explicit-size overload draws at any size regardless
		of the currently pushed font scale, but calc_text_size() always
		measures at the *current* (full) size -- so the small size's own
		footprint is estimated by scaling that measurement down, rather
		than actually re-measuring at `scale` (glyph metrics scale
		linearly with size in ImGui's font atlas, so this is exact, not
		an approximation)."""
		font = imgui.get_font()
		full_size = imgui.get_font_size()
		small_size = full_size * scale
		full_text_size = imgui.calc_text_size(text)
		text_w, text_h = full_text_size.x * scale, full_text_size.y * scale

		item_max = imgui.get_item_rect_max()
		pos = (item_max.x - text_w - 1, item_max.y - text_h - 1)
		shadow_pos = (pos[0] + 1, pos[1] + 1)
		draw_list = imgui.get_window_draw_list()
		draw_list.add_text(font, small_size, shadow_pos, imgui.get_color_u32((0.0, 0.0, 0.0, 1.0)), text)
		draw_list.add_text(font, small_size, pos, imgui.get_color_u32((1.0, 1.0, 1.0, 1.0)), text)

	@staticmethod
	def _draw_preview_workspace_border(thickness=2):
		"""Outlines whichever preview button/thumbnail was just drawn in
		_TEXTURE_IN_WORKSPACE_COLOR -- same green already used for a texture
		reference's own text field (see _draw_simple_material_row()/
		_draw_multi_bitmap_editor()), now also on the thumbnail itself.
		Manually drawn (not a pushed Col_.border style color): image_button()
		only actually shows a border when the current style's frame border
		size is non-zero, which isn't guaranteed."""
		item_min = imgui.get_item_rect_min()
		item_max = imgui.get_item_rect_max()
		color = imgui.get_color_u32(_TEXTURE_IN_WORKSPACE_COLOR)
		imgui.get_window_draw_list().add_rect(item_min, item_max, color, thickness=thickness)

	def _get_color_texture_ref(self, color):
		"""A 1x1 solid-color Texture (see shape_geometry.solid_color_texture)
		wrapped as an imgui.ImTextureRef, cached by color."""
		key = tuple(round(c, 3) for c in color)
		tex_ref = self._color_texture_refs.get(key)
		if tex_ref is not None:
			return tex_ref
		tex_ref = self.imgui.loadTexture(solid_color_texture(color))
		self._color_texture_refs[key] = tex_ref
		return tex_ref

	def _draw_color_preview_button(self, str_id, color, size=24):
		"""Same image_button widget as _draw_texture_preview_button, but for
		a plain color (e.g. an untextured material's own diffuse color) --
		filled into a tiny solid-color texture rather than hand-drawing a
		look-alike border/hover style around a plain color_button."""
		tex_ref = self._get_color_texture_ref(color)
		tooltip = f"RGBA {tuple(round(c, 2) for c in color)}"
		return self._draw_thumbnail_button(str_id, tex_ref, tooltip, size)

	def _draw_material_color_button(self, material_id, preview_name=...):
		"""A material's color-override button: a flat color swatch once an
		override is set, otherwise a texture preview thumbnail (see
		_draw_texture_preview_button). Either way, clicking it opens the same
		picker (plus a "No color" option) that sets/clears the override,
		visualizing on the 3D model exactly which faces use this material.

		`preview_name`, if given, overrides which texture the preview shows
		-- for a Multi Bitmap slot row, that's the slot's own texture, not
		necessarily the material's currently active one (the default,
		looked up from slot 0 when `preview_name` is left as the sentinel)."""
		override_color = self._material_override_colors.get(material_id)
		if override_color is not None:
			# A forced/manual color: plain color_button, deliberately not the
			# same image_button look as real shape data (texture or diffuse
			# color) below -- that difference in chrome is what marks it as
			# an override at a glance.
			clicked = imgui.color_button(f"##color-{material_id}", override_color, 0, (24, 24))
		else:
			materials = getattr(self.shape_file.value, "materials", None)
			material = materials[material_id] if materials and material_id < len(materials) else None
			if preview_name is ...:
				texture = material.textures[0] if material is not None and material.textures else None
				preview_name = texture.file_name if texture is not None else None
				if preview_name:
					preview_name = self._resolve_panoply_texture_name(preview_name)
			if preview_name:
				clicked = self._draw_texture_preview_button(f"##color-{material_id}", preview_name)
			else:
				# No texture at all -- this material is just a flat color
				# (e.g. anlor_stick.shape's untextured wood/metal parts), so
				# show its own diffuse color rather than a generic placeholder.
				diffuse = rgba_to_color(material.diffuse) if material is not None else (0.5, 0.5, 0.5, 0.4)
				clicked = self._draw_color_preview_button(f"##color-{material_id}", diffuse)
		if clicked:
			imgui.open_popup(f"{_COLOR_POPUP_ID}-{material_id}")

		if imgui.begin_popup(f"{_COLOR_POPUP_ID}-{material_id}"):
			if imgui.button("No color"):
				self._set_material_override_color(material_id, None)
				imgui.close_current_popup()
			imgui.separator()
			current = override_color if override_color is not None else (1.0, 1.0, 1.0, 1.0)
			changed, new_color = imgui.color_picker4(f"##picker-{material_id}", current)
			if changed:
				self._set_material_override_color(material_id, new_color)
			imgui.end_popup()

	@staticmethod
	def _set_material_diffuse_color(material, color):
		r, g, b, a = color.x, color.y, color.z, color.w
		material.diffuse = Rgba(round(r * 255), round(g * 255), round(b * 255), round(a * 255))

	def _draw_texture_color_button(self, material_id, material):
		"""Textures tab only: when a material has no texture at all, its
		swatch button lets you edit CMaterial::diffuse directly -- a real,
		saved modification of the shape (unlike the Materials tab's color
		button, which is a temporary visualization override, never saved).
		Once a texture is set, this just previews it: the material is a
		texture now, not a color, so it's no longer clickable for color."""
		badge = str(material_id)
		texture = material.textures[0] if material.textures else None
		if texture is not None and texture.file_name:
			self._draw_texture_preview_static(texture.file_name, badge=badge)
			return

		diffuse = rgba_to_color(material.diffuse)
		if self._draw_color_preview_button(f"##diffuse-{material_id}", diffuse):
			imgui.open_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}")
		self._draw_preview_badge(badge)

		if imgui.begin_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}"):
			changed, new_color = imgui.color_picker4(f"##diffuse-picker-{material_id}", diffuse)
			if changed:
				self._set_material_diffuse_color(material, new_color)
				self._reapply_material(material_id)
			imgui.end_popup()

	def _start_texture_browse(self, key, on_result):
		"""Opens a native file picker for a texture; on_result(file_name) is
		called with just the chosen file's base name once picked, matching
		how texture references are stored (name only, no path). Starts
		wherever the shape itself came from, if known -- a texture is far
		more likely to sit next to (or under) it than anywhere else."""
		start_dir = str(self._shape_source_path.parent) if self._shape_source_path is not None else ""
		dialog = pfd.open_file("Choose texture", start_dir, ["Textures", "*.tga *.dds *.png"])
		self._texture_browse_dialogs[key] = (dialog, on_result)

	def _poll_texture_browse_dialogs(self):
		for key in list(self._texture_browse_dialogs.keys()):
			dialog, on_result = self._texture_browse_dialogs[key]
			if not dialog.ready(0):
				continue
			del self._texture_browse_dialogs[key]
			result = dialog.result()
			if result:
				# Texture names aren't case-sensitive; lower-cased consistently
				# rather than mixing whatever case a file happens to be on disk.
				on_result(Path(result[0]).name.lower())

	def _multi_bitmap_entries(self):
		"""(material_id, texture) for every material whose slot-0 texture is a
		CTextureMultiFile (see docs/material_options.md, "Plusieurs images
		alternatives par emplacement (Multi Bitmap)"). Only slot 0 is covered
		since it's the only slot _apply_material actually renders today."""
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			return []
		entries = []
		for material_id, material in enumerate(materials):
			texture = material.textures[0] if material.textures else None
			if texture is not None and texture.class_name == "CTextureMultiFile" and texture.file_names:
				entries.append((material_id, texture))
		return entries

	@staticmethod
	def _ensure_multi_bitmap_slot(texture, index):
		"""Pads texture.file_names up to `index` with "" if it's shorter --
		matches how the 3dsMax exporter itself leaves gaps for skipped slots
		(see e.g. xmas_tr_mektoub_selle.shape's slot 3, an empty string)."""
		while len(texture.file_names) <= index:
			texture.file_names.append("")

	def _select_multi_bitmap_slot(self, entries, index):
		"""The "Select" button: switches every Multi Bitmap material of the
		shape to the same slot index at once -- picking "Medium Quality" is a
		whole-object appearance choice, not a per-material one."""
		for material_id, texture in entries:
			self._ensure_multi_bitmap_slot(texture, index)
			texture.selected_index = index
			texture.file_name = texture.file_names[index]
			self._reapply_material(material_id)

	def _draw_multi_bitmap_editor(self):
		entries = self._multi_bitmap_entries()
		if not entries:
			return
		entries_by_id = dict(entries)

		imgui.separator()
		imgui.text("Multi Bitmap")
		doc = self.material_docs.get("multi-bitmap")
		hovered_hint = doc.summary if (doc and imgui.is_item_hovered()) else None

		representative = entries_by_id.get(0, entries[0][1])
		slot_count = max(len(_MULTI_BITMAP_SLOT_LABELS), max(len(t.file_names) for _, t in entries))

		for index in range(slot_count):
			imgui.push_id(f"mb-slot-{index}")

			is_active = representative.selected_index == index
			if is_active:
				imgui.push_style_color(imgui.Col_.button.value, (0.2, 0.65, 0.2, 1.0))
				imgui.push_style_color(imgui.Col_.button_hovered.value, (0.25, 0.7, 0.25, 1.0))
				imgui.push_style_color(imgui.Col_.button_active.value, (0.15, 0.55, 0.15, 1.0))
			if _icon_button(fa_icons.ICON_FA_CHECK, "Select this set for the whole shape"):
				self._select_multi_bitmap_slot(entries, index)
			if is_active:
				imgui.pop_style_color(3)
			imgui.same_line()

			expanded = index in self._multi_bitmap_expanded
			expand_icon = fa_icons.ICON_FA_CHEVRON_DOWN if expanded else fa_icons.ICON_FA_CHEVRON_RIGHT
			if _icon_button(expand_icon, "Collapse" if expanded else "Expand: edit per-material"):
				if expanded:
					self._multi_bitmap_expanded.discard(index)
				else:
					self._multi_bitmap_expanded.add(index)
			imgui.same_line()

			imgui.text(_multi_bitmap_slot_label(index))
			if doc and imgui.is_item_hovered():
				hovered_hint = doc.summary

			preview = representative.file_names[index] if index < len(representative.file_names) else ""
			imgui.text_disabled(preview or "(empty)")

			if expanded:
				for material_id, texture in entries:
					self._ensure_multi_bitmap_slot(texture, index)
					imgui.push_id(f"mb-mat-{material_id}")

					slot_name = texture.file_names[index]
					badge = str(material_id)
					if slot_name:
						self._draw_texture_preview_static(slot_name, badge=badge)
					else:
						imgui.dummy((24, 24))
						self._draw_preview_badge(badge)
					imgui.same_line()

					def _set_slot(value, material_id=material_id, texture=texture, index=index):
						texture.file_names[index] = value
						if texture.selected_index == index:
							texture.file_name = value
							self._reapply_material(material_id)

					current_value = texture.file_names[index]
					in_workspace = self._is_texture_in_workspace(current_value)
					imgui.push_style_color(
						imgui.Col_.text.value, _TEXTURE_IN_WORKSPACE_COLOR if in_workspace else _TEXTURE_NORMAL_COLOR)
					changed, new_value = self._draw_texture_name_combo(f"##combo-{index}", current_value)
					imgui.pop_style_color()
					if imgui.is_item_hovered() and current_value:
						source_path = self._texture_source_path_text(current_value)
						tooltip = source_path or current_value
						if in_workspace:
							tooltip += "\n(in the active workspace)"
						imgui.set_tooltip(tooltip)
					if changed and new_value != current_value:
						_set_slot(new_value)
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
						def _on_result(file_name, texture=texture, index=index, _set_slot=_set_slot):
							self._ensure_multi_bitmap_slot(texture, index)
							_set_slot(file_name)
						self._start_texture_browse(("multi-bitmap", material_id, index), _on_result)
					imgui.same_line()
					self._draw_texture_copy_button(current_value, _set_slot)

					imgui.pop_id()

			imgui.pop_id()
			imgui.separator()

		if hovered_hint:
			self.sysinfo.set_status(hovered_hint, color=_STATUS_HINT_COLOR)
			self._multi_bitmap_hint_shown = True
		elif self._multi_bitmap_hint_shown:
			self.sysinfo.set_status("")
			self._multi_bitmap_hint_shown = False

	def _workspace_shape_save_path(self):
		"""Where Save writes to -- always <active workspace>/shapes/<name>,
		regardless of where the shape was originally loaded from (even from
		inside a .bnp, which used to disable Save entirely). None if there's
		no active workspace configured yet, or no shape name is known."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None or not self._shape_source_name:
			return None
		return workspace_dir / "shapes" / self._shape_source_name

	def _texture_source_path_text(self, file_name):
		"""Human-readable full path of whatever file `file_name` actually
		resolves to (a plain path, or `<bnp path> (<entry name>)` for a
		texture living inside a .bnp) -- see the texture combo's tooltip
		(_draw_texture_name_combo() callers), so hovering it shows exactly
		which source file it takes rather than just the bare name already
		visible in the combo itself. None if it doesn't resolve to anything."""
		ref = self._resolve_texture(file_name)
		if ref is None:
			return None
		if ref.fs_path is not None:
			return str(ref.fs_path)
		if ref.bnp_path is not None:
			return f"{ref.bnp_path} ({ref.name})"
		return None

	def _resolve_texture(self, file_name):
		"""resolve_texture_ref(), pre-bound to this shape's own
		_texture_search_dirs/finder -- the same lookup the viewport itself
		uses to display a texture."""
		if not file_name:
			return None
		return resolve_texture_ref(file_name, self._texture_search_dirs, self.search_paths_dialog.find_texture)

	def _workspace_texture_names(self, subdir="tex"):
		"""Sorted list of texture file names sitting in the active
		workspace's `subdir` folder (default "tex") -- rescanned fresh on
		every call (a single Path.iterdir(), cheap enough to just always
		redo rather than cache/invalidate) so _draw_texture_name_combo()
		always shows what's actually on disk the moment its dropdown opens."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return []
		try:
			entries = list((workspace_dir / subdir).iterdir())
		except OSError:
			return []
		return sorted(entry.name for entry in entries if entry.suffix.lower() in _WORKSPACE_TEXTURE_EXTENSIONS)

	def _draw_texture_name_combo(self, imgui_id, current_value):
		"""Combo box listing the active workspace's tex/ textures (see
		_workspace_texture_names()) -- returns (changed, new_value), same
		shape as imgui.input_text()'s return value, so call sites only
		needed to swap the widget itself. current_value is always shown as
		the combo's own preview text even when it isn't one of the listed
		names (not yet copied into the workspace, or set via Browse from
		somewhere else) -- picking a listed entry is the only way this
		widget itself changes it; Browse/Copy stay the way to set anything
		else, same buttons as before, unchanged."""
		imgui.set_next_item_width(220)
		changed = False
		new_value = current_value
		if imgui.begin_combo(imgui_id, current_value):
			for name in self._workspace_texture_names():
				clicked, _ = imgui.selectable(name, name == current_value)
				if clicked:
					new_value = name
					changed = True
			imgui.end_combo()
		return changed, new_value

	def _texture_copy_destination(self, ref, subdir="tex"):
		"""<active workspace>/<subdir>/<ref's own bare name>, or None without
		an active workspace configured. `subdir` defaults to "tex" (a real
		material texture); pass "masks" for a Panoply mask file instead (see
		_draw_panoply_masks_for()). A `.dds` source always lands as `.png`
		instead (see _copy_texture_to_workspace()) -- `.dds` is a compiled,
		non-editable format (also auto-(re)generated into dds/ from tex/ by
		Patina itself, see tex_dds_sync.py), never something the workspace's
		own editable folders should hold."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return None
		name = ref.name
		if Path(name).suffix.lower() == ".dds":
			name = f"{Path(name).stem}.png"
		return workspace_dir / subdir / name

	def _is_texture_in_workspace(self, file_name, subdir="tex"):
		"""True if `file_name` currently resolves to a file already sitting
		in the active workspace's own `subdir` folder -- used to color-code
		texture references in the UI (see _draw_texture_copy_button())."""
		ref = self._resolve_texture(file_name)
		if ref is None or ref.fs_path is None:
			return False
		dest = self._texture_copy_destination(ref, subdir)
		return dest is not None and ref.fs_path.resolve() == dest.resolve()

	def _copy_texture_to_workspace(self, file_name, subdir="tex"):
		"""Resolves `file_name` (absolute path or bare name alike) and
		copies it into the active workspace's `subdir` folder, unless it's
		already sitting there. A `.dds` source is decoded and written out as
		`.png` instead of a raw byte copy (see _texture_copy_destination()
		-- `.dds` is never an editable workspace source). Returns the
		resulting bare (lowercased, matching how a typed/browsed texture
		name is normalized elsewhere) file name to store back on the
		material, or None if nothing could be done -- no active workspace,
		unresolvable/undecodable reference, or a write failure (logged, not
		raised; the caller just leaves the material's current reference
		untouched in that case)."""
		ref = self._resolve_texture(file_name)
		if ref is None:
			print(f"[object_editor] could not resolve texture {file_name!r} to copy")
			return None
		dest = self._texture_copy_destination(ref, subdir)
		if dest is None:
			return None
		if not (ref.fs_path is not None and ref.fs_path.resolve() == dest.resolve()):
			try:
				dest.parent.mkdir(parents=True, exist_ok=True)
				if Path(ref.name).suffix.lower() == ".dds":
					panda_texture = load_panda_texture(ref.name, finder=lambda _name, _ref=ref: _ref)
					if panda_texture is None:
						print(f"[object_editor] could not decode {ref.name!r} to convert to .png")
						return None
					texture_to_pnm_image(panda_texture).write(str(dest))
				else:
					dest.write_bytes(ref.read_bytes())
			except OSError as exc:
				print(f"[object_editor] could not copy texture {ref.name!r} to workspace: {exc}")
				return None
		return dest.name.lower()

	def _draw_texture_copy_button(self, current_value, on_copied, subdir="tex"):
		"""Small icon button, next to a texture reference field: copies
		that one texture into the active workspace's `subdir` folder and
		rewrites the reference to the resulting bare name via
		`on_copied(new_name)` -- deliberately opt-in per texture rather
		than an automatic copy-everything-on-save, so the workspace's
		tex/ (or masks/) only ever holds files the user actually chose to
		bring in for editing. Once it's already there, this becomes an
		"Edit" button instead (see _draw_texture_edit_button()) -- copying
		it again would be a no-op, so there's nothing useful left for this
		button to do besides launch an editor on it."""
		if self._is_texture_in_workspace(current_value, subdir):
			self._draw_texture_edit_button(current_value)
			return
		disabled = not current_value or self.workspace_setup_dialog.active_workspace_dir is None
		imgui.begin_disabled(disabled)
		if _icon_button(fa_icons.ICON_FA_DOWNLOAD, f"Copy this file into the active workspace's {subdir}/"):
			new_name = self._copy_texture_to_workspace(current_value, subdir)
			if new_name is not None:
				on_copied(new_name)
		imgui.end_disabled()

	def _draw_texture_edit_button(self, current_value):
		"""Launches the user's configured external image editor (Settings
		tab -> Tools) on `current_value`'s resolved file -- shown instead
		of the copy button once a texture already lives in the active
		workspace. Always clickable (2026-08-29): if no editor is
		configured yet, jumps to Settings > Tools and flashes the field
		instead of just sitting disabled with a tooltip (see
		ForgeryApp.request_settings_attention())."""
		editor_path = app_settings.load().image_editor_path
		if _icon_button(fa_icons.ICON_FA_EDIT, "Edit this texture in the configured image editor"):
			if not editor_path:
				self.request_settings_attention("Tools", "image_editor_path")
			else:
				resolved = self._resolve_texture(current_value)
				if resolved is not None and resolved.fs_path is not None:
					subprocess.Popen([editor_path, str(resolved.fs_path)])

	def _write_shape(self, path):
		try:
			# Whatever rotation the object is currently at becomes the
			# shape's own default_rot_quat on save: this is meant to be an
			# authoring tool for that value, not just a live-viewer aid, so
			# it must survive a save/reload round trip rather than silently
			# reverting. Read model_root's WORLD rotation (not just
			# _object_pivot's own) -- _transform_node("rotation") lets the
			# Rotation panel edit either node depending on that row's pivot
			# lock, and model_root's world quat always reflects the total
			# either way (it's pivot's rotation composed with model_root's
			# own local one, identity when unlocked).
			base = getattr(self.shape_file.value, "base", None)
			if base is not None:
				total_quat = self.model_root.get_quat(self.render)
				# Panda3D's LQuaternion stores (real, i, j, k) internally, and
				# its inherited get_x/y/z/w() accessors read that raw slot
				# order rather than remapping to (i, j, k, real) -- confirmed
				# empirically (an identity quat's get_x() came back 1.0, the
				# real part, not 0.0) and consistent with how _display_shape()
				# already builds a Panda Quat via Quat(rot.w, rot.x, rot.y,
				# rot.z) (real first, positionally) elsewhere in this file.
				base.default_rot_quat = Quaternion(
					x=total_quat.get_y(), y=total_quat.get_z(), z=total_quat.get_w(), w=total_quat.get_x())
			path.parent.mkdir(parents=True, exist_ok=True)
			save_shape(path, self.shape_file)
			self._save_status = f"Saved to {path}"
			print(f"[object_editor] {self._save_status}")
			# The pivot's rotation is now baked into default_rot_quat above,
			# so it's also the natural new reset baseline -- otherwise
			# Ctrl+Reset would keep snapping back to the pre-save orientation
			# forever.
			self._object_pivot_base_quat = Quat(self._object_pivot.get_quat())
		except (OSError, ShapeWriteError) as exc:
			self._save_status = f"Save failed: {exc}"
			print(f"[object_editor] {self._save_status}")

	def _on_save_clicked(self):
		save_path = self._workspace_shape_save_path()
		if save_path is None:
			return
		if self._save_overwrite_confirmed:
			self._write_shape(save_path)
		else:
			self._pending_save_path = save_path
			self._confirm_overwrite_open = True
			imgui.open_popup(_OVERWRITE_POPUP_ID)

	def _draw_save_confirmation_popup(self):
		if not self._confirm_overwrite_open:
			return

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_OVERWRITE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text(f"Overwrite this file?\n{self._pending_save_path}")
		imgui.text_wrapped("You won't be asked again this session.")
		imgui.separator()
		if imgui.button("Overwrite"):
			self._save_overwrite_confirmed = True
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
			self._write_shape(self._pending_save_path)
		imgui.same_line()
		if imgui.button("Cancel"):
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
		imgui.end_popup()

	def _poll_skeleton_file_dialog(self):
		if self._skeleton_file_dialog is None or not self._skeleton_file_dialog.ready(0):
			return
		result = self._skeleton_file_dialog.result()
		self._skeleton_file_dialog = None
		if result:
			path = Path(result[0])
			self._load_skeleton_bytes(path.read_bytes(), path.name)

	def _poll_animation_file_dialog(self):
		if self._animation_file_dialog is None or not self._animation_file_dialog.ready(0):
			return
		result = self._animation_file_dialog.result()
		self._animation_file_dialog = None
		if result:
			path = Path(result[0])
			self._apply_bone_preview_animation_bytes(path.read_bytes(), path.name)

	def _poll_image_editor_dialog(self):
		if self._image_editor_dialog is None or not self._image_editor_dialog.ready(0):
			return
		result = self._image_editor_dialog.result()
		self._image_editor_dialog = None
		if result:
			fresh = app_settings.load()
			fresh.image_editor_path = result[0]
			app_settings.save(fresh)

	def _poll_text_editor_dialog(self):
		if self._text_editor_dialog is None or not self._text_editor_dialog.ready(0):
			return
		result = self._text_editor_dialog.result()
		self._text_editor_dialog = None
		if result:
			fresh = app_settings.load()
			fresh.text_editor_path = result[0]
			app_settings.save(fresh)

	def _draw_image_editor_settings(self):
		"""Settings tab -- lets the user pick an external image editor
		executable, used by the Textures tab's "Edit" button (see
		_draw_texture_edit_button()) once a texture already lives in the
		active workspace."""
		settings = app_settings.load()
		label = "Image editor: "
		path_text = settings.image_editor_path or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		flashing = self._begin_attention_flash("image_editor_path")
		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if settings.image_editor_path and imgui.is_item_hovered():
			imgui.set_tooltip(settings.image_editor_path)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##image-editor", "Choose an image editor executable..."):
			self._image_editor_dialog = pfd.open_file("Choose image editor executable")
		self._end_attention_flash(flashing)

	def _draw_text_editor_settings(self):
		"""Settings tab -- lets the user pick an external text editor
		executable, used by the Panoply section's "Edit" button (see
		_draw_global_panoply_section()) once a workspace panoply.cfg already
		exists. Same pattern as _draw_image_editor_settings() above."""
		settings = app_settings.load()
		label = "Text editor: "
		path_text = settings.text_editor_path or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		flashing = self._begin_attention_flash("text_editor_path")
		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if settings.text_editor_path and imgui.is_item_hovered():
			imgui.set_tooltip(settings.text_editor_path)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##text-editor", "Choose a text editor executable..."):
			self._text_editor_dialog = pfd.open_file("Choose text editor executable")
		self._end_attention_flash(flashing)

	def _poll_workspace_sync_folder_dialog(self):
		if self._workspace_sync_folder_dialog is None or not self._workspace_sync_folder_dialog.ready(0):
			return
		result = self._workspace_sync_folder_dialog.result()
		self._workspace_sync_folder_dialog = None
		workspace_name = self.workspace_setup_dialog.active_workspace_name
		if not result or workspace_name is None:
			return
		fresh = app_settings.load()
		fresh.workspace_sync_folders[workspace_name] = result
		fresh.last_workspace_sync_folder = result
		app_settings.save(fresh)
		self.workspace_sync.set_sync_folder(result)

	def _draw_workspace_sync_settings(self):
		"""Settings tab -- lets the user pick an external folder the active
		workspace's anims/shapes/skels/tex get auto-mirrored into (see
		workspace_sync.py). Per-workspace: switching the active workspace
		shows/edits that workspace's own folder, not a single global one
		(see _on_active_workspace_changed())."""
		workspace_name = self.workspace_setup_dialog.active_workspace_name
		label = "Sync folder: "
		path_text = "(no active workspace)" if workspace_name is None else (
			app_settings.load().workspace_sync_folders.get(workspace_name) or "(not set)")

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		imgui.text(label)
		imgui.same_line()
		imgui.begin_disabled(workspace_name is None)
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if workspace_name is not None and imgui.is_item_hovered():
			imgui.set_tooltip(path_text)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##sync-folder", "Choose a folder to mirror this workspace's anims/shapes/skels/tex into..."):
			current = app_settings.load().workspace_sync_folders.get(workspace_name) if workspace_name else None
			self._workspace_sync_folder_dialog = pfd.select_folder("Choose sync folder", current or "")
		imgui.end_disabled()

		# Only relevant once a sync folder is actually configured -- catches
		# up anything that predates the live watch (see
		# WorkspaceSyncWatcher.refresh_fully_synced()).
		sync_folder_set = workspace_name is not None and app_settings.load().workspace_sync_folders.get(workspace_name)
		if sync_folder_set and not self.workspace_sync.is_fully_synced():
			imgui.begin_disabled(self.workspace_sync.is_syncing())
			if _colored_button("Sync now", _SYNC_NOW_COLOR):
				self.workspace_sync.sync_now()
			imgui.end_disabled()
			if self.workspace_sync.is_syncing():
				imgui.same_line()
				imgui.text_disabled("Syncing...")

	def _poll_repository_paths_dialog(self):
		if self._repository_paths_dialog is None or not self._repository_paths_dialog.ready(0):
			return
		result = self._repository_paths_dialog.result()
		repo_name = self._repository_paths_dialog_repo
		self._repository_paths_dialog = None
		self._repository_paths_dialog_repo = None
		if not result:
			return
		repository_paths.set_path(repo_name, result)

	def _draw_repository_paths_settings(self):
		"""Settings tab -- one folder picker per pynel.repository_paths.REPOSITORIES
		entry, so any tool built on pynel (this app included -- see
		_bake_panoply_real()'s ryzom-data dependency) can resolve "where is
		ryzom-data on this machine" without asking the user again. Stored
		outside Forgery's own settings.toml (see repository_paths.py's own
		docstring on why)."""
		configured = repository_paths.load()
		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2

		for repo_name in repository_paths.REPOSITORIES:
			label = f"{repo_name}: "
			path_text = configured.get(repo_name) or "(not set)"
			available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
			             - button_width - style.item_spacing.x)

			flashing = self._begin_attention_flash(repo_name)
			imgui.text(label)
			imgui.same_line()
			imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
			if repo_name in configured and imgui.is_item_hovered():
				imgui.set_tooltip(configured[repo_name])
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##repo-{repo_name}", f"Choose the {repo_name} checkout..."):
				self._repository_paths_dialog_repo = repo_name
				self._repository_paths_dialog = pfd.select_folder(f"Choose {repo_name}", configured.get(repo_name, ""))
			self._end_attention_flash(flashing)

	def _draw_ui_font_settings(self):
		"""Settings tab -- lets the user pick the UI text font/size (see
		ForgeryApp._load_ui_font(), Settings.ui_font_name/ui_font_size).
		Takes effect on the next launch only -- no live font-atlas rebuild."""
		settings = app_settings.load()
		font_names = list(_AVAILABLE_FONTS)
		current_index = font_names.index(settings.ui_font_name) if settings.ui_font_name in font_names else 0

		imgui.text("Font: ")
		imgui.same_line()
		imgui.set_next_item_width(200)
		changed, new_index = imgui.combo("##ui-font-name", current_index, font_names)
		if changed:
			settings.ui_font_name = font_names[new_index]
			app_settings.save(settings)

		imgui.text("Size: ")
		imgui.same_line()
		imgui.set_next_item_width(100)
		changed, new_size = imgui.drag_float("##ui-font-size", settings.ui_font_size, v_speed=0.5, v_min=8.0, v_max=32.0)
		if changed:
			settings.ui_font_size = new_size
			app_settings.save(settings)

		if imgui.button("Restart now"):
			self.relaunch()
		imgui.same_line()
		imgui.text_disabled("Restart Patina for a font/size change to take effect.")

	def _draw_bottom_bar(self):
		"""Pinned at the very bottom of the panel: Save (only for a loaded,
		writable shape -- always targets the active workspace, see the
		Workspaces chantier in `.todo/forgery-object-editor.md`), Export
		(format picker, then the existing ExportDialog flow -- see
		ExportDialog.export() -- applied to the shape's current in-memory
		state, edits included, not a re-read from disk/bnp), and Quit flush
		against the right edge. Always drawn (even with nothing loaded) so
		Quit stays reachable. The active-workspace row used to live here too
		-- moved to the top of the Explorer window instead (see
		Explorer.extra_header in __init__), always reachable there regardless
		of what's loaded in this panel."""
		imgui.separator()

		writable = self.shape_file is not None and self.shape_file.type_name in _WRITABLE_SHAPE_TYPES
		if writable:
			save_path = self._workspace_shape_save_path()
			imgui.begin_disabled(save_path is None)
			if _colored_button(f"{fa_icons.ICON_FA_SAVE} Save", _SAVE_BUTTON_COLOR):
				self._on_save_clicked()
			imgui.end_disabled()
			if imgui.is_item_hovered() and save_path is None:
				imgui.set_tooltip("Save unavailable -- set up a Workspaces folder and pick an active workspace first")
			imgui.same_line()

			workspace_dir = self.workspace_setup_dialog.active_workspace_dir
			if workspace_dir is not None:
				if _colored_button("Export", _QUICK_EXPORT_BUTTON_COLOR):
					imgui.open_popup("##quick-export-format-popup")
				if imgui.is_item_hovered():
					imgui.set_tooltip(f"Export straight to {workspace_dir / 'exports'}, no prompts")
				if imgui.begin_popup("##quick-export-format-popup"):
					for export_format in EXPORT_FORMATS:
						clicked, _ = imgui.selectable(f"{export_format.label} (.{export_format.extension})", False)
						if clicked:
							exports_dir = workspace_dir / "exports"
							exports_dir.mkdir(parents=True, exist_ok=True)
							self.export_dialog.quick_export(
								self.shape_file.value, self._shape_source_name or "shape", export_format,
								self.search_paths_dialog.find_texture, exports_dir)
					imgui.separator()
					bnp_clicked, _ = imgui.selectable(f"Full workspace ({workspace_dir.name}.bnp)", False)
					if bnp_clicked:
						exports_dir = workspace_dir / "exports"
						exports_dir.mkdir(parents=True, exist_ok=True)
						self.export_dialog.quick_export_workspace_bnp(workspace_dir, exports_dir)
					imgui.end_popup()
				imgui.same_line()

			if _colored_button("Export as...", _EXPORT_AS_BUTTON_COLOR):
				imgui.open_popup("##export-format-popup")
			if imgui.begin_popup("##export-format-popup"):
				for export_format in EXPORT_FORMATS:
					clicked, _ = imgui.selectable(f"{export_format.label} (.{export_format.extension})", False)
					if clicked:
						source_folder = (
							self._shape_source_path.parent if self._shape_source_path is not None else None)
						self.export_dialog.export(
							self.shape_file.value, self._shape_source_name or "shape", export_format,
							self.search_paths_dialog.find_texture, source_folder=source_folder)
				if workspace_dir is not None:
					imgui.separator()
					bnp_clicked, _ = imgui.selectable(f"Full workspace ({workspace_dir.name}.bnp)", False)
					if bnp_clicked:
						source_folder = (
							self._shape_source_path.parent if self._shape_source_path is not None else None)
						self.export_dialog.export_workspace_bnp(workspace_dir, source_folder=source_folder)
				imgui.end_popup()
			imgui.same_line()

		quit_label = "Quit"
		quit_width = imgui.calc_text_size(quit_label).x + imgui.get_style().frame_padding.x * 2
		avail = imgui.get_content_region_avail().x
		if avail > quit_width:
			imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + avail - quit_width)
		if _colored_button(quit_label, _QUIT_BUTTON_COLOR):
			self.userExit()

		if writable:
			self._draw_save_confirmation_popup()
			if self._save_status:
				imgui.text_wrapped(self._save_status)

	def _draw_textures_tab(self):
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			imgui.text("No materials.")
			return

		multi_bitmap_ids = {material_id for material_id, _ in self._multi_bitmap_entries()}

		self._draw_global_panoply_section()

		imgui.text("Simple textures")
		imgui.separator()
		simple_ids = [material_id for material_id in range(len(materials)) if material_id not in multi_bitmap_ids]
		if not simple_ids:
			imgui.text_disabled("(none)")
		for material_id in simple_ids:
			self._draw_simple_material_row(material_id, materials[material_id])

		self._draw_multi_bitmap_editor()
		self._poll_texture_browse_dialogs()

	def _toggle_material_flag(self, material_id, material, flag):
		material.flags ^= flag
		self._reapply_material(material_id)

	def _draw_materials_tab(self):
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			imgui.text("No materials.")
			return

		multi_bitmap_ids = {material_id for material_id, _ in self._multi_bitmap_entries()}
		hovered_hint = None

		self._draw_global_panoply_section()

		for material_id, material in enumerate(materials):
			imgui.push_id(f"mat-row-{material_id}")

			self._draw_material_color_button(material_id)
			imgui.same_line()
			imgui.text(f"#{material_id}")
			imgui.same_line()

			is_multi = material_id in multi_bitmap_ids
			texture = material.textures[0] if material.textures else None
			has_texture = texture is not None and bool(texture.file_name)
			if is_multi:
				badge = "Multi Bitmap"
			elif has_texture:
				badge = "Simple bitmap"
			else:
				badge = "Color"
			imgui.text_disabled(badge)
			imgui.same_line()

			if badge != "Color":
				double_sided = bool(material.flags & _IDRV_MAT_DOUBLE_SIDED)
				changed, double_sided = imgui.checkbox("Double-sided", double_sided)
				if changed:
					self._toggle_material_flag(material_id, material, _IDRV_MAT_DOUBLE_SIDED)

			if not is_multi:
				imgui.same_line()
				if _icon_button(fa_icons.ICON_FA_CLONE, "Convert to Multi Bitmap (add season/quality/ecosystem variants)"):
					self._convert_to_multi_bitmap(material_id, material)
			else:
				populated = self._multi_bitmap_populated_slots(texture)
				if not populated:
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_UNDO, "Convert to Color (no set has a texture)"):
						self._convert_multi_bitmap_to_color(material_id, material)
				elif populated == [0]:
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_UNDO, "Convert to Simple bitmap (only Low Quality has a texture)"):
						self._convert_multi_bitmap_to_simple(material_id, material)

			imgui.same_line()
			expanded = material_id in self._material_expanded
			expand_icon = fa_icons.ICON_FA_CHEVRON_DOWN if expanded else fa_icons.ICON_FA_CHEVRON_RIGHT
			if _icon_button(expand_icon, "Collapse" if expanded else "Expand: edit every material property"):
				if expanded:
					self._material_expanded.discard(material_id)
				else:
					self._material_expanded.add(material_id)

			if expanded:
				imgui.indent()
				section_hint = self._draw_material_section(
					material_id, "transparency", "Transparency", self._draw_material_transparency_section, material)
				if section_hint:
					hovered_hint = section_hint
				if badge != "Color":
					section_hint = self._draw_material_section(
						material_id, "texture-filtering", "Texture filtering",
						self._draw_material_texture_filtering_section, material)
					if section_hint:
						hovered_hint = section_hint
					section_hint = self._draw_material_section(
						material_id, "texture-transform", "Texture offset/tiling/rotation",
						self._draw_material_texture_transform_section, material)
					if section_hint:
						hovered_hint = section_hint
				imgui.unindent()

			self._draw_panoply_masks_for(texture.file_name if texture is not None else None)

			imgui.pop_id()
			imgui.separator()

		if hovered_hint:
			self.sysinfo.set_status(hovered_hint, color=_STATUS_HINT_COLOR)
			self._material_hint_shown = True
		elif self._material_hint_shown:
			self.sysinfo.set_status("")
			self._material_hint_shown = False

	def _draw_material_section(self, material_id, key, label, draw_fn, material):
		"""One collapsible category (e.g. "Transparency") within a material's
		expanded property editor (see _draw_materials_tab()) -- same
		expand/collapse chevron pattern as the Multi Bitmap slot rows and the
		material row itself, one level deeper. `self._material_section_expanded`
		is keyed by (material_id, key) since the same category key repeats
		across materials. Its own expand chevron uses the exact same icon
		glyph as the material row's own expand chevron (see
		_draw_materials_tab()) -- both live under that same
		push_id(f"mat-row-{material_id}") scope, so without a push_id of its
		own here, ImGui saw two visible buttons with an identical ID
		(warning: "2 visible items with same ID") whenever both happened to
		show the same collapsed/expanded icon."""
		imgui.push_id(key)
		section_id = (material_id, key)
		expanded = section_id in self._material_section_expanded
		expand_icon = fa_icons.ICON_FA_CHEVRON_DOWN if expanded else fa_icons.ICON_FA_CHEVRON_RIGHT
		if _icon_button(expand_icon, "Collapse" if expanded else f"Expand: edit {label}"):
			if expanded:
				self._material_section_expanded.discard(section_id)
			else:
				self._material_section_expanded.add(section_id)
		imgui.same_line()
		imgui.text(label)

		hint = None
		if expanded:
			imgui.indent()
			hint = draw_fn(material_id, material)
			imgui.unindent()
		imgui.pop_id()
		return hint

	def _doc_hint_if_hovered(self, key):
		"""Returns docs/material_options.md's player-facing summary for `key`
		(see material_docs.py) if the item just drawn is currently hovered,
		else None -- same "orange status bar hint" mechanism the Multi Bitmap
		editor already uses (_draw_multi_bitmap_editor()), reused here so the
		material property editor's own, much less self-explanatory options
		(blend funcs, alpha test...) get the same inline documentation."""
		if not imgui.is_item_hovered():
			return None
		doc = self.material_docs.get(key)
		return doc.summary if doc else None

	def _draw_material_transparency_section(self, material_id, material):
		"""CMaterial's transparency-related fields: BLEND (advanced blending,
		e.g. glass/additive effects), ALPHA_TEST (cutout transparency, e.g.
		foliage), and the "simple" opacity carried by diffuse's own alpha
		channel when neither of those flags is set. See
		docs/material_options.md's "Mélange"/"Test alpha"/"Opacité" sections
		for the player-facing explanation of each -- returns whichever of
		those was hovered this frame (see _draw_material_section()), or None."""
		hint = None

		blend_on = bool(material.flags & _IDRV_MAT_BLEND)
		changed, blend_on = imgui.checkbox("Blend (advanced transparency)", blend_on)
		hint = self._doc_hint_if_hovered("blend") or hint
		if changed:
			self._toggle_material_flag(material_id, material, _IDRV_MAT_BLEND)

		if blend_on:
			imgui.indent()
			if imgui.button("Alpha Blend"):
				material.src_blend, material.dst_blend = _TBLEND_PRESET_ALPHA
				self._reapply_material(material_id)
			hint = self._doc_hint_if_hovered("blend") or hint
			imgui.same_line()
			if imgui.button("Additive"):
				material.src_blend, material.dst_blend = _TBLEND_PRESET_ADDITIVE
				self._reapply_material(material_id)
			hint = self._doc_hint_if_hovered("blend") or hint
			imgui.set_next_item_width(180)
			src_changed, src_index = imgui.combo("Src Blend", material.src_blend, _TBLEND_NAMES)
			hint = self._doc_hint_if_hovered("blend-factors") or hint
			if src_changed and src_index != material.src_blend:
				material.src_blend = src_index
				self._reapply_material(material_id)
			imgui.set_next_item_width(180)
			dst_changed, dst_index = imgui.combo("Dst Blend", material.dst_blend, _TBLEND_NAMES)
			hint = self._doc_hint_if_hovered("blend-factors") or hint
			if dst_changed and dst_index != material.dst_blend:
				material.dst_blend = dst_index
				self._reapply_material(material_id)
			imgui.unindent()

		alpha_test_on = bool(material.flags & _IDRV_MAT_ALPHA_TEST)
		changed, alpha_test_on = imgui.checkbox("Alpha test (cutout transparency)", alpha_test_on)
		hint = self._doc_hint_if_hovered("alpha-test") or hint
		if changed:
			self._toggle_material_flag(material_id, material, _IDRV_MAT_ALPHA_TEST)

		if alpha_test_on:
			imgui.indent()
			imgui.set_next_item_width(180)
			changed, threshold = imgui.slider_float("Threshold", material.alpha_test_threshold, 0.0, 1.0)
			hint = self._doc_hint_if_hovered("alpha-test") or hint
			if changed and threshold != material.alpha_test_threshold:
				material.alpha_test_threshold = threshold
				self._reapply_material(material_id)
			imgui.unindent()

		if blend_on or alpha_test_on:
			# Diffuse alpha has no visible effect at all otherwise: with
			# neither flag set, the material renders fully opaque regardless
			# of this value (confirmed against driver_opengl_material.cpp --
			# blend is what actually consumes alpha as a blend factor, and
			# alpha test is the only other consumer, via the default TexEnv's
			# Modulate/AlphaArg1=Diffuse combining texture*diffuse alpha
			# before the glAlphaFunc() cutoff). Hiding it otherwise avoids
			# offering a slider that visibly does nothing.
			imgui.set_next_item_width(180)
			current_opacity = material.diffuse.a / 255.0
			changed, opacity = imgui.slider_float("Opacity (diffuse alpha)", current_opacity, 0.0, 1.0)
			hint = self._doc_hint_if_hovered("opacity") or hint
			if changed:
				d = material.diffuse
				material.diffuse = Rgba(d.r, d.g, d.b, round(opacity * 255))
				self._reapply_material(material_id)

		return hint

	def _draw_material_texture_filtering_section(self, material_id, material):
		"""Slot 0's texture sampling params (wrap S/T, mag/min filter) -- see
		docs/material_options.md's "Filtrage et répétition des textures".
		Scoped to slot 0 only, same as _draw_simple_material_row(): the
		simplified editor doesn't expose the other 3 texture stages either."""
		hint = None
		texture = material.textures[0] if material.textures else None
		if texture is None:
			imgui.text_disabled("No texture on this material.")
			return hint

		def _changed_texture_setting():
			# A cache hit in load_panda_texture() skips straight past
			# wrap/filter (see its own docstring) -- only a fresh decode
			# picks up the new value, so the cache must be dropped first.
			self._texture_cache.clear()
			self._reapply_material(material_id)

		imgui.set_next_item_width(180)
		wrap_s_index = texture.wrap_s if texture.wrap_s is not None else _TEXTURE_WRAP_DEFAULT
		changed, new_index = imgui.combo("Wrap S (horizontal)", wrap_s_index, _TEXTURE_WRAP_NAMES)
		hint = self._doc_hint_if_hovered("texture-filtering") or hint
		if changed and new_index != texture.wrap_s:
			texture.wrap_s = new_index
			_changed_texture_setting()

		imgui.set_next_item_width(180)
		wrap_t_index = texture.wrap_t if texture.wrap_t is not None else _TEXTURE_WRAP_DEFAULT
		changed, new_index = imgui.combo("Wrap T (vertical)", wrap_t_index, _TEXTURE_WRAP_NAMES)
		hint = self._doc_hint_if_hovered("texture-filtering") or hint
		if changed and new_index != texture.wrap_t:
			texture.wrap_t = new_index
			_changed_texture_setting()

		imgui.set_next_item_width(180)
		mag_index = texture.mag_filter if texture.mag_filter is not None else _TEXTURE_MAG_FILTER_DEFAULT
		changed, new_index = imgui.combo("Mag Filter (close up)", mag_index, _TEXTURE_MAG_FILTER_NAMES)
		hint = self._doc_hint_if_hovered("texture-filtering") or hint
		if changed and new_index != texture.mag_filter:
			texture.mag_filter = new_index
			_changed_texture_setting()

		imgui.set_next_item_width(180)
		min_index = texture.min_filter if texture.min_filter is not None else _TEXTURE_MIN_FILTER_DEFAULT
		changed, new_index = imgui.combo("Min Filter (far away)", min_index, _TEXTURE_MIN_FILTER_NAMES)
		hint = self._doc_hint_if_hovered("texture-filtering") or hint
		if changed and new_index != texture.min_filter:
			texture.min_filter = new_index
			_changed_texture_setting()

		grayscale_as_alpha = bool(texture.load_grayscale_as_alpha)
		changed, grayscale_as_alpha = imgui.checkbox(
			"Grayscale image = alpha mask (not a visible gray color)", grayscale_as_alpha)
		hint = self._doc_hint_if_hovered("texture-filtering") or hint
		if changed:
			texture.load_grayscale_as_alpha = grayscale_as_alpha
			_changed_texture_setting()

		return hint

	def _draw_material_texture_transform_section(self, material_id, material):
		"""Slot 0's UV offset/tiling/rotation (Material.tex_user_mat[0]) --
		see docs/material_options.md's "Matrice de texture exportée". Scoped
		to slot 0 only, same as the other simplified per-texture sections.

		NOTE: reflected in the 3D viewport via _apply_material_texture()'s
		set_tex_transform() call, using shape_geometry.py's
		uv_matrix_to_panda_mat4() -- Offset U/V and the V-mirror correction
		in there are confirmed correct against the real client (2026-08-28);
		Rotation's sign is not yet (still applied as-is, untested against a
		real rotated shape -- see examples/test_cube_wrap.shape). The value
		is correctly stored/round-tripped in the .shape either way."""
		hint = None
		stage0_flag = _IDRV_MAT_USER_TEX_MAT_STAGE0
		enabled = bool(material.flags & _IDRV_MAT_TEX_ADDR) and bool(material.flags & stage0_flag)
		changed, enabled = imgui.checkbox("Offset/tile/rotate this texture", enabled)
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		if changed:
			if enabled:
				material.flags |= _IDRV_MAT_TEX_ADDR | stage0_flag
				if material.tex_user_mat.get(0) is None:
					material.tex_user_mat[0] = compose_uv_matrix(0.0, 0.0, 1.0, 1.0, 0.0)
			else:
				material.flags &= ~stage0_flag
				# Other 3 stages' enable bits (0x00200000/0x00400000/0x00800000,
				# see material.h's enableUserTexMat()) -- IDRV_MAT_TEX_ADDR only
				# turns off once none of the 4 are set anymore.
				if not (material.flags & 0x00F00000):
					material.flags &= ~_IDRV_MAT_TEX_ADDR
			self._reapply_material(material_id)

		if not enabled:
			return hint

		# Tracked independently per material_id rather than re-derived from
		# Material.tex_user_mat[0] every frame, so a slider drag doesn't
		# fight with decompose_uv_matrix()'s own re-derivation. Seeded once
		# (best-effort, via decompose) the first time this section is drawn
		# for a material that already had a matrix from elsewhere (a fresh
		# import, or a real 3dsMax-authored shape).
		state = self._tex_transform_ui_state.get(material_id)
		if state is None:
			offset_u, offset_v, scale_u, scale_v, rotation = decompose_uv_matrix(material.tex_user_mat.get(0))
			state = dict(
				offset_u=offset_u, offset_v=offset_v, scale_u=scale_u, scale_v=scale_v, rotation=rotation)
			self._tex_transform_ui_state[material_id] = state

		imgui.indent()
		any_changed = False

		imgui.set_next_item_width(180)
		changed, state["offset_u"] = imgui.drag_float(
			"Offset U", state["offset_u"], v_speed=0.005, v_min=-10.0, v_max=10.0, format="%.3f")
		any_changed = any_changed or changed
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		imgui.set_next_item_width(180)
		changed, state["offset_v"] = imgui.drag_float(
			"Offset V", state["offset_v"], v_speed=0.005, v_min=-10.0, v_max=10.0, format="%.3f")
		any_changed = any_changed or changed
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		# Rotation combined with tiling (Scale != 1) produces a GPU wrap
		# artifact (extra/missing visible tile repeats along the rotated
		# axis, confirmed 2026-08-28 -- inherent to how texture-matrix
		# rotation interacts with hardware repeat-wrap, not fixable here)
		# and isn't used by any real shape in ryzom_live/data (0/2600
		# scanned). Mutually exclusive in the UI rather than silently
		# producing that artifact: whichever is already non-neutral blocks
		# editing the other until it's reset back to neutral (0 degrees /
		# scale 1) first.
		rotation_active = abs(state["rotation"]) > 0.01
		scale_active = abs(state["scale_u"] - 1.0) > 0.001 or abs(state["scale_v"] - 1.0) > 0.001

		imgui.begin_disabled(rotation_active)
		imgui.set_next_item_width(180)
		changed, state["scale_u"] = imgui.drag_float(
			"Scale U (tiling)", state["scale_u"], v_speed=0.01, v_min=0.01, v_max=100.0, format="%.3f")
		any_changed = any_changed or changed
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		imgui.set_next_item_width(180)
		changed, state["scale_v"] = imgui.drag_float(
			"Scale V (tiling)", state["scale_v"], v_speed=0.01, v_min=0.01, v_max=100.0, format="%.3f")
		any_changed = any_changed or changed
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		imgui.end_disabled()
		if rotation_active and imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
			imgui.set_tooltip("Reset Rotation to 0 first -- rotation + tiling together isn't supported (GPU wrap artifact)")

		imgui.begin_disabled(scale_active)
		imgui.set_next_item_width(180)
		changed, state["rotation"] = imgui.drag_float(
			"Rotation (degrees)", state["rotation"], v_speed=0.5, v_min=-360.0, v_max=360.0, format="%.2f")
		any_changed = any_changed or changed
		hint = self._doc_hint_if_hovered("export-texture-matrix") or hint
		imgui.end_disabled()
		if scale_active and imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
			imgui.set_tooltip("Reset Scale U/V to 1 first -- rotation + tiling together isn't supported (GPU wrap artifact)")

		imgui.unindent()

		if any_changed:
			material.tex_user_mat[0] = compose_uv_matrix(
				state["offset_u"], state["offset_v"], state["scale_u"], state["scale_v"], state["rotation"])
			self._reapply_material(material_id)

		return hint

	@staticmethod
	def _set_simple_material_texture(material, file_name):
		"""Sets slot 0's texture file name, creating a plain CTextureFile
		there if that slot was empty -- preserves any other texture slots
		the material might have rather than replacing the whole list."""
		if material.textures and material.textures[0] is not None:
			material.textures[0].file_name = file_name
		elif material.textures:
			material.textures[0] = Texture(class_name="CTextureFile", file_name=file_name)
		else:
			material.textures = [Texture(class_name="CTextureFile", file_name=file_name)]

	def _convert_to_multi_bitmap(self, material_id, material):
		"""Turns slot 0's plain texture into a `CTextureMultiFile` with a
		single set (the current texture, at index 0) -- e.g. for a material
		imported from .obj/.dae, always single-texture, that needs to become
		editable in the Multi Bitmap section (season/quality/ecosystem
		variants)."""
		texture = material.textures[0] if material.textures else None
		current_name = texture.file_name if texture else ""
		new_texture = Texture(
			class_name="CTextureMultiFile", file_name=current_name, file_names=[current_name], selected_index=0)
		if material.textures:
			material.textures[0] = new_texture
		else:
			material.textures = [new_texture]
		self._reapply_material(material_id)

	@staticmethod
	def _multi_bitmap_populated_slots(texture):
		return [i for i, name in enumerate(texture.file_names) if name]

	def _convert_multi_bitmap_to_simple(self, material_id, material):
		"""The reverse of _convert_to_multi_bitmap(): only offered when slot
		0 (Low Quality) is the Multi Bitmap's only populated set, so nothing
		is lost -- swaps it for a plain CTextureFile using that texture."""
		texture = material.textures[0]
		name = texture.file_names[0] if texture.file_names else ""
		material.textures[0] = Texture(class_name="CTextureFile", file_name=name)
		self._reapply_material(material_id)

	def _convert_multi_bitmap_to_color(self, material_id, material):
		"""Only offered when a Multi Bitmap material has no texture set in
		any slot -- clears its texture entirely, making it a plain color."""
		material.textures = []
		self._reapply_material(material_id)

	def _draw_simple_material_row(self, material_id, material):
		imgui.push_id(f"mat-simple-{material_id}")

		self._draw_texture_color_button(material_id, material)
		imgui.same_line()

		texture = material.textures[0] if material.textures else None
		current_value = texture.file_name if (texture and texture.file_name) else ""
		in_workspace = self._is_texture_in_workspace(current_value)
		imgui.push_style_color(
			imgui.Col_.text.value, _TEXTURE_IN_WORKSPACE_COLOR if in_workspace else _TEXTURE_NORMAL_COLOR)
		changed, new_value = self._draw_texture_name_combo("##texture-combo", current_value)
		imgui.pop_style_color()
		if imgui.is_item_hovered() and current_value:
			source_path = self._texture_source_path_text(current_value)
			tooltip = source_path or current_value
			if in_workspace:
				tooltip += "\n(in the active workspace)"
			imgui.set_tooltip(tooltip)
		if changed and new_value != current_value:
			self._set_simple_material_texture(material, new_value)
			self._reapply_material(material_id)
		imgui.same_line()

		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
			def _on_result(file_name, material_id=material_id, material=material):
				self._set_simple_material_texture(material, file_name)
				self._reapply_material(material_id)
			self._start_texture_browse(("simple", material_id), _on_result)
		imgui.same_line()

		def _on_copied(new_name, material_id=material_id, material=material):
			self._set_simple_material_texture(material, new_name)
			self._reapply_material(material_id)
		self._draw_texture_copy_button(current_value, _on_copied)

		self._draw_panoply_masks_for(current_value)

		imgui.pop_id()

	def _on_exit(self):
		"""Runs right before the process actually exits, whether that was
		requested via the in-app Quit button or the OS's own window-close
		control -- see app.py's ForgeryApp.__init__()/_on_exit()."""
		self._save_session_state()

	def panel_title(self):
		return self._shape_source_name or "Panel"

	def draw_panel(self):
		self.nav_cube.draw_controls()
		self._draw_transform_panel()
		self._draw_viewport_toggles()
		self._draw_wind_controls()
		self._draw_bone_preview_controls()
		self._draw_reference_shapes_toggles()
		self.export_dialog.draw()
		self.import_dialog.draw()
		self.search_paths_dialog.draw()
		self.workspace_setup_dialog.draw()
		self._poll_skeleton_file_dialog()
		self._poll_animation_file_dialog()
		self._poll_image_editor_dialog()
		self._poll_text_editor_dialog()
		self._poll_workspace_sync_folder_dialog()
		self._poll_repository_paths_dialog()
		self._draw_replace_match_popup()
		self._draw_restore_scan_popup()
		self._draw_bake_progress_popup()
		self._draw_import_conflict_popup()
		self._flush_pending_import_status()

		if self.shape_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self.shape_error)

		if self.shape_file is not None:
			imgui.text(f"Type: {self.shape_file.type_name}")
			imgui.separator()
		else:
			imgui.text("Select a .shape file in the explorer.")

		# The tab bar itself is always shown (not gated behind a loaded
		# shape) so Settings -- an app-wide preference, not tied to any one
		# shape -- stays reachable even with nothing loaded yet. The other
		# three tabs only make sense once there's a shape to describe.
		if imgui.begin_tab_bar("##panel-tabs"):
			if self.shape_file is not None:
				_push_tab_color(_TAB_COLOR_TEXTURES)
				if _begin_tab_item_with_icon(fa_icons.ICON_FA_IMAGE, "Textures"):
					self._draw_textures_tab()
					imgui.end_tab_item()
				_pop_tab_color()
				_push_tab_color(_TAB_COLOR_MATERIALS)
				if _begin_tab_item_with_icon(fa_icons.ICON_FA_PAINT_BRUSH, "Materials"):
					self._draw_materials_tab()
					imgui.end_tab_item()
				_pop_tab_color()
				_push_tab_color(_TAB_COLOR_ALL_PROPERTIES)
				if _begin_tab_item_with_icon(fa_icons.ICON_FA_TABLE, "All Properties"):
					draw_properties(self.shape_file.value)
					imgui.end_tab_item()
				_pop_tab_color()
			_push_tab_color(_TAB_COLOR_SETTINGS)
			if _begin_tab_item_with_icon(fa_icons.ICON_FA_COG, "Settings", flags=self._consume_settings_tab_flags()):
				# Workspaces folder folded in here (not its own header) --
				# it's a path setting like search_paths, just the one
				# special-cased root instead of a priority-ordered list.
				self._consume_settings_section_open("Paths")
				if imgui.collapsing_header("Paths"):
					self.workspace_setup_dialog.draw_settings_content()
					imgui.separator()
					self.search_paths_dialog.draw_settings_content()
					imgui.separator()
					self._draw_repository_paths_settings()
				self._consume_settings_section_open("Tools")
				if imgui.collapsing_header("Tools"):
					self._draw_image_editor_settings()
					imgui.separator()
					self._draw_text_editor_settings()
					imgui.separator()
					self._draw_workspace_sync_settings()
					imgui.separator()
					self._draw_ui_font_settings()
				imgui.end_tab_item()
			_pop_tab_color()
			imgui.end_tab_bar()

		self._draw_bottom_bar()


def main(argv=None):
	ObjectEditorApp().run()


if __name__ == "__main__":
	main()
