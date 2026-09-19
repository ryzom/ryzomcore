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

"""Orchestrates the full native landscape pipeline (project-todos/forgery/
landscape_editor__land_composition.md step 7) that turns a `.land`
composition into final, installable game data -- `.zonel`/`.ig`, the PACS
collision region bank, and the AI pathfinding walkability maps:

	land_export -> zone_welder -> zone_elevation -> zone_welder ->
	zone_dependencies -> zone_lighter -> zone_ig_lighter -> build_ig_boxes ->
	build_rbank -> build_indoor_rbank -> ai_wmap_export

Order confirmed in `nel/tools/build_gamedata/processes/zone/2_build.py` and
validated end-to-end 2026-09-11 against real `nexus` data (project-todos/
pynel/land_pipeline.md step 5), and again 2026-09-12 through this
orchestrator itself against real `bagne` data. PACS
(project-todos/forgery/landscape_editor__pacs_export.md) and
`ai_wmap_export` (project-todos/forgery/ai_wmap_flora_collision.md) were
added later, 2026-09-16/17, as Stages 8 and 9 below.

Every stage's own logic lives in a private `_run_*()` function, all sharing
a `_BuildContext` resolved once (`_resolve_context()`) -- `run_full_build()`
is a thin orchestrator chaining them in order, and `run_single_step()`
(project-todos/forgery/landscape_editor__pacs_export.md step 7i, Nuno
2026-09-18) reuses the exact same functions to rebuild ONE step (or its
shared-indicator group) right now, without waiting for the next full Build.

Unlike zone_tools.py's `run_zone_welder()` (a single on-demand zone, reusing
already-loaded Atyscape state), this module drives a from-scratch build of
an ENTIRE continent directly against the config from
`continent_pipeline_reference.csv` (`continent_pipeline_reference.py`) --
raises `LandBuildError` and stops the whole build on the first
unrecoverable failure, rather than tolerate partial failures the way the
more granular "Generate missing .zonew" button does (a from-scratch build
only makes sense as all-or-nothing).
"""

import json
import os
import platform
import re
import shutil
import threading
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, List, Optional, Set, Tuple

from pynel import ryzom_land_tools, ryzom_pacs_tools, ryzom_zone_tools
from pynel.ryzom_zone import parse_zone, ZoneParseError

from . import ai_wmap_export
from . import continent_pipeline_reference as cpr
from . import ig_editing
from . import region_loader
from . import settings as app_settings
from .zone_geometry import zone_to_cache_data

_BINARY_NAMES = (
	"land_export", "zone_welder", "zone_elevation", "zone_dependencies", "zone_lighter", "zone_ig_lighter",
	"build_ig_boxes", "build_rbank", "build_indoor_rbank",
)

_ZONE_NAME_RE = re.compile(r"^(\d+)_([A-Za-z]{2})$")

# Fixed, ordered list of run_full_build()'s stages (project-todos/forgery/
# landscape_editor__pacs_export.md step 7b) -- always 15 entries in this
# exact order, regardless of which stages actually do work on a given run
# (e.g. "Merging hand-added flora" is still announced, with fraction=1.0,
# even when live_data_path is None and nothing happens), so the popup's
# "Step X/15" numbering never jumps or depends on the continent/run.
_STEP_NAMES = (
	"Cutting composition into zones (land_export)",
	"Merging hand-added flora",
	"Welding (pass 1, no height)",
	"Applying heightmaps (zone_elevation)",
	"Welding (pass 2, with height)",
	"Computing shadow dependencies (zone_dependencies)",
	"Lighting zones (zone_lighter)",
	"Lighting instance groups (zone_ig_lighter)",
	"Building instance-group boxes (build_ig_boxes)",
	"Checking collision primitives (build_rbank)",
	"Building region bank (build_rbank, per zone)",
	"Merging region bank (build_rbank, global)",
	"Merging interior meshes (build_indoor_rbank)",
	"Finalizing PACS output",
	"Regenerating AI walkability maps (ai_wmap_export)",
)

# Short (2-word) labels for the "Pipeline steps" table (project-todos/
# forgery/landscape_editor__pacs_export.md step 7g, Nuno 2026-09-18) -- the
# tool actually invoked is shown in a tooltip instead (_STEP_TOOLS below),
# _STEP_NAMES itself is unchanged and keeps being used everywhere else
# (progress popup, LandBuildError messages).
_STEP_SHORT_NAMES = (
	"Cutting composition",
	"Merging flora",
	"Welding pass1",
	"Applying heightmaps",
	"Welding pass2",
	"Computing dependencies",
	"Lighting zones",
	"Lighting instances",
	"Building boxes",
	"Checking primitives",
	"Building bank",
	"Merging bank",
	"Merging interiors",
	"Finalizing PACS",
	"Regenerating walkability",
)

# Native tool actually invoked by each stage, shown as a tooltip next to its
# short label -- "None (Python)" for the 2 stages with no native tool at all
# (flora merge is a pure Python in-place .ig edit, the final rename is a
# pure Python file copy).
_STEP_TOOLS = (
	"land_export",
	"None (Python)",
	"zone_welder",
	"zone_elevation",
	"zone_welder",
	"zone_dependencies",
	"zone_lighter",
	"zone_ig_lighter",
	"build_ig_boxes",
	"build_rbank",
	"build_rbank",
	"build_rbank",
	"build_indoor_rbank",
	"None (Python)",
	"ai_build_wmap",
)

# Steps with no real on-disk cache/skip -- always redone on every Build
# regardless of what's already on disk (project-todos/forgery/
# landscape_editor__pacs_export.md step 7f, Nuno 2026-09-17: confirmed by
# rereading each stage's own code above, not assumed).
_UNCACHED_STEP_INDICES = frozenset({0, 1, 5, 8, 9, 12, 13, 14})

# Flora merge (index 1) and build_rbank's check-prims pass (index 9) don't
# themselves produce an output file to check -- both now write their own
# small marker after a successful run (project-todos/forgery/
# landscape_editor__pacs_export.md step 7k, Nuno 2026-09-18) so
# compute_step_statuses() can give them a real tick too, instead of the
# permanently-neutral one they had before this marker existed.
_FLORA_MARKER_SUFFIX = ".floramerge.json"
_CHECK_PRIMS_MANIFEST_NAME = "check_prims_manifest.json"

# Steps sharing a single on-disk cache indicator, grouped under one [rebuild]
# button each (Nuno 2026-09-17: "un bouton par groupe") -- rebuilding any one
# step of a group rebuilds every step of that same group, since none of them
# has an independent indicator of its own. Steps 10 (build_rbank, per zone)
# and 11 (build_rbank, global) are each forceable on their own instead (see
# compute_step_statuses()/_clear_step_cache() below) -- both have a real,
# independent indicator. This grouping stays scoped to the shared on-disk
# indicator (Nuno 2026-09-18) -- it does NOT follow the full cascade a
# rebuild can trigger at the next run (e.g. clearing .zonew also invalidates
# the lighter group and both build_rbank passes downstream): that cascade
# resolves itself the next time anything runs, no need for one giant button.
_STEP_GROUPS = (
	(2, 3, 4),  # weld pass 1, zone_elevation, weld pass 2 -- shared indicator: zone_weld_dir/*.zonew
	(6, 7),  # zone_lighter, zone_ig_lighter -- shared indicator: zone_lighted_dir/*.zonel
)


class LandBuildError(Exception):
	pass


@dataclass
class _BuildContext:
	"""Every path/config run_full_build()/run_single_step() need for one
	continent, resolved once by _resolve_context() and threaded through
	every extracted `_run_*()` stage function below -- replaces the pile of
	local variables the old monolithic run_full_build() used to compute
	inline. `binaries` is filled lazily (_ensure_binaries()) rather than
	upfront, so run_single_step() never has to require every tool's path be
	configured just to rebuild one step that doesn't need most of them."""
	ryzom_data_path: object
	continent: str
	table: object
	land_cfg: object
	props_cfg: object
	out_zone_dir: Path
	zone_dir: Path
	zone_weld_dir: Path
	zone_lighted_dir: Path
	out_ig_dir: Path
	common_dir: Path
	rbank_bbox_dir: Path
	rbank_tessellation_dir: Path
	rbank_preproc_dir: Path
	rbank_retrievers_dir: Path
	rbank_output_dir: Path
	rbank_cmb_export_dir: Path
	rbank_smooth_dir: Path
	ig_other_effective_dir: Path
	ai_wmap_dir: Path
	binaries: Dict[str, Path] = field(default_factory=dict)


