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
composition into final, installable `.zonel`/`.ig` files:

	land_export -> zone_welder -> zone_elevation -> zone_welder ->
	zone_dependencies -> zone_lighter -> zone_ig_lighter

Order confirmed in `nel/tools/build_gamedata/processes/zone/2_build.py` and
validated end-to-end 2026-09-11 against real `nexus` data (project-todos/
pynel/land_pipeline.md step 5), and again 2026-09-12 through this
orchestrator itself against real `bagne` data.

Unlike zone_tools.py's `run_zone_welder()` (a single on-demand zone, reusing
already-loaded Atyscape state), this module drives a from-scratch build of
an ENTIRE continent directly against the config from
`continent_pipeline_reference.csv` (`continent_pipeline_reference.py`) --
raises `LandBuildError` and stops the whole build on the first
unrecoverable failure, rather than tolerate partial failures the way the
more granular "Generate missing .zonew" button does (a from-scratch build
only makes sense as all-or-nothing).
"""

import os
import platform
import re
import threading
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Callable, List, Optional, Tuple

from pynel import ryzom_land_tools, ryzom_zone_tools
from pynel.ryzom_zone import parse_zone, ZoneParseError

from . import continent_pipeline_reference as cpr
from . import region_loader
from . import settings as app_settings
from .zone_geometry import zone_to_cache_data

_BINARY_NAMES = ("land_export", "zone_welder", "zone_elevation", "zone_dependencies", "zone_lighter", "zone_ig_lighter")

_ZONE_NAME_RE = re.compile(r"^(\d+)_([A-Za-z]{2})$")


class LandBuildError(Exception):
	pass


def _zone_sort_key(zone_name: str) -> Tuple[int, int]:
	"""(row, col) as real integers, for sorting zone names in ascending
	row/col order -- required by `zone_dependencies`' own broken min/max
	swap (see run_full_build()'s Stage 5 comment): a plain string sort of
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
	`run_full_build()`'s Stage 5 needs (see its own comment)."""
	return f"{row}_{chr(ord('A') + col // 26)}{chr(ord('A') + col % 26)}"


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


def run_full_build(
	ryzom_data_path, continent: str,
	on_progress: Optional[Callable[[str], None]] = None,
	on_zone_ready: Optional[Callable[[str, object], None]] = None,
) -> List[str]:
	"""Runs every stage for `continent`, returns the final `.zonel` paths
	produced. `on_progress(message)`, if given, is called with a short
	human-readable status after each stage -- the caller runs this on a
	background thread and polls for the latest message the same way
	`_run_generate_missing_zonew()` already does for its own progress.
	`on_zone_ready(name, cache_data)`, if given, is called once per zone
	right after `zone_lighter` produces its final `.zonel` (`cache_data` a
	`zone_cache.ZoneCacheData`, ready to hand straight to `_rebuild_zone_node()`
	-- same live, one-zone-at-a-time convention as `_run_generate_missing_
	zonew()`'s own `progress["ready"]`, Nuno 2026-09-12)."""
	def progress(message: str) -> None:
		if on_progress is not None:
			on_progress(message)

	binaries = {name: _resolve_binary(name) for name in _BINARY_NAMES}
	table = cpr.load_reference_table(ryzom_data_path)
	land_cfg = cpr.build_land_export_config(ryzom_data_path, continent, table)
	props_cfg = cpr.build_properties_config(ryzom_data_path, continent, table)

	# out_zone_dir ("ligo_zones", `land_cfg.out_zone_dir`, the legacy
	# `OutZoneDir` land_export itself writes .zonenh/.zonenhw into) is only
	# ever a STAGING area for those two intermediate extensions -- neither
	# is a real extension `region_loader.py` recognizes, so it never matters
	# where they live. The 3 REAL stages Atyscape actually looks for
	# (.zone/.zonew/.zonel, region_loader._PIPELINE_EXPORT_ZONE_SUBDIRS) are
	# instead written directly into their own real pipeline-export
	# subdirectories below, so a Build's output is immediately visible to
	# Atyscape's own disk scan without any separate "install" step (found
	# 2026-09-12: writing everything into out_zone_dir alone would have left
	# every built zone invisible to _build_land_driven_refs(), which only
	# scans zone/zone_weld/zone_lighted).
	out_zone_dir = Path(land_cfg.out_zone_dir)
	zone_dir, zone_weld_dir, zone_lighted_dir = region_loader.continent_zone_dirs(ryzom_data_path, continent)
	for directory in (
		out_zone_dir, zone_dir, zone_weld_dir, zone_lighted_dir,
		land_cfg.out_ig_dir, land_cfg.out_cmb_dir, land_cfg.additionnal_ig_out_dir,
	):
		Path(directory).mkdir(parents=True, exist_ok=True)

	# Stage 1 -- land_export: cuts the .land composition into flat
	# (no-height) .zonenh files, one per used grid cell, into out_zone_dir.
	progress("Running land_export...")
	cfg_path = out_zone_dir / f"{continent}_land_export.cfg"
	result = ryzom_land_tools.run_land_export(binaries["land_export"], land_cfg, cfg_path)
	zonenh_files = sorted(out_zone_dir.glob("*.zonenh"))
	if not zonenh_files:
		raise LandBuildError(f"land_export produced no .zonenh files in {out_zone_dir}:\n{result.stdout}")
	progress(f"land_export produced {len(zonenh_files)} zone(s).")

	# Incremental rebuild (Nuno 2026-09-12), per REAL STAGE independently --
	# NOT "has .zonel" alone (found 2026-09-12 against real bagne data, two
	# bugs from that single binary check):
	# 1. A zone already welded (has `.zonew`) but not yet lit (no `.zonel`)
	#    was wrongly re-run through elevation+weld AGAIN just because it
	#    lacked a `.zonel` -- Nuno: "quand je build, il me weld encore des
	#    zones ayant deja des .zonew".
	# 2. A zone can have a real `.zonel` on disk with NO `.zonew` next to it
	#    (confirmed on 4 real bagne zones, pre-existing data, not caused by
	#    this session) -- treating "has .zonel" as "everything upstream is
	#    fine" crashed Stage 4's post-check ("missing .zonew ... never
	#    built") since nothing had ever rebuilt that missing `.zonew`.
	# So: needing weld/elevation is its OWN check (missing `.zonew`,
	# regardless of `.zonel`), and needing lighting is a SEPARATE check
	# (missing `.zonel`, OR the zone's `.zonew` was just rebuilt this run --
	# its content changed, so any old `.zonel` next to it is now stale even
	# if it happened to exist).
	to_reweld = [p for p in zonenh_files if not (zone_weld_dir / f"{p.stem}.zonew").is_file()]
	reweld_names = {p.stem for p in to_reweld}
	up_to_date_weld_names = {p.stem for p in zonenh_files} - reweld_names
	progress(f"{len(up_to_date_weld_names)}/{len(zonenh_files)} zone(s) already welded, skipping re-weld.")

	# Stage 2 -- zone_welder, pass 1 (.zonenh -> .zonenhw, still no real
	# height): sequential over the sorted list so each zone sees its
	# already-welded neighbors already on disk by the time its own turn
	# comes, same single-pass convention as zone_tools.py's own "Generate
	# missing .zonew" button. Only zones actually needing a rebuild.
	progress("Welding (pass 1, no height)...")
	zonenhw_files = []
	for zonenh_path in to_reweld:
		zonenhw_path = zonenh_path.with_suffix(".zonenhw")
		result = ryzom_zone_tools.run_zone_welder(binaries["zone_welder"], zonenh_path, zonenhw_path)
		if not zonenhw_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonenhw_path.name}:\n{result.stdout}\n{result.stderr}")
		zonenhw_files.append(zonenhw_path)

	# Stage 3 -- zone_elevation: applies the continent's two heightmaps to
	# give each zone its real terrain Z. Written directly to zone_dir (the
	# real pipeline-export "zone" subdir), not out_zone_dir.
	progress("Applying heightmaps (zone_elevation)...")
	zone_files = []
	for zonenhw_path in zonenhw_files:
		zone_path = zone_dir / zonenhw_path.with_suffix(".zone").name
		elevation_cfg = ryzom_land_tools.ElevationConfig(
			input_zonenhw=str(zonenhw_path),
			output_zonew=str(zone_path),
			land_file=land_cfg.zone_region_file,
			cell_size=land_cfg.cell_size,
			z_factor=land_cfg.z_factor_1,
			height_map=land_cfg.height_map_file_1 or None,
			z_factor_2=land_cfg.z_factor_2,
			height_map_2=land_cfg.height_map_file_2 or None,
			extend_coords=land_cfg.extend_coords,
		)
		result = ryzom_land_tools.run_zone_elevation(binaries["zone_elevation"], elevation_cfg)
		if not zone_path.is_file():
			raise LandBuildError(f"zone_elevation failed to produce {zone_path.name}:\n{result.stdout}\n{result.stderr}")
		zone_files.append(zone_path)
	progress(f"Elevated {len(zone_files)} zone(s).")

	# Stage 4 -- zone_welder, pass 2 (.zone -> .zonew): fixes any seams
	# zone_elevation's per-zone heightmap sampling introduced at borders.
	# Written to zone_weld_dir (the real pipeline-export "zone_weld" subdir).
	progress("Welding (pass 2, with height)...")
	rebuilt_zonew_names = set()
	for zone_path in zone_files:
		zonew_path = zone_weld_dir / zone_path.with_suffix(".zonew").name
		result = ryzom_zone_tools.run_zone_welder(binaries["zone_welder"], zone_path, zonew_path)
		if not zonew_path.is_file():
			raise LandBuildError(f"zone_welder failed to produce {zonew_path.name}:\n{result.stdout}\n{result.stderr}")
		rebuilt_zonew_names.add(zone_path.stem)

	# The full current .zonew set (rebuilt this run + already up to date,
	# still on disk from a previous Build) -- zone_dependencies' range and
	# stage 6/7 both need every zone, not just the ones just rebuilt.
	zonew_files = [zone_weld_dir / f"{p.stem}.zonew" for p in zonenh_files]
	missing_zonew = [p for p in zonew_files if not p.is_file()]
	if missing_zonew:
		raise LandBuildError(f"missing .zonew for {len(missing_zonew)} zone(s) (never built): {[p.name for p in missing_zonew[:5]]}")

	# Stage 5 -- zone_dependencies: one shadow-dependency pass over the
	# whole continent's zone range at once (docs/zone_tools.md §4).
	#
	# Two real native bugs, already found/documented 2026-09-11 running this
	# same tool over nexus (project-todos/pynel/land_pipeline.md step 4,
	# nel/tools/pynel/docs/zone_tools.md §4), both handled here:
	# 1. `firstZone`/`lastZone` must decode to an already-ascending
	#    [minRow..maxRow] x [minCol..maxCol] range on BOTH axes independently
	#    -- zone_dependencies' own min/max swap has a copy-paste bug that
	#    collapses an axis needing a swap to a single row/column instead.
	#    Picking the two REAL zones with the smallest/largest (row, col)
	#    tuple is NOT enough (their columns could still be in the wrong
	#    order relative to each other even if rows are ascending) -- instead,
	#    build two SYNTHETIC corner names (`<minRow>_<minCol>`/
	#    `<maxRow>_<maxCol>`, independently computed per axis) that are
	#    guaranteed ascending on both axes at once. Neither name needs to
	#    correspond to a real file (docs/zone_tools.md §4: "never actually
	#    opened as files", only decoded for coordinates).
	# 2. The tool writes ONE `.depend` FILE PER ZONE, not a single file
	#    matching argv[4] literally -- argv[4] only supplies the output
	#    DIRECTORY and EXTENSION (`getDir`/`getExt`), the actual per-zone
	#    filename is `<out_dir>/<zone_name>.depend`, always lowercased
	#    (`toLowerAscii`, zone_dependencies.cpp:528) regardless of the real
	#    zone name's case.
	progress("Computing shadow dependencies (zone_dependencies)...")
	properties_cfg_path = out_zone_dir / f"{continent}_properties.cfg"
	properties_cfg_path.write_text(ryzom_land_tools.build_properties_cfg(props_cfg))
	zone_keys = [_zone_sort_key(p.stem) for p in zonew_files]
	min_row = min(row for row, _ in zone_keys)
	max_row = max(row for row, _ in zone_keys)
	min_col = min(col for _, col in zone_keys)
	max_col = max(col for _, col in zone_keys)
	first_zone_name = _zone_name_from_row_col(min_row, min_col)
	last_zone_name = _zone_name_from_row_col(max_row, max_col)
	depend_arg_path = zone_weld_dir / f"{continent}.depend"
	result = ryzom_land_tools.run_zone_dependencies(
		binaries["zone_dependencies"], properties_cfg_path,
		zone_weld_dir / f"{first_zone_name}.zonew", zone_weld_dir / f"{last_zone_name}.zonew", depend_arg_path,
	)

	def depend_path_for(zone_stem: str) -> Path:
		return zone_weld_dir / f"{zone_stem.lower()}.depend"

	produced_depends = [p for p in zonew_files if depend_path_for(p.stem).is_file()]
	if not produced_depends:
		raise LandBuildError(f"zone_dependencies produced no .depend file in {out_zone_dir}:\n{result.stdout}\n{result.stderr}")
	progress(f"Computed dependencies for {len(produced_depends)}/{len(zonew_files)} zone(s).")

	# Stage 6 -- zone_lighter (.zonew -> .zonel), written to zone_lighted_dir
	# (the real pipeline-export "zone_lighted" subdir) -- a zone needs
	# lighting if it has no `.zonel` yet OR its `.zonew` was just rebuilt
	# this run (rebuilt_zonew_names): a freshly rewelded `.zonew` makes any
	# OLD `.zonel` next to it stale even if one happened to exist (see this
	# function's own Stage-2 comment for the real data case that surfaced
	# this). An already up-to-date zone's real `.zonel` is left untouched.
	# Each success also fires on_zone_ready() with the freshly-lit zone's
	# own ZoneCacheData, for the live 3D viewport update (Nuno 2026-09-12) --
	# parsing the just-written .zonel here (not deferred to the caller)
	# keeps this the one place that knows the real file just landed on disk.
	# Up-to-date zones don't fire on_zone_ready either: the viewport already
	# shows them correctly from before this Build, re-parsing every
	# unchanged .zonel would defeat the whole point of skipping them.
	# Parallelized across zones (Nuno 2026-09-11, project-todos/pynel/
	# land_pipeline.md step 5's own real-machine validation script,
	# .land_pipeline_full_test.py -- rediscovered 2026-09-12 after first
	# reimplementing this sequentially): zone_lighter dominates per-zone
	# time (~2s each), and each zone's own input/output files are fully
	# independent -- no shared mutable state between threads except
	# `progress`/`on_zone_ready`, both plain dict/list writes already safe
	# under the GIL elsewhere in this codebase. `zone_lighter`'s OWN
	# internal multi-threading (`cpu_num`) is deliberately left at 1
	# (continent_pipeline_reference.py's own docstring) -- oversubscribing
	# both process-level AND thread-level parallelism at once would fight
	# the OS scheduler for no benefit; this is PROCESS-level concurrency,
	# one native `zone_lighter` per worker.
	already_lit_names = {p.stem for p in zonew_files if (zone_lighted_dir / f"{p.stem}.zonel").is_file()}
	to_light = [p for p in zonew_files if p.stem in rebuilt_zonew_names or p.stem not in already_lit_names]
	progress(f"Lighting zones (zone_lighter)... 0/{len(to_light)}")
	lit_count = 0
	lit_count_lock = threading.Lock()

	def light_one(zonew_path):
		nonlocal lit_count
		zonel_path = zone_lighted_dir / zonew_path.with_suffix(".zonel").name
		zone_depend_path = depend_path_for(zonew_path.stem)
		result = ryzom_zone_tools.run_zone_lighter(
			binaries["zone_lighter"], zonew_path, zonel_path, properties_cfg_path, zone_depend_path,
		)
		if not zonel_path.is_file():
			raise LandBuildError(f"zone_lighter failed to produce {zonel_path.name}:\n{result.stdout}\n{result.stderr}")
		with lit_count_lock:
			lit_count += 1
			progress(f"Lighting zones (zone_lighter)... {lit_count}/{len(to_light)}")
		if on_zone_ready is not None:
			try:
				zone = parse_zone(zonel_path.read_bytes())
				on_zone_ready(zonel_path.stem, zone_to_cache_data(zone))
			except (OSError, ZoneParseError):
				pass

	worker_count = os.cpu_count() or 4
	with ThreadPoolExecutor(max_workers=worker_count) as pool:
		list(pool.map(light_one, to_light))
	progress(f"Lit {len(to_light)} zone(s) ({len(zonew_files) - len(to_light)} already up to date).")

	# The full current .zonel set (rebuilt this run + already up to date).
	zonel_files = [zone_lighted_dir / f"{p.stem}.zonel" for p in zonew_files]
	missing_zonel = [p for p in zonel_files if not p.is_file()]
	if missing_zonel:
		raise LandBuildError(f"missing .zonel for {len(missing_zonel)} zone(s): {[p.name for p in missing_zonel[:5]]}")

	# Stage 7 -- zone_ig_lighter: lights each zone JUST RELIT's own .ig
	# (looked up automatically by the native tool via properties_cfg's
	# search_pathes, see run_zone_ig_lighter()'s own docstring, pynel.
	# ryzom_land_tools) -- same `to_light` set as Stage 6 (not just
	# rebuilt_zonew_names): a zone that only needed lighting for the first
	# time (never rewelded) still needs its IG lit for the first time too.
	# A cell with no matching .ig anywhere in search_pathes is simply
	# skipped (not every zone has instance groups), never a hard failure. An
	# already up-to-date zone's `.ig` was already lit by a previous Build.
	progress("Lighting instance groups (zone_ig_lighter)...")
	out_ig_dir = Path(land_cfg.out_ig_dir)
	lit_ig_count = 0
	to_light_names = {p.stem for p in to_light}
	for zonel_path in (p for p in zonel_files if p.stem in to_light_names):
		output_ig_path = out_ig_dir / f"{zonel_path.stem}.ig"
		zone_depend_path = depend_path_for(zonel_path.stem)
		ryzom_land_tools.run_zone_ig_lighter(
			binaries["zone_ig_lighter"], zonel_path, output_ig_path, properties_cfg_path, zone_depend_path,
		)
		if output_ig_path.is_file():
			lit_ig_count += 1

	progress(f"Build complete: {len(zonel_files)} zone(s), {lit_ig_count} instance group(s).")
	return [str(p) for p in zonel_files]
