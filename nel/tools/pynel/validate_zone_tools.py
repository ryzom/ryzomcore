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

"""Standalone validation script for project-todos/pynel/zone_read_write.md
step 4 -- NOT part of the pynel package, not installed as a console script:
a one-off smoke test for pynel.ryzom_zone_tools, run manually against real
zone_welder/zone_lighter binaries and a real .zonel taken from ryzom-data
(re-fed as a plain .zone input -- same binary CZone format, see
ryzom_zone.py module docstring).

Usage:
	python3 validate_zone_tools.py --welder /path/to/zone_welder --lighter /path/to/zone_lighter \\
		--input-zonel /home/ulukyn/repos/ryzom-data/final_bnps/undernexus_zones/55_CC.zonel \\
		--work-dir /tmp/zone_tools_test
"""

import argparse
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from pynel import ryzom_zone as rz
from pynel import ryzom_zone_tools as rzt


PARAMETER_FILE_CFG = """\
bank_name = "";
search_pathes = {};
additionnal_ig = {};
load_ig = 0;
shadow = 0;
sun_direction = { -0.776685, 0.216619, -0.59147 };
sun_center = { 14240, -10880, 0 };
sun_distance = 50000;
sun_fov = 0.52359877;
sun_radius = 5000;
zbuffer_landscape_size = 512;
zbuffer_object_size = 512;
soft_shadow_samples_sqrt = 1;
soft_shadow_jitter = 0.4;
sun_contribution = 1;
sky_contribution = 0;
sky_intensity = 0.2;
global_illumination_cell_size = 5;
water_shadow_bias = 0.8;
water_ambient = 0.3;
water_diffuse = 1.0;
modulate_water_color = 0;
sky_contribution_for_water = 0;
global_illumination_length = 600;
quad_grid_size = 512;
quad_grid_cell_size = 1;
cpu_num = 1;
vegetable_height = 2;
"""


def main() -> int:
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("--welder", required=True, help="path to the built zone_welder executable")
	parser.add_argument("--lighter", required=True, help="path to the built zone_lighter executable")
	parser.add_argument("--input-zonel", required=True, help="a real .zonel/.zone file, grid-named (e.g. 55_CC.zonel)")
	parser.add_argument("--work-dir", required=True, help="scratch directory for inputs/outputs (created if missing)")
	args = parser.parse_args()

	input_zonel = Path(args.input_zonel)
	work_dir = Path(args.work_dir)
	zone_name = input_zonel.stem  # e.g. "55_CC"

	in_dir = work_dir / "in"
	welded_dir = work_dir / "welded"
	lighted_dir = work_dir / "lighted"
	for d in (in_dir, welded_dir, lighted_dir):
		d.mkdir(parents=True, exist_ok=True)

	input_zone = in_dir / f"{zone_name}.zone"
	shutil.copyfile(input_zonel, input_zone)

	print(f"--- Running zone_welder on {input_zone.name} (no neighbors present -- solo pass) ---")
	welded_zone = welded_dir / f"{zone_name}.zonew"
	result = rzt.run_zone_welder(args.welder, input_zone, welded_zone)
	print(f"returncode={result.returncode}")
	print(result.stdout)
	if result.stderr:
		print("stderr:", result.stderr)
	if not welded_zone.exists():
		print(f"FAILED: {welded_zone} was not produced")
		return 1
	try:
		rz.load_zone(welded_zone)
		print(f"OK: {welded_zone.name} parsed with pynel.ryzom_zone.load_zone()")
	except Exception as e:
		print(f"FAILED to parse {welded_zone.name}: {e}")
		return 1

	dependency_file = work_dir / f"{zone_name}.depend"
	dependency_file.write_text("dependencies = {};\n\n", encoding="utf-8")
	parameter_file = work_dir / "properties_test.cfg"
	parameter_file.write_text(PARAMETER_FILE_CFG, encoding="utf-8")

	print(f"\n--- Running zone_lighter on {welded_zone.name} ---")
	lighted_zone = lighted_dir / f"{zone_name}.zonel"
	result = rzt.run_zone_lighter(args.lighter, welded_zone, lighted_zone, parameter_file, dependency_file)
	print(f"returncode={result.returncode}")
	print(result.stdout)
	if result.stderr:
		print("stderr:", result.stderr)
	if not lighted_zone.exists():
		print(f"FAILED: {lighted_zone} was not produced (zone_lighter's exit code is always 0, doesn't reflect this)")
		return 1
	try:
		rz.load_zone(lighted_zone)
		print(f"OK: {lighted_zone.name} parsed with pynel.ryzom_zone.load_zone()")
	except Exception as e:
		print(f"FAILED to parse {lighted_zone.name}: {e}")
		return 1

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
