"""Panda3D glue for live Panoply recoloring (Phase A Step 4, see
.todo/forgery-object-editor.md): decodes a resolved texture reference into
the HxWx4 uint8 RGBA array panoply_colorize.py's functions expect, and
builds a Panda3D Texture back from a recolored result. No disk I/O of its
own -- `ref` is anything with .name/.read_bytes() (search_paths.FoundEntry
duck type, same as shape_geometry.load_panda_texture()'s `finder` result).

Row order is whatever Texture.get_ram_image_as()/set_ram_image_as() use
internally -- verified (on the real machine, sampling real texture files)
that the two agree with each other and round-trip losslessly, but not what
that order actually is relative to PNMImage's own top-down convention (it
isn't -- get_ram_image_as()'s row 0 is PNMImage's *last* row). That's never
relied on here since panoply_colorize's operations are all per-pixel, never
spatial, so a consistent whole-image flip between extraction and rebuild is
invisible to them.
"""

from typing import Optional

import numpy
from panda3d.core import PNMImage, StringStream, Texture as PandaTexture


def ref_to_rgba_array(ref) -> Optional[numpy.ndarray]:
	"""Decodes `ref`'s bytes into an HxWx4 uint8 array via a throwaway Panda
	Texture, or None if it can't be read/decoded. Not meant for .dds (no
	real Panoply base texture or mask ever is one)."""
	try:
		data = ref.read_bytes()
	except OSError:
		return None
	image = PNMImage()
	if not image.read(StringStream(data), ref.name):
		return None
	texture = PandaTexture()
	texture.load(image)
	width, height = texture.get_x_size(), texture.get_y_size()
	raw = texture.get_ram_image_as("RGBA")
	return numpy.frombuffer(raw, dtype=numpy.uint8).reshape(height, width, 4).copy()


def rgba_array_to_texture(rgba_array: numpy.ndarray) -> PandaTexture:
	"""Builds a new Panda3D Texture from an HxWx4 uint8 array -- the
	counterpart of ref_to_rgba_array(), see this module's docstring on row
	order."""
	height, width = rgba_array.shape[:2]
	texture = PandaTexture()
	texture.setup_2d_texture(width, height, PandaTexture.T_unsigned_byte, PandaTexture.F_rgba)
	texture.set_ram_image_as(numpy.ascontiguousarray(rgba_array).tobytes(), "RGBA")
	return texture
