"""Writes .dds files the Ryzom client can load, from an RGBA bitmap --
Panda3D-based counterpart of nel/tools/3d/tga_2_dds/tga2dds.cpp +
nel/tools/3d/s3tc_compressor_lib/s3tc_compressor.cpp.

Not aiming for bit-exact parity with the original libsquish-based
compressor: that library is a 20-year-old external dependency
(CMakeModules/FindSquish.cmake), its exact historical version/fork is
unrecoverable, and its internal colour-quantization choices don't need to
match -- only the DDS container and the DXT block format need to be
correct, since that's the only part the client's DDS loader
(CBitmap::readDDS, nel/src/misc/bitmap.cpp:530) actually parses. Panda3D's
own DXT compressor (Texture.compress_ram_image) produces standard
block-conformant DXT1/DXT3/DXT5 data -- verified on the real machine
(2026-08-27) to give the expected block-count-based sizes per mip level.

DDS_HEADER/DDS_PIXELFORMAT layout matches s3tc_compressor.h:66-96
field-for-field; the exact field offsets were cross-checked against
CBitmap::readDDS's index-based reads (bitmap.cpp:568-611:
_DDSSurfaceDesc[2]/[3]=height/width, [6]=mipmap count, [19]/[20]/[21]=
ddpf.dwFlags/dwFourCC/dwRGBBitCount).

One known incompleteness deliberately kept for compatibility: like the
original tga2dds.cpp (s3tc_compressor.cpp:46, "TODO: add special headers
flags for DXTC1a"), DXT1A textures are written with the same FourCC/header
as plain DXT1 (dwRGBBitCount stays 0) -- the client only distinguishes
them heuristically at load time, not through this header field.

Mipmap chain downsampling matches CBitmap::buildMipMaps() (bitmap.cpp:1749)
-- a deterministic 2x2 box filter, silently producing just the base level
for non-power-of-2 dimensions, same as the original -- rather than using
Panda3D's own mipmap generation, for visual parity with existing game
assets. Each level is then compressed independently via Panda3D.

Observed production convention for "_spec" (specular) maps (cross-checked
against several real `*_spec.dds` in ryzom-data/final_bnps, e.g.
objects/ge_partylamps_spec.dds, fauna_maps/tr_mo_kamiguard_spec.dds,
fauna_maps/fo_canitree_spec.dds -- all DXT1, full mipmap chain, no alpha
usage, i.e. `build_dds(rgba, DXT1, build_mipmaps=True)`): the source is a
pure single-channel (grayscale) image, and must be loaded with
`grayscale_as_luminance=True` (`-g`) so the intensity ends up in RGB where
a specular shader actually samples it, not smuggled into an alpha channel
DXT1 doesn't meaningfully carry here. Used on 2026-08-27 to fix three
`_spec.dds` files that had been mistakenly saved as plain PNGs with a
`.dds` extension (objects/occ_stuff/anlor/halloween_mo_statue_0{1,2,3}_spec.dds).
"""

import struct

import numpy
from panda3d.core import Texture

DXT1 = 1
DXT1A = 11
DXT3 = 3
DXT5 = 5

_DDSD_CAPS = 0x00000001
_DDSD_WIDTH = 0x00000004
_DDSD_HEIGHT = 0x00000002
_DDSD_PIXELFORMAT = 0x00001000
_DDSD_MIPMAPCOUNT = 0x00020000
_DDSD_LINEARSIZE = 0x00080000

_DDPF_FOURCC = 0x00000004

_DDSCAPS_TEXTURE = 0x00001000
_DDSCAPS_MIPMAP = 0x00400000

_ALGO_FOURCC = {
	DXT1: b"DXT1",
	DXT1A: b"DXT1",
	DXT3: b"DXT3",
	DXT5: b"DXT5",
}

_ALGO_COMPRESSION_MODE = {
	DXT1: Texture.CM_dxt1,
	DXT1A: Texture.CM_dxt1,
	DXT3: Texture.CM_dxt3,
	DXT5: Texture.CM_dxt5,
}


def _is_power_of_2(n: int) -> bool:
	return n > 0 and (n & (n - 1)) == 0


