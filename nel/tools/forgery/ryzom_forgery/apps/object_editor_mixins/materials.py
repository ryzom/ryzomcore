"""ObjectEditorApp mixin: material apply/reapply (the 3D-render side),
material color/texture editing (Materials/Textures tabs), and the
Single/Multi/specular conversion helpers. Split out of object_editor.py, see
the "Split object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui
from panda3d.core import (
	AlphaTestAttrib, ColorBlendAttrib, DepthTestAttrib, Material as PandaMaterial, RenderAttrib,
	Texture as PandaTexture, TextureStage, TransformState,
)

from pynel.ryzom_shape import Rgba, Texture

from ryzom_forgery.shape_geometry import (
	compose_uv_matrix, decompose_uv_matrix, load_panda_cube_texture, load_panda_texture, resolve_texture_ref,
	rgba_to_color, uv_matrix_to_panda_mat4,
)
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_icon_button, _IDRV_MAT_SHADER_SPECULAR, _STATUS_HINT_COLOR, _TEXTURE_IN_WORKSPACE_COLOR, _TEXTURE_NORMAL_COLOR,
)

# CMaterial flag/enum values (nel/include/nel/3d/material.h), needed to render
# translucent materials (e.g. glass) correctly instead of opaque.
_IDRV_MAT_ZWRITE = 0x00000004
_IDRV_MAT_BLEND = 0x00000080
_IDRV_MAT_DOUBLE_SIDED = 0x00000100
_IDRV_MAT_ALPHA_TEST = 0x00000200
_IDRV_MAT_TEX_ADDR = 0x00000400
# CMaterial::TShader (material.h) -- Material.shader_type's value, not a flag
# bit. _IDRV_MAT_SHADER_SPECULAR itself lives in
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


class MaterialsMixin:
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
		"""Switching TO Specular with no stage-1 texture yet creates an
		empty CTextureCube there (6 blank faces) -- a shape-wide scan across
		the whole indexed data (2026-09-02, 6559 shapes, 988 real Specular
		materials found) confirmed every single one uses a cube map, none a
		flat file, so cube is the only default actually backed by real
		content. The 6 faces are kept in sync as one unit by
		_set_specular_material_texture()/_draw_multi_bitmap_editor()'s own
		specular-slot handling, same as real content (which always
		duplicates the same file across all 6 faces rather than genuinely
		varying per-face). Each face gets `file_names=[]` (not the dataclass
		default of None) even though this material might not be Multi
		Bitmap yet -- _draw_multi_bitmap_editor()'s own specular-slot sync
		(_set_specular_slot()) always indexes/appends into face.file_names
		unconditionally once *any* Multi Bitmap editing happens on this
		material, regardless of the face's own class_name; a bare None
		there crashed with a TypeError the first time that ran (found
		2026-09-02) since nothing else ever initializes it first."""
		material.shader_type = shader_type
		if shader_type == _IDRV_MAT_SHADER_SPECULAR:
			while len(material.textures) < 2:
				material.textures.append(None)
			if material.textures[1] is None:
				material.textures[1] = Texture(
					class_name="CTextureCube",
					sub_textures=[
						Texture(class_name="CTextureFile", file_name="", file_names=[]) for _ in range(6)])
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
				section_hint = self._draw_material_section(
					material_id, "lighting", "Lighting", self._draw_material_lighting_section, material)
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

	def _draw_material_lighting_section(self, material_id, material):
		"""CMaterial's ambient/specular/emissive/shininess fields -- already
		read and applied to the Panda3D preview (_apply_material_common()),
		but never editable until now: with the scene's only 2 lights fixed/
		uncontrollable (see the Lighting viewport panel,
		viewport_transform.py's _draw_light_controls()), there was no way to
		actually see any of them react. `ambient` only reacts to the
		scene's AmbientLight; `specular`/`shininess` only to the sun (a
		directional light); `emissive` never reacts to any light at all
		(self-illumination, always visible regardless of scene lighting).
		Alpha is preserved as-is on write -- these 3 colors don't use it for
		anything CMaterial-side, only RGB is meaningful here."""
		hint = None

		for field_name, label, doc_key in (
			("ambient", "Ambient", "basic-colors"),
			("specular", "Specular", "specular-glossiness"),
			("emissive", "Emissive", "self-illumination"),
		):
			current = rgba_to_color(getattr(material, field_name))[:3]
			imgui.set_next_item_width(180)
			changed, new_color = imgui.color_edit3(label, current)
			hint = self._doc_hint_if_hovered(doc_key) or hint
			if changed:
				existing = getattr(material, field_name)
				r, g, b = new_color
				setattr(material, field_name, Rgba(round(r * 255), round(g * 255), round(b * 255), existing.a))
				self._reapply_material(material_id)

		imgui.set_next_item_width(180)
		changed, shininess = imgui.slider_float("Shininess", material.shininess, 0.0, 128.0)
		hint = self._doc_hint_if_hovered("specular-glossiness") or hint
		if changed:
			material.shininess = shininess
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
			self._draw_texture_preview_static(current_specular, badge="S", str_id="##preview-specular")
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

			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##specular", "Browse for a specular/gloss map file"):
				def _on_specular_result(file_name, material_id=material_id, material=material):
					self._set_specular_material_texture(material, file_name)
					self._reapply_material(material_id)
				self._start_texture_browse(("specular", material_id), _on_specular_result)
			imgui.same_line()

			def _on_specular_copied(new_name, material_id=material_id, material=material):
				self._set_specular_material_texture(material, new_name)
				self._reapply_material(material_id)
			self._draw_texture_copy_button(current_specular, _on_specular_copied, str_id_suffix="##specular")

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