def _resolve_context(ryzom_data_path, continent: str) -> _BuildContext:
	"""Resolves every directory run_full_build()/run_single_step() read/write
	for `continent`, without running anything -- out_ig_dir.parent is the
	common "CommonPath" root every PACS scratch directory sits next to, same
	convention as every other pipeline-export subdirectory already in use."""
	table = cpr.load_reference_table(ryzom_data_path)
	land_cfg = cpr.build_land_export_config(ryzom_data_path, continent, table)
	props_cfg = cpr.build_properties_config(ryzom_data_path, continent, table)
	out_zone_dir = Path(land_cfg.out_zone_dir)
	zone_dir, zone_weld_dir, zone_lighted_dir = region_loader.continent_zone_dirs(ryzom_data_path, continent)
	out_ig_dir = Path(land_cfg.out_ig_dir)
	common_dir = out_ig_dir.parent
	return _BuildContext(
		ryzom_data_path=ryzom_data_path, continent=continent, table=table,
		land_cfg=land_cfg, props_cfg=props_cfg,
		out_zone_dir=out_zone_dir, zone_dir=zone_dir, zone_weld_dir=zone_weld_dir, zone_lighted_dir=zone_lighted_dir,
		out_ig_dir=out_ig_dir, common_dir=common_dir,
		rbank_bbox_dir=common_dir / "rbank_bbox",
		rbank_tessellation_dir=common_dir / "rbank_tessellation",
		rbank_preproc_dir=common_dir / "rbank_preproc",
		rbank_retrievers_dir=common_dir / "rbank_retrievers",
		rbank_output_dir=common_dir / "rbank_output",
		rbank_cmb_export_dir=common_dir / "rbank_cmb_export",
		rbank_smooth_dir=common_dir / "rbank_smooth",
		ig_other_effective_dir=common_dir / "ig_other_effective",
		ai_wmap_dir=common_dir / "ai_wmap",
	)


def resolve_build_dirs(ryzom_data_path, continent) -> Dict[str, Path]:
	"""Public subset of _resolve_context(), kept for any external caller
	that only needs the directories, not the full config objects."""
	ctx = _resolve_context(ryzom_data_path, continent)
	return {
		"out_zone_dir": ctx.out_zone_dir,
		"zone_weld_dir": ctx.zone_weld_dir,
		"zone_lighted_dir": ctx.zone_lighted_dir,
		"out_ig_dir": ctx.out_ig_dir,
		"rbank_bbox_dir": ctx.rbank_bbox_dir,
		"rbank_smooth_dir": ctx.rbank_smooth_dir,
		"rbank_retrievers_dir": ctx.rbank_retrievers_dir,
		"rbank_output_dir": ctx.rbank_output_dir,
		"ai_wmap_dir": ctx.ai_wmap_dir,
	}


def _resolve_binary(name: str) -> Path:
	settings = app_settings.load()
	if not settings.ryzom_tools_path:
		raise LandBuildError("Ryzom tools folder is not configured (Settings > Ryzom Paths).")
	tools_dir = Path(settings.ryzom_tools_path)
	binary_name = f"{name}.exe" if platform.system() == "Windows" else name
	binary_path = tools_dir / binary_name
	if not binary_path.is_file():
		raise LandBuildError(f"{binary_name} not found in {tools_dir}.")
	return binary_path


def _ensure_binaries(ctx: _BuildContext, names) -> None:
	for name in names:
		if name not in ctx.binaries:
			ctx.binaries[name] = _resolve_binary(name)


def _ensure_zone_dirs(ctx: _BuildContext) -> None:
	for directory in (
		ctx.out_zone_dir, ctx.zone_dir, ctx.zone_weld_dir, ctx.zone_lighted_dir,
		ctx.out_ig_dir, ctx.land_cfg.out_cmb_dir, ctx.land_cfg.additionnal_ig_out_dir,
	):
		Path(directory).mkdir(parents=True, exist_ok=True)


def _depend_path_for(zone_weld_dir: Path, zone_stem: str) -> Path:
	"""zone_dependencies writes ONE `.depend` FILE PER ZONE, not a single
	file matching its own argv[4] literally -- always lowercased
	(`toLowerAscii`, zone_dependencies.cpp:528) regardless of the real zone
	name's case."""
	return zone_weld_dir / f"{zone_stem.lower()}.depend"


def _zone_sort_key(zone_name: str) -> Tuple[int, int]:
	"""(row, col) as real integers, for sorting zone names in ascending
	row/col order -- required by `zone_dependencies`' own broken min/max
	swap (see _run_zone_dependencies()'s own comment): a plain string sort of
	the zone name is NOT equivalent whenever row digit widths differ (e.g.
	"9_AB" would sort after "10_AB" as a string, but 9 < 10 numerically)."""
	match = _ZONE_NAME_RE.match(zone_name)
	if match is None:
		raise LandBuildError(f"not a valid zone name: {zone_name!r}")
	row = int(match.group(1))
	letters = match.group(2).upper()
	col = (ord(letters[0]) - ord("A")) * 26 + (ord(letters[1]) - ord("A"))
	return (row, col)


def _zone_name_from_row_col(row: int, col: int) -> str:
	"""Inverse of `_zone_sort_key()` -- builds a `<row>_<letters>` zone name
	from a (row, col) pair, for the synthetic bounding-box corners
	`_run_zone_dependencies()`/`_build_rbank_config()` need (see their own
	comments)."""
	return f"{row}_{chr(ord('A') + col // 26)}{chr(ord('A') + col % 26)}"


def _zone_needs_rbank_zone_rebuild(
	zone_weld_dir: Path, rbank_smooth_dir: Path, zone_stem: str,
	all_zone_keys: Dict[Tuple[int, int], str], rebuilt_zonew_names=frozenset(),
) -> bool:
	"""True if `zone_stem`'s own `build_rbank` (per zone) output (`.lr`) is
	missing or stale -- shared by _run_build_rbank_zones() (the real
	incremental decision at Build time) AND compute_step_statuses() (the
	"Pipeline steps" status shown in the UI), so the two can never disagree
	the way they used to (project-todos/forgery/landscape_editor__
	pacs_export.md step 7h: compute_step_statuses() used to only check
	`.lr` PRESENCE, never the neighbor-mtime staleness below, so it could
	show a zone as up to date when a real Build would still rebuild it). A
	zone needs rebuilding if its own `.lr` is missing, or its own `.zonew`
	was just rebuilt THIS run (`rebuilt_zonew_names`, always empty outside
	an actual run_full_build() -- a pure status check has nothing "just
	rebuilt"), or ANY of its 8 neighbors' `.zonew` is newer than its own
	`.lr` (build_rbank's own moulinage samples neighboring zones for border
	continuity)."""
	lr_path = rbank_smooth_dir / f"{zone_stem}.lr"
	if zone_stem in rebuilt_zonew_names:
		return True
	try:
		lr_mtime = lr_path.stat().st_mtime
	except FileNotFoundError:
		# Polled every frame from compute_step_statuses() while a background
		# rebuild thread can delete/rewrite this same .lr concurrently
		# (_clear_step_cache()) -- missing (whether never built or mid-rebuild
		# right now) both mean "needs rebuild" from this function's own
		# contract, same as the old `not lr_path.is_file()` check above.
		return True
	row, col = _zone_sort_key(zone_stem)
	for d_row in (-1, 0, 1):
		for d_col in (-1, 0, 1):
			neighbor_stem = all_zone_keys.get((row + d_row, col + d_col))
			if neighbor_stem is None:
				continue
			neighbor_zonew = zone_weld_dir / f"{neighbor_stem}.zonew"
			try:
				if neighbor_zonew.stat().st_mtime > lr_mtime:
					return True
			except FileNotFoundError:
				continue
	return False


