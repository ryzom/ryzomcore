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

"""Drives the native PACS collision pipeline (`build_ig_boxes`, `build_rbank`,
`build_indoor_rbank` -- `nel/tools/pacs/*/main.cpp`, ryzom-core) as
subprocesses (project-todos/forgery/landscape_editor__pacs_export.md).

Unlike `land_export`/`zone_welder`/`zone_lighter` (an explicit `.cfg` path as
argv), all 3 of these tools hard-load a FIXED filename from their current
working directory (`cf.load("build_ig_boxes.cfg")`/`"build_rbank.cfg"`/
`"build_indoor_rbank.cfg")`, confirmed directly in each one's own
`main.cpp` -- so every `run_*()` here takes a `work_dir` and runs the binary
with `cwd=work_dir`, after writing the `.cfg` there under that exact name
(removed again after the call, same convention as the real
`processes/rbank/2_build.py` this reimplements).

All 3 tools return a plain 0 on completion (confirmed: every `main()`'s
own final `return 0`, no special exit code convention unlike `land_export`)
-- a `CalledProcessError`-free run is not proof of success (diagnostics go
through `nlwarning`/stdout, never a non-zero exit on their own), so the
caller is responsible for validating whatever output file(s) were expected.

`build_rbank`'s CLI flags (confirmed in `main.cpp`, argv parsing), 3 calls
matching `landscape_editor__pacs_export.md`'s own step split:
  - "check prims": `-C -p -g` (CheckPrims on, no other pass) -- step 2.
  - "per zone" (moulinage): `-c -P -g <zone1> <zone2> ...` -- step 3.
  - "global" (retrievers): `-c -P -G`, no zone names -- step 4.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Union
import subprocess


class PacsToolError(Exception):
	pass


def _cfg_string(value: str) -> str:
	if '"' in value or "\\" in value:
		raise PacsToolError(f"unsupported character in .cfg string value: {value!r}")
	return f'"{value}"'


def _cfg_string_array(values: List[str]) -> str:
	return "{ " + ", ".join(_cfg_string(v) for v in values) + " }"


def _cfg_bool(value: bool) -> str:
	return "1" if value else "0"


@dataclass
class IgBoxesConfig:
	"""`build_ig_boxes.cfg` fields (`main.cpp`'s own `getVar("Pathes")`/
	`getVar("IGs")`/`getVar("Output")`)."""
	pathes: List[str]
	igs: List[str]
	output: str


def build_ig_boxes_cfg(config: IgBoxesConfig) -> str:
	lines = [
		f"Pathes = {_cfg_string_array(config.pathes)};",
		f"IGs = {_cfg_string_array(config.igs)};",
		f"Output = {_cfg_string(config.output)};",
	]
	return "\n".join(lines) + "\n"


def run_build_ig_boxes(binary_path: Union[str, Path], config: IgBoxesConfig, work_dir: Union[str, Path]) -> subprocess.CompletedProcess:
	"""Runs `build_ig_boxes` with `work_dir/build_ig_boxes.cfg` as its
	(hardcoded-name) config -- the caller must check `config.output` was
	actually produced, a 0 exit code alone doesn't confirm it."""
	work_dir = Path(work_dir)
	work_dir.mkdir(parents=True, exist_ok=True)
	cfg_path = work_dir / "build_ig_boxes.cfg"
	cfg_path.write_text(build_ig_boxes_cfg(config), encoding="utf-8")
	try:
		return subprocess.run([str(binary_path)], cwd=str(work_dir), capture_output=True, text=True)
	finally:
		cfg_path.unlink(missing_ok=True)


