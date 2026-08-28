"""Ryzom Forgery DDS export: command-line TGA/PNG -> .dds converter.

Usage:
	dds_export.py INPUT.{tga,png} [-o OUTPUT.dds] [-a {1,1a,3,5}] [-m] [-g] [-r N]

Python/Panda3D counterpart of nel/tools/3d/tga_2_dds/tga2dds.cpp -- see
ryzom_forgery/dds_export.py's module docstring for what "correct" means
here (a DDS the client can load, not necessarily byte-identical to the
libsquish-based original).
"""

import argparse
from pathlib import Path

from ryzom_forgery.dds_export import DXT1, DXT1A, DXT3, DXT5, build_dds, load_rgba, pick_default_algo

_ALGO_CHOICES = {"1": DXT1, "1a": DXT1A, "3": DXT3, "5": DXT5}


def main(argv=None):
	parser = argparse.ArgumentParser(description="Convert a TGA/PNG file to a Ryzom .dds file.")
	parser.add_argument("input", type=Path, help="Source TGA or PNG file")
	parser.add_argument("-o", "--output", type=Path, default=None,
						 help="Destination .dds path (default: same name, .dds extension)")
	parser.add_argument("-a", "--algo", choices=sorted(_ALGO_CHOICES), default=None,
						 help="DXT algo (default: DXT1 if the source has no alpha, DXT5 otherwise)")
	parser.add_argument("-m", "--mipmap", action="store_true", help="Build mipmaps")
	parser.add_argument("-g", "--grayscale", action="store_true",
						 help="Load a grayscale source as visible luminance instead of an alpha mask")
	parser.add_argument("-r", "--reduce", type=int, default=0, choices=range(0, 9),
						 help="Halve the bitmap size this many times (0-8) before compressing")
	args = parser.parse_args(argv)

	try:
		rgba = load_rgba(str(args.input), grayscale_as_luminance=args.grayscale)
	except RuntimeError as exc:
		raise SystemExit(f"Failed to read {args.input}: {exc}")

	algo = _ALGO_CHOICES[args.algo] if args.algo is not None else pick_default_algo(rgba)

	output = args.output or args.input.with_suffix(".dds")
	output.parent.mkdir(parents=True, exist_ok=True)
	data = build_dds(rgba, algo, build_mipmaps=args.mipmap, reduce=args.reduce)
	output.write_bytes(data)
	print(output)


if __name__ == "__main__":
	main()
