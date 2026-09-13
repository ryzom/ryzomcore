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

"""Drive the native `land_export` tool (ryzom/tools/leveldesign/world_editor/
land_export/main.cpp) as a subprocess.

Thin wrapper only -- no automatic binary discovery (the caller passes an
explicit path to the built binary) -- same principle as intended for
pynel.ryzom_zone_tools (zone_welder/zone_lighter), but this module stands on
its own: that module doesn't exist yet, so nothing is shared/reused here.

Unlike zone_welder/zone_lighter, `land_export`'s `.cfg` has ~20 fields with
no real per-scene template to copy by hand (see the real example,
ryzom-data/leveldesign/workspace/continents/nexus/generated/land_exporter.cfg)
-- so this module DOES generate the `.cfg` text from a dataclass, via
build_land_export_cfg(), unlike the caller-supplies-the-cfg convention
documented for zone_lighter in docs/zone_tools.md.

`land_export`'s CLI contract (confirmed in main.cpp, NOT the usual 0/1 exit
code convention):
  - argc != 2, or a config/land-file load failure (SOptions::loadFromCfg /
    loadLand returning false): main returns -1 directly.
  - CExportCB::dispError() (main.cpp:93-98) prints "ERROR : <msg>" to stdout
    and calls exit(-1) immediately, from anywhere inside CExport::export_.
  - Otherwise main always returns 1 -- CExport::export_'s own bool return
    value is never checked (main.cpp:309-311). So **1 means success, -1
    means failure**; any other exit code is unexpected.
  - All diagnostic output (dispWarning/dispError/progress) goes to stdout
    (main.cpp's outString(), via NLMISC InfoLog), not stderr.

Read-only orchestration: this module never inspects/loads the resulting
.zone/.zonel/.zonenh/.ig files, that's the caller's job (e.g. via
pynel.ryzom_zone.load_zone to confirm a real, valid file was produced).

Usage:
	from pynel import ryzom_land_tools as rlt
	cfg = rlt.LandExportConfig(
		out_zone_dir="...", ref_zone_dir="...", zone_region_file="nexus.land", ...
	)
	rlt.run_land_export("/path/to/land_export", cfg, Path("/tmp/land_exporter.cfg"))
"""

import subprocess
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple, Union


class LandExportError(Exception):
	pass


@dataclass
class LandExportConfig:
	"""land_export's .cfg fields (SOptions::loadFromCfg, land_export/main.cpp:155-201),
	confirmed exhaustive against a real .cfg (nexus/generated/land_exporter.cfg).

	`color_map_file` (optional, empty string if unused) IS actually read and
	applied -- corrected 2026-09-12: an earlier version of this docstring
	claimed it was a dead field never read by `loadFromCfg`, contradicted by
	`main.cpp:174` (`this->ColorMapFile = getStr("ColorMapFile")`) and its
	real use in `CExport::addColorMap()`/`getColor()` (`export.cpp:1170,2225`)
	to tint every patch vertex from the image, a per-continent optional
	color layer on top of the tile textures."""
	out_zone_dir: str
	ref_zone_dir: str
	ref_ig_dir: str
	out_ig_dir: str
	ligo_bank_dir: str
	tile_bank_file: str
	height_map_file_1: str
	z_factor_1: float
	height_map_file_2: str
	z_factor_2: float
	extend_coords: bool
	zone_light: int  # 0/1/2 = none/patch/noise
	cell_size: float
	threshold: float
	zone_region_file: str
	export_collisions: bool
	export_additionnal_igs: bool
	ref_cmb_dir: str
	out_cmb_dir: str
	additionnal_ig_in_dir: str
	additionnal_ig_out_dir: str
	dfn_dir: str
	continent_file: str
	continents_dir: str
	color_map_file: str = ""


def _cfg_string(value: str) -> str:
	if '"' in value or "\\" in value:
		raise LandExportError(f"unsupported character in .cfg string value: {value!r}")
	return f'"{value}"'


