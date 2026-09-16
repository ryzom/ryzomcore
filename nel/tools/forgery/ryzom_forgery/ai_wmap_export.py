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

"""Drives the native `ai_build_wmap` binary to (re)generate a continent's
`ai_wmap` (the AI pathfinding walkability maps, `<continent>_0/1/2.tga`) --
project-todos/forgery/ai_wmap_flora_collision.md.

`ai_build_wmap`'s `loadPacsPrims()` (continent_container.cpp:322) reads its
`.ig` list from `sheet.LandscapeIG`, a plain manifest ("<continent>_ig.txt",
one `.ig` filename per line) that must live NEXT TO the real `.ig` files
(`zone_lighted_ig_land/`), per
`nel/tools/build_gamedata/processes/ig/2_build.py:251-259`. Without it (and
without that directory in `ai_build_wmap.cfg`'s `Paths=`), `pacsCrunch`
silently loads 0 IGs / adds 0 primitive blocs, no error -- the whole reason
this module writes that manifest itself before every run rather than
assuming some earlier pipeline stage already did.

Not wrapped through `pynel` (unlike the rest of the native pipeline, cf.
`land_build.py`'s use of `pynel.ryzom_land_tools`/`ryzom_pacs_tools`) --
decision Nuno 2026-09-16: `ai_wmap` isn't a `land_build.py` pipeline stage
(yet), so this stays a standalone shared module any future app can import,
with its own local subprocess calls.
"""

import platform
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import List

from . import continent_ecosystem
from . import settings as app_settings

_WMAP_INDICES = (0, 1, 2)


class AiWmapExportError(Exception):
	pass


def _resolve_binary() -> Path:
	settings = app_settings.load()
	if not settings.ryzom_tools_path:
		raise AiWmapExportError("Ryzom tools folder is not configured (Settings > Ryzom Paths).")
	tools_dir = Path(settings.ryzom_tools_path)
	binary_name = "ai_build_wmap.exe" if platform.system() == "Windows" else "ai_build_wmap"
	binary_path = tools_dir / binary_name
	if not binary_path.is_file():
		raise AiWmapExportError(f"{binary_name} not found in {tools_dir}.")
	return binary_path


def _run(binary_path: Path, *args: str, cwd: Path, log_path: Path) -> subprocess.CompletedProcess:
	result = subprocess.run([str(binary_path), *args], capture_output=True, text=True, cwd=str(cwd))
	log_path.write_text(f"{result.stdout}\n{result.stderr}", encoding="utf-8")
	if result.returncode != 0:
		raise AiWmapExportError(f"{args[0]} {' '.join(args[1:])} failed (exit code {result.returncode}):\n{result.stdout}\n{result.stderr}")
	return result


@dataclass
class AiWmapResult:
	wmap_paths: List[Path]
	pacscrunch_log: str


