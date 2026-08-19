"""Pure NumPy port of the color-shift algorithm real Panoply variants are
baked with -- nel/tools/3d/panoply_maker/color_modifier.cpp
(CColorModifier::evalBitmapStats/convertBitmap) plus the HLS conversions from
nel/src/misc/rgba.cpp (CRGBA::convertToHLS/buildFromHLS/CBGRA::blendFromui).
No I/O, no Panda3D dependency -- operates on plain uint8 NumPy arrays so it
can be unit-tested standalone; wiring this into object_editor's texture
resolution (PNMImage/Panda Texture) is a later step, see
.todo/forgery-object-editor.md "génération live des textures Panoply".

Not aiming for bit-exact parity with the C++ (game-asset colors, no
perceptible divergence expected -- accepted with the user). One deliberate
behavioral difference: evalBitmapStats's hue average uses a standard
intensity-weighted circular mean (sin/cos vector average) instead of
color_modifier.cpp's order-dependent running-unwrap trick -- the vectorized
form needs an order-independent formula, and this is the textbook way to
average circular quantities, not an approximation of the original.
"""

import numpy

# Panoply's blend weight for a fully-covering mask pixel -- see
# CBGRA::blendFromui's ">>8" divisor (coef is a uint8 mask value used as an
# 8-bit fixed-point fraction of 256, so a mask value of 255 -- not 256 -- is
# the practical "fully applied" ceiling).
_BLEND_DIVISOR = 256.0


def rgb_to_hls(rgb):
	"""rgb: float array [...,3] in [0, 1]. Returns (h[..., ] in [0, 360),
	l[...], s[...], achromatic[...] bool) -- port of CRGBA::convertToHLS."""
	r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
	max_v = numpy.maximum(numpy.maximum(r, g), b)
	min_v = numpy.minimum(numpy.minimum(r, g), b)
	achromatic = min_v == max_v

	l = 0.5 * (max_v + min_v)
	diff = max_v - min_v
	safe_diff = numpy.where(achromatic, 1.0, diff)

	s = numpy.where(
		l > 0.5,
		diff / numpy.clip(2.0 - max_v - min_v, 1e-8, None),
		diff / numpy.clip(max_v + min_v, 1e-8, None),
	)

	h = numpy.where(
		max_v == r, (g - b) / safe_diff,
		numpy.where(max_v == g, 2.0 + (b - r) / safe_diff, 4.0 + (r - g) / safe_diff),
	)
	h = h * 60.0
	h = numpy.where(h < 0.0, h + 360.0, h)

	h = numpy.where(achromatic, 0.0, h)
	s = numpy.where(achromatic, 0.0, s)
	return h, l, s, achromatic


def _hls_value(h, v1, v2):
	"""Port of rgba.cpp's file-local HLSValue() -- note this is a single
	conditional wrap, not a full modulo, exactly like the original."""
	h = numpy.where(h > 360.0, h - 360.0, numpy.where(h < 0.0, h + 360.0, h))
	return numpy.select(
		[h < 60.0, h < 180.0, h < 240.0],
		[v1 + (v2 - v1) * h / 60.0, v2, v1 + (v2 - v1) * (240.0 - h) / 60.0],
		default=v1,
	)


def hls_to_rgb(h, l, s):
	"""Port of CRGBA::buildFromHLS. Returns a float [...,3] array in [0, 1]."""
	l = numpy.clip(l, 0.0, 1.0)
	s = numpy.clip(s, 0.0, 1.0)
	v2 = numpy.where(l <= 0.5, l * (1.0 + s), l + s - l * s)
	v1 = 2.0 * l - v2
	achromatic = s == 0.0

	r = numpy.where(achromatic, l, numpy.clip(_hls_value(h + 120.0, v1, v2), 0.0, 1.0))
	g = numpy.where(achromatic, l, numpy.clip(_hls_value(h, v1, v2), 0.0, 1.0))
	b = numpy.where(achromatic, l, numpy.clip(_hls_value(h - 120.0, v1, v2), 0.0, 1.0))
	return numpy.stack([r, g, b], axis=-1)


def _to_uint8(float_pixels_0_255):
	"""(uint8) cast semantics from the C++ side -- truncation, not rounding,
	on values already clamped into [0, 255]."""
	return numpy.floor(numpy.clip(float_pixels_0_255, 0.0, 255.0)).astype(numpy.uint8)


def _brightness_contrast(intensity_u8, luminosity, contrast, mean_grey):
	"""Port of color_modifier.cpp's CalcBrightnessContrast()."""
	f_contrast = 0.01 * (contrast + 100.0)
	result = luminosity + mean_grey + f_contrast * (intensity_u8.astype(numpy.float64) - mean_grey)
	return numpy.clip(result, 0.0, 255.0)