@dataclass
class RbankConfig:
	"""`build_rbank.cfg` fields, field order matching `processes/rbank/
	2_build.py`'s own generation (not a real per-continent example on disk,
	unlike `land_export`'s own `.cfg` -- this tool never wrote one to a
	persistent location in the legacy pipeline, only a transient file in
	its own working directory, removed right after the call)."""
	verbose: bool
	check_consistency: bool
	zone_path: str
	banks_path: str
	bank: str
	ig_boxes: str
	level_design_world_path: str
	ig_land_path: str
	ig_village_path: str
	tessellation_path: str
	tessellate_level: int
	output_root_path: str
	smooth_directory: str
	raw_directory: str
	reduce_surfaces: bool
	smooth_borders: bool
	compute_elevation: bool
	compute_levels: bool
	link_elements: bool
	cut_edges: bool
	use_zone_square: bool
	zone_ul: str
	zone_dr: str
	preprocess_directory: str
	global_retriever: str
	retriever_bank: str
	zones: List[str] = field(default_factory=list)
	pathes: List[str] = field(default_factory=list)
	zone_ext: str = ".zonew"
	zone_nh_ext: str = ".zonenhw"
	water_threshold: float = 1.0
	tessellate_zones: int = 0
	mouline_zones: int = 0
	process_retrievers: int = 0
	process_global: int = 0


def build_rbank_cfg(config: RbankConfig) -> str:
	lines = [
		f"Verbose = {_cfg_bool(config.verbose)};",
		f"CheckConsistency = {_cfg_bool(config.check_consistency)};",
		f"ZonePath = {_cfg_string(config.zone_path)};",
		f"BanksPath = {_cfg_string(config.banks_path)};",
		f"Bank = {_cfg_string(config.bank)};",
		f"ZoneExt = {_cfg_string(config.zone_ext)};",
		f"ZoneNHExt = {_cfg_string(config.zone_nh_ext)};",
		f"IGBoxes = {_cfg_string(config.ig_boxes)};",
		f"LevelDesignWorldPath = {_cfg_string(config.level_design_world_path)};",
		f"IgLandPath = {_cfg_string(config.ig_land_path)};",
		f"IgVillagePath = {_cfg_string(config.ig_village_path)};",
		f"TessellationPath = {_cfg_string(config.tessellation_path)};",
		f"TessellateLevel = {config.tessellate_level};",
		f"WaterThreshold = {config.water_threshold};",
		f"OutputRootPath = {_cfg_string(config.output_root_path)};",
		f"SmoothDirectory = {_cfg_string(config.smooth_directory)};",
		f"RawDirectory = {_cfg_string(config.raw_directory)};",
		f"ReduceSurfaces = {_cfg_bool(config.reduce_surfaces)};",
		f"SmoothBorders = {_cfg_bool(config.smooth_borders)};",
		f"ComputeElevation = {_cfg_bool(config.compute_elevation)};",
		f"ComputeLevels = {_cfg_bool(config.compute_levels)};",
		f"LinkElements = {_cfg_bool(config.link_elements)};",
		f"CutEdges = {_cfg_bool(config.cut_edges)};",
		f"UseZoneSquare = {_cfg_bool(config.use_zone_square)};",
		f"ZoneUL = {_cfg_string(config.zone_ul)};",
		f"ZoneDR = {_cfg_string(config.zone_dr)};",
		f"PreprocessDirectory = {_cfg_string(config.preprocess_directory)};",
		f"GlobalRetriever = {_cfg_string(config.global_retriever)};",
		f"RetrieverBank = {_cfg_string(config.retriever_bank)};",
		f"GlobalUL = {_cfg_string(config.zone_ul)};",
		f"GlobalDR = {_cfg_string(config.zone_dr)};",
		f"TessellateZones = {config.tessellate_zones};",
		f"MoulineZones = {config.mouline_zones};",
		f"ProcessRetrievers = {config.process_retrievers};",
		f"ProcessGlobal = {config.process_global};",
		f"Zones = {_cfg_string_array(config.zones)};",
		f"Pathes = {_cfg_string_array(config.pathes)};",
	]
	return "\n".join(lines) + "\n"


def _run_build_rbank(
	binary_path: Union[str, Path], config: RbankConfig, work_dir: Union[str, Path], flags: List[str],
) -> subprocess.CompletedProcess:
	work_dir = Path(work_dir)
	work_dir.mkdir(parents=True, exist_ok=True)
	cfg_path = work_dir / "build_rbank.cfg"
	cfg_path.write_text(build_rbank_cfg(config), encoding="utf-8")
	try:
		return subprocess.run([str(binary_path)] + flags, cwd=str(work_dir), capture_output=True, text=True)
	finally:
		cfg_path.unlink(missing_ok=True)