def _rbank_global_needs_rebuild(rbank_smooth_dir: Path, zones_needing_rebuild=()) -> bool:
	"""True if `build_rbank` (global)'s own output (`temp.rbank`) is missing
	or stale -- shared by _run_build_rbank_global() and
	compute_step_statuses() for the same reason as
	_zone_needs_rbank_zone_rebuild() above (step 7h: this used to be a
	presence-only check in compute_step_statuses(), unlike the real
	mtime-based age indicator run_full_build() already used)."""
	rbank_global_output = rbank_smooth_dir / "temp.rbank"
	if zones_needing_rebuild or not rbank_global_output.is_file():
		return True
	global_mtime = rbank_global_output.stat().st_mtime
	newer_found = False
	for p in rbank_smooth_dir.glob("*.lr"):
		try:
			if p.stat().st_mtime > global_mtime:
				newer_found = True
				break
		except FileNotFoundError:
			# This function polls disk state every frame (compute_step_
			# statuses(), called from draw_ui()) while a background rebuild
			# thread can delete/rewrite these same .lr files concurrently
			# (_clear_step_cache() at the start of a step rebuild) -- a file
			# that disappears between glob() and stat() just means a rebuild
			# is mid-flight, not that it's genuinely missing; the next
			# frame's poll sees a consistent state once it settles.
			continue
	return newer_found


@dataclass(frozen=True)
class StepStatus:
	"""One row of the "Pipeline steps" section (project-todos/forgery/
	landscape_editor__pacs_export.md steps 7f-7k). `has_own_button` is False
	only for a step that shares its indicator with an earlier step in the
	same `_STEP_GROUPS` entry -- only the group's FIRST step draws a
	[rebuild] button, which rebuilds the whole group; every other step
	(including all 8 always-redone ones) has its own button. `up_to_date` is
	`True` (green tick) or `False` (pink tick); `warning`, if not `None`, is
	a message the UI shows instead of the plain tick (e.g. flora merge with
	no `live_data_path` configured -- data will silently be incomplete,
	Nuno 2026-09-18: "s'il manque quoi que ce soit, c'est warning
	obligatoire"). For a checkable stage, `built_count` out of `total_count`
	zones (or files) are already up to date on disk -- `total_count == 1`
	for a continent-wide indicator rather than a per-zone one, `total_count
	== 3` for ai_wmap_export's 3 maps. `group_size` is how many consecutive
	entries (starting at this one, only meaningful when `has_own_button` is
	True) share this step's single [rebuild] button and table row -- 1 for
	every standalone step, `len(_STEP_GROUPS[0])`/`len(_STEP_GROUPS[1])` for
	the 2 grouped steps (project-todos/forgery/landscape_editor__
	pacs_export.md step 7l, Nuno 2026-09-18: "garde la cellule mais fais 3
	lignes" -- one table row per group, one line per member, single tick
	column and single button, not 3 separate rows)."""
	index: int
	name: str
	short_name: str
	tool: str
	has_own_button: bool
	built_count: Optional[int]
	total_count: Optional[int]
	up_to_date: Optional[bool]
	warning: Optional[str] = None
	group_size: int = 1


def _flora_merge_status(ctx: _BuildContext, zone_names: List[str], live_data_path) -> Tuple[int, int, bool, Optional[str]]:
	"""Status for flora merge (index 1, project-todos/forgery/
	landscape_editor__pacs_export.md step 7k). No `live_data_path`
	configured is a WARNING, not a plain pink tick (Nuno 2026-09-18): the
	merge is silently skipped by run_full_build() in that case, so the .ig
	produced is missing whatever hand-placed flora/collision markers
	`live_data` has for this continent, with no other indication anything
	is wrong. Otherwise, a zone is up to date only if its own
	`<zone>.floramerge.json` marker still matches BOTH the current `.ig`
	(mtime+size -- stale the moment land_export reruns and rewrites it) and
	the current flora diff file under `ig_editing.land_ig_work_dir()`
	(mtime+size, or the marker recording no diff at all if none exists).
	A zone with no `.ig` at all (no land_export ref content, no live_data
	flora either -- a normal undecorated terrain zone, confirmed 2026-09-19
	real: 223/236 `newbieland` zones) is up to date once its marker
	confirms that (`ig_mtime`/`ig_size` both `None`, `_write_flora_merge_
	marker()`) and the real `.ig` still doesn't exist -- treated the same
	as any other genuinely stale-free zone, never a permanent "not up to
	date"."""
	if live_data_path is None:
		return 0, len(zone_names), False, (
			"live_data_path is not configured -- flora merge is skipped, "
			"the .ig produced will be missing any hand-placed flora/collision markers."
		)
	work_dir = ig_editing.land_ig_work_dir(ctx.ryzom_data_path, ctx.continent)
	up_to_date_count = 0
	for zone_name in zone_names:
		ig_path = ctx.out_ig_dir / f"{zone_name}.ig"
		marker_path = ctx.out_ig_dir / f"{zone_name}{_FLORA_MARKER_SUFFIX}"
		if not marker_path.is_file():
			continue
		try:
			marker = json.loads(marker_path.read_text())
		except (OSError, ValueError):
			continue
		try:
			ig_stat = ig_path.stat()
		except FileNotFoundError:
			# Polled every frame while a background rebuild thread can
			# delete/rewrite this same .ig concurrently -- same race as
			# _zone_needs_rbank_zone_rebuild() above, not the "confirmed no
			# content" case (ig_mtime=None) a legitimately undecorated zone
			# leaves behind.
			if marker.get("ig_mtime") is not None:
				continue
		else:
			if marker.get("ig_mtime") != ig_stat.st_mtime or marker.get("ig_size") != ig_stat.st_size:
				continue
		diff_path = work_dir / f"{zone_name}.ig"
		try:
			diff_stat = diff_path.stat()
		except FileNotFoundError:
			if marker.get("diff_mtime") is not None:
				continue
		else:
			if marker.get("diff_mtime") != diff_stat.st_mtime or marker.get("diff_size") != diff_stat.st_size:
				continue
		up_to_date_count += 1
	total = len(zone_names)
	return up_to_date_count, total, total > 0 and up_to_date_count == total, None


def _check_prims_status(ctx: _BuildContext, zone_names: List[str]) -> Tuple[int, int, bool]:
	"""Status for build_rbank's check-prims pass (index 9, project-todos/
	forgery/landscape_editor__pacs_export.md step 7k) -- up to date only if
	`check_prims_manifest.json` exists, covers exactly `zone_names` (no zone
	added/removed since), and every recorded `.zonew` mtime+size still
	matches the real file."""
	manifest_path = ctx.rbank_preproc_dir / _CHECK_PRIMS_MANIFEST_NAME
	total = len(zone_names)
	if not manifest_path.is_file():
		return 0, total, False
	try:
		manifest = json.loads(manifest_path.read_text()).get("zones", {})
	except (OSError, ValueError):
		return 0, total, False
	if set(manifest.keys()) != set(zone_names):
		return 0, total, False
	up_to_date_count = 0
	for zone_name in zone_names:
		zonew_path = ctx.zone_weld_dir / f"{zone_name}.zonew"
		recorded = manifest.get(zone_name)
		if recorded is None:
			continue
		try:
			stat = zonew_path.stat()
		except FileNotFoundError:
			continue
		if recorded == [stat.st_mtime, stat.st_size]:
			up_to_date_count += 1
	return up_to_date_count, total, total > 0 and up_to_date_count == total


