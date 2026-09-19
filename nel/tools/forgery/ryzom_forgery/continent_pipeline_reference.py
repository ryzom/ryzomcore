#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Reads `<ryzom-data>/leveldesign/world/continent_pipeline_reference.csv`
(project-todos/forgery/landscape_editor__land_composition.md step 7) and
turns one continent's row into the `pynel.ryzom_land_tools.LandExportConfig`/
`PropertiesConfig` dataclasses the Build button feeds to `land_export`/
`zone_dependencies`/`zone_lighter`/`zone_ig_lighter`.

The CSV is TRANSPOSED (Nuno 2026-09-12, for human readability): one row per
field, one column per continent, first column holding the field name -- NOT
the original one-row-per-continent layout. `continent`, the first row, holds
the header (continent identifiers, matching `selected_continent_name`/
`ContLoc.continent_name`).

Every path-shaped column is relative to `ryzom_data_path` (confirmed
2026-09-12 against real values, e.g. `pipeline/export/ecosystems/jungle/
ligo_es/zones`, `leveldesign/landscape/jungle/big_nexus.tga`) -- resolved
here via `_resolve()`, never assumed absolute.

A handful of `LandExportConfig` fields are NOT CSV columns at all: confirmed
2026-09-12 identical across all 24 real `land_exporter.cfg` files under
`ryzom-data/leveldesign/workspace/continents/*/generated/` (a one-time
extraction, per `project_forgery_pipeline_vs_legacy_workspace` -- never read
at runtime), so they're hardcoded constants below instead of per-continent
data: `z_factor_1`=1.0, `z_factor_2`=0.5, `zone_light`=0 (Ligo light
prefixing stays out of scope, see `nel/tools/pynel/docs/land_pipeline.md`
step... land_pipeline.md's own "Note ZoneLight"), `export_collisions`=True,
`export_additionnal_igs`=True, `cell_size`=160.0, `threshold`=1.0.
`extend_coords` genuinely varies per continent (0 for most, 1 for the r2_*
variants and undernexus) and IS a CSV column.

`PropertiesConfig.cpu_num` is NEVER read from the CSV either, despite a
`cpu_num` column existing there (a leftover from the original one-time
extraction, since removed) -- `zone_lighter` reads this value to set its
OWN internal multi-threading (`lighterDesc.NumCPU`, `zone_lighter.cpp:
419-420`, capped at `MAX_CPU_PROCESS`=10), which would have been a property
of the MACHINE running the Build, not of the continent, if used -- baking
one machine's own core count into `continent_pipeline_reference.csv` (a
file every Forgery user's `ryzom-data` shares and reads) would ship that
one machine's CPU count to everyone else's, the exact same class of mistake
as assuming every user has Nuno's own `workspace/`/`graphics/` layout (see
the `feedback-forgery-multi-user-no-personal-disk` memory) -- just for a
hardware fact instead of a disk path.

