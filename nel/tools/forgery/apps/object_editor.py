"""Ryzom Forgery object editor: browse, inspect and edit .shape files.

3D display is supported for CMesh, CMeshMRM (finest LOD) and CMeshMultiLod
(slot 0, whose geometry is itself a CMesh/CMeshMRM). Other shape types
(skeleton, water, flare, particles, ...) show their properties only, no 3D
render yet.
"""

from math import ceil, cos, pi, radians, sin
from pathlib import Path

import numpy

from panda3d.core import (
	AlphaTestAttrib, ClockObject, ColorBlendAttrib, Geom, GeomNode, GeomTriangles, GeomVertexData,
	GeomVertexFormat, GeomVertexWriter, InternalName, LineSegs, Material as PandaMaterial, Point3, Quat,
	Texture as PandaTexture, TransparencyAttrib, Vec3,
)

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx, portable_file_dialogs as pfd

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import ObjectManipulator, OrbitCamera
from ryzom_forgery.asset_index import AssetIndex
from ryzom_forgery.export_dialog import ExportDialog
from ryzom_forgery.import_dialog import ImportDialog
from ryzom_forgery.material_docs import load_material_docs
from ryzom_forgery.navcube import NavigationCube
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.shape_geometry import (
	iter_render_passes, load_panda_texture, rgba_to_color, shape_bbox, shape_geom, solid_color_texture,
)
from ryzom_forgery.shape_import import texture_search_dirs_for

from pynel.ryzom_shape import (
	Rgba, ShapeFile, ShapeParseError, ShapeWriteError, Texture, WindTreeParams, parse_shape, save_shape,
)

DEFAULT_DATA_ROOT = Path("~/.local/share/Ryzom/ryzom_live/data").expanduser()

# Shape types pynel's save_shape() can actually write back out -- matches
# ryzom_shape.py's _SHAPE_CLASS_NAMES, the Save/Save As UI only shows for these.
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

_STATUS_HINT_COLOR = (1.0, 0.6, 0.15, 1.0)  # orange, for material_options.md hints shown in the status bar
_COLOR_POPUP_ID = "material-color-picker"
_DIFFUSE_COLOR_POPUP_ID = "material-diffuse-picker"

# CMaterial flag/enum values (nel/include/nel/3d/material.h), needed to render
# translucent materials (e.g. glass) correctly instead of opaque.
_IDRV_MAT_ZWRITE = 0x00000004
_IDRV_MAT_BLEND = 0x00000080
_IDRV_MAT_DOUBLE_SIDED = 0x00000100
_IDRV_MAT_ALPHA_TEST = 0x00000200

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


