"""Ryzom Forgery panoply_maker: offline Panoply texture-baking tool, port of
nel/tools/3d/panoply_maker/panoply_maker.cpp -- generates every color variant
of an item's base texture (skin/user/hair/eyes masks) plus the matching
.hlsinfo, the same output the real panoply_maker.exe produces. See
/repos/project-todos/ryzom-core/panoply-runtime-tint.md "panoply_maker port
(Forgery)" for the full algorithm writeup this is the CLI driver for.

Usage:
	panoply_maker.py --input DIR --output DIR --build DIR [options]
	panoply_maker.py CFG.cfg

Autonomous mode (no CFG given): colors come from panoply_config (the bundled
panoply.cfg, or an active workspace's own override via --workspace) -- never
a user-picked file. Every baked variant is written into --output; the
matching .hlsinfo, an updated panoply_files.txt, and an updated
characters.hlsbank are written into --build (or <workspace>/build if
--workspace is given and --build isn't) -- starting from, but never
overwriting, the real production files under ryzom-data's
final_bnps/characters_maps_hr/ (see panoply_bake.bake_and_write()'s
docstring). Requires ryzom-data to be configured via
pynel.repository_paths -- refuses to run at all otherwise.

Explicit-cfg mode (CFG given, positional, matches the real tool's own single
argv): every setting (paths, colors, mask_extensions, output_format, ...)
comes from CFG instead, in the real production shape (one race per file,
unprefixed keys, e.g. a real panoply_common.cfg + panoply_<race>.cfg merged
by hand like current_panoply.cfg). Writes plain .hlsinfo files only (no
build_dir/characters.hlsbank/panoply_files.txt involved, matching the real
panoply_maker.exe's own behavior exactly) -- this is the mode needed to
byte-exact cross-validate against that real binary (same cfg fed to both).
"""

import argparse
from pathlib import Path
from typing import List, Optional

from pynel import config_file, repository_paths

from ryzom_forgery import dds_export, panoply_bake, panoply_config
from ryzom_forgery.panoply_maker import build_masks_from_config

_MASK_SUFFIXES = ("_skin", "_user", "_hair", "_eyes")


def _iter_source_files(input_path: Path, bitmap_extensions):
	"""Every file directly under `input_path` (non-recursive, matches the
	real production layout where masks live in a sibling directory -- see
	current_panoply.cfg's additionnal_paths) whose extension is in
	`bitmap_extensions`, skipping anything that looks like a mask file
	itself (name ends in "_skin"/"_user"/"_hair"/"_eyes") in case masks
	happen to sit alongside sources."""
	extensions = {e.lower().lstrip(".") for e in bitmap_extensions}
	for entry in sorted(input_path.iterdir()):
		if not entry.is_file():
			continue
		if entry.suffix.lstrip(".").lower() not in extensions:
			continue
		if entry.stem.lower().endswith(_MASK_SUFFIXES):
			continue
		yield entry


def _find_mask(input_dirs: List[Path], stem: str, mask_ext: str, ext: str) -> Optional[Path]:
	name = f"{stem}_{mask_ext}{ext}"
	for directory in input_dirs:
		candidate = directory / name
		if candidate.is_file():
			return candidate
	return None