def build_land_export_cfg(config: LandExportConfig) -> str:
	"""Generates land_export's .cfg text (CConfigFile syntax, docs/zone_tools.md
	§3) from a LandExportConfig -- avoids hand-writing this ~20-field format
	for every call. Field order follows SOptions::loadFromCfg."""
	lines = [
		f"OutZoneDir = {_cfg_string(config.out_zone_dir)};",
		f"OutIGDir = {_cfg_string(config.out_ig_dir)};",
		f"RefZoneDir = {_cfg_string(config.ref_zone_dir)};",
		f"RefIGDir = {_cfg_string(config.ref_ig_dir)};",
		f"LigoBankDir = {_cfg_string(config.ligo_bank_dir)};",
		f"TileBankFile = {_cfg_string(config.tile_bank_file)};",
		f"HeightMapFile1 = {_cfg_string(config.height_map_file_1)};",
		f"ZFactor1 = {config.z_factor_1};",
		f"HeightMapFile2 = {_cfg_string(config.height_map_file_2)};",
		f"ZFactor2 = {config.z_factor_2};",
		f"ExtendCoords = {1 if config.extend_coords else 0};",
		f"ZoneLight = {config.zone_light};",
		f"CellSize = {config.cell_size};",
		f"Threshold = {config.threshold};",
		f"ZoneRegionFile = {_cfg_string(config.zone_region_file)};",
		f"ExportCollisions = {1 if config.export_collisions else 0};",
		f"ExportAdditionnalIGs = {1 if config.export_additionnal_igs else 0};",
		f"RefCMBDir = {_cfg_string(config.ref_cmb_dir)};",
		f"OutCMBDir = {_cfg_string(config.out_cmb_dir)};",
		f"AdditionnalIGInDir = {_cfg_string(config.additionnal_ig_in_dir)};",
		f"AdditionnalIGOutDir = {_cfg_string(config.additionnal_ig_out_dir)};",
		f"DFNDir = {_cfg_string(config.dfn_dir)};",
		f"ContinentFile = {_cfg_string(config.continent_file)};",
		f"ContinentsDir = {_cfg_string(config.continents_dir)};",
	]
	if config.color_map_file:
		lines.append(f"ColorMapFile = {_cfg_string(config.color_map_file)};")
	# A file ending in a comment with no trailing newline throws on load
	# (config_file.h quirk) -- always end with a blank line.
	return "\n".join(lines) + "\n\n"


def run_land_export(
	binary_path: Union[str, Path],
	config: LandExportConfig,
	cfg_path: Union[str, Path],
) -> subprocess.CompletedProcess:
	"""Writes config's .cfg to cfg_path, then runs land_export on it.
	Raises LandExportError on anything but exit code 1 (see module docstring
	for land_export's non-standard exit code convention). The caller is
	still responsible for validating the produced .zone/.zonel/.zonenh/.ig
	files -- a 1 exit code only means main() reached its end, not that every
	individual cell was exported without a dispWarning."""
	Path(cfg_path).write_text(build_land_export_cfg(config), encoding="utf-8")

	result = subprocess.run(
		[str(binary_path), str(cfg_path)],
		capture_output=True,
		text=True,
	)
	if result.returncode != 1:
		raise LandExportError(
			f"land_export failed (exit code {result.returncode}):\n{result.stdout}"
		)
	return result


class ZoneElevationError(Exception):
	pass


@dataclass
class ElevationConfig:
	"""zone_elevation's CLI arguments (docs/zone_tools.md §3, confirmed
	against nel/tools/3d/zone_elevation/zone_elevation.cpp). Exactly one of
	`land_file` or (`zone_min` and `zone_max`) must be given -- run_zone_elevation()
	raises ZoneElevationError otherwise, rather than let the native tool's
	own EXIT_FAILURE (a bare exit code, no message on stdout for this
	particular case) stand in for a caller mistake."""
	input_zonenhw: str
	output_zonew: str
	land_file: Optional[str] = None
	zone_min: Optional[str] = None
	zone_max: Optional[str] = None
	cell_size: float = 160.0
	z_factor: float = 1.0
	height_map: Optional[str] = None
	z_factor_2: float = 1.0
	height_map_2: Optional[str] = None
	extend_coords: bool = False