def eval_bitmap_stats(rgb_u8, mask_u8):
	"""rgb_u8: HxWx3 uint8, mask_u8: HxW uint8 (mask weight in [0, 255]).
	Returns (H, S, L, grey) -- mask-weighted averages, H a circular mean in
	[0, 360), grey a float in [0, 255] (kept as float here -- callers that
	need the uint8 truncation from color_modifier.cpp's greyLevel should
	apply it themselves; convert_bitmap() below does).
	Port of CColorModifier::evalBitmapStats."""
	rgb_f = rgb_u8.astype(numpy.float64) / 255.0
	h, l, s, achromatic = rgb_to_hls(rgb_f)
	intensity = mask_u8.astype(numpy.float64) / 255.0

	weight = intensity.sum()
	grey_per_pixel = 0.299 * rgb_u8[..., 0] + 0.587 * rgb_u8[..., 1] + 0.114 * rgb_u8[..., 2]
	l_total = (intensity * l).sum()
	s_total = (intensity * s).sum()
	g_total = (intensity * grey_per_pixel.astype(numpy.float64)).sum()

	chromatic_weight = numpy.where(achromatic, 0.0, intensity)
	h_weight = chromatic_weight.sum()
	if h_weight != 0.0:
		h_rad = numpy.deg2rad(h)
		sin_sum = (chromatic_weight * numpy.sin(h_rad)).sum()
		cos_sum = (chromatic_weight * numpy.cos(h_rad)).sum()
		hue = numpy.degrees(numpy.arctan2(sin_sum, cos_sum))
		if hue < 0.0:
			hue += 360.0
	else:
		hue = 0.0

	saturation = s_total / weight if weight != 0.0 else 0.0
	lightness = l_total / weight if weight != 0.0 else 0.0
	grey = g_total / weight if weight != 0.0 else 0.0
	return float(hue), float(saturation), float(lightness), float(grey)


def convert_bitmap(current_rgb_u8, mask_u8, hue, lightness, saturation, luminosity, contrast):
	"""One axis's color shift, chained the same way panoply_maker.cpp applies
	successive masks in place (`cm.convertBitmap(resultBitmap, resultBitmap,
	masks[l].Mask, ...)`): pass the previous axis's output (or a copy of the
	base texture for the first axis) as current_rgb_u8, get back the next
	HxWx3 uint8 array to feed into the following axis. Alpha isn't part of
	this array -- callers keep the base texture's own alpha unchanged
	throughout, exactly like convertBitmap()'s `dest->A = src->A`.
	Port of CColorModifier::convertBitmap."""
	target_hue, _target_s, _target_l, grey_level = eval_bitmap_stats(current_rgb_u8, mask_u8)
	delta_h = hue - target_hue
	grey_level = float(int(grey_level))  # truncate like color_modifier.cpp's uint8 greyLevel

	rgb_f = current_rgb_u8.astype(numpy.float64) / 255.0
	h, l, s, achromatic = rgb_to_hls(rgb_f)
	h = numpy.where(achromatic, 0.0, h)

	shifted_rgb = hls_to_rgb(h + delta_h, l + lightness, s + saturation) * 255.0
	shifted_u8 = _to_uint8(shifted_rgb)

	contrasted = numpy.stack(
		[_brightness_contrast(shifted_u8[..., c], luminosity, contrast, grey_level) for c in range(3)],
		axis=-1,
	)
	contrasted_u8 = _to_uint8(contrasted)

	coef = mask_u8.astype(numpy.float64)[..., None]
	blended = numpy.floor(
		(current_rgb_u8.astype(numpy.float64) * (_BLEND_DIVISOR - coef) + contrasted_u8.astype(numpy.float64) * coef)
		/ _BLEND_DIVISOR
	)
	return _to_uint8(blended)


def colorize(base_rgba_u8, axis_masks):
	"""Applies convert_bitmap() once per (mask, params) in axis_masks, in
	order, chaining each axis's RGB output into the next -- same as
	panoply_maker.cpp applying successive masks onto one resultBitmap in
	place. axis_masks: an iterable of (mask_u8 HxW, params), params being
	anything with .hue/.lightness/.saturation/.luminosity/.contrast (e.g.
	panoply_config.ColorParams). Alpha is preserved from base_rgba_u8
	throughout, untouched by any axis -- matches convertBitmap()'s own
	`dest->A = src->A`."""
	rgb = base_rgba_u8[..., :3]
	for mask_u8, params in axis_masks:
		rgb = convert_bitmap(rgb, mask_u8, params.hue, params.lightness, params.saturation, params.luminosity, params.contrast)
	out = numpy.empty_like(base_rgba_u8)
	out[..., :3] = rgb
	out[..., 3] = base_rgba_u8[..., 3]
	return out