def _build_mip_chain(rgba: numpy.ndarray) -> list:
	"""Port of CBitmap::buildMipMaps() (bitmap.cpp:1749)."""
	height, width = rgba.shape[:2]
	levels = [rgba]
	if not (_is_power_of_2(width) and _is_power_of_2(height)):
		return levels

	w, h = width, height
	while w > 1 or h > 1:
		prev = levels[-1].astype(numpy.uint16)
		prev_w, prev_h = w, h
		w = (w + 1) // 2
		h = (h + 1) // 2
		mul_w = prev_w // w
		mul_h = prev_h // h

		i0 = numpy.arange(h) * mul_h
		i1 = i0 if mul_h == 1 else i0 + 1
		j0 = numpy.arange(w) * mul_w
		j1 = j0 if mul_w == 1 else j0 + 1

		c0 = prev[numpy.ix_(i0, j0)]
		c1 = prev[numpy.ix_(i0, j1)]
		c2 = prev[numpy.ix_(i1, j0)]
		c3 = prev[numpy.ix_(i1, j1)]
		level = ((c0 + c1 + c2 + c3 + 2) // 4).astype(numpy.uint8)
		levels.append(level)

	return levels


def _reduce_size(rgba: numpy.ndarray, factor: int) -> numpy.ndarray:
	"""Port of dividSize() (tga2dds.cpp:223): halves width/height `factor`
	times via a floor-divide-by-4 2x2 box filter -- deliberately no +2
	rounding (unlike _build_mip_chain's box filter), to match the
	original's `>>2` exactly."""
	for _ in range(factor):
		height, width = rgba.shape[:2]
		new_width = width // 2
		new_height = height // 2
		src = rgba[:new_height * 2, :new_width * 2].astype(numpy.uint16)
		c0 = src[0::2, 0::2]
		c1 = src[0::2, 1::2]
		c2 = src[1::2, 0::2]
		c3 = src[1::2, 1::2]
		rgba = ((c0 + c1 + c2 + c3) // 4).astype(numpy.uint8)
	return rgba


def _compress_level(rgba: numpy.ndarray, algo: int) -> bytes:
	height, width = rgba.shape[:2]
	tex = Texture()
	tex.setup_2d_texture(width, height, Texture.T_unsigned_byte, Texture.F_rgba)
	tex.set_ram_image_as(numpy.ascontiguousarray(rgba).tobytes(), "RGBA")
	if not tex.compress_ram_image(_ALGO_COMPRESSION_MODE[algo]):
		raise RuntimeError(f"panda3d failed to DXT-compress a {width}x{height} level")
	return bytes(tex.get_ram_image())


def _build_dds_header(width: int, height: int, mipmap_count: int, algo: int, base_level_size: int) -> bytes:
	fourcc = _ALGO_FOURCC[algo]
	pixelformat = struct.pack(
		"<8I",
		32,  # dwSize (sizeof DDS_PIXELFORMAT)
		_DDPF_FOURCC,  # dwFlags
		int.from_bytes(fourcc, "little"),  # dwFourCC
		0, 0, 0, 0, 0,  # dwRGBBitCount, dwR/G/B/ABitMask
	)
	flags = _DDSD_CAPS | _DDSD_WIDTH | _DDSD_HEIGHT | _DDSD_PIXELFORMAT | _DDSD_LINEARSIZE | _DDSD_MIPMAPCOUNT
	caps = _DDSCAPS_TEXTURE | _DDSCAPS_MIPMAP
	header = struct.pack(
		"<18I",
		124,  # dwSize (sizeof DDS_HEADER)
		flags,
		height,
		width,
		base_level_size,  # dwLinearSize
		0,  # dwDepth
		mipmap_count,
		*([0] * 11),  # dwReserved1
	)
	header += pixelformat
	header += struct.pack("<5I", caps, 0, 0, 0, 0)  # dwCaps, dwCaps2, dwCaps3, dwCaps4, dwReserved2
	return header


def build_dds(rgba: numpy.ndarray, algo: int, build_mipmaps: bool = False, reduce: int = 0) -> bytes:
	"""Assemble a full .dds file (bytes) from an HxWx4 uint8 RGBA array.

	`algo` is one of DXT1/DXT1A/DXT3/DXT5. `reduce` (0-8) shrinks the
	bitmap before compression, same as tga2dds.cpp's `-r`. Matches the
	container format written by s3tc_compressor.cpp, with each mip level's
	DXT blocks compressed via Panda3D instead of libsquish (see module
	docstring).
	"""
	if algo not in _ALGO_FOURCC:
		raise ValueError(f"unsupported DXT algo: {algo}")

	if reduce:
		rgba = _reduce_size(rgba, min(reduce, 8))

	height, width = rgba.shape[:2]
	levels = _build_mip_chain(rgba) if build_mipmaps else [rgba]
	compressed_levels = [_compress_level(level, algo) for level in levels]

	header = _build_dds_header(width, height, len(compressed_levels), algo, len(compressed_levels[0]))
	return b"DDS " + header + b"".join(compressed_levels)


def load_rgba(path: str, grayscale_as_luminance: bool = False) -> numpy.ndarray:
	"""Loads `path` into an HxWx4 uint8 RGBA array in top-down row order
	(row 0 = top of the image, matching both PNMImage and the DDS spec).

	Texture.load(PNMImage) flips rows internally (Panda's RAM image is
	bottom-up, like OpenGL) -- see panoply_texture.py's docstring, which
	verified this same flip. That's invisible to per-pixel colorize
	operations, but this module writes raw rows straight into a DDS file
	(top-down), so the flip must be undone here before anything else.

	If the source is a pure single-channel (grayscale) image,
	`grayscale_as_luminance` picks how it's interpreted, matching
	CBitmap::_LoadGrayscaleAsAlpha (bitmap.h/cpp) as used by tga2dds.cpp's
	`-g`: default (False) treats it as an alpha-only mask -- RGB forced to
	white, alpha = the grey value (CBitmap::alphaToRGBA, bitmap.cpp:806) --
	True treats it as visible luminance -- RGB = grey repeated, alpha = 255
	(CBitmap::luminanceToRGBA, bitmap.cpp:778), which is what Panda3D's own
	PNMImage->Texture load already produces, so nothing to undo in that
	case."""
	from panda3d.core import PNMImage

	image = PNMImage()
	if not image.read(path):
		raise RuntimeError(f"can't read image: {path}")
	is_pure_grayscale = image.get_num_channels() == 1

	tex = Texture()
	tex.load(image)
	width, height = tex.get_x_size(), tex.get_y_size()
	raw = tex.get_ram_image_as("RGBA")
	rgba = numpy.frombuffer(raw, dtype=numpy.uint8).reshape(height, width, 4)
	rgba = numpy.flipud(rgba).copy()

	if is_pure_grayscale and not grayscale_as_luminance:
		grey = rgba[..., 0].copy()
		rgba[..., 0] = 255
		rgba[..., 1] = 255
		rgba[..., 2] = 255
		rgba[..., 3] = grey

	return rgba


def pick_default_algo(rgba: numpy.ndarray) -> int:
	"""Same default heuristic as tga2dds.cpp: DXT5 if the bitmap has any
	non-opaque pixel, DXT1 otherwise."""
	has_alpha = bool((rgba[..., 3] != 255).any())
	return DXT5 if has_alpha else DXT1
