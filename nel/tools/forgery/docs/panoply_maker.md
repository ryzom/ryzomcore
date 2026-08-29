# `panoply_maker.py` — offline panoply generation port

Status: **in progress**. Python port of `nel/tools/3d/panoply_maker/panoply_maker.cpp`
-- the offline tool that bakes an item's colorized texture variants (and the
`.hlsinfo` used to build `characters.hlsbank`, see pynel's
`docs/hls_texture_bank.md`) from a base texture + per-axis masks + a
`panoply_*.cfg`. See `/repos/project-todos/ryzom-core/panoply-runtime-tint.md`
"panoply_maker port (Forgery)" for the full chantier status and algorithm
notes this doc summarizes.

## Why this is separate from `panoply_colorize.py`

`ryzom_forgery/panoply_colorize.py` already ports the same underlying
per-pixel color-shift math (`CColorModifier::convertBitmap`/
`evalBitmapStats`), but for a different use case: object_editor's live
Panoply preview, where speed matters and its docstring explicitly says
"not aiming for bit-exact parity" -- its hue average uses a vectorized
order-independent circular mean (sin/cos) instead of the real algorithm's
order-dependent running-mean-with-360°-unwrap correction.

For offline generation, this divergence matters: only `DHue` is *measured*
(`DLum`/`DSat` are copied straight from the config), so a diverging hue
average changes the `.hlsinfo` this tool writes, breaking byte-exact
cross-validation against the real `panoply_maker` binary. Confirmed with
Nuno (2026-08-29): keep `panoply_colorize.py`'s fast approximation for the
interactive live-preview path, and use this module's exact port for
generation (runs once per item, speed doesn't matter).

## What's ported so far

- **`resample()`**: port of `CBitmap::resamplePicture32`/
  `resamplePicture32Fast`/`resamplePicture8`/`resamplePicture8Fast`
  (`nel/src/misc/bitmap.cpp`) -- the separable area-average/half-pixel
  filter `panoply_maker` uses to build a `.hlsinfo`'s low-def `SrcBitmap`/
  `Masks`. Deliberately **not** `dds_export.py`'s `_build_mip_chain`/
  `_reduce_size` (a different, simpler box filter -- would give different
  pixel values). Handles equal-size, magnifying, and minifying cases via a
  weight-matrix + `numpy.einsum` approach, plus the exact-2x integer-
  rounding fast path (which is not just a performance shortcut -- its
  rounding differs from what the general float pipeline would produce for
  the same input, so it must stay a separate code path, same as the C++).
  Validated so far only with synthetic self-consistency tests (identity,
  uniform-image preservation, fast-2x-path vs manual `avg4`) -- **not yet
  cross-validated against a real `.hlsinfo`'s actual pixels** (no DXT5
  decoder available to compare against the compressed `SrcBitmap`).
- **`eval_bitmap_stats_exact()`/`convert_bitmap_exact()`/`colorize_exact()`**:
  exact (order-dependent) port of `CColorModifier::evalBitmapStats`/
  `convertBitmap` (`nel/tools/3d/panoply_maker/color_modifier.cpp`). Reuses
  `panoply_colorize.py`'s vectorized per-pixel helpers (`rgb_to_hls`/
  `hls_to_rgb`/`_brightness_contrast`/`_to_uint8` -- all order-independent)
  for everything except the hue accumulation, which is a literal
  pixel-by-pixel port of the C++'s running-mean-with-360°-unwrap loop
  (inherently sequential, can't vectorize with NumPy). Not yet run against
  real data (same validation blocker as `resample()`).

## What's not started yet

- `BuildMasksFromConfigFile` (cfg -> per-axis `CColorModifier` list) --
  straightforward now that `pynel.config_file` exists to read the
  `panoply_*.cfg`.
- `BuildColoredVersionForOneBitmap`'s combinatorial loop (multi-radix
  odometer over each active mask's color IDs, chaining `convert_bitmap_exact`
  calls onto one accumulating result bitmap per combination) and the mask
  discovery/lookup logic (`<name>_<mask_ext>.<ext>` next to the source
  texture, or under `additionnal_paths`).
- Cross-validation against the real `panoply_maker` binary (available via
  `ryzom-docker/studio/output/`, same recipe as the `hls_bank_maker`
  cross-validation documented in pynel's `docs/hls_texture_bank.md`) --
  needs the sandbox/real-machine numpy blocker resolved first (this repo's
  sandbox has no numpy/panda3d; validation has to run on the real machine).

## Validation blocker

Everything in this module past `resample()`'s synthetic self-tests needs
running on a real machine with Forgery's Python environment (numpy,
panda3d) -- this repo's development sandbox doesn't have those installed,
and project policy is to never execute project code inside that sandbox
anyway. Next step once unblocked: run `resample()`/`colorize_exact()`
against the real `.hlsinfo` sources in
`ryzom-data/useful_stuff/_tools/panoply/` and compare.
