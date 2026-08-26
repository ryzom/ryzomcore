"""Ryzom Forgery shape importer: command-line mesh format -> .shape converter.

Usage:
	shape_importer.py INPUT.{obj,dae,fbx} OUTPUT.shape

The input format is picked from INPUT's extension. Produces a `CMesh` -- no
LOD levels (see ryzom_forgery/shape_import.py's module docstring for why
CMeshMRM can't be built this way).
"""

import argparse
from pathlib import Path

from ryzom_forgery.shape_import import IMPORTERS, ShapeImportError, find_importer

from pynel.ryzom_shape import ShapeFile, save_shape


def _find_importer(input_path: Path):
	importer = find_importer(input_path)
	if importer is None:
		supported = ", ".join(sorted(IMPORTERS))
		raise SystemExit(f"Unsupported input format {input_path.suffix!r} (supported: {supported})")
	return importer


def main(argv=None):
	parser = argparse.ArgumentParser(description="Import a mesh file into a Ryzom .shape file.")
	parser.add_argument("input", type=Path, help="Source mesh file (.obj/.dae/.fbx)")
	parser.add_argument("output", type=Path, help="Destination .shape file")
	args = parser.parse_args(argv)

	importer = _find_importer(args.input)

	try:
		mesh = importer(args.input)
	except (OSError, ShapeImportError) as exc:
		raise SystemExit(f"Failed to import {args.input}: {exc}")

	args.output.parent.mkdir(parents=True, exist_ok=True)
	save_shape(args.output, ShapeFile(type_name="Mesh", value=mesh))
	print(args.output)


if __name__ == "__main__":
	main()
