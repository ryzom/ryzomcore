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
from dataclasses import dataclass
from pathlib import Path
from typing import Union


class LandExportError(Exception):
	pass


@dataclass
class LandExportConfig:
	"""land_export's .cfg fields (SOptions::loadFromCfg, land_export/main.cpp:155-201),
	confirmed exhaustive against a real .cfg (nexus/generated/land_exporter.cfg).
	ColorMapFile appears in that real file but is never actually read by
	loadFromCfg -- a dead field, intentionally omitted here."""
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
