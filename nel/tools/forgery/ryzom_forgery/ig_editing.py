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

"""Direct `.ig` editing (project-todos/forgery/landscape_editor__ig_editing.md,
project-todos/forgery/pipeline_export_dirs_rename.md): brings the real,
hand-added content of `live_data` into our own editable working files,
without ever depending on the stale/relocated World Editor `.primitive`
flora sources (`landscape_editor__ig_land_export.md`, abandoned).

Two kinds of override file under `assets_src/landscape/`, edited by Nuno
directly (`land_ig_work_dir()`/`other_ig_work_dir()`) -- always take
priority over the extracted base when merging (`merge_land_ig()`/
`merge_other_ig()` below).

The extracted base itself (`extract_land_flora()`/`extract_other_ig()`) is
a COMPLETE copy of `live_data`'s own `.ig` content, redone every Build (no
diff, decision Nuno 2026-09-19: the previous diff-based approach only ever
served to validate the export process, not a real requirement) -- safe to
always overwrite since hand edits live in the separate override files
above, never in the extracted base itself.
"""

import shutil
from pathlib import Path
from typing import List, Optional, Tuple

from pynel import ryzom_ig
from pynel.ryzom_ig import Instance, InstanceGroup, Vector3, load_ig, save_ig

from . import ig_full_load

# Matching tolerance between live_data's and land_export's own instance
# positions -- both ultimately come from the same Ligo bank data, so any
# real match should be exact modulo float round-trip noise; not meant to
# fuzzy-match genuinely distinct instances.
_POSITION_EPSILON = 0.05


def land_ig_work_dir(ryzom_data_path, continent: str) -> Path:
	"""Working directory for `continent`'s "land" flora diff files -- one
	subfolder per continent (Nuno, 2026-09-15: zone names like "39_CF"
	repeat across continents, a flat shared folder would be confusing)."""
	return Path(ryzom_data_path) / "assets_src" / "landscape" / "ig" / continent


def other_ig_work_dir(ryzom_data_path) -> Path:
	"""Working directory for "other" `.ig` (village/decor/sky/water) --
	flat, no continent subfolder: these names are already unique across
	the whole game (Nuno, 2026-09-15)."""
	return Path(ryzom_data_path) / "assets_src" / "landscape" / "ig_other"


def _instance_key(inst: Instance) -> Tuple[str, int, int, int]:
	"""shape_name + position, rounded to _POSITION_EPSILON -- robust
	against float noise between two independently-produced files, never
	against genuinely distinct instances of the same shape (fine-grained
	enough: real placements are never that close together)."""
	scale = 1.0 / _POSITION_EPSILON
	return (
		inst.shape_name.lower(),
		round(inst.pos.x * scale),
		round(inst.pos.y * scale),
		round(inst.pos.z * scale),
	)


def _new_instance_group(instances: List[Instance]) -> InstanceGroup:
	return InstanceGroup(
		real_time_sun_contribution=False,
		surface_light=None,
		point_lights=[],
		point_light_groups=[],
		global_pos=Vector3(0.0, 0.0, 0.0),
		clusters=[],
		portals=[],
		instances=instances,
	)


def _find_live_ref(live_data_path, continent: str, name: str):
	name_lower = name.lower()
	for ref in ig_full_load.list_continent_ig_refs_visualisation(live_data_path, continent):
		if ref.name.lower() == name_lower:
			return ref
	return None


def extract_land_flora(ryzom_data_path, live_data_path, continent: str, zone_name: str, out_dir: Path) -> Optional[Path]:
	"""Writes `zone_name`'s COMPLETE `live_data` `.ig` content (not a diff,
	decision Nuno 2026-09-19) to `out_dir/<zone_name>.ig` (the "flora"
	extraction, `igs/flora/` -- project-todos/forgery/
	pipeline_export_dirs_rename.md), unconditionally overwriting any
	previous content there -- safe every Build, since hand edits live
	separately in `land_ig_work_dir()`'s override files, never here.
	Returns `None` (nothing written) if `zone_name` isn't found in
	`live_data` for `continent` at all."""
	live_ref = _find_live_ref(live_data_path, continent, zone_name)
	if live_ref is None:
		return None
	live_data_ig = ryzom_ig.parse_ig(ig_full_load.read_ig_ref_bytes(live_ref))
	out_path = out_dir / f"{zone_name}.ig"
	out_path.parent.mkdir(parents=True, exist_ok=True)
	save_ig(out_path, live_data_ig)
	return out_path