def compute_step_statuses(ryzom_data_path, continent, zone_names, live_data_path=None) -> List[StepStatus]:
	"""Per-stage status for `continent`'s "Pipeline steps" section, from
	`zone_names` (the continent's real zone stems, already known by the
	caller) and whatever's already on disk. `live_data_path`, if given,
	feeds the flora-merge check (index 1) -- see _flora_merge_status().
	Never runs anything itself."""
	names = list(zone_names)
	total = len(names)
	ctx = _resolve_context(ryzom_data_path, continent)

	def count_present(directory: Path, ext: str) -> int:
		return sum(1 for name in names if (directory / f"{name}{ext}").is_file())

	zonenh_count = count_present(ctx.out_zone_dir, ".zonenh")
	zonew_count = count_present(ctx.zone_weld_dir, ".zonew")
	zonel_count = count_present(ctx.zone_lighted_dir, ".zonel")
	depend_count = sum(1 for name in names if _depend_path_for(ctx.zone_weld_dir, name).is_file())
	ig_boxes_present = (ctx.rbank_bbox_dir / "ig_boxes.bbox").is_file()
	indoor_present = (
		(ctx.rbank_retrievers_dir / "tempMerged.rbank").is_file()
		and (ctx.rbank_retrievers_dir / "tempMerged.gr").is_file()
	)
	rename_present = (
		(ctx.rbank_output_dir / f"{continent}.rbank").is_file()
		and (ctx.rbank_output_dir / f"{continent}.gr").is_file()
	)
	ai_wmap_present_count = sum(1 for i in (0, 1, 2) if (ctx.ai_wmap_dir / f"{continent}_{i}.tga").is_file())
	global_up_to_date = not _rbank_global_needs_rebuild(ctx.rbank_smooth_dir)

	flora_built, flora_total, flora_up_to_date, flora_warning = _flora_merge_status(ctx, names, live_data_path)
	check_prims_built, check_prims_total, check_prims_up_to_date = _check_prims_status(ctx, names)

	all_zone_keys = {_zone_sort_key(name): name for name in names}
	rbank_zone_up_to_date_count = sum(
		1 for name in names
		if not _zone_needs_rbank_zone_rebuild(ctx.zone_weld_dir, ctx.rbank_smooth_dir, name, all_zone_keys)
	)

	# `total > 0 and` guards every per-zone ratio below: with zero known
	# zones (an empty/unresolvable .land, never the normal case now that the
	# caller passes every used cell's expected name -- see
	# landscape_editor_edit_mode.py's own _draw_pipeline_steps_section()),
	# `count == total` would otherwise be vacuously True (0 == 0) and read
	# as "up to date" for a continent that's actually never been built --
	# the exact regression found 2026-09-18 on a totally empty `bagne/`.
	per_step = {
		0: (zonenh_count, total, total > 0 and zonenh_count == total, None),
		1: (flora_built, flora_total, flora_up_to_date, flora_warning),
		2: (zonew_count, total, total > 0 and zonew_count == total, None),
		3: (zonew_count, total, total > 0 and zonew_count == total, None),
		4: (zonew_count, total, total > 0 and zonew_count == total, None),
		5: (depend_count, total, total > 0 and depend_count == total, None),
		6: (zonel_count, total, total > 0 and zonel_count == total, None),
		7: (zonel_count, total, total > 0 and zonel_count == total, None),
		8: (1 if ig_boxes_present else 0, 1, ig_boxes_present, None),
		9: (check_prims_built, check_prims_total, check_prims_up_to_date, None),
		10: (rbank_zone_up_to_date_count, total, total > 0 and rbank_zone_up_to_date_count == total, None),
		11: (1 if global_up_to_date else 0, 1, global_up_to_date, None),
		12: (1 if indoor_present else 0, 1, indoor_present, None),
		13: (1 if rename_present else 0, 1, rename_present, None),
		14: (ai_wmap_present_count, 3, ai_wmap_present_count == 3, None),
	}

	statuses = []
	for index, name in enumerate(_STEP_NAMES):
		built_count, total_count, up_to_date, warning = per_step[index]
		if index in _STEP_GROUPS[0]:
			has_own_button = index == _STEP_GROUPS[0][0]
			group_size = len(_STEP_GROUPS[0])
		elif index in _STEP_GROUPS[1]:
			has_own_button = index == _STEP_GROUPS[1][0]
			group_size = len(_STEP_GROUPS[1])
		else:
			has_own_button = True
			group_size = 1
		statuses.append(StepStatus(
			index, name, _STEP_SHORT_NAMES[index], _STEP_TOOLS[index],
			has_own_button, built_count, total_count, up_to_date, warning, group_size,
		))
	return statuses


def _clear_step_cache(ctx: _BuildContext, step_index: int) -> None:
	"""Deletes step_index's own on-disk cache file(s), if any -- and, for a
	step inside one of `_STEP_GROUPS`, every other step of that same group
	too, since they share one indicator. A no-op for an always-redone step
	(`_UNCACHED_STEP_INDICES`): it has nothing on disk to clear, it already
	reruns/overwrites unconditionally every time."""
	if step_index in _STEP_GROUPS[0]:
		for path in ctx.zone_weld_dir.glob("*.zonew"):
			path.unlink(missing_ok=True)
	elif step_index in _STEP_GROUPS[1]:
		for path in ctx.zone_lighted_dir.glob("*.zonel"):
			path.unlink(missing_ok=True)
	elif step_index == 10:
		for path in ctx.rbank_smooth_dir.glob("*.lr"):
			path.unlink(missing_ok=True)
	elif step_index == 11:
		for filename in ("temp.rbank", "temp.gr"):
			(ctx.rbank_smooth_dir / filename).unlink(missing_ok=True)


def force_step(ryzom_data_path, continent, step_index: int) -> None:
	"""Public wrapper kept for any external caller that only wants the
	cache-clearing half of run_single_step() (below) without the immediate
	rerun."""
	ctx = _resolve_context(ryzom_data_path, continent)
	_clear_step_cache(ctx, step_index)


def _prepare_pacs_dirs(ctx: _BuildContext) -> None:
	"""Creates every PACS scratch directory and refreshes
	ig_other_effective_dir -- "other" ig merged into one real directory
	build_rbank can point a single IgVillagePath at: native
	ig_other_lighted_dir first, then our own working-copy override
	(landscape_editor__ig_editing.md) on top, same precedence as
	ig_full_load.py's own override logic (duplicated here rather than
	imported -- that module is Panda3D-app-facing, this one isn't)."""
	for directory in (
		ctx.rbank_bbox_dir, ctx.rbank_tessellation_dir, ctx.rbank_preproc_dir,
		ctx.rbank_retrievers_dir, ctx.rbank_output_dir, ctx.rbank_cmb_export_dir, ctx.ig_other_effective_dir,
	):
		directory.mkdir(parents=True, exist_ok=True)
	for source_dir in (Path(ctx.land_cfg.ig_other_lighted_dir), ig_editing.other_ig_work_dir(ctx.ryzom_data_path)):
		if source_dir.is_dir():
			for ig_path in source_dir.glob("*.ig"):
				shutil.copyfile(ig_path, ctx.ig_other_effective_dir / ig_path.name)


def _run_land_export(ctx: _BuildContext) -> List[Path]:
	"""Stage 1 -- land_export: cuts the .land composition into flat
	(no-height) .zonenh files, one per used grid cell, into out_zone_dir."""
	cfg_path = ctx.out_zone_dir / f"{ctx.continent}_land_export.cfg"
	result = ryzom_land_tools.run_land_export(ctx.binaries["land_export"], ctx.land_cfg, cfg_path)
	# land_export's own main() (ryzom-core, land_export/main.cpp) never
	# returns 0 -- it returns 1 on a normal completed run (even one full of
	# logged warnings) and -1 only for a hard failure (bad args, or its cfg/
	# land file failing to load). A first version of this check compared
	# against 0 and would have raised on every single successful run --
	# caught by reading that source directly (project-todos/forgery/
	# landscape_editor__pacs_export.md step 8, 2026-09-19) before it shipped.
	if result.returncode < 0:
		raise LandBuildError(f"land_export failed (returncode={result.returncode}):\n{result.stdout}")
	zonenh_files = sorted(ctx.out_zone_dir.glob("*.zonenh"))
	if not zonenh_files:
		raise LandBuildError(f"land_export produced no .zonenh files in {ctx.out_zone_dir}:\n{result.stdout}")
	return zonenh_files


