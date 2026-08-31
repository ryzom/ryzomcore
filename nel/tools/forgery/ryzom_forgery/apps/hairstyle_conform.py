"""Ryzom Forgery hairstyle conform: command-line cross-race boundary snap.

Usage:
	hairstyle_conform.py SOURCE.shape SOURCE_RACE_KEY TARGET_RACE_KEY OUTPUT.shape --search-path PATH [PATH ...] [--workspace DIR]

Snaps SOURCE's boundary vertices (the open-edge loop welded onto its own
race/gender's face mesh -- identified via SOURCE_RACE_KEY's own reference
data, see ryzom_forgery.race_reference) onto the equivalent position on
TARGET_RACE_KEY's own canonical seam ring, so OUTPUT has no seam gap when
placed against the target race's head -- see
ryzom_forgery/shape_geometry.py's conform_hairstyle_boundary() for the
algorithm (rigid pre-alignment by seam centroid, then per-vertex
angle-interpolated snap onto the target seam ring).

SOURCE_RACE_KEY/TARGET_RACE_KEY (e.g. "fy_hof", "tr_hof") are looked up in
race_reference.cfg (bundled default, or --workspace DIR's own override) --
each key names that race/gender's own face mesh and a reference hairstyle
that fully covers the skull, resolved by name through --search-path (same
.bnp-aware lookup used for every texture reference elsewhere in this
suite).

Restyling the hair's interior for the new head shape is explicitly out of
scope -- every hairstyle differs too much for a general algorithm; that
part is manual, per-hairstyle, in a 3D tool, starting from OUTPUT.

Only CMeshMRMSkinned shapes are supported (the only type with a pynel
geometry writer -- see pynel/docs/shape_format.md).
"""

import argparse
from pathlib import Path

from pynel.ryzom_shape import MeshMRMSkinned, ShapeParseError, ShapeWriteError, parse_shape, save_shape

from ryzom_forgery import race_reference, search_paths
from ryzom_forgery.settings import SearchPathDir
from ryzom_forgery.shape_geometry import conform_hairstyle_boundary, iter_render_passes, seam_loop_by_angle_indexed


def _load_positions_indices(shape_value):
	positions = []
	indices = []
	base = 0
	for vertex_buffer, _material_id, idx in iter_render_passes(shape_value):
		positions.extend(vertex_buffer.positions)
		indices.extend(i + base for i in idx)
		base += len(vertex_buffer.positions)
	return positions, indices


def _require_mrm_skinned(shape_file, path: Path):
	if not isinstance(shape_file.value, MeshMRMSkinned):
		raise SystemExit(
			f"{path}: unsupported shape type {shape_file.type_name!r} -- only CMeshMRMSkinned "
			f"is supported (the only type with a pynel geometry writer)"
		)


def main(argv=None):
	parser = argparse.ArgumentParser(
		description="Snap a hairstyle's boundary onto another race/gender's head, so it has no seam gap there."
	)
	parser.add_argument("source", type=Path, help="Hairstyle .shape to adapt")
	parser.add_argument("source_race_key", help="race_reference.cfg key for SOURCE's own race/gender (e.g. fy_hof)")
	parser.add_argument("target_race_key", help="race_reference.cfg key for the target race/gender (e.g. tr_hof)")
	parser.add_argument("output", type=Path, help="Where to write the conformed .shape")
	parser.add_argument(
		"--search-path", type=Path, action="append", default=[], required=True,
		help="Directory (recursive, .bnp-aware) to resolve race_reference.cfg's face/hairstyle names from -- "
		"repeatable")
	parser.add_argument(
		"--workspace", type=Path, default=None,
		help="Workspace directory whose own race_reference.cfg, if present, overrides the bundled default")
	args = parser.parse_args(argv)

	try:
		source_shape_file = parse_shape(args.source.read_bytes())
	except (OSError, ShapeParseError) as exc:
		raise SystemExit(f"Failed to read {args.source}: {exc}")
	_require_mrm_skinned(source_shape_file, args.source)

	race_reference.set_workspace_dir(args.workspace)
	search_dirs = [SearchPathDir(path=str(p), recursive=True) for p in args.search_path]
	entries_by_lower_name = search_paths.build_texture_index(search_dirs)

	try:
		source_ref = race_reference.get_reference(args.source_race_key, entries_by_lower_name)
		target_ref = race_reference.get_reference(args.target_race_key, entries_by_lower_name)
	except (FileNotFoundError, KeyError) as exc:
		raise SystemExit(f"Failed to resolve race reference: {exc}")

	if not target_ref.seam_ring:
		raise SystemExit(f"{args.target_race_key}: reference hairstyle has no seam loop welded to its own face")

	source_positions, source_indices = _load_positions_indices(source_shape_file.value)
	if not seam_loop_by_angle_indexed(source_positions, source_indices, source_ref.face_index):
		raise SystemExit(f"{args.source}: no boundary loop welded to {args.source_race_key}'s face found")

	new_positions = conform_hairstyle_boundary(
		source_positions, source_indices, source_ref.face_index, target_ref.seam_ring)

	geom = source_shape_file.value.geom
	try:
		geom.packed_vertices = [
			pv.with_pos(new_positions[i], geom.decompact_scale) for i, pv in enumerate(geom.packed_vertices)
		]
	except ShapeWriteError as exc:
		raise SystemExit(f"Failed to write conformed positions back: {exc}")

	args.output.parent.mkdir(parents=True, exist_ok=True)
	save_shape(args.output, source_shape_file)
	print(args.output)


if __name__ == "__main__":
	main()
