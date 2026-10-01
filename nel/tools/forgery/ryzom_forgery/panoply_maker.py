"""Pure NumPy port of nel/src/misc/bitmap.cpp's CBitmap::resamplePicture32/
resamplePicture32Fast/resamplePicture8/resamplePicture8Fast -- the downscale
filter panoply_maker.cpp uses to build a .hlsinfo's low-def SrcBitmap/Masks
(BuildColoredVersionForOneBitmap step 2/4, see
/repos/project-todos/ryzom-core/panoply-runtime-tint.md "panoply_maker port
(Forgery)" for the full algorithm writeup). This is a separable 2-pass area
filter for minifying (our actual case, low_def_shift), a half-pixel-phase
linear filter for magnifying, plus an exact-2x fast path that rounds instead
of going through the general float pipeline -- resample() below picks the
right one exactly like the C++ does, since the fast path is not just a
performance shortcut: its integer rounding ("+1)>>2") gives a different
result than the general path's float truncation would for the same input.

Deliberately NOT reusing dds_export.py's _build_mip_chain/_reduce_size (a
different, simpler box filter) -- panoply_maker's masks/low-def source need
this exact filter to match real .hlsinfo output.

Not aiming for float bit-parity with the C++ (which mixes float32 and
float64 across branches) -- this module uses float64 throughout, matching
the "close enough that a DXT5 compress of the result looks right" bar
accepted for this port (unlike hls_texture_bank.py's writer, which achieved
actual byte-exact output). Confirm against real .hlsinfo once wired into the
rest of the pipeline.

Also has an exact (order-dependent) port of CColorModifier::convertBitmap/
evalBitmapStats (color_modifier.cpp), used for this offline generation
path -- confirmed with Nuno (2026-08-29): unlike panoply_colorize.py's
fast vectorized approximation (built for interactive live-preview, and
explicitly not aiming for bit-exact hue-average parity), this path runs
once per item and needs to match real .hlsinfo/.hlsbank output, so it pays
for a literal per-pixel port of the hue-average's running-mean-with-360deg-
unwrap correction instead -- see eval_bitmap_stats_exact()'s docstring.
"""

from dataclasses import dataclass, field
from typing import Iterator, List, Tuple

import numpy

from ryzom_forgery.panoply_colorize import _brightness_contrast, _to_uint8, hls_to_rgb, rgb_to_hls
from ryzom_forgery.panoply_config import ColorParams

# Panoply's blend weight divisor, see panoply_colorize.py's _BLEND_DIVISOR.
_BLEND_DIVISOR = 256.0


def _equal_weights(n: int) -> numpy.ndarray:
	return numpy.eye(n, dtype=numpy.float64)


def _magnify_weights(src_len: int, dest_len: int) -> numpy.ndarray:
	"""Port of resamplePicture32's bXMag branch: half-pixel-phase linear
	interpolation. Returns a (dest_len, src_len) weight matrix."""
	w = numpy.zeros((dest_len, src_len), dtype=numpy.float64)
	delta = src_len / dest_len
	for nx in range(dest_len):
		fx = nx * delta
		i0 = int(numpy.floor(fx))
		frac = fx - i0
		if frac >= 0.5:
			if fx < src_len - 1:
				w[nx, i0] += 1.5 - frac
				w[nx, i0 + 1] += frac - 0.5
			else:
				w[nx, i0] += 1.0
		else:
			if fx >= 1.0:
				w[nx, i0] += 0.5 + frac
				w[nx, i0 - 1] += 0.5 - frac
			else:
				w[nx, i0] += 1.0
	return w


def _minify_weights(src_len: int, dest_len: int) -> numpy.ndarray:
	"""Port of resamplePicture32's minifying branch: true area-average over
	src/dest-sized windows. Returns a (dest_len, src_len) weight matrix."""
	w = numpy.zeros((dest_len, src_len), dtype=numpy.float64)
	delta = src_len / dest_len
	fx = 0.0
	for nx in range(dest_len):
		final = fx + delta
		while fx < final and int(fx) != src_len:
			nxt = numpy.floor(fx) + 1.0
			if nxt > final:
				nxt = final
			idx = int(numpy.floor(fx))
			w[nx, idx] += nxt - fx
			fx = nxt
		fx = final
		w[nx, :] /= delta
	return w


def _axis_weights(src_len: int, dest_len: int) -> numpy.ndarray:
	if dest_len == src_len:
		return _equal_weights(src_len)
	if dest_len >= src_len:
		return _magnify_weights(src_len, dest_len)
	return _minify_weights(src_len, dest_len)