def run_build_rbank_check_prims(binary_path: Union[str, Path], config: RbankConfig, work_dir: Union[str, Path]) -> subprocess.CompletedProcess:
	"""`build_rbank -C -p -g` -- validates the collision primitives referenced
	by `config.zones`, no `.lr`/`.rbank`/`.gr` output (project-todos/forgery/
	landscape_editor__pacs_export.md step 2)."""
	return _run_build_rbank(binary_path, config, work_dir, ["-C", "-p", "-g"])


def run_build_rbank_zones(
	binary_path: Union[str, Path], config: RbankConfig, work_dir: Union[str, Path], zone_names: List[str],
) -> subprocess.CompletedProcess:
	"""`build_rbank -c -P -g <zone1> <zone2> ...` -- tessellates+mouline only
	`zone_names` (not necessarily all of `config.zones`), producing one
	`.lr` per zone under `config.smooth_directory` (step 3)."""
	return _run_build_rbank(binary_path, config, work_dir, ["-c", "-P", "-g"] + list(zone_names))


def run_build_rbank_global(binary_path: Union[str, Path], config: RbankConfig, work_dir: Union[str, Path]) -> subprocess.CompletedProcess:
	"""`build_rbank -c -P -G` -- merges every zone's `.lr` into
	`temp.rbank`/`temp.gr` under `config.preprocess_directory` (step 4)."""
	return _run_build_rbank(binary_path, config, work_dir, ["-c", "-P", "-G"])


@dataclass
class IndoorRbankConfig:
	"""`build_indoor_rbank.cfg` fields (`processes/rbank/2_build.py`'s own
	generation -- `MergeInputPrefix`/`MergeOutputPrefix` fixed to
	`"temp"`/`"tempMerged"`, matching `build_rbank`'s own global-pass output
	name and the final rename step's own expected input prefix)."""
	mesh_path: str
	meshes: List[str]
	output_path: str
	merge_path: str
	output_prefix: str = "unused"
	merge: bool = True
	merge_input_prefix: str = "temp"
	merge_output_prefix: str = "tempMerged"
	add_to_retriever: bool = True


def build_indoor_rbank_cfg(config: IndoorRbankConfig) -> str:
	lines = [
		f"MeshPath = {_cfg_string(config.mesh_path)};",
		f"Meshes = {_cfg_string_array(config.meshes)};",
		f"OutputPath = {_cfg_string(config.output_path)};",
		f"OutputPrefix = {_cfg_string(config.output_prefix)};",
		f"Merge = {_cfg_bool(config.merge)};",
		f"MergePath = {_cfg_string(config.merge_path)};",
		f"MergeInputPrefix = {_cfg_string(config.merge_input_prefix)};",
		f"MergeOutputPrefix = {_cfg_string(config.merge_output_prefix)};",
		f"AddToRetriever = {_cfg_bool(config.add_to_retriever)};",
	]
	return "\n".join(lines) + "\n"


def run_build_indoor_rbank(binary_path: Union[str, Path], config: IndoorRbankConfig, work_dir: Union[str, Path]) -> subprocess.CompletedProcess:
	"""Runs `build_indoor_rbank` with `work_dir/build_indoor_rbank.cfg` as
	its (hardcoded-name) config -- merges `config.meshes` (`.cmb` collision
	meshes) into the `tempMerged.*` retriever bank produced by
	`run_build_rbank_global()` (step 5)."""
	work_dir = Path(work_dir)
	work_dir.mkdir(parents=True, exist_ok=True)
	cfg_path = work_dir / "build_indoor_rbank.cfg"
	cfg_path.write_text(build_indoor_rbank_cfg(config), encoding="utf-8")
	try:
		return subprocess.run([str(binary_path)], cwd=str(work_dir), capture_output=True, text=True)
	finally:
		cfg_path.unlink(missing_ok=True)
