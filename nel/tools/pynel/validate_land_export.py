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

"""Standalone validation script for project-todos/pynel/land_pipeline.md step 5
-- NOT part of the pynel package, not installed as a console script: this is a
one-off smoke test, run manually once a Linux `land_export` binary exists
(see project-todos/pynel/land_pipeline.md step 3's CMake note -- land_export
was WIN32-gated by mistake, fixed in ryzom/tools/leveldesign/CMakeLists.txt,
still needs a rebuild before this script can run).

Drives `land_export` on the real nexus continent (jungle ecosystem), using
paths under a real ryzom-data checkout, then validates every produced
.zonenh with pynel.ryzom_zone.load_zone().

Known gaps in this ryzom-data checkout (confirmed absent, 2026-09-06) --
this script routes around them rather than failing outright:
  - No big_nexus.tga/noise_nexus.tga (only a single nexus.tga) -- used as
    HeightMapFile1, HeightMapFile2 left empty (valid: CExport::export_ skips
    loading it when empty, export.cpp:243).
  - No .ig files at all under leveldesign/landscape/jungle -- RefIGDir/
    AdditionnalIGInDir point at real-but-empty directories. Per-brick IG
    export will very likely just find nothing per cell (same fileExist-based
    skip logic as zones) rather than fail -- NOT verified against the actual
    cutIG/transformIG codepath, since there is no local data to trace it
    against. Watch the output for this.
  - No .cmb (collision mesh build) data -- ExportCollisions left at 0 to
    avoid exercising that path with directories that don't exist at all.

Usage:
	python3 validate_land_export.py --binary /path/to/land_export --ryzom-data /home/ulukyn/repos/ryzom-data --work-dir /tmp/land_export_test
"""

import argparse
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from pynel import ryzom_land_tools as rlt
from pynel import ryzom_zone as rz


def _check_exists(label: str, path: Path, must_have_files: bool = False) -> bool:
	if not path.exists():
		print(f"MISSING {label}: {path}")
		return False
	if must_have_files and not any(path.iterdir()):
		print(f"WARNING {label} exists but is empty: {path}")
	return True


def main() -> int:
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("--binary", required=True, help="path to the built land_export executable")
	parser.add_argument("--ryzom-data", required=True, help="path to a ryzom-data checkout")
	parser.add_argument("--work-dir", required=True, help="scratch directory for .cfg + outputs (created if missing)")
	args = parser.parse_args()

	ryzom_data = Path(args.ryzom_data)
	work_dir = Path(args.work_dir)
	jungle = ryzom_data / "leveldesign" / "landscape" / "jungle"

	print("--- Checking real input paths ---")
	ok = True
	ok &= _check_exists("RefZoneDir", jungle / "zones", must_have_files=True)
	ok &= _check_exists("LigoBankDir", jungle / "zoneligos", must_have_files=True)
	ok &= _check_exists("TileBankFile", ryzom_data / "graphics/landscape/_texture_tiles/jungle/jungle.bank")
	ok &= _check_exists("HeightMapFile1", ryzom_data / "graphics/landscape/ligo/jungle/nexus.tga")
	ok &= _check_exists("ZoneRegionFile", jungle / "nexus.land")
	ok &= _check_exists("DFNDir", ryzom_data / "leveldesign/DFN", must_have_files=True)
	ok &= _check_exists("ContinentFile", ryzom_data / "leveldesign/world/lecarrefour/lecarrefour.continent")
	ok &= _check_exists("ContinentsDir", ryzom_data / "leveldesign/world")
	if not ok:
		print("Some real inputs are missing -- fix the --ryzom-data path or this checkout is incomplete.")
		return 1

	out_zone_dir = work_dir / "out_zones"
	out_ig_dir = work_dir / "out_igs"
	out_cmb_dir = work_dir / "out_cmb"
	ref_ig_dir = work_dir / "empty_ref_igs"  # no real .ig data for jungle in this checkout, see module docstring
	additionnal_ig_dir = work_dir / "empty_additionnal_igs"
	for d in (out_zone_dir, out_ig_dir, out_cmb_dir, ref_ig_dir, additionnal_ig_dir):
		d.mkdir(parents=True, exist_ok=True)

	config = rlt.LandExportConfig(
		out_zone_dir=str(out_zone_dir),
		ref_zone_dir=str(jungle / "zones"),
		ref_ig_dir=str(ref_ig_dir),
		out_ig_dir=str(out_ig_dir),
		ligo_bank_dir=str(jungle / "zoneligos"),
		tile_bank_file=str(ryzom_data / "graphics/landscape/_texture_tiles/jungle/jungle.bank"),
		height_map_file_1=str(ryzom_data / "graphics/landscape/ligo/jungle/nexus.tga"),
		z_factor_1=1.0,
		height_map_file_2="",
		z_factor_2=0.5,
		extend_coords=False,
		zone_light=0,
		cell_size=160.0,
		threshold=1.0,
		zone_region_file=str(jungle / "nexus.land"),
		export_collisions=False,
		export_additionnal_igs=False,
		ref_cmb_dir="",
		out_cmb_dir=str(out_cmb_dir),
		additionnal_ig_in_dir=str(additionnal_ig_dir),
		additionnal_ig_out_dir=str(work_dir / "out_additionnal_igs"),
		dfn_dir=str(ryzom_data / "leveldesign/DFN"),
		continent_file=str(ryzom_data / "leveldesign/world/lecarrefour/lecarrefour.continent"),
		continents_dir=str(ryzom_data / "leveldesign/world"),
	)

	cfg_path = work_dir / "land_exporter.cfg"
	print(f"\n--- Running land_export (cfg: {cfg_path}) ---")
	try:
		result = rlt.run_land_export(args.binary, config, cfg_path)
	except rlt.LandExportError as e:
		print(f"land_export FAILED: {e}")
		return 1
	print(result.stdout)

	print("\n--- Validating produced .zonenh files with pynel.ryzom_zone.load_zone() ---")
	zonenh_files = sorted(out_zone_dir.glob("*.zonenh"))
	if not zonenh_files:
		print("No .zonenh files were produced -- check the log above.")
		return 1
	failures = 0
	for f in zonenh_files:
		try:
			rz.load_zone(f)
		except Exception as e:
			print(f"FAILED to parse {f.name}: {e}")
			failures += 1
	print(f"{len(zonenh_files) - failures}/{len(zonenh_files)} .zonenh files parsed OK")
	return 1 if failures else 0


if __name__ == "__main__":
	raise SystemExit(main())