def run_zone_elevation(
	binary_path: Union[str, Path],
	config: ElevationConfig,
) -> subprocess.CompletedProcess:
	"""Runs `zone_elevation <input_zonenhw> <output_zonew> [options]`.
	Unlike every other tool in this module, zone_elevation uses a real
	EXIT_SUCCESS/EXIT_FAILURE exit code (docs/zone_tools.md §3) -- raises
	ZoneElevationError on anything else. `--heightmap`/`--heightmap2` are
	optional even when given (a missing/unreadable file just leaves that
	elevation source unapplied, never fatal) -- the caller is responsible
	for checking the produced .zonew's actual heights if that matters."""
	if (config.land_file is None) == (config.zone_min is None and config.zone_max is None):
		raise ZoneElevationError(
			"ElevationConfig needs exactly one of land_file or (zone_min and zone_max)"
		)
	if (config.zone_min is None) != (config.zone_max is None):
		raise ZoneElevationError("ElevationConfig needs both zone_min and zone_max, or neither")

	args = [str(binary_path), str(config.input_zonenhw), str(config.output_zonew)]
	if config.land_file is not None:
		args += ["--land", str(config.land_file)]
	else:
		args += ["--zonemin", str(config.zone_min), "--zonemax", str(config.zone_max)]
	args += ["--cellsize", str(config.cell_size)]
	args += ["--zfactor", str(config.z_factor)]
	if config.height_map is not None:
		args += ["--heightmap", str(config.height_map)]
	args += ["--zfactor2", str(config.z_factor_2)]
	if config.height_map_2 is not None:
		args += ["--heightmap2", str(config.height_map_2)]
	if config.extend_coords:
		args.append("--extendcoords")

	result = subprocess.run(args, capture_output=True, text=True)
	if result.returncode not in (0, 1):
		# EXIT_SUCCESS/EXIT_FAILURE are 0/1 on every platform this tool
		# actually builds for (docs/zone_tools.md §3) -- anything else is
		# genuinely unexpected (e.g. a crash), not a documented failure.
		raise ZoneElevationError(
			f"zone_elevation exited with an unexpected code {result.returncode}:\n{result.stderr}"
		)
	if result.returncode != 0:
		raise ZoneElevationError(f"zone_elevation failed:\n{result.stderr}")
	return result


@dataclass
class PropertiesConfig:
	"""`properties.cfg` fields actually read by zone_lighter/zone_dependencies/
	zone_ig_lighter (docs/zone_tools.md §2, §4, §5 -- confirmed exhaustive
	against all 3 tools' real C++ source, 2026-09-11, after two corrections:
	see project-todos/pynel/land_pipeline.md step 4 for the full history,
	notably the 7 fields wrongly pulled in from the unrelated `ig_lighter`
	tool on the first pass). Field grouping below matches which tool(s)
	actually require it -- there is no meaningful "PropertiesConfig for just
	zone_lighter" vs "for zone_ig_lighter", every real continent's .cfg is
	one shared file read by whichever of the 3 tools runs against it."""
	# Base -- required by all 3 tools.
	bank_name: str
	search_pathes: List[str]
	load_ig: bool
	shadow: bool
	sun_direction: Tuple[float, float, float]
	sun_center: Tuple[float, float, float]
	sun_distance: float
	sun_fov: float
	sun_radius: float
	zbuffer_landscape_size: int
	zbuffer_object_size: int
	soft_shadow_samples_sqrt: int
	soft_shadow_jitter: float
	sun_contribution: bool
	sky_contribution: bool
	sky_intensity: float
	global_illumination_cell_size: float
	water_shadow_bias: float
	water_ambient: float
	water_diffuse: float
	modulate_water_color: bool
	sky_contribution_for_water: bool
	global_illumination_length: float
	quad_grid_size: float
	quad_grid_cell_size: float
	cpu_num: int
	vegetable_height: float
	additionnal_ig: List[str] = field(default_factory=list)
	# Specific to zone_ig_lighter.
	ig_oversampling: int = 16
	# Specific to zone_dependencies.
	compute_dependencies_with_igs: bool = True
	# Optional -- wrapped in try/catch(EUnknownVar) on the C++ side, used by
	# zone_dependencies' computeIGBBoxFromContinent() only when
	# compute_dependencies_with_igs is true. Silently omitted from the
	# generated .cfg (not written as empty strings) when left None, matching
	# their real absence from a .cfg that doesn't set them.
	continent_name: Optional[str] = None
	level_design_directory: Optional[str] = None
	level_design_world_directory: Optional[str] = None
	level_design_dfn_directory: Optional[str] = None


def _cfg_bool(value: bool) -> str:
	return "1" if value else "0"


def _cfg_vec3(value: Tuple[float, float, float]) -> str:
	return f"{{ {value[0]}, {value[1]}, {value[2]} }}"


def _cfg_string_array(values: List[str]) -> str:
	return "{ " + ", ".join(_cfg_string(v) for v in values) + " }"


