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
import time
from pathlib import Path

import numpy

from panda3d.core import (
	AlphaTestAttrib, ClockObject, ColorBlendAttrib, DepthTestAttrib, GeomNode, Material as PandaMaterial, NodePath,
	PNMImage, Quat, RenderAttrib, Shader, Texture as PandaTexture, TextureStage, TransformState,
)

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx, portable_file_dialogs as pfd

from ryzom_forgery.app import _AVAILABLE_FONTS, _dpi_scale, ForgeryApp
from ryzom_forgery.camera import ObjectManipulator, OrbitCamera
from ryzom_forgery import creature_ref
from ryzom_forgery import dds_export
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
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.search_paths_dialog import SearchPathsDialog
from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.shape_geometry import (
	compose_uv_matrix, decompose_uv_matrix, iter_render_passes, load_panda_cube_texture, load_panda_texture,
	resolve_texture_ref, rgba_to_color, shape_bbox, shape_geom, uv_matrix_to_panda_mat4,
)
from ryzom_forgery.tex_dds_sync import TexDdsSyncWatcher
from ryzom_forgery import virtual_categories
from ryzom_forgery.workspace_setup_dialog import WorkspaceSetupDialog, _truncate_path_to_width
from ryzom_forgery.workspace_sync import WorkspaceSyncWatcher, SYNCED_SUBDIRS
from ryzom_forgery.workspace_watch import WorkspaceWatcher
from ryzom_forgery.workspaces import ensure_structure

from pynel.ryzom_animation import (
	AnimationParseError, animation_duration, evaluate_all_bone_world_matrices, parse_animation,
)
from pynel.ryzom_shape import (
	MeshMRMSkinned, Rgba, ShapeFile, ShapeParseError, SkeletonShape, Texture, WindTreeParams, parse_shape,
)
from pynel import repository_paths

from ryzom_forgery.apps.object_editor_mixins.geometry_helpers import (
	_AXIS_LENGTH, _AXIS_MARGIN_FACTOR, _build_axes_geom, _build_geom, _build_vertex_data, _is_shape_skinned,
)
from ryzom_forgery.apps.object_editor_mixins.mesh_import import MeshImportMixin
from ryzom_forgery.apps.object_editor_mixins.reference_shapes import ReferenceShapesMixin
from ryzom_forgery.apps.object_editor_mixins.settings_dialogs import SettingsDialogsMixin
from ryzom_forgery.apps.object_editor_mixins.shape_io import ShapeIOMixin
from ryzom_forgery.apps.object_editor_mixins.skin_state_helpers import (
	_build_mrm_skin_state, _build_skin_state, _MrmSkinState, _reskin_mrm_state, _reskin_state, _SkinState,
)
from ryzom_forgery.apps.object_editor_mixins.texture_widgets import TextureWidgetsMixin
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_colored_button, _CONFIRM_NO_COLOR, _CONFIRM_YES_COLOR, _icon_button, _IDRV_MAT_SHADER_SPECULAR,
	_OBJECT_TRANSPARENCY_ALPHA, _STATUS_HINT_COLOR, _TEXTURE_IN_WORKSPACE_COLOR, _TEXTURE_NORMAL_COLOR,
	_VIEWPORT_TOGGLE_MARGIN_PX,
)
from ryzom_forgery.apps.object_editor_mixins.viewport_transform import ViewportTransformMixin

# See _scan_active_workspace_virtual_categories().
_VIRTUAL_CATEGORIES_RESCAN_INTERVAL = 1.0
_BAKE_PROGRESS_POPUP_ID = "Baking Panoply variants"

# draw_panel()'s tab bar (_push_tab_color()).
_TAB_COLOR_TEXTURES = (0.565, 0.933, 0.565, 1.0)  # lightgreen
_TAB_COLOR_MATERIALS = (0.878, 1.0, 1.0, 1.0)  # lightcyan
_TAB_COLOR_ALL_PROPERTIES = (0.5, 0.5, 0.5, 1.0)  # gray
_TAB_COLOR_SETTINGS = (0.8, 0.75, 0.15, 1.0)  # yellow

# CMaterial flag/enum values (nel/include/nel/3d/material.h), needed to render
# translucent materials (e.g. glass) correctly instead of opaque.
_IDRV_MAT_ZWRITE = 0x00000004
_IDRV_MAT_BLEND = 0x00000080
_IDRV_MAT_DOUBLE_SIDED = 0x00000100
_IDRV_MAT_ALPHA_TEST = 0x00000200
_IDRV_MAT_TEX_ADDR = 0x00000400
# CMaterial::TShader (material.h) -- Material.shader_type's value, not a flag
# bit. _IDRV_MAT_SHADER_SPECULAR itself now lives in
# object_editor_mixins.ui_helpers (shared with texture_widgets.py).
_IDRV_MAT_SHADER_NORMAL = 0
_IDRV_MAT_SHADER_LIGHTMAP = 3

# CMaterial::TTexSource (material.h) -- TexEnv.src_arg*_alpha's values.
_TEX_SOURCE_TEXTURE = 0
_TEX_SOURCE_PREVIOUS = 1
# (Diffuse=2, Constant=3 also exist but aren't distinguished below -- both
# mean "not texture-derived" for _material_alpha_from_texture()'s purposes.)
# CMaterial::TTexOperator (material.h) -- TexEnv.op_alpha's values.
_TEX_OP_REPLACE = 0


def _stage_alpha_sources(tex_env):
	"""TTexSource values that actually feed this one texture stage's alpha
	result, per its own op_alpha (CMaterial::TTexOperator, material.h) --
	Replace only reads arg0, every other op (Modulate/Add/...) combines
	arg0 and arg1. Doesn't model arg2/interpolation ops precisely (none of
	the real character materials found so far use them for alpha) -- good
	enough to tell "still texture-derived" from "overridden to a constant"."""
	if tex_env.op_alpha == _TEX_OP_REPLACE:
		return (tex_env.src_arg0_alpha,)
	return (tex_env.src_arg0_alpha, tex_env.src_arg1_alpha)


def _material_alpha_from_texture(material):
	"""True if this material's final composited alpha (after ALL its
	texture stages, not just stage 0) still traces back to a texture's own
	alpha -- False if some later stage's texenv overrides it away to a
	constant (Diffuse or Constant, CMaterial::TTexSource).

	Needed because a common Ryzom multi-texture pattern -- e.g. a 2nd decal/
	makeup texture stage modulated onto stage 0's RGB, but with
	op_alpha=Replace sourced from Diffuse -- deliberately discards the base
	texture's own alpha for the final draw (makes the whole pass opaque,
	regardless of either texture's content). Confirmed real, 2026-08-30:
	fy_hof_visage.shape's makeup-layer material has exactly this shape, and
	alpha-testing stage 0's (grayscale-as-alpha) texture alone -- as if it
	were the final result -- wrongly discarded the whole pass depending on
	which part of the face texture that stage's own UVs happened to sample
	(worked fine for fy_hom_visage.shape's equivalent material only by
	UV-content coincidence, not because the logic was actually correct).

	Only tracks alpha, not RGB compositing -- RGB blending across stages is
	a separate visual-fidelity concern (a missing decal layer looks
	incomplete, not literally invisible), out of scope for this fix."""
	tex_envs = material.tex_envs or []
	textures = material.textures or []
	depends_on_texture = True
	for texture, tex_env in zip(textures, tex_envs):
		if texture is None or tex_env is None:
			continue
		sources = _stage_alpha_sources(tex_env)
		if _TEX_SOURCE_TEXTURE in sources:
			depends_on_texture = True
		elif _TEX_SOURCE_PREVIOUS in sources:
			pass  # inherits whatever depends_on_texture already was
		else:
			depends_on_texture = False  # Diffuse or Constant -- overridden away from any texture
	return depends_on_texture

# Render-mode dropdown options (_draw_material_render_mode_section): only
# modes with confirmed real usage in ryzom-data are listed at all (a
# shape-wide scan found zero shapes using UserColor/PerPixelLighting/Water),
# and only the ones Patina actually renders differently are selectable --
# LightMap is real (44 shapes) but not implemented yet, so it's listed
# disabled rather than dropped, unlike the zero-usage modes.
_SHADER_MODE_OPTIONS = [
	(_IDRV_MAT_SHADER_NORMAL, "Normal", True),
	(_IDRV_MAT_SHADER_SPECULAR, "Specular", True),
	(_IDRV_MAT_SHADER_LIGHTMAP, "LightMap", False),
]
# Per-stage "this stage's tex_user_mat is enabled" bit -- material.cpp's
# CMaterial::enableUserTexMat()/getFlags(): stage 0 is bit 0x00100000.
_IDRV_MAT_USER_TEX_MAT_STAGE0 = 0x00100000

# Specular overlay pass (_update_specular_overlay()): a tiny, self-contained
# GLSL shader for the reflection-vector cubemap sample only -- fixed-function
# TexGenAttrib (GL_TEXTURE_GEN/reflection map) turned out not to work at all
# under Panda3D's modern (core-profile) OpenGL path (confirmed empirically:
# a flat-color test on the same instanced node rendered fine, but the
# identical setup with TexGenAttrib.M_world_cube_map bound never showed
# anything). The overlay was already unlit/unmaterialed by design (the real
# engine's own additive specular pass just samples raw texels, no
# relighting), so this shader doesn't need to reimplement any of Panda3D's
# own ambient/diffuse/light pipeline -- it only computes the world-space
# reflection vector and does `specular_map(reflectDir).rgb * diffuse_map
# (texcoord).a`. `is_cube_map` picks between reflection-vector sampling
# (CTextureCube) and plain UV0 sampling (a flat file specular map) at
# runtime -- cheap enough for this preview tool not to bother with two
# separate shader variants.
_SPECULAR_OVERLAY_VERTEX_SHADER = """
#version 150
uniform mat4 p3d_ModelViewProjectionMatrix;
uniform mat4 p3d_ModelMatrix;
uniform mat4 p3d_ViewMatrixInverse;
in vec4 p3d_Vertex;
in vec3 p3d_Normal;
in vec2 p3d_MultiTexCoord0;
out vec2 texcoord;
out vec3 world_normal;
out vec3 view_dir;

void main() {
	gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
	texcoord = p3d_MultiTexCoord0;
	vec3 world_pos = (p3d_ModelMatrix * p3d_Vertex).xyz;
	world_normal = normalize(mat3(p3d_ModelMatrix) * p3d_Normal);
	vec3 camera_pos = p3d_ViewMatrixInverse[3].xyz;
	view_dir = normalize(world_pos - camera_pos);
}
"""
_SPECULAR_OVERLAY_FRAGMENT_SHADER = """
#version 150
uniform sampler2D diffuse_map;
uniform samplerCube specular_cube_map;
uniform sampler2D specular_flat_map;
uniform int is_cube_map;
in vec2 texcoord;
in vec3 world_normal;
in vec3 view_dir;
out vec4 p3d_FragColor;

void main() {
	float diffuse_alpha = texture(diffuse_map, texcoord).a;
	vec3 specular_color;
	if (is_cube_map != 0) {
		vec3 reflect_dir = reflect(view_dir, normalize(world_normal));
		specular_color = texture(specular_cube_map, reflect_dir).rgb;
	} else {
		specular_color = texture(specular_flat_map, texcoord).rgb;
	}
	p3d_FragColor = vec4(specular_color * diffuse_alpha, 1.0);
}
"""

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

