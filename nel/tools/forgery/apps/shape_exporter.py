"""Ryzom Forgery shape exporter: command-line .shape -> mesh format converter.

Usage:
	shape_exporter.py INPUT.shape OUTPUT.{obj,dae,stl,gltf,glb} [--data-root PATH] [--texture-mode {copy_png,reference_only}]

The output format is picked from OUTPUT's extension. A .shape's materials
reference textures by file name only, not by path -- resolving that name to
an actual file (to copy a decoded .png next to the export) needs a Ryzom
data tree, given via --data-root. Without --data-root, textures are left as
a reference to their original file name instead.
"""

import argparse
from pathlib import Path

from ryzom_forgery.asset_index import AssetIndex
from ryzom_forgery.export_config import TEXTURE_MODE_COPY_PNG, TEXTURE_MODE_REFERENCE_ONLY
from ryzom_forgery.shape_export import EXPORT_FORMATS

from pynel.ryzom_shape import ShapeParseError, parse_shape


def _find_format(output_path: Path):
	extension = output_path.suffix.lstrip(".").lower()
	for export_format in EXPORT_FORMATS:
		if export_format.extension == extension:
			return export_format
	supported = ", ".join(f.extension for f in EXPORT_FORMATS)
	raise SystemExit(f"Unsupported output format {output_path.suffix!r} (supported: {supported})")


def main(argv=None):
	parser = argparse.ArgumentParser(description="Export a Ryzom .shape file to another mesh format.")
	parser.add_argument("input", type=Path, help="Source .shape file")
	parser.add_argument("output", type=Path, help="Destination file; its extension selects the export format")
	parser.add_argument(
		"--data-root", type=Path, default=None,
		help="Ryzom data tree to resolve textures from, for --texture-mode copy_png")
	parser.add_argument(
		"--texture-mode", choices=[TEXTURE_MODE_COPY_PNG, TEXTURE_MODE_REFERENCE_ONLY], default=None,
		help="How to handle material textures (default: copy_png if --data-root is given, else reference_only)")
	args = parser.parse_args(argv)

	export_format = _find_format(args.output)

	texture_mode = args.texture_mode
	if texture_mode is None:
		texture_mode = TEXTURE_MODE_COPY_PNG if args.data_root else TEXTURE_MODE_REFERENCE_ONLY
	if texture_mode == TEXTURE_MODE_COPY_PNG and args.data_root is None:
		raise SystemExit("--texture-mode copy_png requires --data-root")

	asset_index = None
	if args.data_root is not None:
		asset_index = AssetIndex(args.data_root)
		asset_index.build()

	try:
		shape_file = parse_shape(args.input.read_bytes())
	except (OSError, ShapeParseError) as exc:
		raise SystemExit(f"Failed to read {args.input}: {exc}")

	materials = getattr(shape_file.value, "materials", None)
	args.output.parent.mkdir(parents=True, exist_ok=True)

	try:
		written = export_format.export(shape_file.value, materials, args.output, texture_mode, asset_index)
	except ValueError as exc:
		raise SystemExit(f"Export failed: {exc}")

	for path in written:
		print(path)


if __name__ == "__main__":
	main()