def _write_flora_merge_marker(ig_path: Path, diff_path: Path) -> None:
	"""Marker confirming flora merge really ran for this zone (project-todos/
	forgery/landscape_editor__pacs_export.md step 7k) -- read back by
	_flora_merge_status() to give the "Pipeline steps" tick a real
	up-to-date/stale signal instead of the permanently-neutral one it had
	before this marker existed. Records the merged `.ig`'s OWN mtime+size
	(stale the moment land_export reruns and rewrites it) and the flora
	diff's mtime+size at merge time (`None` if no diff file existed).

	`ig_mtime`/`ig_size` are themselves `None` if `ig_path` doesn't exist --
	a zone can legitimately end up with no `.ig` at all after the merge
	(no ref content from land_export, no live_data flora either, confirmed
	2026-09-19 real: 223/236 `newbieland` zones), a valid "confirmed empty"
	terminal state, not an error -- crashing here on a raw stat() (as a
	first version of this function did) blocked the flora-merge step
	entirely for any continent with normal, undecorated terrain zones."""
	if ig_path.is_file():
		ig_stat = ig_path.stat()
		marker = {"ig_mtime": ig_stat.st_mtime, "ig_size": ig_stat.st_size, "diff_mtime": None, "diff_size": None}
	else:
		marker = {"ig_mtime": None, "ig_size": None, "diff_mtime": None, "diff_size": None}
	if diff_path.is_file():
		diff_stat = diff_path.stat()
		marker["diff_mtime"] = diff_stat.st_mtime
		marker["diff_size"] = diff_stat.st_size
	marker_path = ig_path.with_name(f"{ig_path.stem}{_FLORA_MARKER_SUFFIX}")
	marker_path.write_text(json.dumps(marker))


def _run_flora_merge(ctx: _BuildContext, zonenh_files: List[Path], live_data_path, progress) -> None:
	"""Stage 1bis -- merge each zone's own hand-added "land" flora diff
	(live_data vs land_export's own freshly-produced .ig) onto the .ig
	land_export just wrote to out_ig_dir -- BEFORE zone_ig_lighter relights
	it. Skipped entirely (land_export's own output used as-is) if
	live_data_path isn't available."""
	progress(1)
	if live_data_path:
		for i, zonenh_path in enumerate(zonenh_files):
			zone_name = zonenh_path.stem
			land_export_ig_path = ctx.out_ig_dir / f"{zone_name}.ig"
			flora_diff_path = ig_editing.ensure_land_flora_diff(
				ctx.ryzom_data_path, live_data_path, ctx.continent, zone_name, land_export_ig_path,
			)
			ig_editing.merge_land_flora_diff(land_export_ig_path, flora_diff_path)
			_write_flora_merge_marker(land_export_ig_path, flora_diff_path)
			progress(1, (i + 1) / len(zonenh_files))
	else:
		progress(1, 1.0)


def _run_weld_pass1(ctx: _BuildContext, to_reweld: List[Path], progress) -> List[Path]:
	"""Stage 2 -- zone_welder, pass 1 (.zonenh -> .zonenhw, still no real
	height): sequential over the sorted list so each zone sees its
	already-welded neighbors already on disk by the time its own turn
	comes, same single-pass convention as zone_tools.py's own "Generate
	missing .zonew" button."""
	progress(2, 1.0 if not to_reweld else 0.0)
	zonenhw_files = []
	for i, zonenh_path in enumerate(to_reweld):
		zonenhw_path = zonenh_path.with_suffix(".zonenhw")
		result = ryzom_zone_tools.run_zone_welder(ctx.binaries["zone_welder"], zonenh_path, zonenhw_path)
		if not zonenhw_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonenhw_path.name}:\n{result.stdout}\n{result.stderr}")
		zonenhw_files.append(zonenhw_path)
		progress(2, (i + 1) / len(to_reweld))
	return zonenhw_files


def _run_elevation(ctx: _BuildContext, zonenhw_files: List[Path], progress) -> List[Path]:
	"""Stage 3 -- zone_elevation: applies the continent's two heightmaps to
	give each zone its real terrain Z. Written directly to zone_dir (the
	real pipeline-export "zone" subdir), not out_zone_dir."""
	progress(3, 1.0 if not zonenhw_files else 0.0)
	zone_files = []
	for i, zonenhw_path in enumerate(zonenhw_files):
		zone_path = ctx.zone_dir / zonenhw_path.with_suffix(".zone").name
		elevation_cfg = ryzom_land_tools.ElevationConfig(
			input_zonenhw=str(zonenhw_path),
			output_zonew=str(zone_path),
			land_file=ctx.land_cfg.zone_region_file,
			cell_size=ctx.land_cfg.cell_size,
			z_factor=ctx.land_cfg.z_factor_1,
			height_map=ctx.land_cfg.height_map_file_1 or None,
			z_factor_2=ctx.land_cfg.z_factor_2,
			height_map_2=ctx.land_cfg.height_map_file_2 or None,
			extend_coords=ctx.land_cfg.extend_coords,
		)
		result = ryzom_land_tools.run_zone_elevation(ctx.binaries["zone_elevation"], elevation_cfg)
		if not zone_path.is_file():
			raise LandBuildError(f"zone_elevation failed to produce {zone_path.name}:\n{result.stdout}\n{result.stderr}")
		zone_files.append(zone_path)
		progress(3, (i + 1) / len(zonenhw_files))
	return zone_files


def _run_weld_pass2(ctx: _BuildContext, zone_files: List[Path], progress) -> Set[str]:
	"""Stage 4 -- zone_welder, pass 2 (.zone -> .zonew): fixes any seams
	zone_elevation's per-zone heightmap sampling introduced at borders.
	Written to zone_weld_dir (the real pipeline-export "zone_weld" subdir).
	Returns the set of zone stems actually rewelded this run."""
	progress(4, 1.0 if not zone_files else 0.0)
	rebuilt_zonew_names = set()
	for i, zone_path in enumerate(zone_files):
		zonew_path = ctx.zone_weld_dir / zone_path.with_suffix(".zonew").name
		result = ryzom_zone_tools.run_zone_welder(ctx.binaries["zone_welder"], zone_path, zonew_path)
		if not zonew_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonew_path.name}:\n{result.stdout}\n{result.stderr}")
		rebuilt_zonew_names.add(zone_path.stem)
		progress(4, (i + 1) / len(zone_files))
	return rebuilt_zonew_names