_COMPATIBLE_COLOR = (0.35, 0.85, 0.35, 1.0)  # green -- "this .skel matches the loaded shape's bones"


def _push_tab_color(color):
	"""Tints one panel tab (Textures/Materials/All Properties/Settings --
	see draw_panel()) so each is visually distinct at a glance instead of
	every tab looking alike. Same lighter/darker-variant idea as
	object_editor_mixins.ui_helpers._colored_button(), just for Col_.tab* instead of Col_.button*: the
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


# A visually distinct palette (bright orange/lime/violet) for the Bind
# preview's attach-point marker (_apply_loaded_shape_to_creature()) -- the
# actual target the loaded shape's own pivot needs to be moved onto for a
# correct in-game grip point, see that marker's own docstring (2026-08-31,
# Nuno: "je dois OBLIGATOIREMENT visualiser ce point de préhension [...] je
# veux savoir précisement ce que je dois changer... le pivot de mon arme
# n'est pas sur le point d'attache"). Built via geometry_helpers._build_axes_geom(),
# same as the world/pivot/reference axes (see that module for their own
# colors/constants).
_ATTACH_POINT_AXIS_COLORS = {"x": (1.0, 0.55, 0.05, 1.0), "y": (0.55, 0.95, 0.15, 1.0), "z": (0.65, 0.3, 0.95, 1.0)}


def _nel_matrix_to_panda_mat4(m):
	"""Converts one of pynel.ryzom_animation.evaluate_all_bone_world_matrices()'s
	raw 4x4 matrices (a tuple of rows, m[row][col], NeL's row-major/column-3-
	translation convention -- v' = M * v as a column vector, same convention
	_mat_translate()/_mat_mul() in ryzom_animation.py use) into a Panda3D
	Mat4, whose own convention is the transpose of that (row-vector on the
	left, v' = v * M, translation in the LAST ROW -- see
	shape_geometry.py-adjacent uv_matrix_to_panda_mat4()'s own Mat4(...)
	calls for another example of this same row/last-row-translation shape).
	NOT independently verified against a real bind visually yet (no GUI in
	this dev environment) -- if a bound shape ends up offset/misoriented,
	this transpose direction is the first thing to double check."""
	from panda3d.core import Mat4
	return Mat4(
		m[0][0], m[1][0], m[2][0], 0.0,
		m[0][1], m[1][1], m[2][1], 0.0,
		m[0][2], m[1][2], m[2][2], 0.0,
		m[0][3], m[1][3], m[2][3], 1.0,
	)


APP_INFO = {
	"id": "object_editor",
	"name": "Patina",
	"subtitle": "Object Editor",
	"description": "Browse, inspect and edit .shape files.",
}


class ObjectEditorApp(
		MeshImportMixin, ReferenceShapesMixin, SettingsDialogsMixin, ShapeIOMixin, TextureWidgetsMixin,
		ViewportTransformMixin, ForgeryApp):
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
		self._cube_texture_cache = {}
		# Dummy 1x1 textures fed to whichever sampler the specular overlay
		# shader's active branch (is_cube_map) doesn't use -- GLSL still
		# requires every declared sampler to be bound to a texture of the
		# right type, even one it never actually samples this draw.
		self._dummy_2d_texture = PandaTexture("dummy-2d")
		self._dummy_2d_texture.setup_2d_texture(1, 1, PandaTexture.T_unsigned_byte, PandaTexture.F_rgba)
		self._dummy_2d_texture.set_ram_image(bytes(4))
		self._dummy_cube_texture = PandaTexture("dummy-cube")
		self._dummy_cube_texture.setup_cube_map(1, PandaTexture.T_unsigned_byte, PandaTexture.F_rgba)
		self._dummy_cube_texture.set_ram_image(bytes(4 * 6))
		self._specular_overlay_shader = Shader.make(
			Shader.SL_GLSL, vertex=_SPECULAR_OVERLAY_VERTEX_SHADER, fragment=_SPECULAR_OVERLAY_FRAGMENT_SHADER)
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
		# Bind preview's attach-point target marker -- see
		# _apply_loaded_shape_to_creature(), the only place that (re)builds
		# it (not _rebuild_viewport_helpers(), which never runs for this:
		# it's driven by the CREATURE's bone, not the loaded shape's own
		# bbox/geometry).
		self._attach_point_axes_np = self.render.attach_new_node("attach-point-axes-placeholder")
		self._rebuild_viewport_helpers(None)

		self.shape_file = None
		self.shape_error = None
		# Creature/NPC assembler (see _rebuild_assembled_creature()) -- the
		# full skeleton + every resolved body-part shape of the creature
		# currently picked in the Bind preview (_draw_bind_controls()),
		# shown as a standing reference alongside whatever's loaded, same
		# spirit as _reference_root's scale-reference shapes but built from
		# creature_ref data instead of a fixed bundled example.
		self._assembled_creature_root = self.render.attach_new_node("assembled-creature-root")
		self._show_assembled_creature = False
		self._assembled_creature_bone_matrices = {}  # see _frame_camera()
		# slot_name -> NodePath for the creature's own disk-parsed body-part
		# shapes -- built once per creature SELECTION by
		# _rebuild_assembled_creature() (expensive: disk read + shape parse +
		# skin per slot). The loaded shape's own place within the group (one
		# of: replacing one of these slots, a rigid attach-point weapon, or
		# an "undefined" catch-all -- see _apply_loaded_shape_to_creature())
		# is layered on top separately and cheaply (no disk I/O) whenever
		# just the loaded shape/its binding choice changes.
		self._assembled_creature_base_nodes = {}
		# slot_name -> _SkinState for every MeshMRMSkinned body-part shape
		# above (built alongside _assembled_creature_base_nodes, see
		# _build_assembled_shape_geometry()'s own skin_state return value) --
		# read every frame by _update_assembled_creature_skin() while the
		# Bind preview's Play button is active. A plain skinned CMeshMRM
		# body part (the *_visage.shape face pieces, confirmed real
		# 2026-08-31) has no entry here -- stays at whatever pose was baked
		# in at build time, same limitation _rebuild_geometry() already
		# accepts for the main shape's own live re-skin.
		self._assembled_creature_skin_states = {}
		self._assembled_creature_loaded_shape_node = None  # see _apply_loaded_shape_to_creature()
		# The loaded-shape node's own inner "-content" child (see
		# _build_assembled_shape_geometry()) -- kept up to date with
		# self._object_pivot's LIVE rotation every frame (see
		# _update_bound_shape_rotation()), not just a one-time snapshot at
		# build time, so a Ctrl+drag rotation edit on the main shape is
		# reflected on its bound copy immediately too (bug found 2026-08-30,
		# Nuno: "ça marche quand je le load, mais apres je modifie les
		# valeurs.. ça ne change rien du tout").
		self._assembled_creature_loaded_shape_content_root = None
		# True only for the skinned slot-override case (see
		# _apply_loaded_shape_to_creature()) -- there, the skin binding to
		# the creature's own skeleton already fully determines world
		# position, so _update_bound_shape_rotation() must track rotation
		# ONLY for it (matching CMeshBase::DefaultRotQuat's own real-engine
		# semantics, layered on top of skinning); the rigid attach-point/
		# undefined cases track the FULL position+rotation+scale instead
		# (bug found 2026-08-31, Nuno: "t'as fais du caca sur les shape
		# skinnés.. ils sont pas du tout au bon endroit" -- the full-matrix
		# sync added for weapons was wrongly applied to skinned armor too,
		# double-offsetting already-skin-placed vertices by whatever
		# position/scale the main shape's own pivot happened to be at).
		self._assembled_creature_loaded_shape_is_skinned_override = False
		# The loaded shape's own _SkinState when it's a skinned slot override
		# (see above) -- read every frame by _update_assembled_creature_skin()
		# alongside _assembled_creature_skin_states, None for the attach-point/
		# undefined cases (skeleton=None there, see _build_assembled_shape_geometry()).
		self._assembled_creature_loaded_shape_skin_state = None
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
		self._pending_reopen_shape_item = None  # ExplorerItem awaiting a Yes/No decision, or None -- see _restore_session_state()/_draw_reopen_shape_popup()
		self._reopen_shape_prompt_open = False
		self._pending_load_shape_item = None  # ExplorerItem queued to load once the unsaved-changes prompt is answered, or None -- see _request_load_shape()/_draw_load_shape_unsaved_popup()
		self._load_shape_unsaved_prompt_open = False
		self._panoply_cfg_changed = False  # set from WorkspaceWatcher's background thread, see _on_panoply_cfg_settled()/_update_texture_freshness()
		self._virtual_categories_cache = None  # {category: [Path, ...]} or None -- see _scan_active_workspace_virtual_categories()
		self._virtual_categories_cache_time = 0.0
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
		self.workspace_setup_dialog.on_dpi_preview_changed = self.set_live_ui_scale_preview
		self.workspace_setup_dialog.on_setup_finished = self.relaunch
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
		# Bind preview (see _draw_bind_controls()): lets a non-skinned shape
		# (e.g. a weapon) be bound to a chosen attach point (bone) of one of
		# Patina's curated reference creatures, or (for a skinned shape) let
		# it stand in for one slot of the assembled creature (e.g. preview
		# fy_HOM_armor01_bottes.shape in place of the creature's own default
		# fy_HOM_civil01_bottes.shape for "feet") -- see
		# /repos/project-todos/ryzom-core/forgery-object-editor.md "Creature/
		# NPC binder + assembler in Patina". Own dedicated skeleton field,
		# deliberately NOT _bone_preview_skeleton (2026-08-30, Nuno: picking
		# a Bind-preview creature silently swapped that shared field, which
		# _update_skin_preview()'s per-frame task also reads for the *main*
		# shape's live re-skin -- so selecting a creature made that task
		# recompute every frame against a wrong/mismatched skeleton
		# indefinitely, with a real CPU cost independent of whether the
		# assembled creature itself was even visible or in view. See
		# _apply_bone_preview_skeleton()'s own docstring for the feature
		# that field actually belongs to).
		self._bind_creatures = {}  # name -> creature_ref.CreatureRecord, see _ensure_bind_creatures()
		self._bind_cache_rebuild = None  # dict or None -- see _start_bind_cache_rebuild()/_ensure_bind_creatures()
		self._bind_creature_name = ""
		self._bind_skeleton = None  # the picked creature's own skeleton, or None
		self._bind_skeleton_name = ""  # its scanned (on-disk-cased) name, see _resolve_scanned_skeleton_name()
		self._bind_attach_point = ""  # bone name picked in the attach-point combo (non-skinned shapes); "" = unbound
		self._bind_slot_override = ""  # creature_ref.BODY_SLOTS entry or "hair" (skinned shapes); "" = no override
		self._bind_mode = ""  # creature_ref.ANIM_MODES entry; "" = raw bind pose (no animation posing)
		# Live playback of the animation _bind_mode resolves to (Play button,
		# _draw_bind_controls(), 2026-08-31: "Possible d'avoir un bouton pour
		# lancer l'animation?") -- _bind_animation is the parsed Animation
		# object (None whenever _bind_mode is "" or resolution/parsing
		# failed, see _rebuild_assembled_creature()), read every frame by
		# _update_assembled_creature_skin() alongside _bind_anim_time, which
		# _update_bind_anim_time() advances/loops while _bind_anim_playing.
		# Same playing-by-default choice _bone_preview_playing already makes.
		self._bind_animation = None
		self._bind_anim_duration = 0.0
		self._bind_anim_time = 0.0
		self._bind_anim_playing = True
		# self._bind_slot_overrides itself (shape-stem-lowercased ->
		# manually-picked slot, persisted to the active workspace's own
		# build/bind_slot_overrides.json) is set by _on_active_workspace_changed()
		# above, called earlier in __init__ -- not re-initialized here, that
		# would clobber what it just loaded for a workspace already active
		# from a previous session. See its own assignment there for the
		# full docstring.
		self._bind_panel_size = (10.0, 10.0)
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
		self.taskMgr.add(self._update_bound_shape_rotation, "object-editor-bound-shape-rotation")
		self.taskMgr.add(self._update_bind_anim_time, "object-editor-bind-anim-time")
		self.taskMgr.add(self._update_assembled_creature_skin, "object-editor-assembled-creature-skin")
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
		if workspace_dir is not None and not self.workspace_setup_dialog.is_active_workspace_external():
			# Backfills any SUBDIRS missing on disk (a folder introduced in a
			# later Forgery version, or removed by hand) -- idempotent.
			# WorkspaceSetupDialog.set_active_workspace() already does this
			# too, but only when a workspace is picked/created interactively;
			# this covers every other path into this method, in particular
			# __init__ resuming a workspace already active from a previous
			# session (read straight off active_workspace_dir, bypassing
			# set_active_workspace() entirely). Never for an *external*
			# workspace though (see workspaces.py's own module docstring) --
			# a folder registered via <Import Folder> is never restructured,
			# whatever's already there is taken as-is.
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
		# Explorer.virtual_categories_source (see explorer.py): the active
		# workspace's own content, grouped by virtual category
		# (shapes/textures/3d files/masks/anims/skels/others -- see
		# virtual_categories.py) rather than real subfolders, so it stays
		# browsable/loadable-from no matter where the Explorer is currently
		# navigated to. Forcing the cache stale here (rather than leaving
		# the previous workspace's scan showing for up to
		# _VIRTUAL_CATEGORIES_RESCAN_INTERVAL seconds) matters more here
		# than it would for an in-place file change -- switching workspaces
		# is an unmistakable jump, a stale listing from the *previous* one
		# would be actively misleading, not just momentarily outdated.
		self._virtual_categories_cache = None
		if workspace_dir is None:
			self.explorer.virtual_categories_source = None
		else:
			self.explorer.virtual_categories_source = self._scan_active_workspace_virtual_categories
		# Forces a fresh _ensure_bind_creatures() lookup: a different (or no
		# longer) active workspace may have its own creatures_ref.txt
		# override, resolving to a different cache than before.
		self._bind_creatures = {}
		# A rebuild in flight for the PREVIOUS workspace would otherwise
		# block _ensure_bind_creatures() from starting one for this new
		# workspace (it only starts one when self._bind_cache_rebuild is
		# None) -- the old thread still finishes and writes its own
		# workspace's cache file harmlessly (its target path was captured by
		# value), just no longer tracked here.
		self._bind_cache_rebuild = None
		# Loaded once here (startup, and again on every later workspace
		# switch) rather than re-read from disk on every shape load --
		# _auto_detect_bind_slot() only ever reads this in-memory copy;
		# _write_shape() is the only thing that updates the on-disk file
		# (and this copy in lockstep, via _save_bind_slot_override()).
		self._bind_slot_overrides = (
			creature_ref.load_slot_overrides(creature_ref.workspace_slot_overrides_path(workspace_dir))
			if workspace_dir is not None else {})

	def _scan_active_workspace_virtual_categories(self):
		"""Explorer.virtual_categories_source callback (see
		_on_active_workspace_changed()) -- a full recursive scan
		(virtual_categories.scan_workspace()) is real disk I/O, and
		draw() runs every ImGui frame, so this is throttled to at most
		once every _VIRTUAL_CATEGORIES_RESCAN_INTERVAL seconds rather than
		tied to a specific filesystem-watch event (workspace_watch.py's
		own per-subdir registration doesn't cover arbitrary/nonstandard
		nesting, which is exactly the case a virtual category has to
		handle) -- good enough for a single workspace's typically modest
		file count, and self-corrects within about a second of any change
		made through or outside Forgery."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return {}
		now = time.monotonic()
		stale = (self._virtual_categories_cache is None
		         or now - self._virtual_categories_cache_time > _VIRTUAL_CATEGORIES_RESCAN_INTERVAL)
		if stale:
			exclusion_rules = app_settings.load().exclusion_rules
			self._virtual_categories_cache = virtual_categories.scan_workspace(workspace_dir, exclusion_rules)
			self._virtual_categories_cache_time = now
		return self._virtual_categories_cache

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
		(see _draw_bone_preview_controls()) -- this is the *main* loaded
		shape's own skeleton, read every frame by _update_skin_preview()'s
		live re-skin task. NOT used by the Bind preview's creature picker
		(_select_bind_creature() has its own separate _bind_skeleton field
		instead) -- the two used to share this one, which made picking a
		Bind-preview creature silently redirect the main shape's per-frame
		re-skin to the creature's skeleton too (2026-08-30)."""
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
		otherwise, same pattern as _update_wind()/_update_skin_preview_time().
		The actual blend math lives in the module-level _reskin_state(), shared
		with _update_assembled_creature_skin()'s own per-body-part re-skin."""
		state = self._skin_state
		if state is None or state.vdata is None:
			return task.cont
		bone_world_matrices = self._bone_world_matrices_for(state.bone_names)
		_reskin_state(state, bone_world_matrices)
		return task.cont

	def _update_bound_shape_rotation(self, task):
		"""Per-frame: keeps the Bind preview's loaded-shape-on-creature
		copy's own local offset (self._assembled_creature_loaded_shape_content_root,
		see _build_assembled_shape_geometry()) matching the main shape's
		CURRENT position/rotation/scale edits -- a cheap matrix copy, not a
		geometry rebuild, so a live Ctrl+drag or Properties-panel edit on
		the main shape shows up on its bound copy immediately too (bug
		found 2026-08-30, Nuno: rotation alone wasn't enough, "pos et scale
		marchent pas" either).

		model_root.get_mat(self.render) is exactly this offset: _object_pivot
		is parented directly to self.render at true (0,0,0)/scale-1 (only
		ever rotated, seeded from base.default_rot_quat on load, see
		_display_shape()), and model_root itself carries any pivot-locked
		edits on top (_transform_node()) -- so their COMPOSED matrix
		relative to render *is* "whatever the shape's own intrinsic
		placement + every edit since load add up to", with no absolute
		world position baked in to fight with wherever the bound copy is
		already correctly anchored (skinned onto the creature, or set to an
		attach-point bone -- see _apply_loaded_shape_to_creature()).
		content_root.set_mat(matrix) (single-argument form, no coordinate-
		space conversion) applies that composed matrix as content_root's
		own LOCAL transform within its already-correctly-anchored parent,
		exactly like _build_assembled_shape_geometry()'s own initial
		set_quat(base.default_rot_quat) did for rotation alone -- this
		fully supersedes that one-time snapshot every frame from here on.

		EXCEPTION: the skinned slot-override case
		(self._assembled_creature_loaded_shape_is_skinned_override) is left
		alone entirely -- position, rotation, AND scale. Verified 2026-08-31
		(transform.cpp:946 CTransform::updateWorldMatrixFromFather()): a
		skinned instance's own Default{Pos,RotQuat,Scale} has ZERO effect in
		the real game, not just position/scale -- the skin binding to the
		creature's own skeleton alone fully determines every vertex's final
		position (see _build_assembled_shape_geometry()'s own matching
		is_skinned check). Layering ANY of the main shape's own edits on top
		here would just double-offset/rotate already-correctly-placed
		vertices away from a result the real client could ever produce (bug
		found 2026-08-31, Nuno: "t'as fais du caca sur les shape skinnés..
		ils sont pas du tout au bon endroit", then "t'es sur que la rotation
		dois s'appliquer aussi?" -- no, confirmed).

		No-op whenever nothing is currently bound."""
		content_root = self._assembled_creature_loaded_shape_content_root
		if (content_root is not None and not content_root.is_empty()
				and not self._assembled_creature_loaded_shape_is_skinned_override):
			content_root.set_mat(self.model_root.get_mat(self.render))
		return task.cont

	def _update_bind_anim_time(self, task):
		"""Per-frame: advances self._bind_anim_time while playing -- the clock
		driving the Bind preview's live playback (Play button,
		_draw_bind_controls()). _update_assembled_creature_skin() re-skins
		every body-part shape against this time every frame. Same loop-by-
		duration pattern as _update_skin_preview_time()'s own clock."""
		if self._bind_anim_playing and self._bind_anim_duration > 0:
			dt = ClockObject.get_global_clock().get_dt()
			self._bind_anim_time = (self._bind_anim_time + dt) % self._bind_anim_duration
		return task.cont

	def _update_assembled_creature_skin(self, task):
		"""Per-frame: re-skins every live-capable body-part shape of the
		assembled Bind-preview creature (_assembled_creature_skin_states,
		built by _build_assembled_shape_geometry() for each skinned slot --
		either a _SkinState, for CMeshMRMSkinned's packed-vertex format, or a
		_MrmSkinState, for a plain skinned CMeshMRM's own geom.skin_weights
		layout, e.g. *_visage.shape face pieces), plus the loaded shape's own
		skinned-override placement if any
		(_assembled_creature_loaded_shape_skin_state), plus the loaded
		shape's own rigid attach-point placement if any (weapons don't have a
		_SkinState at all -- skeleton=None is passed for that case in
		_build_assembled_shape_geometry() -- but still need their WORLD
		POSITION refreshed every frame, otherwise a weapon stays frozen at
		whatever bone matrix build time happened to compute, bug found
		2026-08-31, Nuno: "l'arme qui ne bouge pas"). No-op whenever no
		animation is currently resolved (self._bind_animation is None -- no
		Mode picked, or resolution/parsing failed, see
		_rebuild_assembled_creature()) or no creature skeleton is loaded.

		Bone world matrices are computed ONCE per frame for the whole
		skeleton and reused by every re-skin call below -- same
		"whole-skeleton batch, not per-bone" reasoning
		_bone_world_matrices_for() already documents, now paying off across
		several shapes at once instead of just the main shape's own."""
		if self._bind_animation is None or self._bind_skeleton is None:
			return task.cont
		bone_world_matrices = evaluate_all_bone_world_matrices(
			self._bind_skeleton, self._bind_animation, self._bind_anim_time)
		self._assembled_creature_bone_matrices = bone_world_matrices
		for state in self._assembled_creature_skin_states.values():
			if isinstance(state, _MrmSkinState):
				_reskin_mrm_state(state, bone_world_matrices)
			else:
				_reskin_state(state, bone_world_matrices)
		loaded_state = self._assembled_creature_loaded_shape_skin_state
		if loaded_state is not None:
			if isinstance(loaded_state, _MrmSkinState):
				_reskin_mrm_state(loaded_state, bone_world_matrices)
			else:
				_reskin_state(loaded_state, bone_world_matrices)
		if (not self._bind_slot_override and self._bind_attach_point
				and self._bind_attach_point in self._bind_skeleton.bone_map
				and self._assembled_creature_loaded_shape_node is not None):
			bone_matrix = bone_world_matrices[self._bind_attach_point]
			panda_mat = _nel_matrix_to_panda_mat4(bone_matrix)
			self._assembled_creature_loaded_shape_node.set_mat(panda_mat)
			if not self._attach_point_axes_np.is_empty():
				self._attach_point_axes_np.set_mat(panda_mat)
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
		driving the loaded shape's own live re-skin (self._bone_preview_skeleton,
		read every frame by _update_skin_preview()) -- only shown for a
		CMeshMRMSkinned, the one shape type that actually needs live
		re-skinning; a rigid shape has nothing for this panel to drive.
		Deliberately separate from _draw_bind_controls()'s own
		self._bind_skeleton (Bind preview's creature picker) -- they used to
		share one field, which made picking a Bind-preview creature silently
		redirect the main shape's per-frame re-skin too (2026-08-30, see
		_bind_skeleton's own docstring). Positioned the same way as
		_draw_wind_controls(), stacked right below it (both top-right of the
		viewport, flush against the panel)."""
		if self.shape_file is None or not _is_shape_skinned(self.shape_file.value):
			return

		shape_bones = set(shape_geom(self.shape_file.value).bones_name)

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
			compatible_names = set(self.search_paths_dialog.compatible_for(shape_bones))
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

	def _ensure_bind_creatures(self):
		"""Lazily loads the curated creature list + its distilled cache (see
		creature_ref.py) for the Bind preview's creature combo -- cheap once
		loaded: the bundled cache is a plain pre-generated JSON read, no
		.packed_sheets parsing involved for the 8 default reference
		creatures. Invalidated (see _on_active_workspace_changed()) whenever
		the active workspace changes, since a different workspace may have
		its own creatures_ref.txt override resolving to a different cache.

		A WORKSPACE override list's own cache never ships pre-generated
		(unlike the bundled list's) -- kicks off a background rebuild (see
		_start_bind_cache_rebuild()) when it's missing or older than the
		list file. 2026-08-30 gap found by Nuno: this was never wired up at
		all -- editing/copying creatures_ref.txt into a workspace produced a
		permanently empty combo (no cache ever existed for it to read), not
		a partial/stale one."""
		if self._bind_cache_rebuild is not None and self._bind_cache_rebuild["done"]:
			if self._bind_cache_rebuild["error"]:
				self._save_status = f"Creature cache rebuild failed: {self._bind_cache_rebuild['error']}"
				print(f"[object_editor] {self._save_status}")
			self._bind_cache_rebuild = None
			self._bind_creatures = {}  # force the reload below, picking up whatever the rebuild just wrote

		if self._bind_creatures:
			return self._bind_creatures
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		list_path = creature_ref.resolve_list_path(workspace_dir)
		cache_path = creature_ref.resolve_cache_path(workspace_dir)
		entries = creature_ref.load_creature_entries(list_path)
		cache = creature_ref.load_cache(cache_path)
		self._bind_creatures = {label: cache[label] for _sheet, label in entries if label in cache}

		is_workspace_list = workspace_dir is not None and list_path == creature_ref.workspace_list_path(workspace_dir)
		if is_workspace_list and self._bind_cache_rebuild is None:
			cache_mtime = cache_path.stat().st_mtime if cache_path.is_file() else None
			list_mtime = list_path.stat().st_mtime
			if cache_mtime is None or list_mtime > cache_mtime:
				self._start_bind_cache_rebuild(workspace_dir, entries)
		return self._bind_creatures

	def _resolve_data_bytes(self, name):
		"""Resolves `name` (a plain file name, e.g. "creature.packed_sheets"
		or "sheet_id.bin") against the same scanned search-path index
		find_texture() already uses for .shape/.skel/texture names -- despite
		the name, it indexes every file found (any extension, .bnp contents
		included) across the user's configured search folders, so it works
		unchanged for these too. None if not found by the last scan."""
		entry = self.search_paths_dialog.find_texture(name)
		return entry.read_bytes() if entry is not None else None

	def _start_bind_cache_rebuild(self, workspace_dir, entries):
		"""Starts a background thread rebuilding the workspace's own
		creatures_cache.json from real .packed_sheets data (see
		creature_ref.build_cache(), ~1-6s depending on how much of
		creature.packed_sheets's 28545 entries needs parsing -- far too slow
		for the main/UI thread). Call from the main thread only, same
		threading split as _start_panoply_bake()/_run_panoply_bake(): only
		_run_bind_cache_rebuild() (pure file I/O/pynel parsing) runs off it."""
		progress = {"done": False, "error": None}
		self._bind_cache_rebuild = progress
		thread = threading.Thread(target=self._run_bind_cache_rebuild, args=(workspace_dir, list(entries), progress), daemon=True)
		thread.start()

	def _run_bind_cache_rebuild(self, workspace_dir, entries, progress):
		"""Background-thread body for _start_bind_cache_rebuild() -- never
		touches imgui, only the `progress` dict passed in (plain dict field
		writes, same safe-under-the-GIL reasoning as _run_panoply_bake()) and
		creature_ref.build_cache()/save_cache()'s own file I/O. Writes to the
		`progress` local, NOT self._bind_cache_rebuild -- switching the
		active workspace mid-rebuild resets that attribute to a fresh dict
		(or None, see _on_active_workspace_changed()), so writing through it
		here could crash this thread (None isn't subscriptable) or mark an
		unrelated newer rebuild as done before its own thread finished
		(found 2026-08-30 via code review, matching this exact reachable
		case -- unlike panoply bake's own self._bake_progress, which has no
		equivalent reset-while-running path)."""
		try:
			from pynel import ryzom_packed_sheets as ps
			needed = ("creature.packed_sheets", "item.packed_sheets", "sitem.packed_sheets", "sheet_id.bin")
			raw = {name: self._resolve_data_bytes(name) for name in needed}
			missing = [name for name, data in raw.items() if data is None]
			if missing:
				raise FileNotFoundError(f"not found in scanned search paths: {', '.join(missing)}")
			sheet_id_names = ps.parse_sheet_id_bin(raw["sheet_id.bin"])
			records = creature_ref.build_cache(
				entries, io.BytesIO(raw["creature.packed_sheets"]), io.BytesIO(raw["item.packed_sheets"]),
				io.BytesIO(raw["sitem.packed_sheets"]), sheet_id_names)
			creature_ref.save_cache(creature_ref.workspace_cache_path(workspace_dir), records)
		except Exception as exc:
			progress["error"] = str(exc)
			print(f"[object_editor] creature cache rebuild failed: {exc}")
		finally:
			progress["done"] = True

	def _resolve_scanned_skeleton_name(self, wanted):
		"""Case-insensitive match of `wanted` against the scanned .skel
		index. creature.packed_sheets stores a skeleton filename in
		whatever case the source .creature sheet happened to use (e.g.
		"FY_HOF_skel.skel"), which doesn't necessarily match the real
		file's on-disk name (confirmed on a real install, 2026-08-30:
		"fy_hof_skel.skel", all lowercase) -- SearchPathsDialog itself
		indexes/looks up skeletons by exact name (see skeleton_for()), so
		this is needed before calling it. Returns the actually-scanned
		name, or None if nothing matches even case-insensitively."""
		wanted_lower = wanted.lower()
		for name in self.search_paths_dialog.scanned_skeleton_names():
			if name.lower() == wanted_lower:
				return name
		return None

	@staticmethod
	def _resolve_bone_name(skeleton, wanted):
		"""Case-insensitive match of `wanted` against `skeleton.bone_map` --
		BodyToBone/weapon attach-point names (creature_ref.bind_attach_points())
		aren't guaranteed to match the skeleton's own bone-name casing
		exactly, same class of mismatch as _resolve_scanned_skeleton_name().
		None if `skeleton` is None or nothing matches even case-insensitively."""
		if skeleton is None:
			return None
		wanted_lower = wanted.lower()
		for name in skeleton.bone_map:
			if name.lower() == wanted_lower:
				return name
		return None

	def _select_bind_creature(self, name):
		"""Picks `name` in the Bind preview's creature combo: loads its
		skeleton (via the same scanned-search-path index the Skinning
		preview uses, see SearchPathsDialog.skeleton_for(), through
		_resolve_scanned_skeleton_name()'s case-insensitive match), and
		shows the assembled creature right away (2026-08-30, Nuno: no
		reason to make picking a creature and toggling it visible two
		separate steps).
		Deliberately does NOT reset _bind_slot_override or
		_bind_attach_point -- comparing how the same loaded shape/weapon
		looks across several creatures is a real use case, so switching
		creature must keep both (2026-08-30, Nuno: "quand je change de pnj
		ça fais sauter la selection du slot" -- an earlier version of this
		method wrongly cleared _bind_slot_override here, meant to fix a
		DIFFERENT bug -- the loaded shape rendering twice, once standalone
		and once on the creature -- that's now fixed at its real source
		instead, see _apply_loaded_shape_to_creature()'s unconditional
		model_root hide. 2026-08-31, Nuno: same complaint for
		_bind_attach_point -- safe to keep too, since WEAPON_ATTACH_POINTS
		is a small fixed list shared by every humanoid skeleton used here,
		not arbitrary skeleton-specific bone names)."""
		self._bind_creature_name = name
		record = self._bind_creatures.get(name)
		if record is None:
			# "(none)": deselecting the creature entirely (name == "" from
			# the combo's own "(none)" entry, or an otherwise-unknown name)
			# -- nothing to show, so unlike a real pick this does NOT force
			# _show_assembled_creature back on.
			self._bind_skeleton = None
			self._bind_skeleton_name = ""
			self._show_assembled_creature = False
			self._rebuild_assembled_creature()
			return
		self._show_assembled_creature = True
		scanned_name = self._resolve_scanned_skeleton_name(record.skel)
		skeleton = self.search_paths_dialog.skeleton_for(scanned_name) if scanned_name is not None else None
		self._bind_skeleton = skeleton
		self._bind_skeleton_name = scanned_name or ""
		# Forces the main shape's own Panoply "skin" (race) selection to
		# match the picked creature's race (CreatureRecord.race, same 0-3
		# EGSPD::CPeople::TPeople indexing as panoply.RACES's own
		# ("fy","ma","tr","zo") order) -- without this, previewing e.g.
		# fy_HOM_armor01_bottes.shape bound to a Fyros creature could still
		# render with whatever race variant (say "zo") happened to be
		# selected from earlier, a mismatched skin tone on the wrong body
		# (bug found 2026-08-30, Nuno: "il faut que tu force le skin
		# panoply en fonction du pnj qu'on charge"). Harmless no-op if the
		# loaded shape's textures have no "skin" variants at all --
		# _reapply_all_materials() just won't find anything to change.
		if 0 <= record.race < len(panoply.RACES):
			self._panoply_selection["skin"] = panoply.RACES[record.race]
			self._reapply_all_materials()
		# Deliberately not routed through _apply_bone_preview_skeleton() --
		# that would also reassign the *main* shape's live-re-skin skeleton
		# (_bone_preview_skeleton) to this creature's, see _bind_skeleton's
		# own docstring for why that's wrong. No need to touch
		# _rebuild_geometry()/model_root here at all: whichever of the loaded
		# shape's three placements applies (see
		# _apply_loaded_shape_to_creature()) is entirely resolved by
		# _rebuild_assembled_creature() below, which calls that method itself.
		self._rebuild_assembled_creature()

	def _copy_creatures_ref_to_workspace(self):
		"""Copies the bundled default creatures_ref.txt into the active
		workspace, as an editable starting point -- same copy-then-edit
		mechanism as _copy_panoply_cfg_to_workspace(), see that method's own
		docstring. Picked from the gear icon in _draw_bind_controls()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		dest = creature_ref.workspace_list_path(workspace_dir)
		try:
			shutil.copyfile(creature_ref.bundled_list_path(), dest)
		except OSError as exc:
			self._save_status = f"Could not copy creatures_ref.txt: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Copied creatures_ref.txt to workspace ({dest})"
		print(f"[object_editor] {self._save_status}")
		# Re-resolve against the new override next _ensure_bind_creatures()
		# call -- note this doesn't yet (re)build a cache for names added to
		# the copy beyond the bundled 8 (see the chantier notes) -- those
		# just won't show up in the combo until that's wired up.
		self._bind_creatures = {}

	def _save_bind_slot_override(self):
		"""Persists the Bind preview's CURRENT slot-combo pick
		(self._bind_slot_override) for the currently loaded shape into the
		active workspace's own bind_slot_overrides.json (see
		creature_ref.workspace_slot_overrides_path()) and self._bind_slot_overrides
		(the in-memory copy _auto_detect_bind_slot() actually reads), so
		it's remembered the next time this shape loads -- higher priority
		there than the bundled shape_slot_index.json. Called ONLY from
		_write_shape(), when the shape itself is actually saved (2026-08-30,
		Nuno: "que ça ne sauve le slot QUE si on sauve le shape" -- picking
		a slot to preview/compare shouldn't silently persist a correction
		the user never actually confirmed by saving). An empty
		self._bind_slot_override removes any existing entry (picking
		"(none)" before saving is a deliberate correction too, e.g. for a
		shape the bundled index got wrong). No-op if there's no active
		workspace or no shape loaded -- nothing to key the entry by, or
		nowhere to write it."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None or not self._shape_source_name:
			return
		stem = Path(self._shape_source_name).stem.lower()
		if self._bind_slot_override:
			self._bind_slot_overrides[stem] = self._bind_slot_override
		else:
			self._bind_slot_overrides.pop(stem, None)
		creature_ref.save_slot_overrides(
			creature_ref.workspace_slot_overrides_path(workspace_dir), self._bind_slot_overrides)

	def _auto_detect_bind_slot(self):
		"""Auto-preselects the Bind preview's slot-override combo from, in
		priority order: (1) the active workspace's own manual corrections
		(self._bind_slot_overrides, loaded once by
		_on_active_workspace_changed() -- see _write_shape()'s own note on
		when this actually gets saved back to disk); (2) the loaded
		shape's own equipment-slot data (SLOTTYPE::TSlotType /
		CItemSheet.slot_bf, via creature_ref.load_shape_slot_index()'s
		bundled index) -- Nuno, 2026-08-30: "je charge
		fy_HOM_armor01_bottes.shape [...] qu'il sache déjà [...] quoi
		remplacer". Called from _display_shape() for every shape load.
		Silent no-op (leaves the combo on "(none)") when neither source has
		an answer, or the shape maps to a hand/weapon slot (out of
		BODY_SLOTS' scope) -- the manual combo/attach-point list remains
		available as a fallback in all cases."""
		self._bind_slot_override = ""
		if not self._shape_source_name:
			return
		stem = Path(self._shape_source_name).stem.lower()
		slot = self._bind_slot_overrides.get(stem)
		if not slot:
			slot = creature_ref.load_shape_slot_index().get(stem)
		if slot:
			self._bind_slot_override = slot
		# Cheap (no disk I/O): reuses whatever's already built for every
		# other slot, see _apply_loaded_shape_to_creature()'s own docstring
		# for why this, and not the expensive _rebuild_assembled_creature(),
		# is what belongs on the per-shape-load path.
		self._apply_loaded_shape_to_creature()

	def _rebuild_assembled_creature(self):
		"""(Re)builds the full-body reference under _assembled_creature_root
		from the currently-picked Bind-preview creature -- skeleton + every
		resolved body-part shape (creature_ref.BODY_SLOTS), plus its first
		hairstyle option if any (HairItemList lists interchangeable style
		*choices*, not several worn at once -- showing just one avoids
		stacking every alternative on top of each other). Static bind-pose
		only, no animation (this is a standing reference, not a preview of
		movement).

		EXPENSIVE (disk read + shape parse + skin per slot, ~seconds for the
		full body pre-vectorization, still visible cost after) -- only call
		this when the creature SELECTION itself changes (picking a creature
		in the combo, toggling the assembled-creature visibility on), never
		on every loaded shape. Where the loaded shape itself fits in (a slot
		override, an attach-point weapon, or neither) is layered on top
		afterwards by _apply_loaded_shape_to_creature(), which this method
		itself calls once at the end -- 2026-08-30, Nuno: routing every
		shape load through this full rebuild (to keep that in sync) made
		ordinary Explorer browsing laggy the moment a Bind-preview creature
		was shown, even though only the loaded shape's own placement
		actually needed to change."""
		self._assembled_creature_root.remove_node()
		self._assembled_creature_root = self.render.attach_new_node("assembled-creature-root")
		self._assembled_creature_base_nodes = {}
		self._assembled_creature_skin_states = {}
		self._assembled_creature_loaded_shape_node = None
		self._assembled_creature_loaded_shape_content_root = None
		# Carries the whole creature's pivot offset ONCE, so every child shape
		# below -- whether positioned at local-space identity (a skinned
		# body-part mesh, its bone_world_matrices already IS its final
		# position) or at a specific bone's matrix (a rigid, non-skinned one,
		# see the slot_name handling below) -- ends up in the same space as
		# the main loaded object, without each needing its own set_pos.
		self._assembled_creature_root.set_pos(self.render, self._object_pivot.get_pos(self.render))
		self._assembled_creature_bone_matrices = {}  # see _frame_camera()
		self._bind_animation = None
		self._bind_anim_duration = 0.0
		self._bind_anim_time = 0.0
		if not self._show_assembled_creature:
			self._apply_loaded_shape_to_creature()
			return
		record = self._bind_creatures.get(self._bind_creature_name)
		skeleton = self._bind_skeleton
		if record is None or skeleton is None:
			self._apply_loaded_shape_to_creature()
			return

		# Bind preview's Mode combo (_draw_bind_controls): poses the standing
		# reference against the real animation MBEHAV::EMode resolves to for
		# this creature (creature_ref.resolve_animation(), via the bundled
		# creatures_anim_cache.json -- see nel/tools/pynel/docs/
		# packed_sheets.md's "animset_list.packed_sheets" section for the
		# full chain), e.g. show it crouched in "SIT" or squared-up in
		# "COMBAT" instead of always the skeleton's raw bind pose. Resolved
		# once here into self._bind_animation -- _update_assembled_creature_skin()
		# reads it every frame (self._bind_anim_time, advanced by
		# _update_bind_anim_time() while the Play button is active) to
		# actually animate every live-capable body-part shape, not just pose
		# a static snapshot at t=0 (2026-08-31, Nuno: "Possible d'avoir un
		# bouton pour lancer l'animation?").
		if self._bind_mode:
			anim_name = creature_ref.resolve_animation(record, self._bind_mode, "Idle")
			if anim_name is not None:
				entry = self.search_paths_dialog.find_texture(anim_name)
				if entry is not None:
					try:
						self._bind_animation = parse_animation(entry.read_bytes())
						self._bind_anim_duration = animation_duration(self._bind_animation)
					except AnimationParseError as exc:
						print(f"[object_editor] Bind preview: cannot parse animation {anim_name!r}: {exc}")
				else:
					print(f"[object_editor] Bind preview: animation {anim_name!r} not found in scanned search paths")

		bone_world_matrices = evaluate_all_bone_world_matrices(skeleton, self._bind_animation, self._bind_anim_time)
		self._assembled_creature_bone_matrices = bone_world_matrices
		shape_entries = list(record.slots.items())  # (slot_name, shape_name)
		if record.hair:
			shape_entries.append(("hair", record.hair[0]))

		built_count = 0
		t_total_start = time.perf_counter()
		for slot_name, shape_name in shape_entries:
			t0 = time.perf_counter()
			entry = self.search_paths_dialog.find_texture(shape_name)
			t1 = time.perf_counter()
			if entry is None:
				print(f"[object_editor] assembled creature: {shape_name!r} not found in scanned search paths")
				continue
			try:
				data = entry.read_bytes()
				t2 = time.perf_counter()
				shape_file = parse_shape(data)
				t3 = time.perf_counter()
			except (ShapeParseError, OSError) as exc:
				print(f"[object_editor] assembled creature: cannot parse {shape_name!r}: {exc}")
				continue
			node_path = self._assembled_creature_root.attach_new_node(shape_name)
			# No special positioning needed here: face/head pieces (e.g.
			# *_visage.shape) turned out to be real skinned CMeshMRM (not
			# CMeshMRMSkinned, see _is_shape_skinned()) -- once
			# _build_assembled_shape_geometry() skins them like any other
			# body part, they land in the right place on their own, same
			# identity-plus-root-offset transform as everything else.
			t4 = time.perf_counter()
			_content_root, skin_state = self._build_assembled_shape_geometry(
				shape_file.value, skeleton, bone_world_matrices, node_path, slot_name)
			t5 = time.perf_counter()
			self._assembled_creature_base_nodes[slot_name] = node_path
			if skin_state is not None:
				self._assembled_creature_skin_states[slot_name] = skin_state
			built_count += 1
		t_total_end = time.perf_counter()
		self._apply_loaded_shape_to_creature()

	def _apply_loaded_shape_to_creature(self):
		"""(Re)shows wherever the currently loaded shape (self.shape_file)
		belongs relative to the Bind-preview creature -- exactly one of:
		(a) skinned into a body-part slot (self._bind_slot_override),
		hiding that slot's own default piece; (b) rigid, positioned at a
		chosen attach-point bone (self._bind_attach_point) -- a hand-held
		weapon; or (c) neither -- an "undefined" catch-all, shown at the
		creature root's own identity transform (the same place it would
		sit standalone). Simplified 2026-08-30 (Nuno: "tu fais une
		structure tres simple displayed_creature avec tous les slots [...]
		et meme un slot undefined si aucun slot n'est choisi") -- one
		unconditional rule replaces the previous scattered
		show()/hide() calls: self.model_root (the loaded shape's own
		standalone rendering, from _rebuild_geometry()) is shown whenever
		no creature is actually being displayed, and hidden -- ALWAYS,
		unconditionally -- whenever one is, because the loaded shape then
		always has a place inside the creature group instead (one of the
		three cases above). Exactly one of the two ever renders the loaded
		shape at a time, never both (bug found 2026-08-30, screenshot: two
		disconnected hand pieces "floating" was model_root rendering
		standalone AND unposed, at the same time as an identical copy
		correctly skinned into the creature).

		Cheap: no disk I/O, reuses _assembled_creature_base_nodes built by
		_rebuild_assembled_creature() -- safe to call every time just the
		loaded shape or its binding choice changes (auto-detect on every
		shape load, manual slot/attach-point combo pick) without
		re-parsing the other ~6 body-part shapes from disk each time -- see
		_rebuild_assembled_creature()'s own docstring for the perf
		regression this split was introduced to fix."""
		if self._assembled_creature_loaded_shape_node is not None:
			self._assembled_creature_loaded_shape_node.remove_node()
			self._assembled_creature_loaded_shape_node = None
			self._assembled_creature_loaded_shape_content_root = None
			self._assembled_creature_loaded_shape_skin_state = None
		for node_path in self._assembled_creature_base_nodes.values():
			node_path.show()
		# Only ever (re)shown in the attach-point branch below -- removed
		# here unconditionally first so every OTHER case (no creature, slot
		# override, "undefined") starts from "not shown" rather than needing
		# its own explicit hide.
		self._attach_point_axes_np.remove_node()

		creature_shown = self._bind_skeleton is not None and bool(self._assembled_creature_bone_matrices)
		if self.shape_file is None or not creature_shown:
			self.model_root.show()
			# Pivot gizmo back under _object_pivot (its normal home) --
			# see below for why it moves to content_root while bound.
			if self._pivot_axes_np.get_parent() != self._object_pivot:
				self._pivot_axes_np.reparent_to(self._object_pivot)
				self._pivot_axes_np.clear_transform()
			self._frame_camera()
			return
		self.model_root.hide()

		override_slot = self._bind_slot_override
		if override_slot:
			base_node = self._assembled_creature_base_nodes.get(override_slot)
			if base_node is not None:
				base_node.hide()
			node_path = self._assembled_creature_root.attach_new_node(f"{override_slot}-override")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, self._bind_skeleton, self._assembled_creature_bone_matrices, node_path, override_slot)
			self._assembled_creature_loaded_shape_is_skinned_override = _is_shape_skinned(self.shape_file.value)
		elif self._bind_attach_point and self._bind_attach_point in self._bind_skeleton.bone_map:
			# Rigid, not skinned (skeleton=None forces iter_render_passes()'s
			# own bind-pose fallback -- see _build_assembled_shape_geometry(),
			# even for a shape that happens to be technically skinned data,
			# same reasoning _rebuild_geometry()'s attach-point case used to
			# rely on before this consolidation), then positioned by hand at
			# the chosen bone's current world matrix.
			node_path = self._assembled_creature_root.attach_new_node("attach-point")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, None, None, node_path, "attach-point")
			self._assembled_creature_loaded_shape_is_skinned_override = False
			bone_matrix = self._assembled_creature_bone_matrices[self._bind_attach_point]
			node_path.set_mat(_nel_matrix_to_panda_mat4(bone_matrix))
			# Visual target marker for the bone itself, distinct from the
			# object's own pivot axes (_pivot_axes_np) -- lets the user
			# directly compare "where my shape's own local origin currently
			# is" (the pivot gizmo, moves with position/rotation/scale edits)
			# against "where the real client will actually stick it"
			# (this marker, fixed at the bone), instead of having to guess
			# (2026-08-31, Nuno: "je dois OBLIGATOIREMENT visualiser ce
			# point de préhension [...] le pivot de mon arme n'est pas sur
			# le point d'attache"). Sized off the loaded shape's own bbox,
			# same reasoning _rebuild_viewport_helpers() uses for the other
			# axes -- a fixed size would be comically large for a dagger and
			# invisible for a two-handed sword.
			bbox = shape_bbox(self.shape_file.value)
			axis_length = (max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1) * _AXIS_MARGIN_FACTOR
			               if bbox is not None else _AXIS_LENGTH)
			self._attach_point_axes_np = self._assembled_creature_root.attach_new_node(
				_build_axes_geom(_ATTACH_POINT_AXIS_COLORS, axis_length))
			self._attach_point_axes_np.set_light_off()
			self._attach_point_axes_np.set_mat(_nel_matrix_to_panda_mat4(bone_matrix))
		else:
			# "undefined": doesn't correspond to any creature part -- shown
			# rigid (same skeleton=None reasoning as the attach-point case
			# above, no creature bone to skin an unrelated shape against) at
			# _assembled_creature_root's own identity transform, i.e. the
			# same place model_root would otherwise sit.
			node_path = self._assembled_creature_root.attach_new_node("undefined")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, None, None, node_path, "undefined")
			self._assembled_creature_loaded_shape_is_skinned_override = False

		self._assembled_creature_loaded_shape_node = node_path
		# _update_bound_shape_rotation() keeps this synced with
		# self._object_pivot's LIVE rotation every frame from here on --
		# the set_quat() from base.default_rot_quat inside
		# _build_assembled_shape_geometry() above was only ever the initial
		# value (matches _object_pivot's own at load time), not a live link.
		self._assembled_creature_loaded_shape_content_root = content_root
		# Pivot gizmo moves here too, off _object_pivot (its standalone-view
		# home, still at the pre-bind world position) -- content_root IS
		# "the object's own local origin", already correctly positioned
		# wherever the bound copy actually sits (skinned slot, attach-point
		# bone, or undefined-at-creature-root) and kept live in sync by
		# _update_bound_shape_rotation(). Without this the gizmo still
		# rotated with edits (it inherited _object_pivot's rotation) but
		# stayed frozen at the pre-bind position, completely disconnected
		# from the actually-rendered mesh (bug found 2026-08-31, Nuno: "mon
		# objet tourne autour d'un axe qui n'est pas le pivot [...] il est
		# resté à sa position avant le bind").
		self._pivot_axes_np.reparent_to(content_root)
		self._pivot_axes_np.clear_transform()
		self._frame_camera()

	def _build_assembled_shape_geometry(self, shape_value, skeleton, bone_world_matrices, parent_node_path, slot_name):
		"""Same idea as _build_reference_geometry() (a non-editable preview
		shape, material_id=None so it never touches the main shape's own
		override-color/multi-bitmap-slot state), but skinned against
		`skeleton`/`bone_world_matrices` like the main shape's own
		_rebuild_geometry() does for a CMeshMRMSkinned -- body-part shapes
		are themselves skinned meshes bound to the whole creature skeleton,
		not static props."""
		materials = getattr(shape_value, "materials", None)
		is_skinned = _is_shape_skinned(shape_value)
		# Same CMeshBase::DefaultRotQuat the main shape's own standalone
		# view seeds _object_pivot with (_display_shape()) -- the engine's
		# real per-instance rotation, layered on TOP of whatever skinning
		# already did, not replaced by it. Missing here made a bound/
		# overridden shape with a non-identity export rotation (confirmed
		# real on some armor pieces) render mis-rotated once shown on a
		# creature (bug found 2026-08-30, Nuno: "les rotations ne sont plus
		# prises en compte"). Applied to a dedicated child node, not
		# `parent_node_path` itself, so it composes correctly regardless of
		# how the CALLER positions parent_node_path afterwards (e.g. the
		# attach-point case's own set_mat() in
		# _apply_loaded_shape_to_creature()).
		#
		# ONLY for a RIGID (not is_skinned) shape, though -- verified
		# 2026-08-31 against the real client's own world-matrix update,
		# transform.cpp:946 CTransform::updateWorldMatrixFromFather():
		# `if(!isSkinned() && _AncestorSkeletonModel) _WorldMatrix =
		# parentWM * _LocalMatrix；` -- when isSkinned() is true, this whole
		# block (and so _LocalMatrix, built from Default{Pos,RotQuat,Scale})
		# is skipped entirely; a skinned mesh's final vertex positions come
		# purely from applySkin()'s bone-matrix math
		# (skeleton_model.h:computeBoneMatrixes3x4(), no instance transform
		# involved at all -- confirmed no PreMul variant is used for regular
		# skinning). So for a skinned piece, DefaultRotQuat (and Pos/Scale)
		# has ZERO effect in the real game, not just position/scale --
		# applying it here for a skinned body-part/override was itself a
		# bug (an earlier "fix" for a rotation report that happened to look
		# right only because that particular shape's default_rot_quat was
		# identity, Nuno: "t'es sur que la rotation dois s'appliquer
		# aussi?").
		base = getattr(shape_value, "base", None)
		content_root = parent_node_path.attach_new_node(f"{slot_name}-content")
		if base is not None and not is_skinned:
			rot = base.default_rot_quat
			content_root.set_quat(Quat(rot.w, rot.x, rot.y, rot.z))
		# Live re-skin support (_update_assembled_creature_skin(), Bind
		# preview's Play button, 2026-08-31: "vu que tout le corps + cheveux
		# bougent, ben le visage doit bouger aussi") -- two different
		# per-vertex data layouts depending on the real shape type, so two
		# different _SkinState-like builders/reskin functions
		# (_update_assembled_creature_skin() dispatches on isinstance()):
		# CMeshMRMSkinned's packed-vertex format (_build_skin_state()/
		# _reskin_state()) vs. a plain skinned CMeshMRM's own
		# geom.skin_weights layout (_build_mrm_skin_state()/
		# _reskin_mrm_state(), e.g. *_visage.shape face pieces).
		skin_state = None
		if is_skinned and skeleton is not None:
			geom_value = shape_geom(shape_value)
			if isinstance(shape_value, MeshMRMSkinned):
				skin_state = _build_skin_state(geom_value, skeleton)
			else:
				skin_state = _build_mrm_skin_state(geom_value, skeleton)
		vdata = None
		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(
				shape_value, skeleton=skeleton if is_skinned else None,
				bone_world_matrices=bone_world_matrices if is_skinned else None):
			if not indices:
				continue
			if vdata is None:
				vdata = _build_vertex_data(vertex_buffer, dynamic=skin_state is not None)
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"assembled-pass-{material_id}")
			geom_node.add_geom(geom)
			node_path = content_root.attach_new_node(geom_node)
			material = materials[material_id] if materials and material_id < len(materials) else None
			self._apply_material_common(node_path, material)
			self._apply_material_texture(node_path, material, None)
			texture_names = [t.file_name if t is not None else None for t in material.textures] if material is not None else None
			pass_count += 1
		if skin_state is not None:
			skin_state.vdata = vdata
		return content_root, skin_state

	def _draw_bind_controls(self):
		"""Floating "Bind preview" window: previews the loaded shape on one
		of Patina's curated reference creatures. Two real cases (confirmed
		2026-08-30, correcting an earlier wrong design -- see
		/repos/project-todos/ryzom-core/forgery-object-editor.md "Creature/
		NPC binder + assembler in Patina"):

		- **Skinned shape** (most equipment, including armor -- confirmed
		  most "armor" pieces are skinned meshes, not rigid props): it's
		  already bound to the whole skeleton, not one point. What's
		  actually useful here is picking which creature *slot* it should
		  stand in for (e.g. preview `fy_HOM_armor01_bottes.shape` in place
		  of the creature's own default `fy_HOM_civil01_bottes.shape` for
		  "feet") -- see `_bind_slot_override`, consumed by
		  _rebuild_assembled_creature().
		- **Non-skinned shape**: the real "bound to one attach point" case,
		  but only for the 2 hand slots in practice -- creature_ref.
		  WEAPON_ATTACH_POINTS (box_arme/box_arme_gauche/Box_bouclier),
		  hardcoded in character_cl.cpp, not general equipment (CONFIRMED
		  WRONG EARLIER: CCharacterSheet.BodyToBone is NOT attach-point
		  data, see CreatureRecord.body_to_bone's own docstring -- almost
		  nothing in real content actually needs a single attach point).

		Shown for any loaded shape (both cases share the creature picker),
		stacked below _draw_bone_preview_controls() ("Skinning preview",
		shown only for a skinned shape) when that one is also showing."""
		if self.shape_file is None:
			return

		is_skinned = _is_shape_skinned(self.shape_file.value)
		creatures = self._ensure_bind_creatures()

		display_width = imgui.get_io().display_size.x
		win_w, win_h = self._bind_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		if self._wind_state is not None:
			y += self._wind_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		if is_skinned:
			# Skinning preview is shown above for a skinned shape (same
			# precondition already checked there) -- stack below its
			# current size (updated earlier this same frame, draw_panel()
			# calls it before this method).
			y += self._bone_preview_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("Bind preview", flags=flags):
			if self._bind_cache_rebuild is not None and not self._bind_cache_rebuild["done"]:
				imgui.text_disabled("Rebuilding creature cache...")
			imgui.set_next_item_width(220)
			preview = self._bind_creature_name or "(choose a creature)"
			if imgui.begin_combo("##bind-creature-combo", preview):
				clicked, _ = imgui.selectable("(none)", self._bind_creature_name == "")
				if clicked:
					self._select_bind_creature("")
				for name in sorted(creatures):
					clicked, _ = imgui.selectable(name, name == self._bind_creature_name)
					if clicked:
						self._select_bind_creature(name)
				imgui.end_combo()
			imgui.same_line()
			workspace_dir = self.workspace_setup_dialog.active_workspace_dir
			has_override = workspace_dir is not None and creature_ref.workspace_list_path(workspace_dir).is_file()
			if has_override:
				editor_path = app_settings.load().text_editor_path
				tooltip = "Edit this workspace's creatures_ref.txt in the configured text editor"
				if _icon_button(f"{fa_icons.ICON_FA_EDIT}##creatures-ref", tooltip):
					if not editor_path:
						self.request_settings_attention("Tools", "text_editor_path")
					else:
						subprocess.Popen([editor_path, str(creature_ref.workspace_list_path(workspace_dir))])
			else:
				tooltip = "Copy the bundled creatures_ref.txt into this workspace, to track more creatures"
				if _icon_button(f"{fa_icons.ICON_FA_COG}##creatures-ref", tooltip, disabled=workspace_dir is None):
					self._copy_creatures_ref_to_workspace()

			# Poses the standing reference against a real animation instead of
			# the skeleton's raw bind pose (creature_ref.resolve_animation(),
			# see _rebuild_assembled_creature()'s own note on the full
			# Mode->AnimSet->.anim resolution chain). Disabled without a
			# creature picked -- nothing to pose.
			imgui.set_next_item_width(220)
			preview = self._bind_mode or "(bind pose)"
			imgui.begin_disabled(not self._bind_creature_name)
			if imgui.begin_combo("##bind-mode-combo", preview):
				clicked, _ = imgui.selectable("(bind pose)", self._bind_mode == "")
				if clicked:
					self._bind_mode = ""
					self._rebuild_assembled_creature()
				for mode_name in creature_ref.ANIM_MODES:
					clicked, _ = imgui.selectable(mode_name, mode_name == self._bind_mode)
					if clicked:
						self._bind_mode = mode_name
						self._rebuild_assembled_creature()
				imgui.end_combo()
			imgui.end_disabled()

			# Live playback (Play button, 2026-08-31: "Possible d'avoir un
			# bouton pour lancer l'animation?") -- only shown once a Mode
			# actually resolved to a real, parsed animation (self._bind_animation,
			# see _rebuild_assembled_creature()). Re-skins every live-capable
			# body-part shape every frame while playing (_update_assembled_creature_skin()),
			# same Play/Pause + time-slider layout as the Skinning preview's
			# own animation controls above.
			if self._bind_animation is not None:
				icon = fa_icons.ICON_FA_PAUSE if self._bind_anim_playing else fa_icons.ICON_FA_PLAY
				if _icon_button(f"{icon}##bind-anim-play", "Play/pause"):
					self._bind_anim_playing = not self._bind_anim_playing
				imgui.same_line()
				imgui.set_next_item_width(160)
				changed, new_time = imgui.slider_float(
					"##bind-anim-time", self._bind_anim_time,
					0.0, max(self._bind_anim_duration, 0.001), "%.2f s")
				if changed:
					self._bind_anim_time = new_time

			if is_skinned:
				slot_names = list(creature_ref.BODY_SLOTS) + ["hair"]
				imgui.set_next_item_width(220)
				preview = self._bind_slot_override or "(none)"
				if imgui.begin_combo("##bind-slot-override-combo", preview):
					clicked, _ = imgui.selectable("(none)", self._bind_slot_override == "")
					if clicked:
						self._bind_slot_override = ""
						self._apply_loaded_shape_to_creature()
					for slot_name in slot_names:
						clicked, _ = imgui.selectable(slot_name, slot_name == self._bind_slot_override)
						if clicked:
							self._bind_slot_override = slot_name
							was_hidden = not self._show_assembled_creature
							self._show_assembled_creature = True
							# Turning visibility on here (rather than via the
							# ICON_FA_MALE toggle) needs the expensive full
							# rebuild first -- _assembled_creature_base_nodes
							# is empty/stale while hidden (see
							# _rebuild_assembled_creature()'s early-return).
							if was_hidden:
								self._rebuild_assembled_creature()
							else:
								self._apply_loaded_shape_to_creature()
					imgui.end_combo()
			else:
				imgui.set_next_item_width(220)
				preview = self._bind_attach_point or "(none)"
				if imgui.begin_combo("##bind-attach-point-combo", preview):
					clicked, _ = imgui.selectable("(none)", self._bind_attach_point == "")
					if clicked:
						self._bind_attach_point = ""
						self._apply_loaded_shape_to_creature()
					for curated_name in creature_ref.WEAPON_ATTACH_POINTS:
						resolved_name = self._resolve_bone_name(self._bind_skeleton, curated_name)
						clicked, _ = imgui.selectable(curated_name, curated_name == self._bind_attach_point)
						if clicked:
							if resolved_name is None:
								print(f"[object_editor] Bind preview: attach point {curated_name!r} not found in "
								      f"the loaded skeleton's own bones")
							self._bind_attach_point = resolved_name if resolved_name is not None else curated_name
							self._apply_loaded_shape_to_creature()
					imgui.end_combo()

			if _icon_button(
					fa_icons.ICON_FA_MALE, "Show this creature fully assembled as a standing reference",
					self._show_assembled_creature, disabled=not self._bind_creature_name):
				self._show_assembled_creature = not self._show_assembled_creature
				self._rebuild_assembled_creature()

			self._bind_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

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

		# Bind preview (_select_bind_creature()) forces "skin" to the picked
		# creature's own race -- same condition _apply_loaded_shape_to_creature()
		# uses for "is a creature actually being shown". Without disabling
		# the other race buttons here, clicking one manually while a
		# creature is shown silently re-mismatches skin tone vs body again
		# (the exact bug this forcing was added to fix) with no visual sign
		# anything is different, which reads as "the force is buggy/
		# unreliable" rather than "this is locked to the creature's race"
		# (2026-08-30, Nuno: "quand tu force le skin de panoply il faut
		# griser les autres.. sinon on pense qu'il y a un bug").
		skin_forced_by_creature = self._bind_skeleton is not None and bool(self._assembled_creature_bone_matrices)

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
				tooltip = f"{self._PANOPLY_AXIS_LABELS[axis]} {value!r}"
				locked = axis == "skin" and skin_forced_by_creature and not active
				if locked:
					tooltip += f" (locked to {self._bind_creature_name}'s own race -- deselect the creature to change)"
				if _icon_button(label, tooltip, active=active, disabled=locked):
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
		center_next_popup()
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

	def _update_specular_overlay(self, node_path, diffuse_texture, specular_texture):
		"""CMaterial::TShader::Specular's highlight, reproduced the way the
		real engine does it -- driver_opengl_material.cpp's
		beginSpecularMultiPass()/setupSpecularPass() literally run this as a
		SECOND additive render pass (material.h's own doc comment: "This is
		done in 2 passes") -- an instanced sibling NodePath (same GeomNode,
		no vertex duplication) drawn on top with additive blending,
		computing `specular_map(reflectDir).rgb * diffuse_map(texcoord).a`.
		Uses a small dedicated GLSL shader (_SPECULAR_OVERLAY_VERTEX_SHADER/
		_SPECULAR_OVERLAY_FRAGMENT_SHADER) just for this -- fixed-function
		TexGenAttrib (GL_TEXTURE_GEN reflection mapping), tried first,
		doesn't work at all under Panda3D's modern OpenGL path (confirmed
		empirically: identical setup minus TexGenAttrib rendered fine with a
		flat test color, but never showed anything once cubemap TexGen was
		involved). The overlay is unlit/unmaterialed by design already (the
		real engine's own additive pass just samples raw texels, no
		relighting), so this shader doesn't reimplement any of Panda3D's own
		ambient/diffuse/light pipeline -- it's scoped to this one pass only,
		the base pass (and its normal fixed-function lighting) is untouched.
		`node_path`'s own python tag tracks the overlay so repeated calls
		(live material edits) update it in place instead of piling up
		copies."""
		overlay = node_path.get_python_tag("specular_overlay") if node_path.has_python_tag("specular_overlay") else None
		if diffuse_texture is None or specular_texture is None:
			if overlay is not None and not overlay.is_empty():
				overlay.remove_node()
			if node_path.has_python_tag("specular_overlay"):
				node_path.clear_python_tag("specular_overlay")
			return

		if overlay is None or overlay.is_empty():
			# A plain attach_new_node(node_path.node()) under the SAME parent
			# doesn't create a genuine second instance -- Panda3D collapses
			# it into the same single edge, so removing "overlay" later would
			# also destroy the original geometry (confirmed empirically,
			# there's no doc warning about this). instance_to() under a
			# fresh, otherwise-empty wrapper node does create an independent,
			# safely-removable second edge; render state set on the wrapper
			# cascades down to the instanced geometry as normal.
			overlay = node_path.get_parent().attach_new_node("specular-overlay")
			node_path.instance_to(overlay)
			node_path.set_python_tag("specular_overlay", overlay)
		overlay.set_transform(node_path.get_transform())

		is_cube_map = specular_texture.get_texture_type() == PandaTexture.TT_cube_map
		overlay.set_shader(self._specular_overlay_shader)
		overlay.set_shader_input("diffuse_map", diffuse_texture)
		overlay.set_shader_input("specular_cube_map", specular_texture if is_cube_map else self._dummy_cube_texture)
		overlay.set_shader_input("specular_flat_map", specular_texture if not is_cube_map else self._dummy_2d_texture)
		overlay.set_shader_input("is_cube_map", 1 if is_cube_map else 0)

		overlay.set_attrib(ColorBlendAttrib.make(ColorBlendAttrib.M_add, ColorBlendAttrib.O_one, ColorBlendAttrib.O_one))
		overlay.set_attrib(DepthTestAttrib.make(RenderAttrib.M_less_equal))
		overlay.set_depth_write(False)
		overlay.set_light_off(1)
		overlay.set_material_off(1)
		overlay.set_bin("transparent", 0)

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
		if material.flags & _IDRV_MAT_ALPHA_TEST and _material_alpha_from_texture(material):
			# Cutout transparency (e.g. foliage): discard texels below the
			# threshold outright rather than blending, matching the engine's
			# own glAlphaFunc(GL_GREATER, threshold) (driver_opengl_material.cpp).
			# Skipped when a later texture stage's own texenv overrides the
			# final alpha away from any texture (see
			# _material_alpha_from_texture()) -- alpha-testing stage 0's
			# texture in isolation would then test against a value the real
			# engine never actually uses for this material.
			node_path.set_attrib(AlphaTestAttrib.make(AlphaTestAttrib.M_greater, material.alpha_test_threshold))

		texture = material.textures[0] if material.textures else None
		panda_texture = None
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

		specular_texture = None
		if material is not None and material.shader_type == _IDRV_MAT_SHADER_SPECULAR and len(material.textures) > 1:
			stage1 = material.textures[1]
			if stage1 is not None and stage1.class_name == "CTextureCube":
				cube_key = tuple(sub.file_name if sub is not None else None for sub in (stage1.sub_textures or []))
				if cube_key in self._cube_texture_cache:
					specular_texture = self._cube_texture_cache[cube_key]
				else:
					specular_texture = load_panda_cube_texture(
						stage1.sub_textures, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
						repeat=self._texture_needs_repeat, finder=self.search_paths_dialog.find_texture)
					self._cube_texture_cache[cube_key] = specular_texture
			elif stage1 is not None and stage1.file_name:
				specular_texture = load_panda_texture(
					stage1.file_name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
					repeat=self._texture_needs_repeat, finder=self.search_paths_dialog.find_texture,
					wrap_s=stage1.wrap_s, wrap_t=stage1.wrap_t,
					min_filter=stage1.min_filter, mag_filter=stage1.mag_filter,
					load_grayscale_as_alpha=stage1.load_grayscale_as_alpha)
		self._update_specular_overlay(node_path, panda_texture, specular_texture)

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

	def _set_material_shader_type(self, material_id, material, shader_type):
		material.shader_type = shader_type
		self._reapply_material(material_id)

	def _draw_material_render_mode_section(self, material_id, material):
		"""CMaterial::TShader picker -- see _SHADER_MODE_OPTIONS for which
		modes are listed/selectable and why. Picking a disabled option is
		not possible (imgui.begin_disabled() around its selectable makes it
		inert), so only Normal<->Specular switches actually do anything.
		No visible label (id-only "##render-mode") and a tight width -- this
		sits inline in the compact material header row, next to the
		Single/Multi picker (_draw_material_kind_selector())."""
		current_label = next(
			(label for value, label, _ in _SHADER_MODE_OPTIONS if value == material.shader_type),
			f"? ({material.shader_type})",
		)
		imgui.set_next_item_width(90)
		if imgui.begin_combo("##render-mode", current_label):
			for value, label, enabled in _SHADER_MODE_OPTIONS:
				imgui.begin_disabled(not enabled)
				clicked, _ = imgui.selectable(label, value == material.shader_type)
				imgui.end_disabled()
				if clicked and enabled and value != material.shader_type:
					self._set_material_shader_type(material_id, material, value)
			imgui.end_combo()
		return self._doc_hint_if_hovered("render-mode")

	def _draw_material_kind_selector(self, material_id, material, is_multi, texture):
		"""Single/Multi picker replacing the old three-way "Color"/"Simple
		bitmap"/"Multi Bitmap" text badge + separate convert icon buttons --
		id-only ("##bitmap-kind"), no visible label, tight width, same
		compact-row spirit as the render-mode combo next to it. "Color"
		(no texture at all) counts as "Single" here, same as a simple
		bitmap -- both convert to "Multi" the same way
		(_convert_to_multi_bitmap()). Multi -> Single is only offered when
		the reverse conversion is actually lossless (same constraint the old
		icon buttons enforced): no slot populated (-> Color) or only the Low
		Quality slot populated (-> Simple bitmap); otherwise "Single" is
		listed but disabled, since real per-quality/season/ecosystem
		variants would be silently discarded."""
		populated = self._multi_bitmap_populated_slots(texture) if is_multi else None
		can_go_single = not is_multi or not populated or populated == [0]
		current_label = "Multi" if is_multi else "Single"
		imgui.set_next_item_width(70)
		if imgui.begin_combo("##bitmap-kind", current_label):
			imgui.begin_disabled(is_multi and not can_go_single)
			single_clicked, _ = imgui.selectable("Single", not is_multi)
			imgui.end_disabled()
			if single_clicked and is_multi and can_go_single:
				if not populated:
					self._convert_multi_bitmap_to_color(material_id, material)
				else:
					self._convert_multi_bitmap_to_simple(material_id, material)
			multi_clicked, _ = imgui.selectable("Multi", is_multi)
			if multi_clicked and not is_multi:
				self._convert_to_multi_bitmap(material_id, material)
			imgui.end_combo()

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
			badge = "Multi Bitmap" if is_multi else ("Simple bitmap" if has_texture else "Color")

			self._draw_material_kind_selector(material_id, material, is_multi, texture)
			imgui.same_line()
			if badge != "Color":
				render_mode_hint = self._draw_material_render_mode_section(material_id, material)
				if render_mode_hint:
					hovered_hint = render_mode_hint

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
				if badge != "Color":
					double_sided = bool(material.flags & _IDRV_MAT_DOUBLE_SIDED)
					changed, double_sided = imgui.checkbox("Double-sided", double_sided)
					if changed:
						self._toggle_material_flag(material_id, material, _IDRV_MAT_DOUBLE_SIDED)
				section_hint = self._draw_material_transparency_section(material_id, material)
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
			# No enable checkbox anymore -- a real edit here is what turns
			# this on now, matching enableUserTexMat()'s own per-stage bit.
			material.flags |= _IDRV_MAT_TEX_ADDR | stage0_flag
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

	@staticmethod
	def _set_specular_material_texture(material, file_name):
		"""Sets stage 1's (Specular shader mode) texture file name -- for a
		CTextureCube, applied to every one of its 6 faces at once (matching
		real content, which always duplicates the same file across all 6
		faces rather than genuinely varying per-face). Only meaningful once
		shader_type is already Specular with a real stage-1 Texture object
		there -- doesn't create one from scratch, unlike
		_set_simple_material_texture()'s slot-0 handling, since there's no
		single obvious default texture class to create it as."""
		stage1 = material.textures[1]
		faces = stage1.sub_textures if stage1.class_name == "CTextureCube" else [stage1]
		for face in faces or []:
			if face is not None:
				face.file_name = file_name

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

		if material.shader_type == _IDRV_MAT_SHADER_SPECULAR and len(material.textures) > 1 and material.textures[1] is not None:
			current_specular = self._specular_preview_name(material) or ""
			self._draw_texture_preview_static(current_specular, badge="S")
			imgui.same_line()
			specular_in_workspace = self._is_texture_in_workspace(current_specular)
			imgui.push_style_color(
				imgui.Col_.text.value, _TEXTURE_IN_WORKSPACE_COLOR if specular_in_workspace else _TEXTURE_NORMAL_COLOR)
			changed, new_value = self._draw_texture_name_combo("##specular-combo", current_specular)
			imgui.pop_style_color()
			if imgui.is_item_hovered() and current_specular:
				source_path = self._texture_source_path_text(current_specular)
				tooltip = source_path or current_specular
				if specular_in_workspace:
					tooltip += "\n(in the active workspace)"
				imgui.set_tooltip(tooltip)
			if changed and new_value != current_specular:
				self._set_specular_material_texture(material, new_value)
				self._reapply_material(material_id)
			imgui.same_line()

			if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a specular/gloss map file"):
				def _on_specular_result(file_name, material_id=material_id, material=material):
					self._set_specular_material_texture(material, file_name)
					self._reapply_material(material_id)
				self._start_texture_browse(("specular", material_id), _on_specular_result)
			imgui.same_line()

			def _on_specular_copied(new_name, material_id=material_id, material=material):
				self._set_specular_material_texture(material, new_name)
				self._reapply_material(material_id)
			self._draw_texture_copy_button(current_specular, _on_specular_copied)

		self._draw_panoply_masks_for(current_value)

		imgui.pop_id()

	@staticmethod
	def _specular_preview_name(material):
		"""A representative file name for material.textures[1] when the
		material is in Specular mode -- for a CTextureCube, its first
		populated face (real content duplicates the same file across all 6
		faces, per the shape-wide scan noted in the specular chantier, so
		any one face is a faithful preview); for a flat file, that file
		directly. None when there's nothing to preview (Normal mode, no
		stage-1 texture, or an all-empty cube)."""
		if material is None or material.shader_type != _IDRV_MAT_SHADER_SPECULAR or len(material.textures) < 2:
			return None
		stage1 = material.textures[1]
		if stage1 is None:
			return None
		if stage1.class_name == "CTextureCube":
			for sub in stage1.sub_textures or []:
				if sub is not None and sub.file_name:
					return sub.file_name
			return None
		return stage1.file_name or None

	@staticmethod
	def _specular_preview_name_for_slot(material, index):
		"""Same as _specular_preview_name(), but resolves a specific Multi
		Bitmap slot index rather than the material's currently *selected*
		variant -- in real data, stage 1's CTextureCube faces are
		themselves CTextureMultiFile, with file_names lining up 1:1 with
		stage 0's own per-slot dye choices (e.g. nospec/spec_base/spec_luxe
		tracking the same c1..c8 index the diffuse texture uses)."""
		if material is None or material.shader_type != _IDRV_MAT_SHADER_SPECULAR or len(material.textures) < 2:
			return None
		stage1 = material.textures[1]
		if stage1 is None:
			return None
		faces = stage1.sub_textures if stage1.class_name == "CTextureCube" else [stage1]
		for face in faces or []:
			if face is None:
				continue
			if face.file_names and index < len(face.file_names) and face.file_names[index]:
				return face.file_names[index]
			if not face.file_names and face.file_name:
				return face.file_name
		return None

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
		self._draw_bind_controls()
		self._draw_reference_shapes_toggles()
		self.export_dialog.draw()
		self.import_dialog.draw()
		self.search_paths_dialog.draw()
		self._poll_skeleton_file_dialog()
		self._poll_animation_file_dialog()
		self._poll_image_editor_dialog()
		self._poll_text_editor_dialog()
		self._poll_workspace_sync_folder_dialog()
		self._poll_repository_paths_dialog()
		self._draw_replace_match_popup()
		self._draw_reopen_shape_popup()
		self._draw_load_shape_unsaved_popup()
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
				if imgui.collapsing_header(f"{fa_icons.ICON_FA_MAP_SIGNS} Paths"):
					self.workspace_setup_dialog.draw_settings_content()
					imgui.separator()
					self._draw_exclusion_rules_settings()
					imgui.separator()
					self.search_paths_dialog.draw_settings_content()
					imgui.separator()
					self._draw_repository_paths_settings()
				self._consume_settings_section_open("Tools")
				if imgui.collapsing_header(f"{fa_icons.ICON_FA_WRENCH} Tools"):
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