Fixed at 1 instead (Nuno 2026-09-12, from his own earlier real-machine
validation, project-todos/pynel/land_pipeline.md step 5): the actual win
came from `land_build.py` running MULTIPLE `zone_lighter` PROCESSES
concurrently (one per zone, via a thread pool), each single-threaded
internally -- oversubscribing BOTH (many concurrent processes each also
spinning up to 10 of their own internal threads) would fight the OS
scheduler for no benefit. `os.cpu_count()` is still used, just to size
`land_build.py`'s own process-level concurrency instead.
"""

import csv
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

from pynel.ryzom_land_tools import LandExportConfig, PrimExportConfig, PropertiesConfig

_CSV_RELATIVE_PATH = Path("leveldesign") / "world" / "continent_pipeline_reference.csv"

# See module docstring -- confirmed identical across all 24 real
# land_exporter.cfg files, 2026-09-12.
Z_FACTOR_1 = 1.0
Z_FACTOR_2 = 0.5
ZONE_LIGHT = 0
EXPORT_COLLISIONS = True
EXPORT_ADDITIONNAL_IGS = True
CELL_SIZE = 160.0
THRESHOLD = 1.0

# `prim_export` only needs the tile bank's FIRST seasonal postfix (decision,
# project-todos/forgery/landscape_editor__ig_land_export__farbank_smallbank.md
# step 2) -- real `MultipleTilesPostfix` order confirmed 2026-09-15 against
# nexus/bagne's own `process.py` (`["_sp", "_su", "_au", "_wi"]`, spring
# first), not a CSV column since the 3 other seasons are never built here.
TILE_POSTFIX = "_sp"


class ContinentPipelineReferenceError(Exception):
	pass


def load_reference_table(ryzom_data_path: Union[str, Path]) -> Dict[str, Dict[str, str]]:
	"""{continent -> {field_name: raw string value}}, from the transposed
	CSV. Raises ContinentPipelineReferenceError if the CSV is missing or
	malformed (no `continent` header row)."""
	csv_path = Path(ryzom_data_path) / _CSV_RELATIVE_PATH
	try:
		with open(csv_path, newline="", encoding="utf-8") as f:
			rows = list(csv.reader(f))
	except OSError as exc:
		raise ContinentPipelineReferenceError(f"Failed to read {csv_path}: {exc}")
	if not rows or rows[0][0] != "continent":
		raise ContinentPipelineReferenceError(f"{csv_path}: missing 'continent' header row")
	continents = rows[0][1:]
	table: Dict[str, Dict[str, str]] = {name: {} for name in continents}
	for row in rows[1:]:
		if not row:
			continue
		field_name = row[0]
		for i, continent in enumerate(continents, start=1):
			table[continent][field_name] = row[i] if i < len(row) else ""
	return table


def _row_for(table: Dict[str, Dict[str, str]], continent: str) -> Dict[str, str]:
	row = table.get(continent)
	if row is None:
		raise ContinentPipelineReferenceError(f"{continent!r} not found in continent_pipeline_reference.csv")
	return row


def _resolve(ryzom_data_path: Union[str, Path], relative_path: str) -> str:
	if not relative_path:
		return ""
	return str(Path(ryzom_data_path) / relative_path)


def _split_list(value: str) -> List[str]:
	return [v.strip() for v in value.split("|") if v.strip()] if value else []


def _to_float(value: str, default: float = 0.0) -> float:
	try:
		return float(value)
	except (TypeError, ValueError):
		return default


def _to_int(value: str, default: int = 0) -> int:
	try:
		return int(float(value))
	except (TypeError, ValueError):
		return default


def _to_bool(value: str) -> bool:
	return str(value).strip() in ("1", "true", "True")


def _to_vec3(value: str) -> Tuple[float, float, float]:
	# Same " | "-separated convention as search_pathes/additionnal_ig, not
	# comma-separated (confirmed 2026-09-12 against real CSV values, e.g.
	# sun_direction = "-0.5 | +0.0 | -0.85").
	parts = [p.strip() for p in value.strip("{} ").split("|")]
	if len(parts) != 3:
		raise ContinentPipelineReferenceError(f"invalid vec3 value: {value!r}")
	try:
		return (float(parts[0]), float(parts[1]), float(parts[2]))
	except ValueError:
		raise ContinentPipelineReferenceError(f"invalid vec3 value: {value!r}")


def build_land_export_config(
	ryzom_data_path: Union[str, Path], continent: str, table: Optional[Dict[str, Dict[str, str]]] = None,
) -> LandExportConfig:
	"""`LandExportConfig` for `continent`, all paths resolved against
	`ryzom_data_path`. Pass an already-loaded `table` (load_reference_table())
	to avoid re-reading the CSV once per config built for the same Build run
	(land_export + zone_dependencies + zone_lighter + zone_ig_lighter each
	need their own config)."""
	row = _row_for(table or load_reference_table(ryzom_data_path), continent)
	return LandExportConfig(
		out_zone_dir=_resolve(ryzom_data_path, row["out_zone_dir"]),
		ref_zone_dir=_resolve(ryzom_data_path, row["ref_zone_dir"]),
		ref_ig_dir=_resolve(ryzom_data_path, row["ref_ig_dir"]),
		out_ig_dir=_resolve(ryzom_data_path, row["out_ig_dir"]),
		ligo_bank_dir=_resolve(ryzom_data_path, row["ligo_bank_dir"]),
		tile_bank_file=_resolve(ryzom_data_path, row["tile_bank_file"]),
		height_map_file_1=_resolve(ryzom_data_path, row["heightmap_file1"]),
		z_factor_1=Z_FACTOR_1,
		height_map_file_2=_resolve(ryzom_data_path, row["heightmap_file2"]),
		z_factor_2=Z_FACTOR_2,
		extend_coords=_to_bool(row["extend_coords"]),
		zone_light=ZONE_LIGHT,
		cell_size=CELL_SIZE,
		threshold=THRESHOLD,
		zone_region_file=_resolve(ryzom_data_path, row["zone_region_file"]),
		export_collisions=EXPORT_COLLISIONS,
		export_additionnal_igs=EXPORT_ADDITIONNAL_IGS,
		ref_cmb_dir=_resolve(ryzom_data_path, row["ref_cmb_dir"]),
		out_cmb_dir=_resolve(ryzom_data_path, row["out_cmb_dir"]),
		additionnal_ig_in_dir=_resolve(ryzom_data_path, row["additionnal_ig_in_dir"]),
		additionnal_ig_out_dir=_resolve(ryzom_data_path, row["additionnal_ig_out_dir"]),
		dfn_dir=_resolve(ryzom_data_path, row["level_design_dfn_directory"]),
		continent_file=_resolve(ryzom_data_path, row["continent_file"]),
		continents_dir=_resolve(ryzom_data_path, row["level_design_world_directory"]),
		color_map_file=_resolve(ryzom_data_path, row["colormap_file"]),
		ig_other_lighted_dir=_resolve(ryzom_data_path, row["ig_other_lighted"]),
	)


def build_properties_config(
	ryzom_data_path: Union[str, Path], continent: str, table: Optional[Dict[str, Dict[str, str]]] = None,
) -> PropertiesConfig:
	"""`PropertiesConfig` for `continent` (`zone_dependencies`/`zone_lighter`/
	`zone_ig_lighter`'s shared `properties.cfg`), all paths resolved against
	`ryzom_data_path`."""
	row = _row_for(table or load_reference_table(ryzom_data_path), continent)
	search_pathes = [_resolve(ryzom_data_path, p) for p in _split_list(row["search_pathes"])]
	additionnal_ig = [_resolve(ryzom_data_path, p) for p in _split_list(row["additionnal_ig"])]
	level_design_directory = _resolve(ryzom_data_path, row["level_design_directory"]) or None
	level_design_world_directory = _resolve(ryzom_data_path, row["level_design_world_directory"]) or None
	level_design_dfn_directory = _resolve(ryzom_data_path, row["level_design_dfn_directory"]) or None
	return PropertiesConfig(
		bank_name=_resolve(ryzom_data_path, row["bank_name"]),
		search_pathes=search_pathes,
		load_ig=_to_bool(row["load_ig"]),
		shadow=_to_bool(row["shadow"]),
		sun_direction=_to_vec3(row["sun_direction"]),
		sun_center=_to_vec3(row["sun_center"]),
		sun_distance=_to_float(row["sun_distance"]),
		sun_fov=_to_float(row["sun_fov"]),
		sun_radius=_to_float(row["sun_radius"]),
		zbuffer_landscape_size=_to_int(row["zbuffer_landscape_size"]),
		zbuffer_object_size=_to_int(row["zbuffer_object_size"]),
		soft_shadow_samples_sqrt=_to_int(row["soft_shadow_samples_sqrt"]),
		soft_shadow_jitter=_to_float(row["soft_shadow_jitter"]),
		sun_contribution=_to_bool(row["sun_contribution"]),
		sky_contribution=_to_bool(row["sky_contribution"]),
		sky_intensity=_to_float(row["sky_intensity"]),
		global_illumination_cell_size=_to_float(row["global_illumination_cell_size"]),
		water_shadow_bias=_to_float(row["water_shadow_bias"]),
		water_ambient=_to_float(row["water_ambient"]),
		water_diffuse=_to_float(row["water_diffuse"]),
		modulate_water_color=_to_bool(row["modulate_water_color"]),
		sky_contribution_for_water=_to_bool(row["sky_contribution_for_water"]),
		global_illumination_length=_to_float(row["global_illumination_length"]),
		quad_grid_size=_to_float(row["quad_grid_size"]),
		quad_grid_cell_size=_to_float(row["quad_grid_cell_size"]),
		cpu_num=1,
		vegetable_height=_to_float(row["vegetable_height"]),
		additionnal_ig=additionnal_ig,
		ig_oversampling=_to_int(row["ig_oversampling"]),
		compute_dependencies_with_igs=_to_bool(row["compute_dependencies_with_igs"]),
		continent_name=row.get("continent_name") or None,
		level_design_directory=level_design_directory,
		level_design_world_directory=level_design_world_directory,
		level_design_dfn_directory=level_design_dfn_directory,
	)


def resolve_smallbank_paths(
	ryzom_data_path: Union[str, Path], continent: str, table: Optional[Dict[str, Dict[str, str]]] = None,
) -> Tuple[str, str]:
	"""(smallbank_path, farbank_path) for `continent` -- these are NOT built
	by Forgery (decision, Nuno 2026-09-15: generating them from raw tiles is
	deferred to project-todos/forgery/future_tile_bank_generation.md, every
	continent in the CSV already has both on disk), this function only
	locates them. Resolved from the CSV's EXISTING `bank_name`/`bankfar_name`
	columns -- not new fields: `bank_name` is already read by
	`build_properties_config()` (it's `PropertiesConfig.bank_name`, what
	`zone_lighter`/`zone_ig_lighter` expect to already exist).

	`farbank_path` is NOT `bankfar_name` taken literally: that column holds
	an un-postfixed name (`.../farbank/jungle.farbank`) that never exists on
	disk -- verified 2026-09-15 both `bank_name` (`jungle.smallbank`) AND
	the real, POSTFIXED farbank files (`jungle_sp.farbank`, `_su`, `_au`,
	`_wi`, one per `MultipleTilesPostfix` entry) already exist for all 25
	continents in the CSV. So `farbank_path` reuses `bankfar_name`'s
	directory but rebuilds its filename as `<ecosystem><TILE_POSTFIX>.farbank`,
	matching the real on-disk convention."""
	row = _row_for(table or load_reference_table(ryzom_data_path), continent)
	smallbank_path = _resolve(ryzom_data_path, row["bank_name"])
	farbank_dir = Path(_resolve(ryzom_data_path, row["bankfar_name"])).parent
	farbank_path = str(farbank_dir / f"{row['ecosystem']}{TILE_POSTFIX}.farbank")
	return smallbank_path, farbank_path


def resolve_displace_dir(
	ryzom_data_path: Union[str, Path], continent: str, table: Optional[Dict[str, Dict[str, str]]] = None,
) -> str:
	"""`prim_export`'s `DisplaceDir` for `continent`. NOT a new CSV column --
	derived from the existing `ecosystem` column: verified 2026-09-15 this
	is always the LAST entry of the CSV's own `search_pathes` column for
	both nexus and bagne (`pipeline/export/ecosystems/<ecosystem>/displace`),
	and that directory exists with real noise textures (e.g.
	`j_crevasse1.png`) on disk for both."""
	row = _row_for(table or load_reference_table(ryzom_data_path), continent)
	return _resolve(ryzom_data_path, f"pipeline/export/ecosystems/{row['ecosystem']}/displace")


def build_prim_export_config(
	ryzom_data_path: Union[str, Path], continent: str,
	small_bank_path: str, far_bank_path: str, out_ig_dir: str, zone_w_dir: str, prim_dirs: List[str],
	table: Optional[Dict[str, Dict[str, str]]] = None,
) -> PrimExportConfig:
	"""`PrimExportConfig` for `continent`. `small_bank_path`/`far_bank_path`
	(from `resolve_smallbank_paths()`), `out_ig_dir`/`zone_w_dir`
	(earlier-stage outputs: the "Prim" ig output dir, and `land_build.py`'s
	existing `zone_weld_dir`), and `prim_dirs` (already-resolved absolute
	paths) are all passed in by the caller. `displace_dir` reuses
	`resolve_displace_dir()`, `form_dir` reuses the CSV's existing
	`level_design_dfn_directory` column (same value as `LandExportConfig.
	dfn_dir`), and `world_editor_files` reuses `level_design_world_directory`
	(verified 2026-09-15: `world_editor_classes.xml`, the file `prim_export`
	looks up inside this directory, sits directly under `leveldesign/world/`
	on Nuno's real data) -- none of these 3 are new CSV columns.

	`prim_dirs` is deliberately NOT resolved in here (unlike every other
	field): the real World Editor flora/tree primitives (e.g.
	`flora_nexus_minor.primitive`) currently live in `ryzom-private-data`
	(`primitives/primeroots/`), a SEPARATE repo from `ryzom_data_path` --
	`graphics/primitive/<continent>` (this function's first guess,
	2026-09-15) was wrong, Nuno confirmed `graphics/` isn't a usable source
	at all. Decision (2026-09-15, Nuno): the primitives get moved INTO
	`ryzom-data` (so a plain `ryzom_data_path`-relative path will work
	again), but the CSV stays untouched until that move happens and the
	exact target layout is decided -- so this function takes `prim_dirs`
	as an opaque, caller-supplied value instead of guessing further."""
	row = _row_for(table or load_reference_table(ryzom_data_path), continent)
	return PrimExportConfig(
		out_ig_dir=out_ig_dir,
		zone_w_dir=zone_w_dir,
		small_bank=small_bank_path,
		far_bank=far_bank_path,
		displace_dir=resolve_displace_dir(ryzom_data_path, continent, table),
		prim_dirs=prim_dirs,
		form_dir=_resolve(ryzom_data_path, row["level_design_dfn_directory"]),
		world_editor_files=_resolve(ryzom_data_path, row["level_design_world_directory"]),
		cell_size=CELL_SIZE,
	)
