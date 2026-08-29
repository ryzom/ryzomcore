"""Panoply real-bake glue: wires panoply_maker.py's pure-math port (resample/
colorize/combinatorial loop) together with dds_export.py (DXT5/mip building,
TGA/PNG I/O) and pynel.hls_bank_texture_info (.hlsinfo writing) into the
per-source-texture pipeline BuildColoredVersionForOneBitmap implements in the
real panoply_maker.cpp -- see
/repos/project-todos/ryzom-core/panoply-runtime-tint.md "panoply_maker port
(Forgery)" for the full algorithm writeup this ports.

Deliberately has no file-lookup logic of its own (unlike e.g. search_paths.py):
apps/panoply_maker.py's CLI (flat directory lookup) and Patina's object_editor.py
(its own search_paths_dialog index) resolve source/mask files very differently,
so this module only takes already-loaded pixel arrays plus a caller-supplied
`mask_loader` callback -- same split as panoply_texture.py (Panda3D decode glue)
vs. panoply_colorize.py (pure math).

A real bake never writes into `ryzom-data` directly (2026-08-29, Nuno):
`.hlsinfo`/`panoply_files.txt`/`characters.hlsbank` all land in a dedicated
`build_dir` (e.g. the active Forgery workspace's `build/`) instead, starting
from -- but never overwriting -- the real production files at
`ryzom-data/final_bnps/characters_maps_hr/` (via `pynel.repository_paths`).
Callers (apps/panoply_maker.py, object_editor.py) are responsible for
resolving `ryzom-data`'s configured path and refusing to bake at all (a
popup in Patina's case) if it isn't configured -- this module itself accepts
`hlsbank_source`/`panoply_files_source` as plain optional paths and doesn't
care where they came from.
"""

from pathlib import Path
from typing import Callable, List, Optional, Sequence, Tuple

import numpy

from pynel import hls_bank_texture_info as hlsinfo
from pynel import hls_texture_bank

from ryzom_forgery import dds_export, panoply_config
from ryzom_forgery.panoply_maker import ActiveMask, ColorMaskAxis, generate_color_combinations, resample

# Candidate axes tried for every source texture, in BuildColoredVersionForOneBitmap's
# mask_extensions order (matches panoply.cfg's/panoply.AXES' own order) -- only
# ones panoply_config actually has a table for (this race, or unconditionally
# for skin/user) and that resolve to a real mask file end up "active".
_CANDIDATE_AXES = ("skin", "user", "hair", "eyes")


def load_mask_luminance(path) -> numpy.ndarray:
	"""Loads a grayscale mask file (TGA/PNG) as an HxW uint8 luminance array
	-- the mask weight panoply_maker.py's ActiveMask.mask_u8 expects. Port of
	the "niveau de gris = alpha" convention documented in pynel/panoply.md."""
	rgba = dds_export.load_rgba(str(path), grayscale_as_luminance=True)
	return rgba[..., 0]


def axes_for_source(stem: str, race: Optional[str] = None) -> List[ColorMaskAxis]:
	"""Every candidate ColorMaskAxis panoply_config actually defines for
	`race` (or unconditionally for skin/user), in _CANDIDATE_AXES order --
	not yet filtered by what mask files exist on disk for this specific
	source (see build_active_masks()). `race` is normally
	panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower()), left as an
	explicit parameter so callers that already know the race (e.g.
	object_editor.py, which resolves it once per shape) don't redo that
	lookup."""
	axes = []
	for mask_ext in _CANDIDATE_AXES:
		color_ids = panoply_config.available_color_ids(mask_ext, race)
		if not color_ids:
			continue
		colors = [panoply_config.get_color_params(mask_ext, color_id, race) for color_id in color_ids]
		axes.append(ColorMaskAxis(mask_ext=mask_ext, colors=colors))
	return axes


def build_active_masks(axes: List[ColorMaskAxis], mask_loader: Callable[[str], Optional[numpy.ndarray]]) -> List[ActiveMask]:
	"""Narrows `axes` (every configured axis, see axes_for_source()/
	build_masks_from_config()) down to the ones that actually have a mask
	file for this specific source texture -- port of
	BuildColoredVersionForOneBitmap step 3 ("only mask extensions with an
	actually-found file become active"). `mask_loader(mask_ext)` resolves
	and loads one mask (via load_mask_luminance() above, once the caller has
	found the file), or returns None if not found -- kept caller-supplied so
	file lookup stays caller-specific (see module docstring)."""
	active = []
	for axis in axes:
		mask_u8 = mask_loader(axis.mask_ext)
		if mask_u8 is not None:
			active.append(ActiveMask(axis=axis, mask_u8=mask_u8))
	return active