def extract_other_ig(ryzom_data_path, live_data_path, continent: str, ig_name: str, out_dir: Path) -> Optional[Path]:
	"""Writes `ig_name`'s COMPLETE `live_data` `.ig` content to
	`out_dir/<ig_name>.ig` (the "other" extraction, `igs/other/` --
	written into the SAME directory `land_export`'s own `ExportAdditionnalIGs`
	repositioning writes to, this function running first: project-todos/
	forgery/pipeline_export_dirs_rename.md). Unconditionally overwritten
	every Build, same reasoning as `extract_land_flora()`. Returns `None`
	if `ig_name` isn't found in `live_data` for `continent` at all."""
	live_ref = _find_live_ref(live_data_path, continent, ig_name)
	if live_ref is None:
		return None
	out_path = out_dir / f"{ig_name}.ig"
	out_path.parent.mkdir(parents=True, exist_ok=True)
	out_path.write_bytes(ig_full_load.read_ig_ref_bytes(live_ref))
	return out_path


def merge_land_ig(land_dir: Path, flora_dir: Path, override_dir: Path, out_dir: Path, zone_names) -> None:
	"""Builds `out_dir/<zone>.ig` (`igs/land_merged/`) for every name in
	`zone_names`, from `land_dir` (`land_export`'s own regenerated Ligo
	content, e.g. water/`.pacs_prim`/outpost markers) as the base, plus
	whichever of `override_dir` (Nuno's hand edits, `land_ig_work_dir()`,
	always wins) or `flora_dir` (the complete `live_data` extraction) has a
	file for that zone -- instances already present in the base (shape+
	position match) are never duplicated (same dedup as the previous diff-
	based approach; the complete `live_data` extraction otherwise
	reintroduces content `land_export` already regenerated on its own,
	project-todos/forgery/pipeline_export_dirs_rename.md). A zone with
	neither a base nor any flora/override content produces no output file
	at all (nothing to write)."""
	out_dir.mkdir(parents=True, exist_ok=True)
	for zone_name in zone_names:
		land_path = land_dir / f"{zone_name}.ig"
		flora_path = override_dir / f"{zone_name}.ig"
		if not flora_path.is_file():
			flora_path = flora_dir / f"{zone_name}.ig"
		base_instances: List[Instance] = list(load_ig(land_path).instances) if land_path.is_file() else []
		flora_instances: List[Instance] = list(load_ig(flora_path).instances) if flora_path.is_file() else []
		if not base_instances and not flora_instances:
			continue
		existing_keys = {_instance_key(inst) for inst in base_instances}
		new_instances = [inst for inst in flora_instances if _instance_key(inst) not in existing_keys]
		merged = _new_instance_group(base_instances + new_instances)
		save_ig(out_dir / f"{zone_name}.ig", merged)


def merge_other_ig(other_dir: Path, override_dir: Path, out_dir: Path) -> None:
	"""Builds `out_dir` (`igs/other_merged/`) from every `.ig` in
	`other_dir` (`igs/other/`, itself `live_data` extraction + `land_export`'s
	repositioning), then overwrites by name with `override_dir`'s content
	(Nuno's hand edits, `other_ig_work_dir()`, always wins) -- whole-file
	replacement, no instance-level dedup needed (unlike `merge_land_ig()`):
	`land_export` never regenerates an "other" `.ig`'s own internal content,
	only repositions the whole file as a rigid block."""
	out_dir.mkdir(parents=True, exist_ok=True)
	if other_dir.is_dir():
		for ig_path in other_dir.glob("*.ig"):
			shutil.copyfile(ig_path, out_dir / ig_path.name)
	if override_dir.is_dir():
		for ig_path in override_dir.glob("*.ig"):
			shutil.copyfile(ig_path, out_dir / ig_path.name)
