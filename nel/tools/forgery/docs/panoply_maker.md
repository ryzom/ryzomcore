# `panoply_maker.py` — offline panoply generation port

Status: **cross-validated against the real `panoply_maker.exe`** (2026-08-29,
see "Cross-validation result" below). Python port of `nel/tools/3d/panoply_maker/panoply_maker.cpp`
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
  Validated with synthetic self-consistency tests (identity, uniform-image
  preservation, fast-2x-path vs manual `avg4`) plus, indirectly, the
  end-to-end cross-validation below (its output feeds a `.hlsinfo`'s
  `SrcBitmap`/`Masks`, which matched in size -- exact pixel-level DXT5
  comparison still not done, no DXT5 decoder available).
- **`eval_bitmap_stats_exact()`/`convert_bitmap_exact()`/`colorize_exact()`**:
  exact (order-dependent) port of `CColorModifier::evalBitmapStats`/
  `convertBitmap` (`nel/tools/3d/panoply_maker/color_modifier.cpp`). Reuses
  `panoply_colorize.py`'s vectorized per-pixel helpers (`rgb_to_hls`/
  `hls_to_rgb`/`_brightness_contrast`/`_to_uint8` -- all order-independent)
  for everything except the hue accumulation, which is a literal
  pixel-by-pixel port of the C++'s running-mean-with-360°-unwrap loop
  (inherently sequential, can't vectorize with NumPy). `convert_bitmap_exact()`
  returns `(result, delta_hue)` -- the delta is the C++'s `retDeltaHue`
  out-param, needed to build a `.hlsinfo` instance's `Mods`. Cross-validated
  end-to-end below -- the resulting `.tga` output is essentially byte-exact
  against the real tool.
- **`ColorMaskAxis`/`build_masks_from_config()`**: port of
  `BuildMasksFromConfigFile` -- reads `mask_extensions` and each extension's
  6 parallel `X_hues`/`X_lightness`/`X_saturations`/`X_luminosities`/
  `X_constrasts`/`X_color_id` arrays via any `pynel.config_file`-shaped
  object (`.get(name) -> list`, duck-typed so this module doesn't hard-depend
  on pynel). Validated against the real, self-contained `current_panoply.cfg`
  (`skin`: 4 colors FY/MA/TR/ZO, `user`: 8 colors U1-U8, values match the
  file) -- this part needs no numpy, so it *could* be tested directly.
- **`ActiveMask`/`generate_color_combinations()`**: port of
  `BuildColoredVersionForOneBitmap`'s combinatorial loop -- a multi-radix
  odometer over each active mask's color IDs (last mask's counter
  increments fastest, matching the C++ exactly), chaining
  `convert_bitmap_exact()` calls onto one accumulating result per
  combination, yielding `(name_suffix, result_rgba, mods)` per combination.
  File I/O and mask discovery/lookup (`<name>_<mask_ext>.<ext>` next to the
  source texture, or under `additionnal_paths`) are deliberately **not**
  in this function -- left to a future glue layer, same split as
  `panoply_colorize.py` (pure math) / `panoply_texture.py` (Panda3D glue).
  Cross-validated end-to-end below.

## Glue layer: `panoply_bake.py` + `apps/panoply_maker.py`

