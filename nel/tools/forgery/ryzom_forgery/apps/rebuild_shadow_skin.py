"""Ryzom Forgery shadow-skin rebuild: standalone CShadowSkin (re)builder.

Usage:
	rebuild_shadow_skin.py SOURCE.shape OUTPUT.shape [--lod N]

Rebuilds SOURCE's CShadowSkin (the shadow-casting proxy mesh, see
pynel/docs/shape_format.md §4) from one of its own existing LODs, mirroring
nel/tools/3d/build_shadow_skin/main.cpp's addShadowMesh() -- see
ryzom_forgery.shape_geometry.rebuild_shadow_skin() for the actual algorithm.
SOURCE and OUTPUT can be the same file.

Which LOD:
  - Default: inferred from SOURCE's existing CShadowSkin (matching its
    Triangles count against each LOD's own render-pass index count), or,
    if SOURCE has none, a content-type-agnostic default rule (coarsest LOD
    with >=~1000 triangles, else LOD index 3 -- see
    shape_geometry.default_shadow_skin_lod_index()'s docstring for why
    there's no way to do better without knowing whether SOURCE is a
    creature or a character piece).
  - --lod N: override with an explicit LOD index (0 = coarsest, higher =
    finer), e.g. to backfill a shape whose CShadowSkin was never built at
    all and where the default rule's guess isn't the desired quality.

Only CMeshMRMSkinned shapes are supported (the only type with a pynel
CShadowSkin writer -- see pynel/docs/shape_format.md)."""

import argparse
from pathlib import Path

from pynel.ryzom_shape import MeshMRMSkinned, ShapeParseError, parse_shape, save_shape

from ryzom_forgery.shape_geometry import infer_shadow_skin_lod_index, rebuild_shadow_skin


def main(argv=None):
	parser = argparse.ArgumentParser(
		description="Rebuild a .shape's CShadowSkin (shadow-casting proxy mesh) from one of its own LODs."
	)
	parser.add_argument("source", type=Path, help="Shape to rebuild the CShadowSkin of")
	parser.add_argument("output", type=Path, help="Where to write the result (can be the same as source)")
	parser.add_argument(
		"--lod", type=int, default=None,
		help="Explicit LOD index to build from (0=coarsest). Default: inferred from the existing "
		"CShadowSkin, or a content-agnostic fallback if there's none -- see this script's own docstring.")
	args = parser.parse_args(argv)

	try:
		shape_file = parse_shape(args.source.read_bytes())
	except (OSError, ShapeParseError) as exc:
		raise SystemExit(f"Failed to read {args.source}: {exc}")

	if not isinstance(shape_file.value, MeshMRMSkinned):
		raise SystemExit(
			f"{args.source}: unsupported shape type {shape_file.type_name!r} -- only CMeshMRMSkinned "
			f"is supported (the only type with a pynel CShadowSkin writer)"
		)

	geom = shape_file.value.geom
	if args.lod is not None:
		if not (0 <= args.lod < len(geom.lods)):
			raise SystemExit(
				f"--lod {args.lod} out of range (shape has {len(geom.lods)} LODs, 0..{len(geom.lods) - 1})"
			)
		lod_index = args.lod
	else:
		lod_index = infer_shadow_skin_lod_index(geom)

	geom.shadow_skin = rebuild_shadow_skin(geom, lod_index)

	args.output.parent.mkdir(parents=True, exist_ok=True)
	save_shape(args.output, shape_file)
	print(
		f"{args.output} (LOD {lod_index}, {len(geom.shadow_skin.vertices)} vertices, "
		f"{geom.shadow_skin.num_triangles} triangles)"
	)


if __name__ == "__main__":
	main()