def bake_source(
	base_rgba_u8: numpy.ndarray, active_masks: List[ActiveMask], low_def_shift: int = 3, default_separator: str = "_",
) -> Tuple[hlsinfo.HLSBankTextureInfo, List[Tuple[str, numpy.ndarray]]]:
	"""Full per-source-texture bake: builds the .hlsinfo's low-def SrcBitmap
	(DXT5, mipmapped, via dds_export.build_dds()) and Masks (resample()'d
	down to the same low-def size), plus every recolored full-res output via
	generate_color_combinations(). Returns (info, combos) where `combos` is
	`[(name_suffix, result_rgba_u8), ...]`, in the same order as
	`info.instances` -- each `info.instances[i].name` is left empty for the
	caller to fill in once it knows the real output file name (stem + suffix
	+ configured output_format), matching combos[i]'s suffix.

	No disk I/O here -- see apps/panoply_maker.py (CLI) and
	object_editor.py's Patina "bake" action for what writes the outputs
	build_dds()/combos actually returns, to whatever destination each caller
	uses (a flat output_path for the CLI, the active workspace for Patina)."""
	height, width = base_rgba_u8.shape[:2]
	low_w = max(1, width >> low_def_shift)
	low_h = max(1, height >> low_def_shift)

	low_def_base = resample(base_rgba_u8, low_w, low_h)
	src_bitmap_dds = dds_export.build_dds(low_def_base, dds_export.DXT5, build_mipmaps=True)

	masks = []
	for active_mask in active_masks:
		low_def_mask = resample(active_mask.mask_u8, low_w, low_h)
		masks.append(hlsinfo.MaskBitmap(width=low_w, height=low_h, pixels=numpy.ascontiguousarray(low_def_mask).tobytes()))

	combos = []
	instances = []
	for suffix, result_rgba, mods in generate_color_combinations(base_rgba_u8, active_masks, default_separator):
		combos.append((suffix, result_rgba))
		instances.append(hlsinfo.TextureInstance(
			name="",
			mods=[hlsinfo.HLSMod(d_hue=d_hue, d_lum=d_lum, d_sat=d_sat) for d_hue, d_lum, d_sat in mods],
		))

	info = hlsinfo.HLSBankTextureInfo(divided_by_2=False, src_bitmap_dds=src_bitmap_dds, masks=masks, instances=instances)
	return info, combos


def _pick_existing(*candidates: Optional[Path]) -> Optional[Path]:
	for candidate in candidates:
		if candidate is not None and candidate.is_file():
			return candidate
	return None


def merge_panoply_files_txt(source_path: Optional[Path], new_names: Sequence[str]) -> str:
	"""Returns updated panoply_files.txt content (one file name per line):
	`source_path`'s lines verbatim, in order (or none, if `source_path` is
	None / doesn't exist yet -- see build_and_update_bank()'s docstring on
	why the real ryzom-data file isn't guaranteed to exist), plus any name
	in `new_names` not already present, appended at the end in the order
	given. Never reorders/rewrites existing lines, to keep a diff against
	the real production file minimal."""
	lines = []
	if source_path is not None and source_path.is_file():
		lines = [line for line in source_path.read_text().splitlines() if line]
	seen = set(lines)
	for name in new_names:
		if name not in seen:
			lines.append(name)
			seen.add(name)
	return "\n".join(lines) + "\n"


def load_or_empty_hlsbank(path: Optional[Path]) -> hls_texture_bank.HLSTextureBank:
	if path is not None and path.is_file():
		return hls_texture_bank.load_hlsbank(path)
	return hls_texture_bank.HLSTextureBank()


def _write_variants(
	stem: str, info: hlsinfo.HLSBankTextureInfo, combos: List[Tuple[str, numpy.ndarray]],
	output_dir: Path, output_format: str,
) -> Tuple[List[Path], List[str]]:
	"""Writes every combo's image into `output_dir`, filling `info.instances[i].name`
	in place as it goes (matches combos' order 1:1). Returns
	(written_texture_paths, output_names) -- shared by both bake_flat() and
	bake_and_write() below, only the .hlsinfo/build-tracking side differs
	between them."""
	output_dir.mkdir(parents=True, exist_ok=True)
	written = []
	output_names = []
	for instance, (suffix, result_rgba) in zip(info.instances, combos):
		out_name = f"{stem}{suffix}.{output_format}"
		instance.name = out_name
		output_names.append(out_name)
		out_path = output_dir / out_name
		dds_export.save_rgba(str(out_path), result_rgba)
		written.append(out_path)
	return written, output_names