def _icon_button(icon, tooltip, active=False, square=False, large_font=None):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory. `active` highlights it (toggle-button style) when the
	feature it controls is currently on. `square` sizes it to the current
	font's frame height on both axes, so a row of these all matches -- without
	it, imgui.button()'s default auto-size makes each button only as wide as
	its own glyph, which visibly varies between Font Awesome icons.
	`large_font`, if given, is an (ImFont, size) pair (app.large_icon_font,
	app.large_icon_font_size) pushed around just the button glyph itself --
	NOT the tooltip below, which needs the normal text font: that font is
	icon-glyphs-only, so a tooltip drawn under it renders as blank/invisible
	text instead of readable words."""
	if active:
		imgui.push_style_color(imgui.Col_.button.value, (0.26, 0.59, 0.98, 0.8))
	if large_font is not None:
		imgui.push_font(*large_font)
	size = (imgui.get_frame_height(), imgui.get_frame_height()) if square else (0, 0)
	clicked = imgui.button(icon, size)
	if large_font is not None:
		imgui.pop_font()
	if active:
		imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


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


class ObjectEditorApp(ForgeryApp):
	def __init__(self, data_root=DEFAULT_DATA_ROOT):
		ForgeryApp.__init__(self, explorer_root=data_root, title="Ryzom Forgery - Object Editor",
		                     explorer_default_filter="*.shape")

		self.data_root = Path(data_root)
		self.material_docs = load_material_docs()
		self.asset_index = AssetIndex(self.data_root)
		self.sysinfo.set_status("Indexing assets...")
		self.asset_index.build()
		self.sysinfo.set_status(f"{len(self.asset_index)} assets indexed")

		self._texture_cache = {}
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
		self._material_node_paths = {}  # material_id -> list[NodePath], to re-apply a material live after an edit
		self._multi_bitmap_expanded = set()  # slot indices currently expanded in the Multi Bitmap editor
		self._multi_bitmap_hint_shown = False  # whether the status bar currently shows one of our doc hints
		self._texture_browse_dialogs = {}  # key -> (in-flight portable_file_dialogs.open_file, on_result callback)
		self._material_override_colors = {}  # material_id -> (r,g,b,a), a manual flat-color override for that material

		self._shape_source_path = None  # Path on disk, or None if loaded from inside a .bnp (Save disabled then)
		self._shape_source_name = None  # original file name, kept even when _shape_source_path is None -- for Save As's default filename
		self._texture_search_dirs = []  # extra folders (see shape_geometry.load_panda_texture's search_dirs) to fall back to for this shape's textures -- an imported mesh's own folder, tex/textures/data subfolders included
		self._texture_needs_repeat = False  # see _uvs_need_repeat() -- whether the loaded shape's own UVs rely on texture tiling
		self._save_overwrite_confirmed = False  # session-scoped: asked once, no more Save confirmations after that
		self._confirm_overwrite_open = False
		self._save_dialog = None  # in-flight portable_file_dialogs.save_file, for Save As
		self._save_status = ""

		self._replace_pending_mesh = None  # imported Mesh awaiting material-count matching (mismatched-count "Replace")
		self._replace_mapping = []  # per pending-mesh-material index: target existing material index, or None = "add as new"
		self._replace_match_popup_opened = False  # whether open_popup() has been called yet for the current _replace_pending_mesh

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
		self.taskMgr.add(self._update_wind, "object-editor-wind")
		self.export_dialog = ExportDialog()
		self.import_dialog = ImportDialog(on_new_shape=self._on_import_new_shape, on_replace=self._on_import_replace)
		self.commands.register_for_extension(".shape", "Load in viewer", self._on_load_command)
		for export_format in EXPORT_FORMATS:
			self.commands.register_for_extension(
				".shape", f"Export to .{export_format.extension}",
				lambda items, fmt=export_format: self._on_export_command(items, fmt))
		self.commands.register_global("Export settings...", lambda items: self.export_dialog.open())
		self.explorer.extra_toolbar = self._draw_import_toolbar_button

	def on_selection_changed(self, items):
		print(f"[object_editor] selection changed: {[item.name for item in items]}")
		if len(items) == 1 and items[0].suffix.lower() == ".shape":
			self._load_shape(items[0])

	def _on_load_command(self, items):
		if items:
			self._load_shape(items[0])

	def _on_export_command(self, items, export_format):
		if items:
			self.export_dialog.export(items[0], export_format, self.asset_index)

	def _draw_import_toolbar_button(self):
		if _icon_button(fa_icons.ICON_FA_UPLOAD, "Import mesh (.obj/.dae/.fbx)..."):
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

	def _reset_shape_state(self):
		"""Common reset before showing any shape in the viewer -- a `.shape`
		opened from the Explorer, or a mesh imported as a brand-new shape."""
		self.model_root.remove_node()
		self.model_root = self._object_pivot.attach_new_node("shape-root")
		self.shape_error = None
		self._material_node_paths = {}
		self._multi_bitmap_expanded = set()
		self._texture_browse_dialogs = {}
		self._material_override_colors = {}
		self._shape_source_path = None
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
		self._preview_texture_refs = {}

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
			grid_center_x, grid_center_y = bbox.center.x, bbox.center.y
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
		preview otherwise."""
		if self._wind_state is None:
			return

		imgui.text("Wind preview")
		_, self._wind_animate = imgui.checkbox("Animate", self._wind_animate)
		imgui.set_next_item_width(160)
		_, self._wind_power = imgui.slider_float("Strength", self._wind_power, 0.0, 1.0)
		imgui.set_next_item_width(160)
		_, self._wind_direction_deg = imgui.slider_float("Direction", self._wind_direction_deg, 0.0, 360.0, "%.0f deg")
		imgui.separator()

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

	def reset_object_rotation(self):
		"""Restores the object pivot to its baseline rotation (the shape's own
		default_rot_quat on load, or whatever rotation was in effect at the
		last successful save -- see _write_shape()). Bound to the gizmo's
		Reset button while Ctrl is held (see navcube.py's draw_controls())."""
		self._object_pivot.set_quat(self._object_pivot_base_quat)

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
		self._texture_needs_repeat = False

		geom_value = shape_geom(self.shape_file.value)
		wind_params = geom_value.vertex_program if geom_value is not None else None
		wind_params = wind_params if isinstance(wind_params, WindTreeParams) else None

		vdata = None
		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(self.shape_file.value):
			if not indices:
				continue
			print(f"[object_editor] pass {pass_count}: material={material_id} "
			      f"verts={vertex_buffer.num_verts} tris={len(indices) // 3} "
			      f"channels={list(vertex_buffer.channels.keys())}")
			if vdata is None:
				# Built once, not per-pass: every pass indexes into the exact
				# same vertex array (see iter_render_passes()), and the wind
				# preview needs a single vdata to update per frame rather than
				# one per pass.
				vdata = _build_vertex_data(vertex_buffer, dynamic=wind_params is not None)
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

		if pass_count == 0:
			self.shape_error = f"No renderable 3D geometry for shape type {self.shape_file.type_name!r}"

		bbox = shape_bbox(self.shape_file.value)
		if bbox is not None:
			center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
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

	def _build_reference_geometry(self, shape_value, parent_node_path):
		materials = getattr(shape_value, "materials", None)
		for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
			if not indices:
				continue
			geom = _build_geom(vertex_buffer, indices)
			geom_node = GeomNode(f"ref-pass-{material_id}")
			geom_node.add_geom(geom)
			node_path = parent_node_path.attach_new_node(geom_node)
			material = materials[material_id] if materials and material_id < len(materials) else None
			self._apply_material_common(node_path, material)
			self._apply_material_texture(node_path, material)

	def _rebuild_reference_shapes(self):
		"""Lines up the currently-active reference shapes side by side, just
		past the main shape's own bbox (bottoms/centers aligned on X, so
		comparing sizes doesn't need any camera repositioning)."""
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
			if bbox is not None:
				node_path.set_x(cursor_x + bbox.half_size.x - bbox.center.x)
			self._build_reference_geometry(shape_file.value, node_path)
			if bbox is not None:
				cursor_x += bbox.half_size.x * 2 + _REFERENCE_GAP

	def _draw_reference_shapes_toggles(self):
		"""Top-left viewport bar for the 3 scale-reference toggles (Cube /
		shortest / tallest character) -- square icon buttons at 2x
		_draw_viewport_toggles()'s icon size (app.large_icon_font), since
		these are a more prominent, deliberately-reached-for control."""
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
				if _icon_button(_REFERENCE_ICONS[label], label, label in self._reference_active,
				                square=True, large_font=large_font):
					self._toggle_reference_shape(label)

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

	def _apply_material_texture(self, node_path, material):
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
			panda_texture = load_panda_texture(
				self.asset_index, texture.file_name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
				repeat=self._texture_needs_repeat)
			if panda_texture is not None:
				node_path.set_texture(panda_texture)

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

		self._apply_material_texture(node_path, material)

	def _reapply_material(self, material_id):
		"""Re-runs _apply_material on every NodePath using this material, e.g.
		after an in-place edit of the parsed Material (texture set change...)."""
		for node_path in self._material_node_paths.get(material_id, []):
			self._apply_material(node_path, material_id)

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
			self.asset_index, name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
			repeat=self._texture_needs_repeat)
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
		# That copy is also where alpha gets dropped (format -> RGB): most of
		# these textures are mostly transparent everywhere (e.g. a shape cut
		# from a square atlas), and alpha blending genuinely hides the real
		# color underneath low-alpha pixels -- no backdrop color can undo
		# that (see _draw_image_opaque_bg(), still used as a fallback for
		# whatever's left translucent). Safe to do on this copy specifically
		# now that it's confirmed independent of the live 3D material's own
		# texture (see above) -- doing this to the shared one would have
		# made the actual 3D-rendered object opaque too.
		preview_texture = panda_texture.make_copy()
		preview_texture.set_format(PandaTexture.F_rgb)
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

	def _draw_texture_preview_static(self, name, size=24):
		"""Same thumbnail button (size, frame, hover-zoom tooltip) as
		_draw_texture_preview_button, but disabled -- for spots where
		clicking it wouldn't do anything (e.g. the Textures tab once a
		texture is already set: the text field/browse icon next to it are
		the actual controls), so it doesn't look clickable when it isn't.
		A plain imgui.image() looked visibly smaller: image_button adds the
		usual button frame padding around the image that a bare image()
		doesn't get, and disabling it (rather than swapping widget types)
		keeps that same size."""
		tex_ref = self._get_preview_texture_ref(name)
		if tex_ref is None:
			imgui.dummy((size, size))
			return
		imgui.begin_disabled(True)
		imgui.image_button("##preview", tex_ref, (size, size), bg_col=_PREVIEW_BG_COLOR)
		imgui.end_disabled()
		if imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
			if imgui.begin_tooltip():
				self._draw_image_opaque_bg(tex_ref, (192, 192))
				imgui.text(name)
				imgui.end_tooltip()

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
		texture = material.textures[0] if material.textures else None
		if texture is not None and texture.file_name:
			self._draw_texture_preview_static(texture.file_name)
			return

		diffuse = rgba_to_color(material.diffuse)
		if self._draw_color_preview_button(f"##diffuse-{material_id}", diffuse):
			imgui.open_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}")

		if imgui.begin_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}"):
			changed, new_color = imgui.color_picker4(f"##diffuse-picker-{material_id}", diffuse)
			if changed:
				self._set_material_diffuse_color(material, new_color)
				self._reapply_material(material_id)
			imgui.end_popup()

	def _start_texture_browse(self, key, on_result):
		"""Opens a native file picker for a texture; on_result(file_name) is
		called with just the chosen file's base name once picked, matching
		how texture references are stored (name only, no path)."""
		dialog = pfd.open_file("Choose texture", str(self.data_root), ["Textures", "*.tga *.dds *.png"])
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
				imgui.indent()
				for material_id, texture in entries:
					self._ensure_multi_bitmap_slot(texture, index)
					imgui.push_id(f"mb-mat-{material_id}")

					slot_name = texture.file_names[index]
					if slot_name:
						self._draw_texture_preview_static(slot_name)
					else:
						imgui.dummy((24, 24))
					imgui.same_line()
					imgui.text(f"#{material_id}")
					imgui.same_line()

					imgui.set_next_item_width(220)
					current_value = texture.file_names[index]
					changed, new_value = imgui.input_text(f"##text-{index}", current_value)
					new_value = new_value.lower()  # texture names aren't case-sensitive
					if changed and new_value != current_value:
						texture.file_names[index] = new_value
						if texture.selected_index == index:
							texture.file_name = new_value
							self._reapply_material(material_id)
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
						def _on_result(file_name, material_id=material_id, texture=texture, index=index):
							self._ensure_multi_bitmap_slot(texture, index)
							texture.file_names[index] = file_name
							if texture.selected_index == index:
								texture.file_name = file_name
								self._reapply_material(material_id)
						self._start_texture_browse(("multi-bitmap", material_id, index), _on_result)

					imgui.pop_id()
				imgui.unindent()

			imgui.pop_id()
			imgui.separator()

		if hovered_hint:
			self.sysinfo.set_status(hovered_hint, color=_STATUS_HINT_COLOR)
			self._multi_bitmap_hint_shown = True
		elif self._multi_bitmap_hint_shown:
			self.sysinfo.set_status("")
			self._multi_bitmap_hint_shown = False

	def _write_shape(self, path):
		try:
			save_shape(path, self.shape_file)
			self._save_status = f"Saved to {path}"
			print(f"[object_editor] {self._save_status}")
			# The viewer-only Ctrl+drag rotation is never written into the
			# shape's own default_rot_quat (see _display_shape()), but a save
			# is a natural point to treat "where the object is now" as the new
			# reset baseline -- otherwise Ctrl+Reset would keep snapping back
			# to the pre-save orientation forever.
			self._object_pivot_base_quat = Quat(self._object_pivot.get_quat())
		except (OSError, ShapeWriteError) as exc:
			self._save_status = f"Save failed: {exc}"
			print(f"[object_editor] {self._save_status}")

	def _on_save_clicked(self):
		if self._save_overwrite_confirmed:
			self._write_shape(self._shape_source_path)
		else:
			self._confirm_overwrite_open = True
			imgui.open_popup(_OVERWRITE_POPUP_ID)

	def _draw_save_confirmation_popup(self):
		if not self._confirm_overwrite_open:
			return

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_OVERWRITE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text(f"Overwrite this file?\n{self._shape_source_path}")
		imgui.text_wrapped("You won't be asked again this session.")
		imgui.separator()
		if imgui.button("Overwrite"):
			self._save_overwrite_confirmed = True
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
			self._write_shape(self._shape_source_path)
		imgui.same_line()
		if imgui.button("Cancel"):
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
		imgui.end_popup()

	def _poll_save_dialog(self):
		if self._save_dialog is None or not self._save_dialog.ready(0):
			return
		result = self._save_dialog.result()
		self._save_dialog = None
		if result:
			self._write_shape(Path(result))

	def _draw_save_buttons(self):
		if self.shape_file.type_name not in _WRITABLE_SHAPE_TYPES:
			return

		imgui.separator()
		if self._shape_source_path is not None:
			if imgui.button("Save"):
				self._on_save_clicked()
			imgui.same_line()
		else:
			imgui.text_disabled("Save unavailable (loaded from inside a .bnp archive)")

		if imgui.button("Save As..."):
			# Prefer the shape's own source path (e.g. saving an edited .shape
			# back near where it came from); fall back to wherever the
			# Explorer is currently browsing, plus the original file name if
			# known (e.g. a shape loaded from inside a .bnp, which has no
			# real on-disk path of its own to reuse) -- this used to open
			# with no default folder or name at all in that case.
			if self._shape_source_path is not None:
				default_path = str(self._shape_source_path)
			else:
				name = self._shape_source_name or "untitled.shape"
				default_path = str(Path(self.explorer.root) / name)
			self._save_dialog = pfd.save_file("Save shape as", default_path, ["Ryzom shape", "*.shape"])

		self._draw_save_confirmation_popup()
		self._poll_save_dialog()
		if self._save_status:
			imgui.text_wrapped(self._save_status)

	def _draw_textures_tab(self):
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			imgui.text("No materials.")
			return

		multi_bitmap_ids = {material_id for material_id, _ in self._multi_bitmap_entries()}

		imgui.text("Simple textures")
		imgui.separator()
		simple_ids = [material_id for material_id in range(len(materials)) if material_id not in multi_bitmap_ids]
		if not simple_ids:
			imgui.text_disabled("(none)")
		for material_id in simple_ids:
			self._draw_simple_material_row(material_id, materials[material_id])

		self._draw_multi_bitmap_editor()
		self._poll_texture_browse_dialogs()

	def _toggle_material_double_sided(self, material_id, material):
		material.flags ^= _IDRV_MAT_DOUBLE_SIDED
		self._reapply_material(material_id)

	def _draw_materials_tab(self):
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			imgui.text("No materials.")
			return

		multi_bitmap_ids = {material_id for material_id, _ in self._multi_bitmap_entries()}

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
					self._toggle_material_double_sided(material_id, material)

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

			imgui.pop_id()
			imgui.separator()

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
		imgui.text(f"#{material_id}")
		imgui.same_line()

		texture = material.textures[0] if material.textures else None
		current_value = texture.file_name if (texture and texture.file_name) else ""
		imgui.set_next_item_width(220)
		changed, new_value = imgui.input_text("##text", current_value)
		new_value = new_value.lower()  # texture names aren't case-sensitive
		if changed and new_value != current_value:
			self._set_simple_material_texture(material, new_value)
			self._reapply_material(material_id)
		imgui.same_line()

		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
			def _on_result(file_name, material_id=material_id, material=material):
				self._set_simple_material_texture(material, file_name)
				self._reapply_material(material_id)
			self._start_texture_browse(("simple", material_id), _on_result)

		imgui.pop_id()

	def draw_panel(self):
		self.nav_cube.draw_controls()
		self._draw_viewport_toggles()
		self._draw_reference_shapes_toggles()
		self.export_dialog.draw()
		self.import_dialog.draw()
		self._draw_replace_match_popup()

		if self.shape_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self.shape_error)

		if self.shape_file is None:
			imgui.text("Select a .shape file in the explorer.")
			return

		imgui.text(f"Type: {self.shape_file.type_name}")
		imgui.separator()

		self._draw_wind_controls()

		if imgui.begin_tab_bar("##panel-tabs"):
			if imgui.begin_tab_item_simple("Textures"):
				self._draw_textures_tab()
				imgui.end_tab_item()
			if imgui.begin_tab_item_simple("Materials"):
				self._draw_materials_tab()
				imgui.end_tab_item()
			if imgui.begin_tab_item_simple("All Properties"):
				draw_properties(self.shape_file.value)
				imgui.end_tab_item()
			imgui.end_tab_bar()

		self._draw_save_buttons()


if __name__ == "__main__":
	ObjectEditorApp().run()