def build_properties_cfg(config: PropertiesConfig) -> str:
	"""Generates `properties.cfg` text (CConfigFile syntax, docs/zone_tools.md
	§6) from a PropertiesConfig. Field order matches the grouping in
	PropertiesConfig's own docstring, not any particular real file's
	layout -- CConfigFile has no notion of section order."""
	lines = [
		f"bank_name = {_cfg_string(config.bank_name)};",
		f"search_pathes = {_cfg_string_array(config.search_pathes)};",
		f"additionnal_ig = {_cfg_string_array(config.additionnal_ig)};",
		f"load_ig = {_cfg_bool(config.load_ig)};",
		f"shadow = {_cfg_bool(config.shadow)};",
		f"sun_direction = {_cfg_vec3(config.sun_direction)};",
		f"sun_center = {_cfg_vec3(config.sun_center)};",
		f"sun_distance = {config.sun_distance};",
		f"sun_fov = {config.sun_fov};",
		f"sun_radius = {config.sun_radius};",
		f"zbuffer_landscape_size = {config.zbuffer_landscape_size};",
		f"zbuffer_object_size = {config.zbuffer_object_size};",
		f"soft_shadow_samples_sqrt = {config.soft_shadow_samples_sqrt};",
		f"soft_shadow_jitter = {config.soft_shadow_jitter};",
		f"sun_contribution = {_cfg_bool(config.sun_contribution)};",
		f"sky_contribution = {_cfg_bool(config.sky_contribution)};",
		f"sky_intensity = {config.sky_intensity};",
		f"global_illumination_cell_size = {config.global_illumination_cell_size};",
		f"water_shadow_bias = {config.water_shadow_bias};",
		f"water_ambient = {config.water_ambient};",
		f"water_diffuse = {config.water_diffuse};",
		f"modulate_water_color = {_cfg_bool(config.modulate_water_color)};",
		f"sky_contribution_for_water = {_cfg_bool(config.sky_contribution_for_water)};",
		f"global_illumination_length = {config.global_illumination_length};",
		f"quad_grid_size = {config.quad_grid_size};",
		f"quad_grid_cell_size = {config.quad_grid_cell_size};",
		f"cpu_num = {config.cpu_num};",
		f"vegetable_height = {config.vegetable_height};",
		f"ig_oversampling = {config.ig_oversampling};",
		f"compute_dependencies_with_igs = {_cfg_bool(config.compute_dependencies_with_igs)};",
	]
	if config.continent_name is not None:
		lines.append(f"continent_name = {_cfg_string(config.continent_name)};")
	if config.level_design_directory is not None:
		lines.append(f"level_design_directory = {_cfg_string(config.level_design_directory)};")
	if config.level_design_world_directory is not None:
		lines.append(f"level_design_world_directory = {_cfg_string(config.level_design_world_directory)};")
	if config.level_design_dfn_directory is not None:
		lines.append(f"level_design_dfn_directory = {_cfg_string(config.level_design_dfn_directory)};")
	# Same trailing-blank-line quirk as build_land_export_cfg().
	return "\n".join(lines) + "\n\n"


def run_zone_dependencies(
	binary_path: Union[str, Path],
	properties_cfg_path: Union[str, Path],
	first_zone_path: Union[str, Path],
	last_zone_path: Union[str, Path],
	output_depend_path: Union[str, Path],
) -> subprocess.CompletedProcess:
	"""Runs `zone_dependencies <properties_cfg_path> <first_zone_path>
	<last_zone_path> <output_depend_path>`. `first_zone_path`/`last_zone_path`
	are never actually opened by the native tool -- only their names (to
	derive the zone coordinate range to scan) and shared directory/extension
	(to reconstruct each real zone's path) are used (docs/zone_tools.md §4),
	so neither needs to exist on disk. Always exits 0 regardless of outcome
	(same non-standard convention as zone_lighter, ryzom_zone_tools.py) --
	never raises here, the caller must validate output_depend_path itself."""
	return subprocess.run(
		[str(binary_path), str(properties_cfg_path), str(first_zone_path), str(last_zone_path), str(output_depend_path)],
		capture_output=True,
		text=True,
	)


def run_zone_ig_lighter(
	binary_path: Union[str, Path],
	input_zonel_path: Union[str, Path],
	output_ig_path: Union[str, Path],
	properties_cfg_path: Union[str, Path],
	dependency_depend_path: Union[str, Path],
) -> subprocess.CompletedProcess:
	"""Runs `zone_ig_lighter <input_zonel_path> <output_ig_path>
	<properties_cfg_path> <dependency_depend_path>`. The zone's own
	`<name>.ig` (matching input_zonel_path's stem) is looked up automatically
	via the search paths in properties_cfg_path -- its absence is the one
	hard, silent failure case in this whole pipeline (docs/zone_tools.md §5:
	immediate `return 0`, a stderr-only message that never reaches log.log),
	so the caller must make sure that file is actually reachable rather than
	rely on any diagnostic from this call. Always exits 0 regardless of
	outcome (same non-standard convention as zone_lighter) -- never raises
	here, the caller must validate output_ig_path itself."""
	return subprocess.run(
		[
			str(binary_path), str(input_zonel_path), str(output_ig_path),
			str(properties_cfg_path), str(dependency_depend_path),
		],
		capture_output=True,
		text=True,
	)