def _run_zone_dependencies(ctx: _BuildContext, zonew_files: List[Path], progress) -> Path:
	"""Stage 5 -- zone_dependencies: one shadow-dependency pass over the
	whole continent's zone range at once (docs/zone_tools.md §4). Returns
	the properties.cfg path it wrote, reused by Stages 6/7.

	Two real native bugs, already found/documented 2026-09-11 running this
	same tool over nexus (project-todos/pynel/land_pipeline.md step 4,
	nel/tools/pynel/docs/zone_tools.md §4), both handled here:
	1. `firstZone`/`lastZone` must decode to an already-ascending
	   [minRow..maxRow] x [minCol..maxCol] range on BOTH axes independently
	   -- zone_dependencies' own min/max swap has a copy-paste bug that
	   collapses an axis needing a swap to a single row/column instead.
	   Picking the two REAL zones with the smallest/largest (row, col)
	   tuple is NOT enough -- instead, build two SYNTHETIC corner names
	   (`<minRow>_<minCol>`/`<maxRow>_<maxCol>`) guaranteed ascending on
	   both axes at once. Neither name needs to correspond to a real file.
	2. The tool writes ONE `.depend` FILE PER ZONE, not a single file
	   matching argv[4] literally -- see _depend_path_for()."""
	progress(5)
	properties_cfg_path = ctx.out_zone_dir / f"{ctx.continent}_properties.cfg"
	properties_cfg_path.write_text(ryzom_land_tools.build_properties_cfg(ctx.props_cfg))
	zone_keys = [_zone_sort_key(p.stem) for p in zonew_files]
	min_row = min(row for row, _ in zone_keys)
	max_row = max(row for row, _ in zone_keys)
	min_col = min(col for _, col in zone_keys)
	max_col = max(col for _, col in zone_keys)
	first_zone_name = _zone_name_from_row_col(min_row, min_col)
	last_zone_name = _zone_name_from_row_col(max_row, max_col)
	depend_arg_path = ctx.zone_weld_dir / f"{ctx.continent}.depend"
	result = ryzom_land_tools.run_zone_dependencies(
		ctx.binaries["zone_dependencies"], properties_cfg_path,
		ctx.zone_weld_dir / f"{first_zone_name}.zonew", ctx.zone_weld_dir / f"{last_zone_name}.zonew", depend_arg_path,
	)
	produced_depends = [p for p in zonew_files if _depend_path_for(ctx.zone_weld_dir, p.stem).is_file()]
	if not produced_depends:
		raise LandBuildError(f"zone_dependencies produced no .depend file in {ctx.out_zone_dir}:\n{result.stdout}\n{result.stderr}")
	return properties_cfg_path


def _run_zone_lighter(
	ctx: _BuildContext, zonew_files: List[Path], rebuilt_zonew_names: Set[str],
	properties_cfg_path: Path, on_zone_ready, progress,
) -> Tuple[List[Path], List[Path]]:
	"""Stage 6 -- zone_lighter (.zonew -> .zonel), written to
	zone_lighted_dir -- a zone needs lighting if it has no `.zonel` yet OR
	its `.zonew` was just rebuilt this run: a freshly rewelded `.zonew`
	makes any OLD `.zonel` next to it stale even if one happened to exist.
	Parallelized across zones (Nuno 2026-09-11): zone_lighter dominates
	per-zone time (~2s each), and each zone's own input/output files are
	fully independent. `zone_lighter`'s OWN internal multi-threading
	(`cpu_num`) is deliberately left at 1 (continent_pipeline_reference.py's
	own docstring) -- this is PROCESS-level concurrency, one native
	`zone_lighter` per worker. Returns (zonel_files, to_light) -- `to_light`
	reused as-is by Stage 7 (zone_ig_lighter needs the exact same set)."""
	already_lit_names = {p.stem for p in zonew_files if (ctx.zone_lighted_dir / f"{p.stem}.zonel").is_file()}
	to_light = [p for p in zonew_files if p.stem in rebuilt_zonew_names or p.stem not in already_lit_names]
	progress(6, 1.0 if not to_light else 0.0)
	lit_count = 0
	lit_count_lock = threading.Lock()

	def light_one(zonew_path):
		nonlocal lit_count
		zonel_path = ctx.zone_lighted_dir / zonew_path.with_suffix(".zonel").name
		zone_depend_path = _depend_path_for(ctx.zone_weld_dir, zonew_path.stem)
		result = ryzom_zone_tools.run_zone_lighter(
			ctx.binaries["zone_lighter"], zonew_path, zonel_path, properties_cfg_path, zone_depend_path,
		)
		if not zonel_path.is_file():
			raise LandBuildError(f"zone_lighter failed to produce {zonel_path.name}:\n{result.stdout}\n{result.stderr}")
		with lit_count_lock:
			lit_count += 1
			progress(6, lit_count / len(to_light))
		if on_zone_ready is not None:
			try:
				zone = parse_zone(zonel_path.read_bytes())
				on_zone_ready(zonel_path.stem, zone_to_cache_data(zone))
			except (OSError, ZoneParseError):
				pass

	worker_count = os.cpu_count() or 4
	with ThreadPoolExecutor(max_workers=worker_count) as pool:
		list(pool.map(light_one, to_light))

	zonel_files = [ctx.zone_lighted_dir / f"{p.stem}.zonel" for p in zonew_files]
	missing_zonel = [p for p in zonel_files if not p.is_file()]
	if missing_zonel:
		raise LandBuildError(f"missing .zonel for {len(missing_zonel)} zone(s): {[p.name for p in missing_zonel[:5]]}")
	return zonel_files, to_light


def _run_zone_ig_lighter(
	ctx: _BuildContext, zonel_files: List[Path], to_light: List[Path], properties_cfg_path: Path, progress,
) -> None:
	"""Stage 7 -- zone_ig_lighter: lights each zone JUST RELIT's own .ig
	(looked up automatically by the native tool via properties_cfg's
	search_pathes) -- same `to_light` set as Stage 6. A cell with no
	matching .ig anywhere in search_pathes is simply skipped, never a hard
	failure."""
	to_light_names = {p.stem for p in to_light}
	ig_to_light = [p for p in zonel_files if p.stem in to_light_names]
	progress(7, 1.0 if not ig_to_light else 0.0)
	for i, zonel_path in enumerate(ig_to_light):
		output_ig_path = ctx.out_ig_dir / f"{zonel_path.stem}.ig"
		zone_depend_path = _depend_path_for(ctx.zone_weld_dir, zonel_path.stem)
		ryzom_land_tools.run_zone_ig_lighter(
			ctx.binaries["zone_ig_lighter"], zonel_path, output_ig_path, properties_cfg_path, zone_depend_path,
		)
		progress(7, (i + 1) / len(ig_to_light))


def _run_build_ig_boxes(ctx: _BuildContext, progress) -> Path:
	"""Stage 8a -- build_ig_boxes: one bbox per real .ig (land + other) and
	every .shape it references."""
	progress(8)
	ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
	ig_stems = sorted({p.stem for p in ctx.out_ig_dir.glob("*.ig")} | {p.stem for p in ctx.ig_other_effective_dir.glob("*.ig")})
	ig_boxes_cfg = ryzom_pacs_tools.IgBoxesConfig(
		pathes=[str(ctx.out_ig_dir), str(ctx.ig_other_effective_dir)] + ctx.props_cfg.search_pathes,
		igs=ig_stems,
		output=str(ig_boxes_path),
	)
	ryzom_pacs_tools.run_build_ig_boxes(ctx.binaries["build_ig_boxes"], ig_boxes_cfg, ctx.rbank_bbox_dir)
	if not ig_boxes_path.is_file():
		raise LandBuildError(f"build_ig_boxes produced no {ig_boxes_path.name}")
	return ig_boxes_path


def _build_rbank_config(ctx: _BuildContext, zonew_files: List[Path], ig_boxes_path: Path) -> "ryzom_pacs_tools.RbankConfig":
	"""Shared `build_rbank` config for Stages 8b/8c/8d (check-prims,
	per-zone, global) -- region bounds use the same synthetic-corner
	convention as Stage 5's own zone_dependencies range, uppercased per
	RbankZoneUl/RbankZoneDr's own real on-disk convention (e.g. "39_BX").
	The ~15 RBank flags below are identical across all 25 continents
	(verified 2026-09-15, every continent's own leveldesign/workspace/
	continents/<c>/process.py) -- never a per-continent CSV column."""
	zone_keys = [_zone_sort_key(p.stem) for p in zonew_files]
	min_row = min(row for row, _ in zone_keys)
	max_row = max(row for row, _ in zone_keys)
	min_col = min(col for _, col in zone_keys)
	max_col = max(col for _, col in zone_keys)
	zone_ul = _zone_name_from_row_col(min_row, min_col).upper()
	zone_dr = _zone_name_from_row_col(max_row, max_col).upper()
	smallbank_path_str, _farbank_path_str = cpr.resolve_smallbank_paths(ctx.ryzom_data_path, ctx.continent, ctx.table)
	return ryzom_pacs_tools.RbankConfig(
		verbose=False, check_consistency=False,
		zone_path=str(ctx.zone_weld_dir) + "/",
		banks_path=str(Path(smallbank_path_str).parent) + "/",
		bank=smallbank_path_str,
		ig_boxes=str(ig_boxes_path),
		level_design_world_path=ctx.land_cfg.continents_dir + "/",
		ig_land_path=str(ctx.out_ig_dir) + "/",
		ig_village_path=str(ctx.ig_other_effective_dir) + "/",
		tessellation_path=str(ctx.rbank_tessellation_dir) + "/",
		tessellate_level=1,
		output_root_path=str(ctx.common_dir) + "/",
		smooth_directory="rbank_smooth/",
		raw_directory="rbank_raw/",
		reduce_surfaces=True, smooth_borders=True, compute_elevation=False,
		compute_levels=True, link_elements=True, cut_edges=True, use_zone_square=False,
		zone_ul=zone_ul, zone_dr=zone_dr,
		preprocess_directory=str(ctx.rbank_preproc_dir) + "/",
		global_retriever="temp.gr", retriever_bank="temp.rbank",
		zones=[f"{p.stem}.zonew" for p in zonew_files],
		pathes=[ctx.land_cfg.continents_dir] + ctx.props_cfg.search_pathes,
	)