def generate_ai_wmap(ryzom_data_path, continent: str) -> AiWmapResult:
	"""Regenerates `continent`'s `ai_wmap` directory from scratch (previous
	content is wiped first, same as the native pipeline's own unconditional
	regeneration): writes the `<continent>_ig.txt` manifest and a fresh
	`ai_build_wmap.cfg`, then runs `checkPackedSheets` -> `pacsCrunch` ->
	`pacsBuildGabarit` -> `pacsBuildWmap`/`pacsBuildBitmap` (one pair per map
	index in `_WMAP_INDICES`). Requires the continent to already have real
	`zone_lighted_ig_land/` (`land_build.py`'s Stage 7) and `rbank_output/`
	(Stage 8) content -- raises `AiWmapExportError` otherwise, and on any
	tool exit code other than 0."""
	ryzom_data_path = Path(ryzom_data_path)
	binary_path = _resolve_binary()

	ecosystem = continent_ecosystem.get_ecosystem_for_continent(continent)
	if not ecosystem:
		raise AiWmapExportError(
			f"Unknown ecosystem for continent {continent!r} -- check "
			f"leveldesign/workspace/continents/{continent}/directories.py in Ryzom Data."
		)

	pacs_prim_dir = ryzom_data_path / "pipeline" / "export" / "ecosystems" / ecosystem / "pacs_prim"
	sheet_id_dir = ryzom_data_path / "final_bnps" / "leveldesign"
	continents_dir = ryzom_data_path / "leveldesign" / "world" / "continents"
	dfn_dir = ryzom_data_path / "leveldesign" / "DFN"
	continent_dir = ryzom_data_path / "pipeline" / "export" / "continents" / continent
	rbank_output_dir = continent_dir / "rbank_output"
	ig_land_dir = continent_dir / "zone_lighted_ig_land"
	ai_wmap_dir = continent_dir / "ai_wmap"

	if not ig_land_dir.is_dir():
		raise AiWmapExportError(f"{ig_land_dir} does not exist -- build the continent first (zone_ig_lighter output).")
	if not rbank_output_dir.is_dir():
		raise AiWmapExportError(f"{rbank_output_dir} does not exist -- build the continent's PACS collision first (build_rbank output).")

	shutil.rmtree(ai_wmap_dir, ignore_errors=True)
	ai_wmap_dir.mkdir(parents=True)

	pacs_prim_manifest = ai_wmap_dir / "pacs_prim_manifest.txt"
	pacs_prim_names = sorted(p.name for p in pacs_prim_dir.glob("*.pacs_prim")) if pacs_prim_dir.is_dir() else []
	pacs_prim_manifest.write_text("\n".join(str(pacs_prim_dir / name) for name in pacs_prim_names), encoding="utf-8")

	ig_manifest = ig_land_dir / f"{continent}_ig.txt"
	ig_names = sorted(p.name for p in ig_land_dir.glob("*.ig"))
	ig_manifest.write_text("\n".join(ig_names), encoding="utf-8")

	cfg_path = rbank_output_dir / "ai_build_wmap.cfg"
	paths = [rbank_output_dir, sheet_id_dir, continents_dir, dfn_dir, ig_land_dir]
	cfg_path.write_text(
		"Paths = { " + ", ".join(f'"{p}/"' for p in paths) + " };\n"
		"NoRecursePaths = { };\n"
		f'PacsPrimPaths = {{ "{pacs_prim_manifest}" }};\n'
		f'OutputPath = "{ai_wmap_dir}/";\n',
		encoding="utf-8",
	)

	try:
		_run(binary_path, "checkPackedSheets", cwd=rbank_output_dir, log_path=ai_wmap_dir / "checkpackedsheets.log")
		pacscrunch_result = _run(binary_path, "pacsCrunch", continent, cwd=rbank_output_dir, log_path=ai_wmap_dir / "pacscrunch.log")
		_run(binary_path, "pacsBuildGabarit", continent, cwd=rbank_output_dir, log_path=ai_wmap_dir / "gabarit.log")

		wmap_paths = []
		for i in _WMAP_INDICES:
			map_name = f"{continent}_{i}"
			_run(binary_path, "pacsBuildWmap", map_name, cwd=rbank_output_dir, log_path=ai_wmap_dir / f"wmap_{i}.log")
			_run(binary_path, "pacsBuildBitmap", map_name, cwd=rbank_output_dir, log_path=ai_wmap_dir / f"bitmap_{i}.log")
			tga_path = ai_wmap_dir / f"{map_name}.tga"
			if not tga_path.is_file():
				raise AiWmapExportError(f"pacsBuildBitmap did not produce {tga_path}")
			wmap_paths.append(tga_path)
	finally:
		cfg_path.unlink(missing_ok=True)
		ig_manifest.unlink(missing_ok=True)
		pacs_prim_manifest.unlink(missing_ok=True)

	return AiWmapResult(wmap_paths=wmap_paths, pacscrunch_log=pacscrunch_result.stdout)
