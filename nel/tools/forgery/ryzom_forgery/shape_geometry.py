"""Shared geometry/material extraction for CMesh/CMeshMRM/CMeshMultiLod
`.shape` values -- used both by the live 3D editor (`apps/object_editor.py`)
and by the .shape exporters (`shape_export.py`), so the two don't each
maintain their own copy of "how to walk a parsed shape's render passes".
"""

import dataclasses

from panda3d.core import PNMImage, StringStream, Texture as PandaTexture

from pynel.ryzom_shape import Mesh, MeshGeom, MeshMRM, MeshMRMGeom, MeshMultiLod


def _passes_from_mesh_geom(geom: MeshGeom):
	for matrix_block in geom.matrix_blocks:
		for rdr_pass in matrix_block.rdr_passes:
			yield geom.vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def _resolve_lod_geomorphs(vertex_buffer, lod):
	"""Returns a copy of vertex_buffer with its geomorph placeholder wedges
	resolved to a static value for this lod.

	A CMeshMRM's progressive mesh reserves a block of empty wedges (position/
	normal/uv all zero, a default-constructed CWedge()) shared for geomorph
	blending between adjacent LODs -- wedge index i for i < len(lod.geomorphs)
	is one of these, meant to be filled at render time by
	CMeshMRMGeom::applyGeomorph, blending toward geomorphs[i]'s "end" wedge as
	detail increases (see mrm_builder.cpp's wedge-decal step). For a static,
	non-transitioning render of this lod (this tool doesn't animate LOD
	blending), the resolved value is always exactly that "end" wedge --
	without this, those vertices render collapsed at the origin.
	"""
	if not lod.geomorphs:
		return vertex_buffer

	channels = {}
	for name, values in vertex_buffer.channels.items():
		resolved = list(values)
		for wedge_index, (_start, end) in enumerate(lod.geomorphs):
			if end < len(values):
				resolved[wedge_index] = values[end]
		channels[name] = resolved

	return dataclasses.replace(vertex_buffer, channels=channels)


def _passes_from_mrm_geom(geom: MeshMRMGeom):
	# lods[-1], not lods[0]: CMeshMRM's progressive mesh is streamed
	# coarsest-first (see CMeshMRMGeom::loadFirstLod/loadNextLod in
	# nel/src/3d/mesh_mrm.cpp, and chooseLod's alphaMRM=0 -> numLod=0
	# mapping to the *lowest* poly count) -- lods[0] is the least detailed,
	# lods[-1] the most, matching MeshMRMGeom.num_triangles's own convention.
	if geom.lods:
		lod = geom.lods[-1]
		vertex_buffer = _resolve_lod_geomorphs(geom.vertex_buffer, lod)
		for rdr_pass in lod.rdr_passes:
			yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def iter_render_passes(shape_value):
	"""Yields (vertex_buffer, material_id, indices) for the renderable
	geometry of a CMesh/CMeshMRM/CMeshMultiLod(slot 0) shape value."""
	if isinstance(shape_value, Mesh):
		yield from _passes_from_mesh_geom(shape_value.geom)
	elif isinstance(shape_value, MeshMRM):
		yield from _passes_from_mrm_geom(shape_value.geom)
	elif isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		slot_geom = shape_value.slots[0].mesh_geom
		if isinstance(slot_geom, MeshGeom):
			yield from _passes_from_mesh_geom(slot_geom)
		elif isinstance(slot_geom, MeshMRMGeom):
			yield from _passes_from_mrm_geom(slot_geom)


def shape_bbox(shape_value):
	if isinstance(shape_value, (Mesh, MeshMRM)):
		return shape_value.bbox
	if isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		slot_geom = shape_value.slots[0].mesh_geom
		return slot_geom.bbox if slot_geom is not None else None
	return None


def rgba_to_color(rgba):
	return (rgba.r / 255.0, rgba.g / 255.0, rgba.b / 255.0, rgba.a / 255.0)


def load_panda_texture(asset_index, name, cache=None):
	"""Resolves and decodes a material texture reference (by base file name,
	as stored in the shape's Texture.file_name) into a Panda3D Texture, via
	the given AssetIndex. Returns None if it can't be found/decoded. `cache`,
	if given, is a dict reused across calls to avoid re-decoding the same
	texture for multiple materials/passes."""
	if cache is not None and name in cache:
		return cache[name]

	texture = None
	ref = asset_index.find_texture(name)
	if ref is None:
		print(f"[shape_geometry] texture not found in asset index: {name!r}")
	else:
		try:
			data = ref.read_bytes()
		except OSError as exc:
			print(f"[shape_geometry] failed to read {ref.name!r}: {exc}")
			data = None

		if data is not None and ref.name.lower().endswith(".dds"):
			# DDS is a compressed/GPU-native format; PNMImage can't read it,
			# Texture has a dedicated loader for it.
			texture = PandaTexture()
			if not texture.read_dds(StringStream(data), ref.name, False):
				print(f"[shape_geometry] failed to decode DDS texture {ref.name!r}")
				texture = None
		elif data is not None:
			image = PNMImage()
			if image.read(StringStream(data), ref.name):
				# NeL stores texture V with the opposite origin from
				# Panda3D; without this the texture renders upside-down.
				image.flip(False, True, False)
				texture = PandaTexture()
				texture.load(image)
			else:
				print(f"[shape_geometry] failed to decode texture {ref.name!r}")

	if cache is not None:
		cache[name] = texture
	return texture


def texture_to_pnm_image(panda_texture):
	"""Reads a Panda3D Texture's pixel data back out into a PNMImage (e.g.
	to save it as a plain .png), regardless of what format it was originally
	decoded from (tga/png/dds all end up as a normal Texture)."""
	image = PNMImage()
	panda_texture.store(image)
	return image