def _run_build_rbank_check(ctx: _BuildContext, rbank_cfg, zonew_files: List[Path], progress) -> None:
	"""Stage 8b -- build_rbank, check prims pass (validates collision
	primitives referenced by every zone, no .lr/.rbank/.gr output of its
	own). Writes `check_prims_manifest.json` (project-todos/forgery/
	landscape_editor__pacs_export.md step 7k) recording every validated
	`.zonew`'s mtime+size, read back by _check_prims_status() to give the
	"Pipeline steps" tick a real signal instead of the permanently-neutral
	one it had before this marker existed."""
	progress(9)
	ryzom_pacs_tools.run_build_rbank_check_prims(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir)
	manifest = {}
	for zonew_path in zonew_files:
		stat = zonew_path.stat()
		manifest[zonew_path.stem] = [stat.st_mtime, stat.st_size]
	(ctx.rbank_preproc_dir / _CHECK_PRIMS_MANIFEST_NAME).write_text(json.dumps({"zones": manifest}))


def _run_build_rbank_zones(
	ctx: _BuildContext, rbank_cfg, zonew_files: List[Path], rebuilt_zonew_names: Set[str], progress,
) -> List[str]:
	"""Stage 8c -- build_rbank, per-zone pass (tessellation + moulinage),
	one .lr per zone under rbank_smooth/ -- incremental: a zone needs
	rebuilding if ITS OWN .zonew just changed this run, was never built, or
	ANY of its 8 neighbors' .zonew is newer than its own .lr (border
	continuity, computed here from zone name row/col arithmetic rather than
	depending on the get_neighbors binary, which this checkout doesn't even
	have source for). Returns the `.zonew` filenames actually rebuilt."""
	ctx.rbank_smooth_dir.mkdir(parents=True, exist_ok=True)
	all_zone_keys = {_zone_sort_key(p.stem): p.stem for p in zonew_files}
	zones_needing_rebuild = [
		f"{p.stem}.zonew" for p in zonew_files
		if _zone_needs_rbank_zone_rebuild(ctx.zone_weld_dir, ctx.rbank_smooth_dir, p.stem, all_zone_keys, rebuilt_zonew_names)
	]
	progress(10, 1.0 if not zones_needing_rebuild else None)
	if zones_needing_rebuild:
		ryzom_pacs_tools.run_build_rbank_zones(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir, zones_needing_rebuild)
	return zones_needing_rebuild


def _run_build_rbank_global(ctx: _BuildContext, rbank_cfg, zones_needing_rebuild: List[str], progress) -> None:
	"""Stage 8d -- build_rbank, global pass (merges every zone's .lr into
	temp.rbank/temp.gr, written under rbank_smooth_dir) -- needed whenever
	any zone was just rebuilt this run OR the merge's own temp.rbank is
	missing/older than the newest .lr on disk (own real age indicator,
	Nuno 2026-09-17 -- NOT just "zones_needing_rebuild" alone, which gave
	this pass no way to be forced independently)."""
	needs_global_rebuild = _rbank_global_needs_rebuild(ctx.rbank_smooth_dir, zones_needing_rebuild)
	progress(11, 1.0 if not needs_global_rebuild else None)
	if needs_global_rebuild:
		ryzom_pacs_tools.run_build_rbank_global(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir)


def _run_build_indoor_rbank(ctx: _BuildContext, progress) -> None:
	"""Stage 8e -- build_indoor_rbank: merges every .cmb (interior collision
	mesh) it finds under rbank_cmb_export_dir into temp.rbank/temp.gr -- an
	empty Meshes list is still a valid, harmless call. Requires
	rbank_smooth_dir/temp.rbank to already exist (its own openAndSerial()
	throws -- an unhandled SIGABRT -- on a missing one)."""
	progress(12)
	cmb_stems = sorted(p.stem for p in ctx.rbank_cmb_export_dir.glob("*.cmb"))
	indoor_cfg = ryzom_pacs_tools.IndoorRbankConfig(
		mesh_path=str(ctx.rbank_cmb_export_dir) + "/",
		meshes=cmb_stems,
		output_path=str(ctx.rbank_retrievers_dir) + "/",
		# rbank_smooth_dir, NOT rbank_preproc_dir: build_rbank's own global
		# pass writes temp.rbank/temp.gr under SmoothDirectory (confirmed on
		# disk 2026-09-16).
		merge_path=str(ctx.rbank_smooth_dir) + "/",
	)
	ryzom_pacs_tools.run_build_indoor_rbank(ctx.binaries["build_indoor_rbank"], indoor_cfg, ctx.rbank_preproc_dir)


def _run_final_rename(ctx: _BuildContext, progress) -> None:
	"""Stage 8f -- final rename: tempMerged.* -> <continent>.* (a real
	filename-prefix swap, not a content transformation). Silently does
	nothing for whichever extension isn't present -- tolerant by design,
	same as the native pipeline's own "Move gr, rbank and lr" step."""
	progress(13)
	for extension in (".rbank", ".gr"):
		src = ctx.rbank_retrievers_dir / f"tempMerged{extension}"
		if src.is_file():
			shutil.copyfile(src, ctx.rbank_output_dir / f"{ctx.continent}{extension}")
	for src in ctx.rbank_retrievers_dir.glob("tempMerged_*.lr"):
		dst_name = ctx.continent + src.name[len("tempMerged"):]
		shutil.copyfile(src, ctx.rbank_output_dir / dst_name)
	progress(13, 1.0)


def _run_ai_wmap(ctx: _BuildContext, progress) -> None:
	"""Stage 9 -- ai_wmap_export: regenerates the continent's AI pathfinding
	walkability maps. `AiWmapExportError` is translated to `LandBuildError`
	so every caller keeps dealing with a single exception type."""
	progress(14)
	try:
		ai_wmap_export.generate_ai_wmap(ctx.ryzom_data_path, ctx.continent)
	except ai_wmap_export.AiWmapExportError as exc:
		raise LandBuildError(str(exc))
	progress(14, 1.0)