def bake_flat(
	stem: str, base_rgba_u8: numpy.ndarray, axes: List[ColorMaskAxis], mask_loader: Callable[[str], Optional[numpy.ndarray]],
	output_dir: Path, hls_info_dir: Path, low_def_shift: int = 3, default_separator: str = "_", output_format: str = "tga",
) -> List[Path]:
	"""End-to-end single-source bake matching the real panoply_maker.exe's
	own behavior exactly: writes baked variants into `output_dir` and one
	plain `{stem}.hlsinfo` into `hls_info_dir` -- no `build_dir`/
	`characters.hlsbank`/`panoply_files.txt` involved (the real tool doesn't
	touch those either -- `hls_bank_maker` is a separate step). This is the
	mode apps/panoply_maker.py's explicit-`.cfg` cross-validation path uses,
	since it needs byte-exact comparison against the real binary's own
	output, not the "next patch" build_dir workflow bake_and_write() below
	implements for autonomous/Patina use."""
	active_masks = build_active_masks(axes, mask_loader)
	info, combos = bake_source(base_rgba_u8, active_masks, low_def_shift, default_separator)
	written, _output_names = _write_variants(stem, info, combos, output_dir, output_format)
	hls_info_dir.mkdir(parents=True, exist_ok=True)
	hlsinfo.save_hlsinfo(hls_info_dir / f"{stem}.hlsinfo", info)
	return written


def bake_and_write(
	stem: str, base_rgba_u8: numpy.ndarray, axes: List[ColorMaskAxis], mask_loader: Callable[[str], Optional[numpy.ndarray]],
	output_dir: Path, build_dir: Path, hlsbank_source: Optional[Path] = None, panoply_files_source: Optional[Path] = None,
	low_def_shift: int = 3, default_separator: str = "_", output_format: str = "tga",
) -> List[Path]:
	"""End-to-end single-source bake, disk I/O included:
	build_active_masks() + bake_source(), then:
	- every baked texture variant written into `output_dir` (e.g. the active
	  workspace's tex/) ;
	- `build_dir/{stem}.hlsinfo`, `build_dir/panoply_files.txt` and
	  `build_dir/characters.hlsbank` (dedicated "next patch" output, never
	  written into `ryzom-data` directly -- see the module docstring and
	  /repos/project-todos/ryzom-core/panoply-runtime-tint.md "Unified
	  panoply.cfg + Patina integration"). `hlsbank_source`/
	  `panoply_files_source` are the real production files (from
	  `ryzom-data/final_bnps/characters_maps_hr/`) to start from -- **but
	  only on the very first bake of a session**: if `build_dir` already has
	  its own `characters.hlsbank`/`panoply_files.txt` from an earlier bake
	  in this same `build_dir` (this or a previous session), that one is
	  preferred instead, so consecutive bakes accumulate into the same
	  running `build_dir` state rather than each restarting fresh from the
	  pristine real files. Either source may not exist yet (e.g.
	  `characters.hlsbank` genuinely isn't copied into `ryzom-data` yet as
	  of 2026-08-29) -- treated as "start empty", not an error.

	Shared by apps/panoply_maker.py's autonomous CLI mode (looping this over
	a whole input directory) and Patina's own "bake real variants" action
	(object_editor.py, one source at a time) -- written once here so the two
	never drift apart. Returns the list of written texture file paths (not
	the `.hlsinfo`/`panoply_files.txt`/`.hlsbank` paths, all always at
	fixed names under `build_dir`)."""
	active_masks = build_active_masks(axes, mask_loader)
	info, combos = bake_source(base_rgba_u8, active_masks, low_def_shift, default_separator)
	written, output_names = _write_variants(stem, info, combos, output_dir, output_format)

	build_dir.mkdir(parents=True, exist_ok=True)
	hlsinfo.save_hlsinfo(build_dir / f"{stem}.hlsinfo", info)

	txt_path = build_dir / "panoply_files.txt"
	txt_source = _pick_existing(txt_path, panoply_files_source)
	txt_path.write_text(merge_panoply_files_txt(txt_source, output_names))

	bank_path = build_dir / "characters.hlsbank"
	bank_source = _pick_existing(bank_path, hlsbank_source)
	bank = load_or_empty_hlsbank(bank_source)
	hls_texture_bank.append_texture_info(bank, info)
	hls_texture_bank.save_hlsbank(bank_path, bank)

	return written