Status: **written, not yet run** (same validation blocker as everything else
in this doc). `ryzom_forgery/panoply_bake.py` is the glue module this
section used to describe as missing: `load_mask_luminance()` (grayscale mask
-> `HxW` uint8, via `dds_export.load_rgba(..., grayscale_as_luminance=True)`),
`axes_for_source()` (autonomous mode -- builds `ColorMaskAxis` candidates
from `panoply_config.py` instead of a `.cfg`, see below), `build_active_masks()`
(narrows candidate axes down to the ones with a real mask file, via a
caller-supplied `mask_loader` callback -- file lookup stays caller-specific,
same split as `panoply_texture.py`/`panoply_colorize.py`), `bake_source()`
(builds the `.hlsinfo`'s low-def `SrcBitmap`/`Masks` + runs
`generate_color_combinations()`), and `bake_and_write()` (adds the actual
disk I/O -- writing every baked variant + the `.hlsinfo` -- shared by both
callers below so they can't drift apart).

`ryzom_forgery/apps/panoply_maker.py` is the CLI built on top of it, **dual-mode**:
- **Autonomous** (no `.cfg` given, `--input`/`--output`/`--hls-info` instead):
  colors resolved via `panoply_config.py` (workspace `panoply.cfg` override,
  else the bundled default -- see `docs/panoply_config.md`), never a
  user-picked file. This is the mode Patina's own real-bake action
  (`object_editor._bake_panoply_real()`) mirrors directly (same
  `panoply_bake` calls, its own search-paths-based mask lookup instead of a
  flat directory).
- **Explicit `.cfg`** (positional arg, matches the real tool's own single
  argv): reads paths/colors/`mask_extensions` from a real production `.cfg`
  via `pynel.config_file` + the existing `build_masks_from_config()` --
  this is the mode needed for byte-exact cross-validation against the real
  binary (below), since `panoply_maker.exe` only understands that shape.

See `/repos/project-todos/ryzom-core/panoply-runtime-tint.md` "Unified
panoply.cfg + Patina integration" for why colors got split into two `.cfg`
shapes (prefixed/unified for the bundled default vs. legacy/per-race for
cross-validation) instead of one.

## Cross-validation result (2026-08-29)

Ran the real `panoply_maker.exe` (`ryzom-docker/studio/output/`,
`LD_LIBRARY_PATH=.../lib panoply_maker <cfg>`) and the Python port
(`apps/panoply_maker.py`'s `.cfg`-explicit mode, `bake_flat()`) side by side
on the real `ryw_hom_caster01_pantabottes_c1/c2/c3` sources
(`ryzom-data/useful_stuff/_tools/panoply/in`), same `current_panoply.cfg`
color tables -- both produced the same 24 `.tga` variants, no errors.

- `.hlsinfo`: same size (38892 bytes), differ byte-for-byte (expected --
  float pixel pipeline + DXT compression heuristics, never claimed
  byte-exact).
- `.tga`: **essentially byte-exact** -- max per-byte diff 1 (float
  rounding), mean diff 0.00, 0% of bytes with diff>20. Far beyond the
  "stretch goal" bar this item was originally scoped with.
- Performance: ~8x slower than the native binary on this 24-variant test
  (~2.4s real vs ~19s Python) -- expected (numpy/Panda3D vs native C++),
  not a concern for the intended usage (occasional offline bake, not a
  hot path).

**Bug found and fixed during this validation**: `dds_export.py`'s
`save_rgba()` wrote 24-bit RGB `.tga` instead of 32-bit RGBA -- a Panda3D
`PNMImage`/TGA-writer limitation (confirmed: the in-memory `PNMImage` has 4
channels/`has_alpha()==True` right up to `write()`, but the written file
always comes back 3-channel on re-read; `.png` via the same code path
preserves alpha fine, so this is TGA-writer-specific). Fixed with a
hand-rolled `_save_tga_rgba()` (18-byte-header uncompressed 32-bit TGA
writer, bypasses Panda3D entirely for `.tga`, same pattern as this module's
existing manual `.dds` writer) -- see its docstring for the exact header
layout, including `imageDescriptor=0x00` (bottom-left origin, no alpha bits
declared) confirmed byte-exact against the real tool's own header.

## Known gaps

- `mustDivideBy2`/`d4/` legacy half-res convention not ported (never
  observed in the 10 real `.hlsinfo` samples).
- Autonomous mode's mask lookup only searches one folder (`--input`), no
  `additionnal_paths` support (not observed as needed on real data).
- No cache (`cache_path`) -- always a full rebuild.