def run_full_build(
	ryzom_data_path, continent: str, live_data_path=None,
	on_progress: Optional[Callable[[int, int, str, Optional[float]], None]] = None,
	on_zone_ready: Optional[Callable[[str, object], None]] = None,
) -> List[str]:
	"""Runs every stage for `continent`, returns the final `.zonel` paths
	produced. `on_progress(step_index, step_count, step_name, fraction)`, if
	given, is called with a 1-based `step_index` (out of the fixed
	`step_count == len(_STEP_NAMES)`) each time a new stage starts, and again
	whenever that stage's own `fraction` (0-1, `None` if the stage can't be
	quantified) is updated. `on_zone_ready(name, cache_data)`, if given, is
	called once per zone right after `zone_lighter` produces its final
	`.zonel`. `live_data_path`, if given, is used to merge each zone's own
	hand-added "land" flora diff onto land_export's freshly-produced `.ig`
	(Stage 1bis) -- `None` (no merge attempted) only for a caller that
	doesn't have it available."""
	def progress(step_index: int, fraction: Optional[float] = None) -> None:
		if on_progress is not None:
			on_progress(step_index + 1, len(_STEP_NAMES), _STEP_NAMES[step_index], fraction)

	ctx = _resolve_context(ryzom_data_path, continent)
	_ensure_binaries(ctx, _BINARY_NAMES)
	_ensure_zone_dirs(ctx)

	# Stage 1 -- land_export.
	progress(0)
	zonenh_files = _run_land_export(ctx)

	# Stage 1bis -- flora merge.
	_run_flora_merge(ctx, zonenh_files, live_data_path, progress)

	# Incremental rebuild (Nuno 2026-09-12), per REAL STAGE independently --
	# NOT "has .zonel" alone: needing weld/elevation is its OWN check
	# (missing `.zonew`, regardless of `.zonel`), and needing lighting is a
	# SEPARATE check (missing `.zonel`, OR the zone's `.zonew` was just
	# rebuilt this run).
	to_reweld = [p for p in zonenh_files if not (ctx.zone_weld_dir / f"{p.stem}.zonew").is_file()]

	# Stages 2-4 -- weld pass 1, elevation, weld pass 2.
	zonenhw_files = _run_weld_pass1(ctx, to_reweld, progress)
	zone_files = _run_elevation(ctx, zonenhw_files, progress)
	rebuilt_zonew_names = _run_weld_pass2(ctx, zone_files, progress)

	# The full current .zonew set (rebuilt this run + already up to date).
	zonew_files = [ctx.zone_weld_dir / f"{p.stem}.zonew" for p in zonenh_files]
	missing_zonew = [p for p in zonew_files if not p.is_file()]
	if missing_zonew:
		raise LandBuildError(f"missing .zonew for {len(missing_zonew)} zone(s) (never built): {[p.name for p in missing_zonew[:5]]}")

	# Stage 5 -- zone_dependencies.
	properties_cfg_path = _run_zone_dependencies(ctx, zonew_files, progress)

	# Stages 6-7 -- zone_lighter, zone_ig_lighter.
	zonel_files, to_light = _run_zone_lighter(ctx, zonew_files, rebuilt_zonew_names, properties_cfg_path, on_zone_ready, progress)
	_run_zone_ig_lighter(ctx, zonel_files, to_light, properties_cfg_path, progress)

	# Stage 8 -- PACS collision (project-todos/forgery/landscape_editor__
	# pacs_export.md): build_ig_boxes -> build_rbank (check prims, per-zone,
	# global) -> build_indoor_rbank -> final rename.
	_prepare_pacs_dirs(ctx)
	ig_boxes_path = _run_build_ig_boxes(ctx, progress)
	rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
	_run_build_rbank_check(ctx, rbank_cfg, zonew_files, progress)
	zones_needing_rebuild = _run_build_rbank_zones(ctx, rbank_cfg, zonew_files, rebuilt_zonew_names, progress)
	_run_build_rbank_global(ctx, rbank_cfg, zones_needing_rebuild, progress)
	_run_build_indoor_rbank(ctx, progress)
	_run_final_rename(ctx, progress)

	# Stage 9 -- ai_wmap_export.
	_run_ai_wmap(ctx, progress)

	return [str(p) for p in zonel_files]


def run_single_step(
	ryzom_data_path, continent: str, step_index: int, live_data_path=None,
	on_progress: Optional[Callable[[int, int, str, Optional[float]], None]] = None,
	on_zone_ready: Optional[Callable[[str, object], None]] = None,
) -> None:
	"""Rebuilds ONE step (or its whole shared-indicator group) right now,
	without waiting for the next full Build (project-todos/forgery/
	landscape_editor__pacs_export.md step 7i, Nuno 2026-09-18) -- reuses the
	same extracted stage functions as run_full_build(), for every zone of
	the continent (resolved fresh from disk, since this function doesn't
	get the caller's already-loaded zone set). Clears step_index's own
	cache first (_clear_step_cache(), a no-op for an always-redone step),
	THEN reruns exactly the stage(s) that step's group covers -- never
	anything upstream: raises LandBuildError, rather than cascading
	silently backwards, if an upstream stage's own output is missing on
	disk (e.g. rebuilding `build_rbank` per zone without `ig_boxes.bbox`
	already there from a previous full Build)."""
	def progress(idx: int, fraction: Optional[float] = None) -> None:
		if on_progress is not None:
			on_progress(idx + 1, len(_STEP_NAMES), _STEP_NAMES[idx], fraction)

	def require(path: Path, message: str) -> None:
		if not path.exists():
			raise LandBuildError(message)

	def existing_zonew_files() -> List[Path]:
		files = sorted(ctx.zone_weld_dir.glob("*.zonew"))
		if not files:
			raise LandBuildError(f"No .zonew found in {ctx.zone_weld_dir} -- run a full Build first.")
		return files

	ctx = _resolve_context(ryzom_data_path, continent)
	_ensure_zone_dirs(ctx)
	_clear_step_cache(ctx, step_index)

	if step_index == 0:
		_ensure_binaries(ctx, ("land_export",))
		_run_land_export(ctx)
	elif step_index == 1:
		zonenh_files = sorted(ctx.out_zone_dir.glob("*.zonenh"))
		if not zonenh_files:
			raise LandBuildError(f"No .zonenh found in {ctx.out_zone_dir} -- run land_export first.")
		_run_flora_merge(ctx, zonenh_files, live_data_path, progress)
	elif step_index in _STEP_GROUPS[0]:
		zonenh_files = sorted(ctx.out_zone_dir.glob("*.zonenh"))
		if not zonenh_files:
			raise LandBuildError(f"No .zonenh found in {ctx.out_zone_dir} -- run land_export first.")
		_ensure_binaries(ctx, ("zone_welder", "zone_elevation"))
		zonenhw_files = _run_weld_pass1(ctx, zonenh_files, progress)
		zone_files = _run_elevation(ctx, zonenhw_files, progress)
		_run_weld_pass2(ctx, zone_files, progress)
	elif step_index == 5:
		zonew_files = existing_zonew_files()
		_ensure_binaries(ctx, ("zone_dependencies",))
		_run_zone_dependencies(ctx, zonew_files, progress)
	elif step_index in _STEP_GROUPS[1]:
		zonew_files = existing_zonew_files()
		properties_cfg_path = ctx.out_zone_dir / f"{continent}_properties.cfg"
		require(properties_cfg_path, f"{properties_cfg_path} missing -- run zone_dependencies first.")
		_ensure_binaries(ctx, ("zone_lighter", "zone_ig_lighter"))
		zonel_files, _ = _run_zone_lighter(ctx, zonew_files, set(), properties_cfg_path, on_zone_ready, progress)
		_run_zone_ig_lighter(ctx, zonel_files, zonew_files, properties_cfg_path, progress)
	elif step_index == 8:
		existing_zonew_files()  # PACS needs a welded continent, even though build_ig_boxes itself only reads .ig
		_ensure_binaries(ctx, ("build_ig_boxes",))
		_prepare_pacs_dirs(ctx)
		_run_build_ig_boxes(ctx, progress)
	elif step_index == 9:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_check(ctx, rbank_cfg, zonew_files, progress)
	elif step_index == 10:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_zones(ctx, rbank_cfg, zonew_files, set(), progress)
	elif step_index == 11:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_global(ctx, rbank_cfg, [], progress)
	elif step_index == 12:
		require(ctx.rbank_smooth_dir / "temp.rbank", f"{ctx.rbank_smooth_dir / 'temp.rbank'} missing -- run build_rbank (global) first.")
		_ensure_binaries(ctx, ("build_indoor_rbank",))
		_run_build_indoor_rbank(ctx, progress)
	elif step_index == 13:
		_run_final_rename(ctx, progress)
	elif step_index == 14:
		_run_ai_wmap(ctx, progress)
	else:
		raise LandBuildError(f"unknown step index: {step_index}")