def _resample_fast(pixels: numpy.ndarray, dest_w: int, dest_h: int) -> numpy.ndarray:
	"""Port of resamplePicture32Fast/resamplePicture8Fast: exact 2x box
	reduction with integer rounding (avg4's "+1)>>2"), not the general
	float pipeline -- see module docstring for why this must stay separate."""
	src = pixels.astype(numpy.uint32)
	a = src[0::2, 0::2]
	b = src[1::2, 0::2]
	c = src[0::2, 1::2]
	d = src[1::2, 1::2]
	return ((a + b + c + d + 1) >> 2).astype(numpy.uint8)


def resample(pixels: numpy.ndarray, dest_w: int, dest_h: int) -> numpy.ndarray:
	"""pixels: (H, W) or (H, W, C) uint8 array. Returns a (dest_h, dest_w[, C])
	uint8 array, same rank as the input. Port of CBitmap::resamplePicture32
	(RGBA/multi-channel) and resamplePicture8 (single-channel) -- both use
	the exact same separable filter, so one implementation covers both."""
	src_h, src_w = pixels.shape[:2]
	if dest_w <= 0 or dest_h <= 0 or src_w <= 0 or src_h <= 0:
		raise ValueError(f"invalid resample dimensions: src={src_w}x{src_h} dest={dest_w}x{dest_h}")

	if (src_w // 2 == dest_w and src_w % 2 == 0) and (src_h // 2 == dest_h and src_h % 2 == 0):
		return _resample_fast(pixels, dest_w, dest_h)

	weights_x = _axis_weights(src_w, dest_w)  # (dest_w, src_w)
	weights_y = _axis_weights(src_h, dest_h)  # (dest_h, src_h)

	f = pixels.astype(numpy.float64)
	if f.ndim == 2:
		f = f[..., None]

	# X pass: (src_h, src_w, C) -> (src_h, dest_w, C), keeping full src height.
	interm = numpy.einsum("dw,hwc->hdc", weights_x, f)
	# Y pass: (src_h, dest_w, C) -> (dest_h, dest_w, C).
	out = numpy.einsum("eh,hdc->edc", weights_y, interm)

	out_u8 = numpy.floor(numpy.clip(out, 0.0, 255.0)).astype(numpy.uint8)
	if pixels.ndim == 2:
		out_u8 = out_u8[..., 0]
	return out_u8


def eval_bitmap_stats_exact(rgb_u8: numpy.ndarray, mask_u8: numpy.ndarray):
	"""Exact (order-dependent) port of CColorModifier::evalBitmapStats
	(color_modifier.cpp:93-156) -- used for the offline generation path,
	where matching real production .hlsinfo/.hlsbank output exactly matters
	and this runs once per item, not interactively. See
	panoply_colorize.eval_bitmap_stats() for the fast vectorized
	approximation used by the live-preview path instead, and the module
	docstring there for why the two can diverge.

	Only the hue average is genuinely order-dependent (each pixel's 360degree
	wraparound correction is relative to the running mean of every pixel
	processed so far in raster order) -- L/S/grey are plain weighted sums,
	so those still use panoply_colorize's vectorized rgb_to_hls() for the
	per-pixel HLS values themselves, only the hue accumulation loop is a
	literal pixel-by-pixel port, same as the C++.

	rgb_u8: HxWx3 uint8. mask_u8: HxW uint8 (mask weight in [0, 255]).
	Returns (H, S, L, grey) -- H a degrees float in [0, 360), grey a
	*truncated* int in [0, 255] (matches the C++'s uint8 greyLevel out
	param, unlike panoply_colorize's float grey)."""
	rgb_f = rgb_u8.astype(numpy.float64) / 255.0
	h, l, s, achromatic = rgb_to_hls(rgb_f)
	intensity = mask_u8.astype(numpy.float64) / 255.0

	weight = intensity.sum()
	grey_per_pixel = 0.299 * rgb_u8[..., 0] + 0.587 * rgb_u8[..., 1] + 0.114 * rgb_u8[..., 2]
	l_total = (intensity * l).sum()
	s_total = (intensity * s).sum()
	g_total = (intensity * grey_per_pixel.astype(numpy.float64)).sum()

	h_flat = h.ravel(order="C")  # row-major: matches the C++'s y-outer/x-inner loop order
	achromatic_flat = achromatic.ravel(order="C")
	intensity_flat = intensity.ravel(order="C")

	h_total = 0.0
	h_weight = 0.0
	for hv, is_achromatic, iv in zip(h_flat.tolist(), achromatic_flat.tolist(), intensity_flat.tolist()):
		if is_achromatic:
			continue
		if h_weight != 0.0:
			h_mean = h_total / h_weight
			if abs(hv - 360.0 - h_mean) < abs(hv - h_mean):
				hv -= 360.0
		h_total += hv * iv
		h_weight += iv

	hue = h_total / h_weight if h_weight != 0.0 else 0.0
	if hue < 0.0:
		hue += 360.0
	saturation = s_total / weight if weight != 0.0 else 0.0
	lightness = l_total / weight if weight != 0.0 else 0.0
	grey = int(g_total / weight) if weight != 0.0 else 0  # truncation, matches (uint8) cast
	return hue, saturation, lightness, grey


def convert_bitmap_exact(current_rgb_u8: numpy.ndarray, mask_u8: numpy.ndarray, hue: float,
						  lightness: float, saturation: float, luminosity: float, contrast: float):
	"""Exact-hue-stats counterpart to panoply_colorize.convert_bitmap() --
	same per-pixel recolor/contrast/blend (all order-independent, so those
	stay vectorized), but measures the axis's hue delta with
	eval_bitmap_stats_exact() instead of the fast approximation. Port of
	CColorModifier::convertBitmap (color_modifier.cpp:30-90); see that
	function's docstring counterpart in panoply_colorize.py for the shared
	per-pixel math. Returns (result_rgb_u8, delta_hue) -- delta_hue is the
	C++'s retDeltaHue out-param, needed by callers building a .hlsinfo
	TextureInstance's Mods."""
	target_hue, _target_s, _target_l, grey_level = eval_bitmap_stats_exact(current_rgb_u8, mask_u8)
	delta_h = hue - target_hue

	rgb_f = current_rgb_u8.astype(numpy.float64) / 255.0
	h, l, s, achromatic = rgb_to_hls(rgb_f)
	h = numpy.where(achromatic, 0.0, h)

	shifted_rgb = hls_to_rgb(h + delta_h, l + lightness, s + saturation) * 255.0
	shifted_u8 = _to_uint8(shifted_rgb)

	contrasted = numpy.stack(
		[_brightness_contrast(shifted_u8[..., c], luminosity, contrast, float(grey_level)) for c in range(3)],
		axis=-1,
	)
	contrasted_u8 = _to_uint8(contrasted)

	coef = mask_u8.astype(numpy.float64)[..., None]
	blended = numpy.floor(
		(current_rgb_u8.astype(numpy.float64) * (_BLEND_DIVISOR - coef) + contrasted_u8.astype(numpy.float64) * coef)
		/ _BLEND_DIVISOR
	)
	return _to_uint8(blended), delta_h


def colorize_exact(base_rgba_u8: numpy.ndarray, axis_masks) -> numpy.ndarray:
	"""Exact-hue-stats counterpart to panoply_colorize.colorize() -- chains
	convert_bitmap_exact() once per (mask, params) in axis_masks, in order,
	same as panoply_maker.cpp applying successive masks onto one
	resultBitmap in place. See colorize()'s docstring for the argument
	shape (unchanged here). Discards delta_hue -- use
	generate_color_combinations() instead when a .hlsinfo needs to be built
	from the result."""
	rgb = base_rgba_u8[..., :3]
	for mask_u8, params in axis_masks:
		rgb, _delta_hue = convert_bitmap_exact(rgb, mask_u8, params.hue, params.lightness, params.saturation, params.luminosity, params.contrast)
	out = numpy.empty_like(base_rgba_u8)
	out[..., :3] = rgb
	out[..., 3] = base_rgba_u8[..., 3]
	return out


@dataclass
class ColorMaskAxis:
	"""One mask extension's full set of configured colors -- port of
	panoply_maker.cpp's CColorMask (TColorMaskVect's element type)."""

	mask_ext: str
	colors: List[ColorParams] = field(default_factory=list)


def build_masks_from_config(cfg) -> List[ColorMaskAxis]:
	"""Port of BuildMasksFromConfigFile (panoply_maker.cpp:418-455): reads
	`mask_extensions` and, for each extension `X`, its six parallel
	`X_hues`/`X_lightness`/`X_saturations`/`X_luminosities`/`X_constrasts`
	(sic, matches the real config key's typo)/`X_color_id` arrays.

	`cfg` is anything with a `.get(name) -> list` method -- a
	`pynel.config_file.Document` or `ConfigFile` in practice, kept
	duck-typed here so this module doesn't need to depend on pynel."""
	mask_extensions = cfg.get("mask_extensions")
	axes = []
	for mask_ext in mask_extensions:
		mask_ext = str(mask_ext)
		hues = cfg.get(f"{mask_ext}_hues")
		lightness = cfg.get(f"{mask_ext}_lightness")
		saturations = cfg.get(f"{mask_ext}_saturations")
		luminosities = cfg.get(f"{mask_ext}_luminosities")
		contrasts = cfg.get(f"{mask_ext}_constrasts")
		color_ids = cfg.get(f"{mask_ext}_color_id")

		lengths = {len(hues), len(lightness), len(saturations), len(luminosities), len(contrasts), len(color_ids)}
		if len(lengths) != 1:
			raise ValueError(f"all color descriptors for mask '{mask_ext}' must have the same number of arguments")

		colors = [
			ColorParams(
				id=str(color_ids[i]), hue=float(hues[i]), lightness=float(lightness[i]),
				saturation=float(saturations[i]), luminosity=float(luminosities[i]), contrast=float(contrasts[i]),
			)
			for i in range(len(color_ids))
		]
		axes.append(ColorMaskAxis(mask_ext=mask_ext, colors=colors))
	return axes


@dataclass
class ActiveMask:
	"""One mask actually found on disk for a given source texture -- port of
	panoply_maker.cpp's CLoopInfo (minus the Counter field, which
	generate_color_combinations() below tracks itself instead of mutating
	shared state, unlike the C++'s reused-across-calls static vector)."""

	axis: ColorMaskAxis
	mask_u8: numpy.ndarray  # HxW uint8, full source resolution


def generate_color_combinations(
	base_rgba_u8: numpy.ndarray, active_masks: List[ActiveMask], default_separator: str = "_",
) -> Iterator[Tuple[str, numpy.ndarray, List[Tuple[float, float, float]]]]:
	"""Port of BuildColoredVersionForOneBitmap's combinatorial loop
	(panoply_maker.cpp:790-880), minus file I/O -- that's left to a caller,
	same split as panoply_colorize.py (pure math) / panoply_texture.py
	(Panda3D glue). For every combination of color IDs across
	`active_masks` (multi-radix odometer, last mask's counter increments
	fastest -- matches the C++'s counter-increment order exactly), yields
	`(name_suffix, result_rgba_u8, mods)`:

	- `name_suffix`: e.g. `"_C1_U3"` for a 2-mask item, in active-mask
	  order -- append to the base file name for the output file name AND
	  the `.hlsinfo` TextureInstance name (matches
	  `outputFileName = fileName + sep + ColID...`).
	- `result_rgba_u8`: the recolored HxWx4 uint8 image for this
	  combination, alpha preserved from `base_rgba_u8`.
	- `mods`: one `(d_hue, d_lum, d_sat)` tuple per active mask, in order --
	  build a `pynel.hls_bank_texture_info.HLSMod`/`TextureInstance` from
	  these directly (`d_lum`/`d_sat` are the configured
	  lightness/saturation verbatim, `d_hue` is convert_bitmap_exact()'s
	  measured delta).

	With no active masks (item has no colorisable axis), yields a single
	`("", base_rgba_u8.copy(), [])` -- matches the C++ still writing one
	(uncolorized) output file in that case."""
	if not active_masks:
		yield "", base_rgba_u8.copy(), []
		return

	counters = [0] * len(active_masks)
	while True:
		rgb = base_rgba_u8[..., :3].copy()
		suffix = ""
		mods = []
		for i, am in enumerate(active_masks):
			color = am.axis.colors[counters[i]]
			rgb, delta_hue = convert_bitmap_exact(rgb, am.mask_u8, color.hue, color.lightness, color.saturation, color.luminosity, color.contrast)
			mods.append((delta_hue, color.lightness, color.saturation))
			suffix += default_separator + color.id

		result = numpy.empty_like(base_rgba_u8)
		result[..., :3] = rgb
		result[..., 3] = base_rgba_u8[..., 3]
		yield suffix, result, mods

		# multi-radix odometer: last mask's counter increments fastest, carries left
		i = len(active_masks) - 1
		while i >= 0:
			counters[i] += 1
			if counters[i] == len(active_masks[i].axis.colors):
				counters[i] = 0
				i -= 1
			else:
				break
		if i < 0:
			break
