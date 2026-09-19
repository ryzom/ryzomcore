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
landscape_editor__land_composition.md step 7, restructured project-todos/
forgery/pipeline_export_dirs_rename.md) that turns a `.land` composition
into final, installable game data -- `.zonel`/`.ig`, the PACS collision
region bank, and the AI pathfinding walkability maps:

	extract (live_data) -> land_export -> merge land -> merge other ->
	zone_welder -> zone_elevation -> zone_welder -> zone_dependencies ->
	zone_lighter -> zone_ig_lighter -> ig_lighter (other, CLI confirmed in
	nel/tools/build_gamedata/processes/ig_light/2_build.py) -> build_ig_boxes
	-> build_rbank -> build_indoor_rbank -> ai_wmap_export

Order confirmed in `nel/tools/build_gamedata/processes/zone/2_build.py` and
validated end-to-end 2026-09-11 against real `nexus` data (project-todos/
pynel/land_pipeline.md step 5), and again 2026-09-12 through this
orchestrator itself against real `bagne` data. PACS
(project-todos/forgery/landscape_editor__pacs_export.md) and
`ai_wmap_export` (project-todos/forgery/ai_wmap_flora_collision.md) were
added later, 2026-09-16/17, as Stages 8 and 9 below.

Directory layout (project-todos/forgery/pipeline_export_dirs_rename.md):
everything under `pipeline/continents/<c>/` is intermediate/preparatory --
never consumed by the game. Only 3 directories under
`pipeline/export/continents/<c>/` are the real, game-ready final output,
named exactly like their `live_data` `.bnp` counterpart: `<c>_zones/`
(`zone_lighter`'s `.zonel`), `<c>_ig/` (`zone_ig_lighter`'s land `.ig` AND
the other-lighting step's `.ig`, same directory, names never collide), and
`<c>_pacs/` (the final renamed PACS collision bank).

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
import subprocess
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
from . import ig_full_load
from . import region_loader
from . import settings as app_settings
from .zone_geometry import zone_to_cache_data

_BINARY_NAMES = (
	"land_export", "zone_welder", "zone_elevation", "zone_dependencies", "zone_lighter", "zone_ig_lighter",
	"ig_lighter", "build_ig_boxes", "build_rbank", "build_indoor_rbank",
)

_ZONE_NAME_RE = re.compile(r"^(\d+)_([A-Za-z]{2})$")

# Fixed, ordered list of run_full_build()'s stages (project-todos/forgery/
# landscape_editor__pacs_export.md step 7b, restructured project-todos/
# forgery/pipeline_export_dirs_rename.md) -- always 18 entries in this exact
# order, regardless of which stages actually do work on a given run (e.g.
# "Merging hand-added flora" is still announced, with fraction=1.0, even
# when live_data_path is None and nothing happens), so the popup's
# "Step X/18" numbering never jumps or depends on the continent/run.
_STEP_NAMES = (
	"Extracting flora/other from live_data",
	"Cutting composition into zones (land_export)",
	"Merging hand-added flora",
	"Merging other instance groups",
	"Welding (pass 1, no height)",
	"Applying heightmaps (zone_elevation)",
	"Welding (pass 2, with height)",
	"Computing shadow dependencies (zone_dependencies)",
	"Lighting zones (zone_lighter)",
	"Lighting land instance groups (zone_ig_lighter)",
	"Lighting other instance groups (ig_lighter)",
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
	"Extracting live_data",
	"Cutting composition",
	"Merging flora",
	"Merging other",
	"Welding pass1",
	"Applying heightmaps",
	"Welding pass2",
	"Computing dependencies",
	"Lighting zones",
	"Lighting land instances",
	"Lighting other instances",
	"Building boxes",
	"Checking primitives",
	"Building bank",
	"Merging bank",
	"Merging interiors",
	"Finalizing PACS",
	"Regenerating walkability",
)

# Native tool actually invoked by each stage, shown as a tooltip next to its
# short label -- "None (Python)" for the stages with no native tool at all
# (extraction, both merges, and the final rename are pure Python file
# copies/edits).
_STEP_TOOLS = (
	"None (Python)",
	"land_export",
	"None (Python)",
	"None (Python)",
	"zone_welder",
	"zone_elevation",
	"zone_welder",
	"zone_dependencies",
	"zone_lighter",
	"zone_ig_lighter",
	"ig_lighter",
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
# rereading each stage's own code above, not assumed). Extraction and both
# merges (indices 0, 2, 3) join this set in the restructured pipeline
# (project-todos/forgery/pipeline_export_dirs_rename.md): safe to always
# redo since hand edits live in separate override files the merge never
# writes to. Other-lighting (index 10) is also always redone -- idempotent
# by design (same reasoning as zone_ig_lighter), and NOT YET a real native
# call anyway.
_UNCACHED_STEP_INDICES = frozenset({0, 1, 2, 3, 7, 10, 11, 12, 15, 16, 17})

# Flora merge (index 2) and build_rbank's check-prims pass (index 12) don't
# themselves produce an output file to check -- both now write their own
# small marker after a successful run (project-todos/forgery/
# landscape_editor__pacs_export.md step 7k, Nuno 2026-09-18) so
# compute_step_statuses() can give them a real tick too, instead of the
# permanently-neutral one they had before this marker existed.
_FLORA_MARKER_SUFFIX = ".floramerge.json"
_CHECK_PRIMS_MANIFEST_NAME = "check_prims_manifest.json"

# Every step used to potentially share a cache indicator/[rebuild] button
# with others (Nuno 2026-09-17: "un bouton par groupe") -- back when several
# consecutive stages rewrote the same file in place, so none of them had an
# independent on-disk signal of its own. No longer true for ANY step since
# project-todos/forgery/pipeline_export_dirs_rename.md gave every stage its
# own real file/directory and (mostly) its own native tool: weld pass 1 (its
# own `.zonenhw` in zone_nhw_dir, a real bug found 2026-09-19 -- it used to
# write next to the input instead), zone_elevation (`.zone` in zone_dir) and
# weld pass 2 (`.zonew` in zone_weld_dir) were the last group standing;
# zone_lighter/zone_ig_lighter (land) were ungrouped the same session. Nuno
# 2026-09-19, both times: "ça n'a ni les mêmes fichiers, ni la même
# commande, c'est débile de les avoir réunis" -- every step now has its own
# button, its own cache indicator (`_clear_step_cache()`), and its own real
# destination folder (`compute_step_statuses()`'s `destination_dirs`).


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
	configured just to rebuild one step that doesn't need most of them.

	Directory layout restructured project-todos/forgery/
	pipeline_export_dirs_rename.md: everything below `common_dir`
	(`pipeline/continents/<continent>/`) is intermediate/preparatory.
	`final_zones_dir`/`final_ig_dir`/`final_pacs_dir` are the only real,
	game-ready output, under `pipeline/export/continents/<continent>/`,
	named exactly like their `live_data` `.bnp` counterpart."""
	ryzom_data_path: object
	continent: str
	table: object
	land_cfg: object
	props_cfg: object
	common_dir: Path
	out_zone_dir: Path
	zones_dir: Path
	zone_nhw_dir: Path
	zone_dir: Path
	zone_weld_dir: Path
	zone_depends_dir: Path
	out_ig_dir: Path
	igs_flora_dir: Path
	igs_land_merged_dir: Path
	igs_other_merged_dir: Path
	rbank_dir: Path
	rbank_bbox_dir: Path
	rbank_tessellation_dir: Path
	rbank_preproc_dir: Path
	rbank_lrs_dir: Path
	rbank_retrievers_dir: Path
	ai_wmap_dir: Path
	final_zones_dir: Path
	final_ig_dir: Path
	final_pacs_dir: Path
	binaries: Dict[str, Path] = field(default_factory=dict)


def _resolve_context(ryzom_data_path, continent: str) -> _BuildContext:
	"""Resolves every directory run_full_build()/run_single_step() read/write
	for `continent`, without running anything -- `common_dir`
	(`pipeline/continents/<continent>/`) is the intermediate root every
	scratch directory sits next to, computed directly from `continent`
	(project-todos/forgery/pipeline_export_dirs_rename.md), NOT derived from
	`out_ig_dir` anymore (`out_ig_dir` now nests one level deeper, under its
	own `igs/` subfolder, so `.parent` alone no longer reaches the real
	continent root). `rbank_lrs_dir` holds BOTH the per-zone `.lr` AND
	`temp.rbank`/`temp.gr` -- the native `build_rbank` tool itself only
	exposes ONE `SmoothDirectory` setting for both, so they can't be split
	into separate folders (found while implementing project-todos/forgery/
	pipeline_export_dirs_rename.md step 9 -- flagged to Nuno, kept as one
	directory rather than guess at an unverified pynel/native-tool API)."""
	table = cpr.load_reference_table(ryzom_data_path)
	land_cfg = cpr.build_land_export_config(ryzom_data_path, continent, table)
	props_cfg = cpr.build_properties_config(ryzom_data_path, continent, table)
	out_zone_dir = Path(land_cfg.out_zone_dir)
	out_ig_dir = Path(land_cfg.out_ig_dir)
	common_dir = Path(ryzom_data_path) / "pipeline" / "continents" / continent
	zones_dir = common_dir / "zones"
	igs_dir = common_dir / "igs"
	rbank_dir = common_dir / "rbank"
	final_dir = region_loader.continent_final_export_dir(ryzom_data_path, continent)
	return _BuildContext(
		ryzom_data_path=ryzom_data_path, continent=continent, table=table,
		land_cfg=land_cfg, props_cfg=props_cfg, common_dir=common_dir,
		out_zone_dir=out_zone_dir,
		zones_dir=zones_dir,
		zone_nhw_dir=zones_dir / "nhw",
		zone_dir=zones_dir / "raw",
		zone_weld_dir=zones_dir / "welded",
		zone_depends_dir=zones_dir / "depends",
		out_ig_dir=out_ig_dir,
		igs_flora_dir=igs_dir / "flora",
		igs_land_merged_dir=igs_dir / "land_merged",
		igs_other_merged_dir=igs_dir / "other_merged",
		rbank_dir=rbank_dir,
		rbank_bbox_dir=rbank_dir / "bbox",
		rbank_tessellation_dir=rbank_dir / "tessellation",
		rbank_preproc_dir=rbank_dir / "preproc",
		rbank_lrs_dir=rbank_dir / "lrs",
		rbank_retrievers_dir=rbank_dir / "merged",
		ai_wmap_dir=common_dir / "wmap",
		final_zones_dir=final_dir / f"{continent}_zones",
		final_ig_dir=final_dir / f"{continent}_ig",
		final_pacs_dir=final_dir / f"{continent}_pacs",
	)


def resolve_build_dirs(ryzom_data_path, continent) -> Dict[str, Path]:
	"""Public subset of _resolve_context(), kept for any external caller
	that only needs the directories, not the full config objects."""
	ctx = _resolve_context(ryzom_data_path, continent)
	return {
		"out_zone_dir": ctx.out_zone_dir,
		"zone_weld_dir": ctx.zone_weld_dir,
		"final_zones_dir": ctx.final_zones_dir,
		"out_ig_dir": ctx.out_ig_dir,
		"final_ig_dir": ctx.final_ig_dir,
		"rbank_bbox_dir": ctx.rbank_bbox_dir,
		"rbank_lrs_dir": ctx.rbank_lrs_dir,
		"rbank_retrievers_dir": ctx.rbank_retrievers_dir,
		"final_pacs_dir": ctx.final_pacs_dir,
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
		ctx.out_zone_dir, ctx.zone_nhw_dir, ctx.zone_dir, ctx.zone_weld_dir, ctx.zone_depends_dir,
		ctx.igs_flora_dir, ctx.igs_land_merged_dir, ctx.igs_other_merged_dir,
		ctx.out_ig_dir, ctx.land_cfg.out_cmb_dir, ctx.land_cfg.additionnal_ig_out_dir,
		ctx.final_zones_dir, ctx.final_ig_dir, ctx.final_pacs_dir,
	):
		Path(directory).mkdir(parents=True, exist_ok=True)


def _depend_path_for(zone_depends_dir: Path, zone_stem: str) -> Path:
	"""zone_dependencies writes ONE `.depend` FILE PER ZONE, not a single
	file matching its own argv[4] literally -- always lowercased
	(`toLowerAscii`, zone_dependencies.cpp:528) regardless of the real zone
	name's case."""
	return zone_depends_dir / f"{zone_stem.lower()}.depend"


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
	zone_weld_dir: Path, rbank_lrs_dir: Path, zone_stem: str,
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
	lr_path = rbank_lrs_dir / f"{zone_stem}.lr"
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


def _rbank_global_needs_rebuild(rbank_lrs_dir: Path, zones_needing_rebuild=()) -> bool:
	"""True if `build_rbank` (global)'s own output (`temp.rbank`) is missing
	or stale -- shared by _run_build_rbank_global() and
	compute_step_statuses() for the same reason as
	_zone_needs_rbank_zone_rebuild() above (step 7h: this used to be a
	presence-only check in compute_step_statuses(), unlike the real
	mtime-based age indicator run_full_build() already used)."""
	rbank_global_output = rbank_lrs_dir / "temp.rbank"
	if zones_needing_rebuild or not rbank_global_output.is_file():
		return True
	global_mtime = rbank_global_output.stat().st_mtime
	newer_found = False
	for p in rbank_lrs_dir.glob("*.lr"):
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
	landscape_editor__pacs_export.md steps 7f-7k). `has_own_button`/
	`group_size` support GROUPING several consecutive steps under one shared
	[rebuild] button and cache indicator (project-todos/forgery/
	landscape_editor__pacs_export.md step 7l, Nuno 2026-09-18: "garde la
	cellule mais fais 3 lignes") -- `has_own_button` False only for a step
	sharing an earlier step's indicator, `group_size` the number of
	consecutive entries such a button/row covers. No step is actually
	grouped anymore as of project-todos/forgery/pipeline_export_dirs_rename.md
	(Nuno 2026-09-19, about the last 2 remaining groups: "ça n'a ni les
	mêmes fichiers, ni la même commande, c'est débile de les avoir réunis")
	-- every step today has `has_own_button=True`/`group_size=1`; the
	mechanism stays in place for a future step that might genuinely need it
	again. `up_to_date` is `True` (green tick) or `False` (pink tick);
	`warning`, if not `None`, is a message the UI shows instead of the plain
	tick (e.g. flora merge with no `live_data_path` configured -- data will
	silently be incomplete, Nuno 2026-09-18: "s'il manque quoi que ce soit,
	c'est warning obligatoire"). For a checkable stage, `built_count` out of
	`total_count` zones (or files) are already up to date on disk --
	`total_count == 1` for a continent-wide indicator rather than a per-zone
	one, `total_count == 3` for ai_wmap_export's 3 maps. `destination_dir`
	(project-todos/forgery/pipeline_export_dirs_rename.md step 15b) is this
	step's own real output directory, `None` for a step with no single
	directory of its own -- always per-STEP, never per-group: a grouped row
	still opens its own real folder, only the [rebuild] button is shared.
	`can_build`/`blocked_reason` (project-todos/forgery/
	pipeline_export_dirs_rename.md step 15c) -- `False` when this step's own
	real input isn't there yet (e.g. no `.zonenh` for weld pass 1), computed
	with the EXACT SAME condition `run_single_step()` itself uses to raise
	`LandBuildError` -- the UI freezes the [rebuild]/[Build] button on this
	rather than letting the user hit the same error after the fact."""
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
	destination_dir: Optional[Path] = None
	can_build: bool = True
	blocked_reason: Optional[str] = None


def _extraction_status(ctx: _BuildContext, live_data_path) -> Tuple[int, int, bool, Optional[str]]:
	"""Status for extraction (index 0, project-todos/forgery/
	pipeline_export_dirs_rename.md) -- a real X/Y count against `live_data`'s
	own ref list (`ig_full_load.list_continent_ig_refs_visualisation()`),
	same `total > 0 and built == total` convention as every other per-item
	check in this module: a bug found by Nuno (2026-09-19, right after
	wiping every output and reloading Atyscape) had this unconditionally
	return "up to date" whenever `live_data_path` was merely configured,
	with zero regard for whether any file had actually been written --
	exactly the vacuous-truth regression already fixed once elsewhere in
	this module (2026-09-18, empty `bagne/`), reintroduced here."""
	if live_data_path is None:
		return 0, 1, False, (
			"live_data_path is not configured -- extraction is skipped, "
			"the .ig produced will be missing any hand-placed flora/collision markers."
		)
	refs = ig_full_load.list_continent_ig_refs_visualisation(live_data_path, ctx.continent)
	total = len(refs)
	if total == 0:
		return 0, 1, False, None
	other_dir = Path(ctx.land_cfg.additionnal_ig_out_dir)
	built = 0
	for ref in refs:
		# Zone-named entries are written under the UPPERCASE-normalized name
		# (project-todos/forgery/pipeline_export_dirs_rename.md -- live_data
		# itself names them lowercase, e.g. "65_bz.ig"), same as
		# `_run_extraction()` itself -- checking presence under the raw
		# `ref.name` here caused the exact same case bug to resurface as a
		# false "incomplete" status right after the real fix (Nuno
		# 2026-09-19: "7/125" shown despite all 118 zone files really there).
		if _ZONE_NAME_RE.match(ref.name):
			out_dir, name = ctx.igs_flora_dir, ref.name.upper()
		else:
			out_dir, name = other_dir, ref.name
		if (out_dir / f"{name}.ig").is_file():
			built += 1
	return built, total, built == total, None


def _land_merge_status(ctx: _BuildContext, zone_names: List[str], live_data_path) -> Tuple[int, int, bool, Optional[str]]:
	"""Status for the land merge (index 2, project-todos/forgery/
	landscape_editor__pacs_export.md step 7k, adapted project-todos/forgery/
	pipeline_export_dirs_rename.md). No `live_data_path` configured is a
	WARNING, not a plain pink tick (Nuno 2026-09-18): extraction is skipped
	in that case, so the merge only ever sees land_export's own content,
	missing whatever hand-placed flora/collision markers `live_data` has.
	Otherwise, a zone is up to date only if its own `<zone>.floramerge.json`
	marker (now under `igs_land_merged_dir`) still matches BOTH the merged
	`.ig` (mtime+size -- stale the moment land_export or the extraction
	rerun) and whichever source was actually used at merge time (the
	override if present, else the flora extraction, or the marker recording
	neither existed). A zone with no `.ig` at all (no land_export ref
	content, no live_data flora either -- a normal undecorated terrain zone,
	confirmed 2026-09-19 real: 223/236 `newbieland` zones) is up to date once
	its marker confirms that and the real `.ig` still doesn't exist."""
	if live_data_path is None:
		return 0, len(zone_names), False, (
			"live_data_path is not configured -- extraction is skipped, "
			"the .ig produced will be missing any hand-placed flora/collision markers."
		)
	override_dir = ig_editing.land_ig_work_dir(ctx.ryzom_data_path, ctx.continent)
	up_to_date_count = 0
	for zone_name in zone_names:
		ig_path = ctx.igs_land_merged_dir / f"{zone_name}.ig"
		marker_path = ctx.igs_land_merged_dir / f"{zone_name}{_FLORA_MARKER_SUFFIX}"
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
		source_path = override_dir / f"{zone_name}.ig"
		if not source_path.is_file():
			source_path = ctx.igs_flora_dir / f"{zone_name}.ig"
		try:
			source_stat = source_path.stat()
		except FileNotFoundError:
			if marker.get("source_mtime") is not None:
				continue
		else:
			if marker.get("source_mtime") != source_stat.st_mtime or marker.get("source_size") != source_stat.st_size:
				continue
		up_to_date_count += 1
	total = len(zone_names)
	return up_to_date_count, total, total > 0 and up_to_date_count == total, None


def _other_merge_status(ctx: _BuildContext) -> Tuple[int, int, bool]:
	"""Status for the other merge (index 3, project-todos/forgery/
	pipeline_export_dirs_rename.md) -- always redone (index 3 is in
	`_UNCACHED_STEP_INDICES`), a plain presence check is enough: up to date
	once `igs_other_merged_dir` has at least one `.ig`."""
	present = ctx.igs_other_merged_dir.is_dir() and any(ctx.igs_other_merged_dir.glob("*.ig"))
	return (1 if present else 0), 1, present


def _other_lighting_status(ctx: _BuildContext) -> Tuple[int, int, bool]:
	"""Status for other-lighting (index 10, project-todos/forgery/
	pipeline_export_dirs_rename.md) -- always redone (`ig_lighter` is
	idempotent, same reasoning as `zone_ig_lighter`): a real count of
	`igs_other_merged_dir` names also present in `final_ig_dir`, `total > 0
	and built == total` (same convention as every other per-item check in
	this module) -- a bug found by Nuno (2026-09-19, right after wiping
	every output and reloading Atyscape) had the "nothing to light yet"
	case read as `True` (up to date) instead of `False`, the same vacuous-
	truth regression as `_extraction_status()`'s own bug just above."""
	other_names = {p.stem for p in ctx.igs_other_merged_dir.glob("*.ig")} if ctx.igs_other_merged_dir.is_dir() else set()
	total = len(other_names)
	if total == 0:
		return 0, 1, False
	final_names = {p.stem for p in ctx.final_ig_dir.glob("*.ig")} if ctx.final_ig_dir.is_dir() else set()
	built = len(other_names & final_names)
	return built, total, built == total


def _check_prims_status(ctx: _BuildContext, zone_names: List[str]) -> Tuple[int, int, bool]:
	"""Status for build_rbank's check-prims pass (index 12, project-todos/
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
	feeds the extraction/land-merge checks (indices 0, 2) -- see
	_extraction_status()/_land_merge_status(). Never runs anything itself."""
	names = list(zone_names)
	total = len(names)
	ctx = _resolve_context(ryzom_data_path, continent)

	def count_present(directory: Path, ext: str) -> int:
		return sum(1 for name in names if (directory / f"{name}{ext}").is_file())

	zonenh_count = count_present(ctx.out_zone_dir, ".zonenh")
	zonenhw_count = count_present(ctx.zone_nhw_dir, ".zonenhw")
	zone_count = count_present(ctx.zone_dir, ".zone")
	zonew_count = count_present(ctx.zone_weld_dir, ".zonew")
	zonel_count = count_present(ctx.final_zones_dir, ".zonel")
	# zone_ig_lighter (Stage 9) only ever produces a `.ig` for a zone that
	# actually HAS real merged content (`igs_land_merged_dir/<zone>.ig`) --
	# most `newbieland` zones genuinely don't (confirmed real: 223/236),
	# a valid "nothing to light" terminal state, not a failure. Comparing
	# against the full zone count (like every other per-zone check in this
	# module) would read as permanently "incomplete" -- Nuno 2026-09-19,
	# right after the merge/lighter fixes finally produced real output
	# (13/236) but the tick still showed stale. Compare against only the
	# zones with real merged input instead.
	land_ig_expected_names = {name for name in names if (ctx.igs_land_merged_dir / f"{name}.ig").is_file()}
	land_ig_total = len(land_ig_expected_names)
	land_ig_count = sum(1 for name in land_ig_expected_names if (ctx.final_ig_dir / f"{name}.ig").is_file())
	depend_count = sum(1 for name in names if _depend_path_for(ctx.zone_depends_dir, name).is_file())
	ig_boxes_present = (ctx.rbank_bbox_dir / "ig_boxes.bbox").is_file()
	indoor_present = (
		(ctx.rbank_retrievers_dir / "tempMerged.rbank").is_file()
		and (ctx.rbank_retrievers_dir / "tempMerged.gr").is_file()
	)
	rename_present = (
		(ctx.final_pacs_dir / f"{continent}.rbank").is_file()
		and (ctx.final_pacs_dir / f"{continent}.gr").is_file()
	)
	ai_wmap_present_count = sum(1 for i in (0, 1, 2) if (ctx.ai_wmap_dir / f"{continent}_{i}.tga").is_file())
	global_up_to_date = not _rbank_global_needs_rebuild(ctx.rbank_lrs_dir)
	# zone_dependencies (Stage 7) writes properties.cfg BEFORE its own real
	# .depend files (Nuno 2026-09-19, deleting newbieland_properties.cfg by
	# hand: depend_count alone missed that this step's own output was now
	# incomplete) -- both are its real output, checked together.
	properties_cfg_path = ctx.zones_dir / f"{continent}_properties.cfg"
	properties_cfg_exists = properties_cfg_path.is_file()

	extraction_built, extraction_total, extraction_up_to_date, extraction_warning = _extraction_status(ctx, live_data_path)
	land_merge_built, land_merge_total, land_merge_up_to_date, land_merge_warning = _land_merge_status(ctx, names, live_data_path)
	other_merge_built, other_merge_total, other_merge_up_to_date = _other_merge_status(ctx)
	other_light_built, other_light_total, other_light_up_to_date = _other_lighting_status(ctx)
	check_prims_built, check_prims_total, check_prims_up_to_date = _check_prims_status(ctx, names)

	all_zone_keys = {_zone_sort_key(name): name for name in names}
	rbank_zone_up_to_date_count = sum(
		1 for name in names
		if not _zone_needs_rbank_zone_rebuild(ctx.zone_weld_dir, ctx.rbank_lrs_dir, name, all_zone_keys)
	)

	# `total > 0 and` guards every per-zone ratio below: with zero known
	# zones (an empty/unresolvable .land, never the normal case now that the
	# caller passes every used cell's expected name -- see
	# landscape_editor_edit_mode.py's own _draw_pipeline_steps_section()),
	# `count == total` would otherwise be vacuously True (0 == 0) and read
	# as "up to date" for a continent that's actually never been built --
	# the exact regression found 2026-09-18 on a totally empty `bagne/`.
	per_step = {
		0: (extraction_built, extraction_total, extraction_up_to_date, extraction_warning),
		1: (zonenh_count, total, total > 0 and zonenh_count == total, None),
		2: (land_merge_built, land_merge_total, land_merge_up_to_date, land_merge_warning),
		3: (other_merge_built, other_merge_total, other_merge_up_to_date, None),
		4: (zonenhw_count, total, total > 0 and zonenhw_count == total, None),
		5: (zone_count, total, total > 0 and zone_count == total, None),
		6: (zonew_count, total, total > 0 and zonew_count == total, None),
		7: (
			depend_count, total,
			total > 0 and depend_count == total and properties_cfg_exists,
			None,
		),
		8: (zonel_count, total, total > 0 and zonel_count == total, None),
		9: (land_ig_count, land_ig_total, land_ig_total > 0 and land_ig_count == land_ig_total, None),
		10: (other_light_built, other_light_total, other_light_up_to_date, None),
		11: (1 if ig_boxes_present else 0, 1, ig_boxes_present, None),
		12: (check_prims_built, check_prims_total, check_prims_up_to_date, None),
		13: (rbank_zone_up_to_date_count, total, total > 0 and rbank_zone_up_to_date_count == total, None),
		14: (1 if global_up_to_date else 0, 1, global_up_to_date, None),
		15: (1 if indoor_present else 0, 1, indoor_present, None),
		16: (1 if rename_present else 0, 1, rename_present, None),
		17: (ai_wmap_present_count, 3, ai_wmap_present_count == 3, None),
	}

	# One real destination directory per step (project-todos/forgery/
	# pipeline_export_dirs_rename.md step 15b). `additionnal_ig_out_dir` is
	# a plain string on `land_cfg` (a pynel dataclass field), wrapped in
	# `Path()` like everywhere else in this module.
	destination_dirs = {
		0: ctx.igs_flora_dir,
		1: ctx.out_zone_dir,
		2: ctx.igs_land_merged_dir,
		3: ctx.igs_other_merged_dir,
		4: ctx.zone_nhw_dir,
		5: ctx.zone_dir,
		6: ctx.zone_weld_dir,
		7: ctx.zone_depends_dir,
		8: ctx.final_zones_dir,
		9: ctx.final_ig_dir,
		10: ctx.final_ig_dir,
		11: ctx.rbank_bbox_dir,
		12: ctx.rbank_preproc_dir,
		13: ctx.rbank_lrs_dir,
		14: ctx.rbank_dir,
		15: ctx.rbank_retrievers_dir,
		16: ctx.final_pacs_dir,
		17: ctx.ai_wmap_dir,
	}

	# Can this step actually run right now (project-todos/forgery/
	# pipeline_export_dirs_rename.md step 15c)? Each condition below is the
	# EXACT SAME one `run_single_step()` uses to raise `LandBuildError` for
	# that step -- the UI freezes the button on this instead of letting the
	# user hit that same error after clicking.
	rbank_global_present = (ctx.rbank_lrs_dir / "temp.rbank").is_file()
	# `build_ig_boxes` (Stage 11) literally reads `final_ig_dir`'s own `.ig`
	# content (land AND/OR other, Stages 9-10) -- checked separately from
	# `zonew_count` (Nuno 2026-09-19, catching a real gap: the pre-existing
	# `existing_zonew_files()` guard in run_single_step()'s own step 11
	# branch was itself never checking this, "PACS needs a welded continent,
	# even though build_ig_boxes itself only reads .ig" -- both conditions
	# now required, and run_single_step()'s step 11 branch updated to match).
	final_ig_present = any(ctx.final_ig_dir.glob("*.ig"))
	can_build_reasons = {
		0: (True, None),
		1: (True, None),
		2: (zonenh_count > 0, None if zonenh_count > 0 else "run land_export first"),
		3: (True, None),
		4: (zonenh_count > 0, None if zonenh_count > 0 else "run land_export first"),
		5: (zonenhw_count > 0, None if zonenhw_count > 0 else "run weld pass 1 first"),
		6: (zone_count > 0, None if zone_count > 0 else "run zone_elevation first"),
		7: (zonew_count > 0, None if zonew_count > 0 else "run weld pass 2 first"),
		8: (zonew_count > 0, None if zonew_count > 0 else "run weld pass 2 first"),
		9: (zonel_count > 0, None if zonel_count > 0 else "run zone_lighter first"),
		# properties.cfg not required here anymore (Nuno 2026-09-19): it's
		# free/deterministic to regenerate, so steps 8/9/10 each ensure it
		# themselves (_ensure_properties_cfg()) instead of depending on
		# zone_dependencies (Stage 7) having run first.
		10: (True, None),
		11: (
			zonew_count > 0 and final_ig_present,
			None if zonew_count > 0 and final_ig_present
			else ("run weld pass 2 first" if zonew_count == 0 else "run zone_ig_lighter/ig_lighter first"),
		),
		12: (
			zonew_count > 0 and ig_boxes_present,
			None if zonew_count > 0 and ig_boxes_present
			else ("run weld pass 2 first" if zonew_count == 0 else "run build_ig_boxes first"),
		),
		13: (
			zonew_count > 0 and ig_boxes_present,
			None if zonew_count > 0 and ig_boxes_present
			else ("run weld pass 2 first" if zonew_count == 0 else "run build_ig_boxes first"),
		),
		14: (
			zonew_count > 0 and ig_boxes_present,
			None if zonew_count > 0 and ig_boxes_present
			else ("run weld pass 2 first" if zonew_count == 0 else "run build_ig_boxes first"),
		),
		15: (rbank_global_present, None if rbank_global_present else "run build_rbank (global) first"),
		16: (indoor_present, None if indoor_present else "run build_indoor_rbank first"),
		17: (
			land_ig_count > 0 and rename_present,
			None if land_ig_count > 0 and rename_present else "finish PACS collision and IG lighting first",
		),
	}

	statuses = []
	for index, name in enumerate(_STEP_NAMES):
		built_count, total_count, up_to_date, warning = per_step[index]
		can_build, blocked_reason = can_build_reasons[index]
		has_own_button = True
		group_size = 1
		statuses.append(StepStatus(
			index, name, _STEP_SHORT_NAMES[index], _STEP_TOOLS[index],
			has_own_button, built_count, total_count, up_to_date, warning, group_size,
			destination_dirs[index], can_build, blocked_reason,
		))
	return statuses


def _clear_step_cache(ctx: _BuildContext, step_index: int) -> None:
	"""Deletes step_index's own on-disk cache file(s), if any. A no-op for
	an always-redone step (`_UNCACHED_STEP_INDICES`): it has nothing on
	disk to clear, it already reruns/overwrites unconditionally every
	time."""
	if step_index == 4:
		for path in ctx.zone_nhw_dir.glob("*.zonenhw"):
			path.unlink(missing_ok=True)
	elif step_index == 5:
		for path in ctx.zone_dir.glob("*.zone"):
			path.unlink(missing_ok=True)
	elif step_index == 6:
		for path in ctx.zone_weld_dir.glob("*.zonew"):
			path.unlink(missing_ok=True)
	elif step_index == 8:
		for path in ctx.final_zones_dir.glob("*.zonel"):
			path.unlink(missing_ok=True)
	elif step_index == 9:
		# Only the zone-named .ig -- final_ig_dir also holds the other-
		# lighting step's own output (Stage 10), never touched here.
		for path in ctx.final_ig_dir.glob("*.ig"):
			if _ZONE_NAME_RE.match(path.stem):
				path.unlink(missing_ok=True)
	elif step_index == 13:
		for path in ctx.rbank_lrs_dir.glob("*.lr"):
			path.unlink(missing_ok=True)
	elif step_index == 14:
		for filename in ("temp.rbank", "temp.gr"):
			(ctx.rbank_lrs_dir / filename).unlink(missing_ok=True)


def force_step(ryzom_data_path, continent, step_index: int) -> None:
	"""Public wrapper kept for any external caller that only wants the
	cache-clearing half of run_single_step() (below) without the immediate
	rerun."""
	ctx = _resolve_context(ryzom_data_path, continent)
	_clear_step_cache(ctx, step_index)


def _prepare_pacs_dirs(ctx: _BuildContext) -> None:
	"""Creates every PACS scratch directory -- `igs_other_merged_dir` itself
	is filled earlier, by `_run_merge_other()` (Stage 2bis), not here
	anymore (project-todos/forgery/pipeline_export_dirs_rename.md: the
	legacy `ig_other_lighted` source is abandoned)."""
	for directory in (
		ctx.rbank_bbox_dir, ctx.rbank_tessellation_dir, ctx.rbank_preproc_dir, ctx.rbank_retrievers_dir,
	):
		directory.mkdir(parents=True, exist_ok=True)


def _run_land_export(ctx: _BuildContext) -> List[Path]:
	"""Stage 1 -- land_export: cuts the .land composition into flat
	(no-height) .zonenh files, one per used grid cell, into out_zone_dir.
	The native tool also writes a `.zonel` per zone into the SAME
	directory as an inert byproduct (Nuno 2026-09-19, confirmed real: it's
	not the real, final `.zonel` -- that one comes from `zone_lighter`,
	Stage 8, into a completely different directory) -- deleted right after,
	so `out_zone_dir` only ever holds the `.zonenh` we actually use."""
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
	for stale_zonel_path in ctx.out_zone_dir.glob("*.zonel"):
		stale_zonel_path.unlink(missing_ok=True)
	return zonenh_files


def _run_extraction(ctx: _BuildContext, live_data_path, progress) -> None:
	"""Stage 0 -- extraction: complete copy of `live_data`'s own `.ig`
	content into `igs_flora_dir` (zone-named entries) and
	`land_cfg.additionnal_ig_out_dir` (every other entry, "other") -- no
	diff anymore (decision Nuno 2026-09-19, project-todos/forgery/
	pipeline_export_dirs_rename.md): the previous diff only ever served to
	validate the export process. Runs before `land_export` (Stage 1), no
	dependency either way -- `land_export`'s own `ExportAdditionnalIGs`
	repositioning (writing into the SAME `additionnal_ig_out_dir`) simply
	runs after and may overwrite a same-named "other" entry with its own
	freshly-repositioned copy, which is the freshest/most authoritative
	placement. Skipped entirely if `live_data_path` isn't available."""
	progress(0)
	if not live_data_path:
		progress(0, 1.0)
		return
	other_dir = Path(ctx.land_cfg.additionnal_ig_out_dir)
	refs = ig_full_load.list_continent_ig_refs_visualisation(live_data_path, ctx.continent)
	for i, ref in enumerate(refs):
		if _ZONE_NAME_RE.match(ref.name):
			# `live_data`'s own .bnp entry names are lowercase (confirmed
			# real, 2026-09-19: "65_bz.ig", not "65_BZ.ig"), but every OTHER
			# zone name in this module (land_export's own .zonenh/.zonew/
			# .zonel stems, _zone_name_from_row_col()) is uppercase --
			# writing the extraction under the lowercase name silently broke
			# merge_land_ig()'s lookup on a case-sensitive filesystem (never
			# found, 0 real flora ever merged in). `_find_live_ref()` itself
			# already matches case-insensitively, so passing the normalized
			# name through still finds the same real live_data content.
			zone_name = ref.name.upper()
			ig_editing.extract_land_flora(ctx.ryzom_data_path, live_data_path, ctx.continent, zone_name, ctx.igs_flora_dir)
		else:
			ig_editing.extract_other_ig(ctx.ryzom_data_path, live_data_path, ctx.continent, ref.name, other_dir)
		progress(0, (i + 1) / len(refs) if refs else 1.0)


def _write_land_merge_marker(ig_path: Path, source_path: Optional[Path], marker_path: Path) -> None:
	"""Marker confirming the land merge really ran for this zone (project-
	todos/forgery/landscape_editor__pacs_export.md step 7k, adapted
	project-todos/forgery/pipeline_export_dirs_rename.md) -- read back by
	_land_merge_status() to give the "Pipeline steps" tick a real
	up-to-date/stale signal. Records the merged `.ig`'s OWN mtime+size
	(stale the moment land_export or the extraction rerun) and the
	flora/override source actually used at merge time (`None` if neither
	existed for this zone). `ig_mtime`/`ig_size` are themselves `None` if
	`ig_path` doesn't exist -- a zone can legitimately end up with no `.ig`
	at all after the merge (no ref content from land_export, no live_data
	flora either, confirmed 2026-09-19 real: 223/236 `newbieland` zones), a
	valid "confirmed empty" terminal state, not an error."""
	if ig_path.is_file():
		ig_stat = ig_path.stat()
		marker = {"ig_mtime": ig_stat.st_mtime, "ig_size": ig_stat.st_size, "source_mtime": None, "source_size": None}
	else:
		marker = {"ig_mtime": None, "ig_size": None, "source_mtime": None, "source_size": None}
	if source_path is not None and source_path.is_file():
		source_stat = source_path.stat()
		marker["source_mtime"] = source_stat.st_mtime
		marker["source_size"] = source_stat.st_size
	marker_path.write_text(json.dumps(marker))


def _run_merge_land(ctx: _BuildContext, zone_names: List[str], progress) -> None:
	"""Stage 2 -- merges igs_flora_dir (complete live_data extraction) and
	land_ig_work_dir()'s override (always wins) onto land_export's own
	`out_ig_dir` content (Stage 1), with dedup (shape+position) so content
	land_export already regenerates from the Ligo bank (water, `.pacs_prim`,
	outpost markers) is never duplicated -- writes `igs_land_merged_dir`,
	a directory SEPARATE from `out_ig_dir` (project-todos/forgery/
	pipeline_export_dirs_rename.md, unlike the previous in-place edit)."""
	progress(2)
	override_dir = ig_editing.land_ig_work_dir(ctx.ryzom_data_path, ctx.continent)
	ig_editing.merge_land_ig(ctx.out_ig_dir, ctx.igs_flora_dir, override_dir, ctx.igs_land_merged_dir, zone_names)
	for i, zone_name in enumerate(zone_names):
		merged_path = ctx.igs_land_merged_dir / f"{zone_name}.ig"
		# ALSO copied back in place over out_ig_dir: zone_ig_lighter (Stage 9)
		# finds its own input .ig via properties_cfg's search_pathes, not a
		# direct path argument (see its own docstring) -- exact mechanism not
		# independently verified (pynel/native tool source, out of reach this
		# session), so out_ig_dir keeps receiving the merged result exactly
		# like the previous in-place-edit design did, alongside the new named
		# igs_land_merged_dir Nuno asked for (project-todos/forgery/
		# pipeline_export_dirs_rename.md).
		if merged_path.is_file():
			shutil.copyfile(merged_path, ctx.out_ig_dir / f"{zone_name}.ig")
		source_path = override_dir / f"{zone_name}.ig"
		if not source_path.is_file():
			source_path = ctx.igs_flora_dir / f"{zone_name}.ig"
		marker_path = ctx.igs_land_merged_dir / f"{zone_name}{_FLORA_MARKER_SUFFIX}"
		_write_land_merge_marker(merged_path, source_path if source_path.is_file() else None, marker_path)
		progress(2, (i + 1) / len(zone_names) if zone_names else 1.0)


def _run_merge_other(ctx: _BuildContext, progress) -> None:
	"""Stage 3 -- merges `additionnal_ig_out_dir` (`igs/other/`, itself the
	live_data extraction + land_export's repositioning, Stage 0/1) with
	`other_ig_work_dir()`'s override (always wins) into
	`igs_other_merged_dir` -- whole-file replacement, no instance-level
	dedup (land_export never regenerates an "other" `.ig`'s own internal
	content, only repositions it as a rigid block)."""
	progress(3)
	ig_editing.merge_other_ig(
		Path(ctx.land_cfg.additionnal_ig_out_dir), ig_editing.other_ig_work_dir(ctx.ryzom_data_path), ctx.igs_other_merged_dir,
	)
	progress(3, 1.0)


def _run_weld_pass1(ctx: _BuildContext, to_reweld: List[Path], progress) -> List[Path]:
	"""Stage 4 -- zone_welder, pass 1 (.zonenh -> .zonenhw, still no real
	height): sequential over the sorted list so each zone sees its
	already-welded neighbors already on disk by the time its own turn
	comes, same single-pass convention as zone_tools.py's own "Generate
	missing .zonew" button. Written to zone_nhw_dir (project-todos/forgery/
	pipeline_export_dirs_rename.md -- a real bug found 2026-09-19: this
	used to write `.zonenhw` next to the input `.zonenh` via a plain
	`with_suffix()`, ignoring the just-added `zone_nhw_dir` field entirely)."""
	progress(4, 1.0 if not to_reweld else 0.0)
	zonenhw_files = []
	for i, zonenh_path in enumerate(to_reweld):
		zonenhw_path = ctx.zone_nhw_dir / zonenh_path.with_suffix(".zonenhw").name
		result = ryzom_zone_tools.run_zone_welder(ctx.binaries["zone_welder"], zonenh_path, zonenhw_path)
		if not zonenhw_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonenhw_path.name}:\n{result.stdout}\n{result.stderr}")
		zonenhw_files.append(zonenhw_path)
		progress(4, (i + 1) / len(to_reweld))
	return zonenhw_files


def _run_elevation(ctx: _BuildContext, zonenhw_files: List[Path], progress) -> List[Path]:
	"""Stage 5 -- zone_elevation: applies the continent's two heightmaps to
	give each zone its real terrain Z. Written directly to zone_dir (the
	real pipeline-export "zone" subdir), not out_zone_dir."""
	progress(5, 1.0 if not zonenhw_files else 0.0)
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
		progress(5, (i + 1) / len(zonenhw_files))
	return zone_files


def _run_weld_pass2(ctx: _BuildContext, zone_files: List[Path], progress) -> Set[str]:
	"""Stage 6 -- zone_welder, pass 2 (.zone -> .zonew): fixes any seams
	zone_elevation's per-zone heightmap sampling introduced at borders.
	Written to zone_weld_dir (the real pipeline-export "zone_weld" subdir).
	Returns the set of zone stems actually rewelded this run."""
	progress(6, 1.0 if not zone_files else 0.0)
	rebuilt_zonew_names = set()
	for i, zone_path in enumerate(zone_files):
		zonew_path = ctx.zone_weld_dir / zone_path.with_suffix(".zonew").name
		result = ryzom_zone_tools.run_zone_welder(ctx.binaries["zone_welder"], zone_path, zonew_path)
		if not zonew_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonew_path.name}:\n{result.stdout}\n{result.stderr}")
		rebuilt_zonew_names.add(zone_path.stem)
		progress(6, (i + 1) / len(zone_files))
	return rebuilt_zonew_names


def _ensure_properties_cfg(ctx: _BuildContext) -> Path:
	"""Writes/refreshes `properties.cfg` from the current CSV state --
	pure Python serialization, no native tool involved, free and
	deterministic to redo every single time (Nuno 2026-09-19: a config
	file isn't a real build artifact the way a `.zonew`/`.ig` is, so no
	step needing it should ever freeze waiting for a SEPARATE step to have
	produced it first -- each one just ensures it's fresh itself)."""
	properties_cfg_path = ctx.zones_dir / f"{ctx.continent}_properties.cfg"
	properties_cfg_path.parent.mkdir(parents=True, exist_ok=True)
	properties_cfg_path.write_text(ryzom_land_tools.build_properties_cfg(ctx.props_cfg))
	return properties_cfg_path


def _run_zone_dependencies(ctx: _BuildContext, zonew_files: List[Path], progress) -> Path:
	"""Stage 7 -- zone_dependencies: one shadow-dependency pass over the
	whole continent's zone range at once (docs/zone_tools.md §4). Returns
	the properties.cfg path it wrote (`_ensure_properties_cfg()`, also
	called independently by Stages 8/9/10 -- project-todos/forgery/
	pipeline_export_dirs_rename.md: free/deterministic to regenerate, no
	step should ever freeze waiting on another step for it).

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
	progress(7)
	properties_cfg_path = _ensure_properties_cfg(ctx)
	zone_keys = [_zone_sort_key(p.stem) for p in zonew_files]
	min_row = min(row for row, _ in zone_keys)
	max_row = max(row for row, _ in zone_keys)
	min_col = min(col for _, col in zone_keys)
	max_col = max(col for _, col in zone_keys)
	first_zone_name = _zone_name_from_row_col(min_row, min_col)
	last_zone_name = _zone_name_from_row_col(max_row, max_col)
	depend_arg_path = ctx.zone_depends_dir / f"{ctx.continent}.depend"
	result = ryzom_land_tools.run_zone_dependencies(
		ctx.binaries["zone_dependencies"], properties_cfg_path,
		ctx.zone_weld_dir / f"{first_zone_name}.zonew", ctx.zone_weld_dir / f"{last_zone_name}.zonew", depend_arg_path,
	)
	produced_depends = [p for p in zonew_files if _depend_path_for(ctx.zone_depends_dir, p.stem).is_file()]
	if not produced_depends:
		raise LandBuildError(f"zone_dependencies produced no .depend file in {ctx.out_zone_dir}:\n{result.stdout}\n{result.stderr}")
	return properties_cfg_path


def _run_zone_lighter(
	ctx: _BuildContext, zonew_files: List[Path], rebuilt_zonew_names: Set[str],
	properties_cfg_path: Path, on_zone_ready, progress,
) -> Tuple[List[Path], List[Path]]:
	"""Stage 8 -- zone_lighter (.zonew -> .zonel), written to
	final_zones_dir -- a zone needs lighting if it has no `.zonel` yet OR
	its `.zonew` was just rebuilt this run: a freshly rewelded `.zonew`
	makes any OLD `.zonel` next to it stale even if one happened to exist.
	Parallelized across zones (Nuno 2026-09-11): zone_lighter dominates
	per-zone time (~2s each), and each zone's own input/output files are
	fully independent. `zone_lighter`'s OWN internal multi-threading
	(`cpu_num`) is deliberately left at 1 (continent_pipeline_reference.py's
	own docstring) -- this is PROCESS-level concurrency, one native
	`zone_lighter` per worker. Returns (zonel_files, to_light) -- `to_light`
	reused as-is by Stage 9 (zone_ig_lighter needs the exact same set)."""
	already_lit_names = {p.stem for p in zonew_files if (ctx.final_zones_dir / f"{p.stem}.zonel").is_file()}
	to_light = [p for p in zonew_files if p.stem in rebuilt_zonew_names or p.stem not in already_lit_names]
	progress(8, 1.0 if not to_light else 0.0)
	lit_count = 0
	lit_count_lock = threading.Lock()

	def light_one(zonew_path):
		nonlocal lit_count
		zonel_path = ctx.final_zones_dir / zonew_path.with_suffix(".zonel").name
		zone_depend_path = _depend_path_for(ctx.zone_depends_dir, zonew_path.stem)
		result = ryzom_zone_tools.run_zone_lighter(
			ctx.binaries["zone_lighter"], zonew_path, zonel_path, properties_cfg_path, zone_depend_path,
		)
		if not zonel_path.is_file():
			raise LandBuildError(f"zone_lighter failed to produce {zonel_path.name}:\n{result.stdout}\n{result.stderr}")
		with lit_count_lock:
			lit_count += 1
			progress(8, lit_count / len(to_light))
		if on_zone_ready is not None:
			try:
				zone = parse_zone(zonel_path.read_bytes())
				on_zone_ready(zonel_path.stem, zone_to_cache_data(zone))
			except (OSError, ZoneParseError):
				pass

	worker_count = os.cpu_count() or 4
	with ThreadPoolExecutor(max_workers=worker_count) as pool:
		list(pool.map(light_one, to_light))

	zonel_files = [ctx.final_zones_dir / f"{p.stem}.zonel" for p in zonew_files]
	missing_zonel = [p for p in zonel_files if not p.is_file()]
	if missing_zonel:
		raise LandBuildError(f"missing .zonel for {len(missing_zonel)} zone(s): {[p.name for p in missing_zonel[:5]]}")
	return zonel_files, to_light


def _run_zone_ig_lighter(
	ctx: _BuildContext, zonel_files: List[Path], to_light: List[Path], properties_cfg_path: Path, progress,
) -> None:
	"""Stage 9 -- zone_ig_lighter: lights each zone JUST RELIT's own .ig
	(looked up automatically by the native tool via properties_cfg's
	search_pathes) -- same `to_light` set as Stage 8. A cell with no
	matching .ig anywhere in search_pathes is simply skipped, never a hard
	failure. Output written directly to `final_ig_dir` (project-todos/
	forgery/pipeline_export_dirs_rename.md -- the real, game-ready final
	`.ig`, same directory Stage 10's other-lighting also writes to, names
	never colliding)."""
	to_light_names = {p.stem for p in to_light}
	ig_to_light = [p for p in zonel_files if p.stem in to_light_names]
	progress(9, 1.0 if not ig_to_light else 0.0)
	for i, zonel_path in enumerate(ig_to_light):
		output_ig_path = ctx.final_ig_dir / f"{zonel_path.stem}.ig"
		zone_depend_path = _depend_path_for(ctx.zone_depends_dir, zonel_path.stem)
		ryzom_land_tools.run_zone_ig_lighter(
			ctx.binaries["zone_ig_lighter"], zonel_path, output_ig_path, properties_cfg_path, zone_depend_path,
		)
		progress(9, (i + 1) / len(ig_to_light) if ig_to_light else 1.0)


def _run_other_lighting(ctx: _BuildContext, properties_cfg_path: Path, progress) -> None:
	"""Stage 10 -- ig_lighter: lights every "other" `.ig` in
	`igs_other_merged_dir` at once (the global, non-per-zone equivalent of
	`zone_ig_lighter`, same `instance_lighter.cpp` engine), mandatory per
	Nuno (2026-09-19) -- editing an "other" `.ig` makes its baked lighting
	stale (a moved building's shadow no longer matches). CLI confirmed in
	`nel/tools/build_gamedata/processes/ig_light/2_build.py`: `ig_lighter
	<srcDir> <destDir> <properties.cfg>`, directory-to-directory, not
	wrapped by `pynel` (same category as `ai_wmap_export.py`'s own tools) --
	a plain local `subprocess.run()`, same pattern. Writes directly into
	`final_ig_dir` (same final directory Stage 9's land lighting also
	writes to, names never colliding)."""
	progress(10)
	source_names = {p.stem for p in ctx.igs_other_merged_dir.glob("*.ig")} if ctx.igs_other_merged_dir.is_dir() else set()
	result = subprocess.run(
		[str(ctx.binaries["ig_lighter"]), str(ctx.igs_other_merged_dir), str(ctx.final_ig_dir), str(properties_cfg_path)],
		capture_output=True, text=True,
	)
	if result.returncode != 0:
		raise LandBuildError(f"ig_lighter failed (returncode={result.returncode}):\n{result.stdout}\n{result.stderr}")
	produced_names = {p.stem for p in ctx.final_ig_dir.glob("*.ig")} & source_names
	if source_names and not produced_names:
		# ig_lighter, same as zone_ig_lighter (project-todos/forgery/
		# pipeline_export_dirs_rename.md), can return 0 (success) while
		# silently producing nothing at all -- never trust the exit code
		# alone, always verify the real output exists.
		raise LandBuildError(
			f"ig_lighter returned 0 but produced no .ig in {ctx.final_ig_dir} for any of "
			f"{len(source_names)} source file(s):\n{result.stdout}\n{result.stderr}"
		)
	progress(10, 1.0)


def _run_build_ig_boxes(ctx: _BuildContext, progress) -> Path:
	"""Stage 11 -- build_ig_boxes: one bbox per real .ig (land + other,
	Stages 9-10, both now the SAME final_ig_dir) and every .shape it
	references."""
	progress(11)
	ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
	ig_stems = sorted(p.stem for p in ctx.final_ig_dir.glob("*.ig"))
	ig_boxes_cfg = ryzom_pacs_tools.IgBoxesConfig(
		pathes=[str(ctx.final_ig_dir)] + ctx.props_cfg.search_pathes,
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
		# Both point at final_ig_dir now (project-todos/forgery/
		# pipeline_export_dirs_rename.md): land (Stage 9) and other
		# (Stage 10) write into the SAME final directory, names never
		# colliding -- functionally equivalent to the previous 2 distinct,
		# non-overlapping directories.
		ig_land_path=str(ctx.final_ig_dir) + "/",
		ig_village_path=str(ctx.final_ig_dir) + "/",
		tessellation_path=str(ctx.rbank_tessellation_dir) + "/",
		tessellate_level=1,
		output_root_path=str(ctx.common_dir) + "/",
		smooth_directory="rbank/lrs/",
		raw_directory="rbank/raw/",
		reduce_surfaces=True, smooth_borders=True, compute_elevation=False,
		compute_levels=True, link_elements=True, cut_edges=True, use_zone_square=False,
		zone_ul=zone_ul, zone_dr=zone_dr,
		preprocess_directory=str(ctx.rbank_preproc_dir) + "/",
		global_retriever="temp.gr", retriever_bank="temp.rbank",
		zones=[f"{p.stem}.zonew" for p in zonew_files],
		pathes=[ctx.land_cfg.continents_dir] + ctx.props_cfg.search_pathes,
	)


def _run_build_rbank_check(ctx: _BuildContext, rbank_cfg, zonew_files: List[Path], progress) -> None:
	"""Stage 12 -- build_rbank, check prims pass (validates collision
	primitives referenced by every zone, no .lr/.rbank/.gr output of its
	own). Writes `check_prims_manifest.json` (project-todos/forgery/
	landscape_editor__pacs_export.md step 7k) recording every validated
	`.zonew`'s mtime+size, read back by _check_prims_status() to give the
	"Pipeline steps" tick a real signal instead of the permanently-neutral
	one it had before this marker existed."""
	progress(12)
	ryzom_pacs_tools.run_build_rbank_check_prims(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir)
	manifest = {}
	for zonew_path in zonew_files:
		stat = zonew_path.stat()
		manifest[zonew_path.stem] = [stat.st_mtime, stat.st_size]
	(ctx.rbank_preproc_dir / _CHECK_PRIMS_MANIFEST_NAME).write_text(json.dumps({"zones": manifest}))


def _run_build_rbank_zones(
	ctx: _BuildContext, rbank_cfg, zonew_files: List[Path], rebuilt_zonew_names: Set[str], progress,
) -> List[str]:
	"""Stage 13 -- build_rbank, per-zone pass (tessellation + moulinage),
	one .lr per zone under rbank_lrs_dir -- incremental: a zone needs
	rebuilding if ITS OWN .zonew just changed this run, was never built, or
	ANY of its 8 neighbors' .zonew is newer than its own .lr (border
	continuity, computed here from zone name row/col arithmetic rather than
	depending on the get_neighbors binary, which this checkout doesn't even
	have source for). Returns the `.zonew` filenames actually rebuilt."""
	ctx.rbank_lrs_dir.mkdir(parents=True, exist_ok=True)
	all_zone_keys = {_zone_sort_key(p.stem): p.stem for p in zonew_files}
	zones_needing_rebuild = [
		f"{p.stem}.zonew" for p in zonew_files
		if _zone_needs_rbank_zone_rebuild(ctx.zone_weld_dir, ctx.rbank_lrs_dir, p.stem, all_zone_keys, rebuilt_zonew_names)
	]
	progress(13, 1.0 if not zones_needing_rebuild else None)
	if zones_needing_rebuild:
		ryzom_pacs_tools.run_build_rbank_zones(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir, zones_needing_rebuild)
	return zones_needing_rebuild


def _run_build_rbank_global(ctx: _BuildContext, rbank_cfg, zones_needing_rebuild: List[str], progress) -> None:
	"""Stage 14 -- build_rbank, global pass (merges every zone's .lr into
	temp.rbank/temp.gr, written under rbank_lrs_dir) -- needed whenever
	any zone was just rebuilt this run OR the merge's own temp.rbank is
	missing/older than the newest .lr on disk (own real age indicator,
	Nuno 2026-09-17 -- NOT just "zones_needing_rebuild" alone, which gave
	this pass no way to be forced independently)."""
	needs_global_rebuild = _rbank_global_needs_rebuild(ctx.rbank_lrs_dir, zones_needing_rebuild)
	progress(14, 1.0 if not needs_global_rebuild else None)
	if needs_global_rebuild:
		ryzom_pacs_tools.run_build_rbank_global(ctx.binaries["build_rbank"], rbank_cfg, ctx.rbank_preproc_dir)


def _run_build_indoor_rbank(ctx: _BuildContext, progress) -> None:
	"""Stage 15 -- build_indoor_rbank: merges every .cmb (interior collision
	mesh) it finds under land_cfg.out_cmb_dir into temp.rbank/temp.gr -- an
	empty Meshes list is still a valid, harmless call. Requires
	rbank_lrs_dir/temp.rbank to already exist (its own openAndSerial()
	throws -- an unhandled SIGABRT -- on a missing one). `land_cfg.out_cmb_dir`
	unified with the old separate `rbank_cmb_export_dir` (project-todos/
	forgery/pipeline_export_dirs_rename.md step 12): both named the exact
	same physical folder already, just via two independent definitions."""
	progress(15)
	out_cmb_dir = Path(ctx.land_cfg.out_cmb_dir)
	cmb_stems = sorted(p.stem for p in out_cmb_dir.glob("*.cmb"))
	indoor_cfg = ryzom_pacs_tools.IndoorRbankConfig(
		mesh_path=str(out_cmb_dir) + "/",
		meshes=cmb_stems,
		output_path=str(ctx.rbank_retrievers_dir) + "/",
		# rbank_lrs_dir, NOT rbank_preproc_dir: build_rbank's own global
		# pass writes temp.rbank/temp.gr under SmoothDirectory (confirmed on
		# disk 2026-09-16).
		merge_path=str(ctx.rbank_lrs_dir) + "/",
	)
	ryzom_pacs_tools.run_build_indoor_rbank(ctx.binaries["build_indoor_rbank"], indoor_cfg, ctx.rbank_preproc_dir)


def _run_final_rename(ctx: _BuildContext, progress) -> None:
	"""Stage 16 -- final rename: tempMerged.* -> <continent>.* (a real
	filename-prefix swap, not a content transformation). Silently does
	nothing for whichever extension isn't present -- tolerant by design,
	same as the native pipeline's own "Move gr, rbank and lr" step."""
	progress(16)
	for extension in (".rbank", ".gr"):
		src = ctx.rbank_retrievers_dir / f"tempMerged{extension}"
		if src.is_file():
			shutil.copyfile(src, ctx.final_pacs_dir / f"{ctx.continent}{extension}")
	for src in ctx.rbank_retrievers_dir.glob("tempMerged_*.lr"):
		dst_name = ctx.continent + src.name[len("tempMerged"):]
		shutil.copyfile(src, ctx.final_pacs_dir / dst_name)
	progress(16, 1.0)


def _run_ai_wmap(ctx: _BuildContext, progress) -> None:
	"""Stage 17 -- ai_wmap_export: regenerates the continent's AI pathfinding
	walkability maps. `AiWmapExportError` is translated to `LandBuildError`
	so every caller keeps dealing with a single exception type. Passes
	`final_ig_dir`/`final_pacs_dir`/`ai_wmap_dir` explicitly (project-todos/
	forgery/pipeline_export_dirs_rename.md step 11) -- `ai_wmap_export.py`
	no longer recomputes its own, independent copies of these paths (the
	source of the `zone_lighted_ig_land` vs `ligo_ig_land` mismatch found
	while auditing this rename)."""
	progress(17)
	try:
		ai_wmap_export.generate_ai_wmap(
			ctx.ryzom_data_path, ctx.continent,
			ig_land_dir=ctx.final_ig_dir, rbank_output_dir=ctx.final_pacs_dir, ai_wmap_dir=ctx.ai_wmap_dir,
		)
	except ai_wmap_export.AiWmapExportError as exc:
		raise LandBuildError(str(exc))
	progress(17, 1.0)


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
	`.zonel`. `live_data_path`, if given, is used for the extraction stage
	(Stage 0) that feeds the land/other merges (Stages 2-3) -- `None` (no
	extraction attempted) only for a caller that doesn't have it available."""
	def progress(step_index: int, fraction: Optional[float] = None) -> None:
		if on_progress is not None:
			on_progress(step_index + 1, len(_STEP_NAMES), _STEP_NAMES[step_index], fraction)

	ctx = _resolve_context(ryzom_data_path, continent)
	_ensure_binaries(ctx, _BINARY_NAMES)
	_ensure_zone_dirs(ctx)

	# Stage 0 -- extraction (live_data -> igs_flora_dir/additionnal_ig_out_dir).
	_run_extraction(ctx, live_data_path, progress)

	# Stage 1 -- land_export.
	progress(1)
	zonenh_files = _run_land_export(ctx)
	zone_names = [p.stem for p in zonenh_files]

	# Stages 2-3 -- land merge, other merge.
	_run_merge_land(ctx, zone_names, progress)
	_run_merge_other(ctx, progress)

	# Incremental rebuild (Nuno 2026-09-12), per REAL STAGE independently --
	# NOT "has .zonel" alone: needing weld/elevation is its OWN check
	# (missing `.zonew`, regardless of `.zonel`), and needing lighting is a
	# SEPARATE check (missing `.zonel`, OR the zone's `.zonew` was just
	# rebuilt this run).
	to_reweld = [p for p in zonenh_files if not (ctx.zone_weld_dir / f"{p.stem}.zonew").is_file()]

	# Stages 4-6 -- weld pass 1, elevation, weld pass 2.
	zonenhw_files = _run_weld_pass1(ctx, to_reweld, progress)
	zone_files = _run_elevation(ctx, zonenhw_files, progress)
	rebuilt_zonew_names = _run_weld_pass2(ctx, zone_files, progress)

	# The full current .zonew set (rebuilt this run + already up to date).
	zonew_files = [ctx.zone_weld_dir / f"{p.stem}.zonew" for p in zonenh_files]
	missing_zonew = [p for p in zonew_files if not p.is_file()]
	if missing_zonew:
		raise LandBuildError(f"missing .zonew for {len(missing_zonew)} zone(s) (never built): {[p.name for p in missing_zonew[:5]]}")

	# Stage 7 -- zone_dependencies.
	properties_cfg_path = _run_zone_dependencies(ctx, zonew_files, progress)

	# Stages 8-10 -- zone_lighter, zone_ig_lighter (land), other-lighting.
	zonel_files, to_light = _run_zone_lighter(ctx, zonew_files, rebuilt_zonew_names, properties_cfg_path, on_zone_ready, progress)
	_run_zone_ig_lighter(ctx, zonel_files, to_light, properties_cfg_path, progress)
	_run_other_lighting(ctx, properties_cfg_path, progress)

	# Stage 11-16 -- PACS collision (project-todos/forgery/landscape_editor__
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

	# Stage 17 -- ai_wmap_export.
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
		_run_extraction(ctx, live_data_path, progress)
	elif step_index == 1:
		_ensure_binaries(ctx, ("land_export",))
		_run_land_export(ctx)
	elif step_index == 2:
		zonenh_files = sorted(ctx.out_zone_dir.glob("*.zonenh"))
		if not zonenh_files:
			raise LandBuildError(f"No .zonenh found in {ctx.out_zone_dir} -- run land_export first.")
		_run_merge_land(ctx, [p.stem for p in zonenh_files], progress)
	elif step_index == 3:
		_run_merge_other(ctx, progress)
	elif step_index == 4:
		zonenh_files = sorted(ctx.out_zone_dir.glob("*.zonenh"))
		if not zonenh_files:
			raise LandBuildError(f"No .zonenh found in {ctx.out_zone_dir} -- run land_export first.")
		_ensure_binaries(ctx, ("zone_welder",))
		_run_weld_pass1(ctx, zonenh_files, progress)
	elif step_index == 5:
		zonenhw_files = sorted(ctx.zone_nhw_dir.glob("*.zonenhw"))
		if not zonenhw_files:
			raise LandBuildError(f"No .zonenhw found in {ctx.zone_nhw_dir} -- run weld pass 1 first.")
		_ensure_binaries(ctx, ("zone_elevation",))
		_run_elevation(ctx, zonenhw_files, progress)
	elif step_index == 6:
		zone_files = sorted(ctx.zone_dir.glob("*.zone"))
		if not zone_files:
			raise LandBuildError(f"No .zone found in {ctx.zone_dir} -- run zone_elevation first.")
		_ensure_binaries(ctx, ("zone_welder",))
		_run_weld_pass2(ctx, zone_files, progress)
	elif step_index == 7:
		zonew_files = existing_zonew_files()
		_ensure_binaries(ctx, ("zone_dependencies",))
		_run_zone_dependencies(ctx, zonew_files, progress)
	elif step_index == 8:
		zonew_files = existing_zonew_files()
		properties_cfg_path = _ensure_properties_cfg(ctx)
		_ensure_binaries(ctx, ("zone_lighter",))
		_run_zone_lighter(ctx, zonew_files, set(), properties_cfg_path, on_zone_ready, progress)
	elif step_index == 9:
		zonew_files = existing_zonew_files()
		zonel_files = [ctx.final_zones_dir / f"{p.stem}.zonel" for p in zonew_files]
		missing_zonel = [p for p in zonel_files if not p.is_file()]
		if missing_zonel:
			raise LandBuildError(f"missing .zonel for {len(missing_zonel)} zone(s) -- run zone_lighter first.")
		properties_cfg_path = _ensure_properties_cfg(ctx)
		_ensure_binaries(ctx, ("zone_ig_lighter",))
		_run_zone_ig_lighter(ctx, zonel_files, zonew_files, properties_cfg_path, progress)
	elif step_index == 10:
		properties_cfg_path = _ensure_properties_cfg(ctx)
		_ensure_binaries(ctx, ("ig_lighter",))
		_run_other_lighting(ctx, properties_cfg_path, progress)
	elif step_index == 11:
		existing_zonew_files()  # PACS needs a welded continent, even though build_ig_boxes itself only reads .ig
		if not any(ctx.final_ig_dir.glob("*.ig")):
			raise LandBuildError(f"No .ig found in {ctx.final_ig_dir} -- run zone_ig_lighter/ig_lighter first.")
		_ensure_binaries(ctx, ("build_ig_boxes",))
		_prepare_pacs_dirs(ctx)
		_run_build_ig_boxes(ctx, progress)
	elif step_index == 12:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_check(ctx, rbank_cfg, zonew_files, progress)
	elif step_index == 13:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_zones(ctx, rbank_cfg, zonew_files, set(), progress)
	elif step_index == 14:
		zonew_files = existing_zonew_files()
		ig_boxes_path = ctx.rbank_bbox_dir / "ig_boxes.bbox"
		require(ig_boxes_path, f"{ig_boxes_path} missing -- run build_ig_boxes first.")
		_ensure_binaries(ctx, ("build_rbank",))
		rbank_cfg = _build_rbank_config(ctx, zonew_files, ig_boxes_path)
		_run_build_rbank_global(ctx, rbank_cfg, [], progress)
	elif step_index == 15:
		require(ctx.rbank_lrs_dir / "temp.rbank", f"{ctx.rbank_lrs_dir / 'temp.rbank'} missing -- run build_rbank (global) first.")
		_ensure_binaries(ctx, ("build_indoor_rbank",))
		_run_build_indoor_rbank(ctx, progress)
	elif step_index == 16:
		_run_final_rename(ctx, progress)
	elif step_index == 17:
		_run_ai_wmap(ctx, progress)
	else:
		raise LandBuildError(f"unknown step index: {step_index}")
