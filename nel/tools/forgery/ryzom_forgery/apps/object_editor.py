"""Ryzom Forgery object editor: browse, inspect and edit .shape files.

3D display is supported for CMesh, CMeshMRM (finest LOD) and CMeshMultiLod
(slot 0, whose geometry is itself a CMesh/CMeshMRM). Other shape types
(skeleton, water, flare, particles, ...) show their properties only, no 3D
render yet.
"""

import time
from pathlib import Path

import numpy

from panda3d.core import NodePath, Shader, Texture as PandaTexture

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui

from ryzom_forgery.app import _AVAILABLE_FONTS, _dpi_scale, ForgeryApp
from ryzom_forgery.camera import ObjectManipulator, OrbitCamera
from ryzom_forgery import creature_ref
from ryzom_forgery.export_dialog import ExportDialog
from ryzom_forgery.import_dialog import ImportDialog
from ryzom_forgery.import_watcher import ImportWatcher
from ryzom_forgery.material_docs import load_material_docs
from ryzom_forgery.navcube import NavigationCube
from ryzom_forgery import panoply_config
from ryzom_forgery import panoply_live
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.search_paths_dialog import SearchPathsDialog
from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.tex_dds_sync import TexDdsSyncWatcher
from ryzom_forgery import virtual_categories
from ryzom_forgery.workspace_setup_dialog import WorkspaceSetupDialog, _truncate_path_to_width
from ryzom_forgery.workspace_sync import WorkspaceSyncWatcher, SYNCED_SUBDIRS
from ryzom_forgery.workspace_watch import WorkspaceWatcher
from ryzom_forgery.workspaces import ensure_structure

from ryzom_forgery.apps.object_editor_mixins.creature_bind import CreatureBindMixin
from ryzom_forgery.apps.object_editor_mixins.materials import MaterialsMixin
from ryzom_forgery.apps.object_editor_mixins.mesh_import import MeshImportMixin
from ryzom_forgery.apps.object_editor_mixins.panoply_ui import PanoplyUIMixin
from ryzom_forgery.apps.object_editor_mixins.reference_shapes import ReferenceShapesMixin
from ryzom_forgery.apps.object_editor_mixins.settings_dialogs import SettingsDialogsMixin
from ryzom_forgery.apps.object_editor_mixins.shape_io import ShapeIOMixin
from ryzom_forgery.apps.object_editor_mixins.texture_widgets import TextureWidgetsMixin
from ryzom_forgery.apps.object_editor_mixins.viewport_transform import ViewportTransformMixin

# See _scan_active_workspace_virtual_categories().
_VIRTUAL_CATEGORIES_RESCAN_INTERVAL = 1.0

# draw_panel()'s tab bar (_push_tab_color()).
_TAB_COLOR_TEXTURES = (0.565, 0.933, 0.565, 1.0)  # lightgreen
_TAB_COLOR_MATERIALS = (0.878, 1.0, 1.0, 1.0)  # lightcyan
_TAB_COLOR_ALL_PROPERTIES = (0.5, 0.5, 0.5, 1.0)  # gray
_TAB_COLOR_SETTINGS = (0.8, 0.75, 0.15, 1.0)  # yellow

# Specular overlay pass (_update_specular_overlay(), materials.py): a tiny,
# self-contained
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


APP_INFO = {
	"id": "object_editor",
	"name": "Patina",
	"subtitle": "Object Editor",
	"description": "Browse, inspect and edit .shape files.",
}


class ObjectEditorApp(
		CreatureBindMixin, MaterialsMixin, MeshImportMixin, PanoplyUIMixin, ReferenceShapesMixin, SettingsDialogsMixin,
		ShapeIOMixin, TextureWidgetsMixin, ViewportTransformMixin, ForgeryApp):
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