def _run_autonomous(args) -> None:
	if args.workspace is not None:
		panoply_config.set_workspace_dir(args.workspace)

	if not repository_paths.is_valid("ryzom-data"):
		raise SystemExit(
			"ryzom-data repository not configured (see pynel.repository_paths) -- "
			"can't bake without its characters.hlsbank/panoply_files.txt sources."
		)
	data_root = repository_paths.get("ryzom-data")
	hlsbank_source = data_root / "final_bnps" / "characters_maps_hr" / "characters.hlsbank"
	panoply_files_source = data_root / "final_bnps" / "characters_maps_hr" / "panoply_files.txt"

	build_dir = args.build
	if build_dir is None:
		if args.workspace is None:
			raise SystemExit("Autonomous mode requires --build (or --workspace, which defaults --build to <workspace>/build)")
		build_dir = args.workspace / "build"

	bitmap_extensions = args.bitmap_extensions or ["tga", "png"]

	for source_path in _iter_source_files(args.input, bitmap_extensions):
		stem = source_path.stem
		ext = source_path.suffix
		race = panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())
		axes = panoply_bake.axes_for_source(stem, race)
		base_rgba = dds_export.load_rgba(str(source_path))

		def _mask_loader(mask_ext, stem=stem, ext=ext):
			path = _find_mask([args.input], stem, mask_ext, ext)
			if path is None:
				return None
			return panoply_bake.load_mask_luminance(path)

		written = panoply_bake.bake_and_write(
			stem, base_rgba, axes, _mask_loader, args.output, build_dir,
			hlsbank_source, panoply_files_source,
			args.low_def_shift, args.default_separator, args.output_format,
		)
		print(f"{source_path.name}: {len(written)} variant(s)")


def _run_from_cfg(cfg_path: Path) -> None:
	cfg = config_file.Document.load(cfg_path)

	def _get_str(name, default):
		return cfg.get_str(name) if cfg.has(name) else default

	def _get_int(name, default):
		return cfg.get_int(name) if cfg.has(name) else default

	input_path = Path(cfg.get_str("input_path"))
	output_path = Path(cfg.get_str("output_path"))
	hls_info_path = Path(_get_str("hls_info_path", "hlsInfo/"))
	default_separator = _get_str("default_separator", "_")
	output_format = _get_str("output_format", "tga")
	low_def_shift = _get_int("low_def_shift", 3)
	bitmap_extensions = cfg.get("bitmap_extensions") if cfg.has("bitmap_extensions") else ["tga", "png"]

	additionnal_paths = [Path(str(p)) for p in cfg.get("additionnal_paths")] if cfg.has("additionnal_paths") else []
	input_dirs = [input_path] + additionnal_paths

	axes = build_masks_from_config(cfg)

	for source_path in _iter_source_files(input_path, bitmap_extensions):
		stem = source_path.stem
		ext = source_path.suffix
		base_rgba = dds_export.load_rgba(str(source_path))

		def _mask_loader(mask_ext, stem=stem, ext=ext):
			path = _find_mask(input_dirs, stem, mask_ext, ext)
			if path is None:
				return None
			return panoply_bake.load_mask_luminance(path)

		written = panoply_bake.bake_flat(
			stem, base_rgba, axes, _mask_loader, output_path, hls_info_path,
			low_def_shift, default_separator, output_format,
		)
		print(f"{source_path.name}: {len(written)} variant(s)")


def main(argv=None):
	parser = argparse.ArgumentParser(description="Bake Ryzom Panoply texture color variants (port of panoply_maker.cpp).")
	parser.add_argument(
		"cfg", nargs="?", type=Path,
		help="Real production panoply_<race>.cfg (byte-exact cross-validation mode) -- omit for autonomous mode")
	parser.add_argument("--input", type=Path, help="Source texture directory (autonomous mode)")
	parser.add_argument("--output", type=Path, help="Output directory for baked textures (autonomous mode)")
	parser.add_argument(
		"--build", type=Path, default=None,
		help="Output directory for .hlsinfo/panoply_files.txt/characters.hlsbank (autonomous mode, default <workspace>/build)")
	parser.add_argument(
		"--workspace", type=Path, default=None,
		help="Forgery workspace to check for a panoply.cfg override, and to default --build from (autonomous mode)")
	parser.add_argument("--low-def-shift", type=int, default=3)
	parser.add_argument("--default-separator", default="_")
	parser.add_argument("--output-format", default="tga")
	parser.add_argument("--bitmap-extensions", nargs="+", default=None)
	args = parser.parse_args(argv)

	if args.cfg is not None:
		_run_from_cfg(args.cfg)
		return

	missing = [name for name, value in (("--input", args.input), ("--output", args.output)) if value is None]
	if missing:
		raise SystemExit(f"Autonomous mode (no cfg given) requires {', '.join(missing)}")
	_run_autonomous(args)


if __name__ == "__main__":
	main()
