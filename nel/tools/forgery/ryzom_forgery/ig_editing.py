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

"""Direct `.ig` editing (project-todos/forgery/landscape_editor__ig_editing.md):
brings the real, hand-added content of `live_data` into our own editable
working files, without ever depending on the stale/relocated World Editor
`.primitive` flora sources (`landscape_editor__ig_land_export.md`, abandoned).

Two kinds of working file, computed/copied ONCE then left untouched forever
(never recomputed automatically -- see each function's own docstring):

- "Land" flora diff (`land_ig_work_dir()`): `land_export` (unchanged, still
  called every Build) already regenerates, from the Ligo bank, everything
  that depends on the current `.land` composition for a zone (water,
  `.pacs_prim`, outpost markers -- confirmed 2026-09-15 against
  `foret-15_zc.ig`'s real content). The only thing missing from its output
  is whatever was added by hand on top in `live_data` -- identified by
  DIFF (shape+position), not by name/extension: real flora in `live_data`
  isn't consistently suffixed (`fy_s2_palmtree_*`/`fy_s2_coconuts_*` have
  no extension at all, mixed with genuinely non-flora content in the same
  "no dot" group), so a name-based classifier would misclassify real
  flora as non-flora.
- "Other" `.ig` copy (`other_ig_work_dir()`): nothing regenerates a
  village/decor/sky/water `.ig`'s own internal content (`land_export`'s
  `ExportAdditionnalIGs` only repositions the whole file as a rigid block,
  per the `.continent` sheet's `Villages[]` array) -- `live_data` is
  copied wholesale, once.
"""

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


def compute_land_flora_diff(live_data_ig: InstanceGroup, land_export_ig: InstanceGroup) -> List[Instance]:
	"""Every instance of `live_data_ig` with no shape+position match in
	`land_export_ig` -- the real, hand-added content beyond land_export's
	own Ligo template, whatever its actual nature (flora or not, cf. this
	module's own docstring)."""
	land_export_keys = {_instance_key(inst) for inst in land_export_ig.instances}
	return [inst for inst in live_data_ig.instances if _instance_key(inst) not in land_export_keys]


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


def ensure_land_flora_diff(
	ryzom_data_path, live_data_path, continent: str, zone_name: str, land_export_ig_path: Path,
) -> Path:
	"""Path to `zone_name`'s land flora diff working file under
	`land_ig_work_dir(ryzom_data_path, continent)`, computing it (from
	`live_data` vs `land_export_ig_path`, land_export's own freshly-produced
	`.ig` for this zone) if it doesn't exist yet -- created EMPTY (valid,
	instance-less `.ig`) only if there is nothing to diff against at all (a
	zone never shipped in `live_data`). A zone land_export produced no `.ig`
	for (confirmed 2026-09-16 on real bagne data: land_export silently skips
	a zone whose Ligo tile has no matching ref `.ig`) is diffed against an
	empty base instead of being treated the same as "nothing to diff" --
	`live_data`'s entire content for that zone is real, hand-placed flora
	(and `.pacs_prim` collision markers Stage 8 needs), not noise. Once
	created, this file is NEVER recomputed automatically: recomputing it
	later would silently discard any edit already made on it (neither
	`live_data` nor land_export's own output know about our own edits)."""
	work_path = land_ig_work_dir(ryzom_data_path, continent) / f"{zone_name}.ig"
	if work_path.is_file():
		return work_path

	diff_instances: List[Instance] = []
	live_ref = _find_live_ref(live_data_path, continent, zone_name)
	land_export_ig_path = Path(land_export_ig_path)
	if live_ref is not None:
		live_data_ig = ryzom_ig.parse_ig(ig_full_load.read_ig_ref_bytes(live_ref))
		land_export_ig = (
			load_ig(land_export_ig_path) if land_export_ig_path.is_file() else _new_instance_group([])
		)
		diff_instances = compute_land_flora_diff(live_data_ig, land_export_ig)

	work_path.parent.mkdir(parents=True, exist_ok=True)
	save_ig(work_path, _new_instance_group(diff_instances))
	return work_path


def ensure_other_ig_copy(ryzom_data_path, live_data_path, continent: str, ig_name: str) -> Optional[Path]:
	"""Path to `ig_name`'s "other" working copy under
	`other_ig_work_dir(ryzom_data_path)`, copying it verbatim from
	`live_data` if it doesn't exist yet. Returns `None` (no working file
	created) if `ig_name` isn't found in `live_data` for `continent` at all.
	Once copied, this file is NEVER recopied automatically -- same reason
	as `ensure_land_flora_diff()`."""
	work_path = other_ig_work_dir(ryzom_data_path) / f"{ig_name}.ig"
	if work_path.is_file():
		return work_path

	live_ref = _find_live_ref(live_data_path, continent, ig_name)
	if live_ref is None:
		return None

	work_path.parent.mkdir(parents=True, exist_ok=True)
	work_path.write_bytes(ig_full_load.read_ig_ref_bytes(live_ref))
	return work_path


def merge_land_flora_diff(land_export_ig_path: Path, flora_diff_path: Path) -> None:
	"""Appends `flora_diff_path`'s instances onto `land_export_ig_path`
	in place (land_export's own freshly-produced `.ig` for a zone) --
	called after land_export (Stage 1) and before zone_ig_lighter
	(Stage 7), so the lighter sees the flora too. A no-op if
	`flora_diff_path` has no instances (nothing to add). Idempotent
	(shape+position key, same as `compute_land_flora_diff`): skips any
	diff instance already present in `land_export_ig_path` -- land_export
	is incremental and may leave an already-merged `.ig` untouched on a
	later build, so re-running this merge must not duplicate instances
	(confirmed 2026-09-16: an unconditional append doubled every flora
	instance on a second build, bagne_ref vs bagne comparison). Creates
	`land_export_ig_path` from `flora_diff_path` alone (a fresh, empty-base
	instance group) when land_export produced no `.ig` for this zone at
	all -- same 2026-09-16 finding as `ensure_land_flora_diff()`'s own
	docstring: `live_data`'s content for such a zone is real, not noise,
	and Stage 8's collision build needs the resulting `.ig` to exist."""
	flora_diff = load_ig(flora_diff_path)
	if not flora_diff.instances:
		return
	land_export_ig_path = Path(land_export_ig_path)
	if land_export_ig_path.is_file():
		land_export_ig = load_ig(land_export_ig_path)
		existing_keys = {_instance_key(inst) for inst in land_export_ig.instances}
		new_instances = [inst for inst in flora_diff.instances if _instance_key(inst) not in existing_keys]
		if not new_instances:
			return
		land_export_ig.instances = list(land_export_ig.instances) + new_instances
	else:
		land_export_ig = _new_instance_group(list(flora_diff.instances))
	save_ig(land_export_ig_path, land_export_ig)
