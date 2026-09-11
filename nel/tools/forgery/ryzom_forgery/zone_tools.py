"""Drives the native `zone_welder` binary (project-todos/forgery/
landscape_editor__zone_render_modes.md step 3) to produce a real `.zonew`
for a zone on demand, from Atyscape's [WELD] render mode.

Never a Python port of the welding algorithm itself -- the native tool is
the real production implementation, and Python would be far too slow for
this (decision Nuno 2026-09-08, see the chantier's own summary). Only a
thin orchestration layer around `pynel.ryzom_zone_tools.run_zone_welder()`:
resolves the binary, gathers the target zone + its already-welded
neighbors into a scratch directory, runs the tool, re-reads the result.

Deliberately does NOT recurse into welding missing neighbors first -- on a
brand new continent every zone is still raw `.zone`, so welding one zone's
neighbors on demand would cascade into welding the whole continent from a
single click. A neighbor that has no `.zonew`/`.zonel` yet is simply left
out of the weld call (that border stays unwelded until either that
neighbor gets its own turn, or the batch "Generate missing .zonew" pass of
step 6 processes the whole set in one go, benefiting from each zone
already written as it goes -- same single-pass behavior as the real
`build_gamedata` pipeline, docs/zone_tools.md).
"""

import platform
import tempfile
from pathlib import Path

from pynel import ryzom_zone_tools
from pynel.ryzom_zone import parse_zone, Zone, ZoneParseError

from . import settings as app_settings
from .region_loader import (
	find_zones_in_region, read_zone_ref_bytes, zone_ref_extension, RegionLoadError, ZoneRef,
)
from .zone_geometry import ZONE_CELL_SIZE

_WELDED_EXTENSIONS = (".zonew", ".zonel")


class ZoneToolError(Exception):
	pass


def _resolve_zone_welder_binary() -> Path:
	settings = app_settings.load()
	if not settings.ryzom_tools_path:
		raise ZoneToolError("Ryzom tools folder is not configured (Settings > Ryzom Paths).")
	tools_dir = Path(settings.ryzom_tools_path)
	binary_name = "zone_welder.exe" if platform.system() == "Windows" else "zone_welder"
	binary_path = tools_dir / binary_name
	if not binary_path.is_file():
		raise ZoneToolError(f"{binary_name} not found in {tools_dir}.")
	return binary_path


def missing_zonew_dest(ryzom_data_path, pipeline_continent_name: str, zone_name: str) -> Path:
	"""Path a generated `.zonew` must be written to for `zone_name`
	(project-todos/forgery/landscape_editor__zone_render_modes.md step 9) --
	always `<ryzom-data>/pipeline/export/continents/<continent>/zone_weld/
	<zone_name>.zonew`, the same pipeline export root already used for
	reading (step 6), never a second `ryzom_tools_path`-style setting.
	Raises ZoneToolError if `ryzom_data_path` is falsy (not configured)."""
	if not ryzom_data_path:
		raise ZoneToolError("Ryzom data path is not configured (Settings > Ryzom Paths).")
	return Path(ryzom_data_path) / "pipeline" / "export" / "continents" / pipeline_continent_name / "zone_weld" / f"{zone_name}.zonew"


def run_zone_welder(ref: ZoneRef, live_data_path, persist_to: Path = None) -> Zone:
	"""Produces (or re-derives, cache-free) the `.zonew` for the single zone
	`ref` points to, by calling the native `zone_welder` on it together with
	whichever of its 8 grid neighbors already have a real `.zonew`/`.zonel`
	on disk (see module docstring for why neighbors lacking one are simply
	skipped rather than welded first). `persist_to`, when given (step 9's
	"Generate missing .zonew" button, see `missing_zonew_dest()`), writes the
	produced `.zonew` there (creating parent directories as needed) before
	parsing it -- the on-disk file becomes the persistent result, the
	returned `Zone` just lets the caller refresh its own display without a
	second disk read."""
	binary_path = _resolve_zone_welder_binary()

	try:
		region = find_zones_in_region(
			live_data_path,
			ref.x - ZONE_CELL_SIZE, ref.y - ZONE_CELL_SIZE,
			ref.x + 2 * ZONE_CELL_SIZE, ref.y + 2 * ZONE_CELL_SIZE,
		)
	except RegionLoadError as exc:
		raise ZoneToolError(f"{ref.name}: {exc}")

	welded_neighbors = [
		neighbor for neighbor in region
		if neighbor.name != ref.name and zone_ref_extension(neighbor) in _WELDED_EXTENSIONS
	]
	print(f"(IA_AGENT_DEBUG) (zone_tools) (zone_tools.py:73) binary={binary_path} target={ref.name} welded_neighbors={[n.name for n in welded_neighbors]}")

	with tempfile.TemporaryDirectory(prefix="forgery_zone_welder_") as tmp:
		tmp_dir = Path(tmp)
		input_path = tmp_dir / f"{ref.name}.zone"
		output_path = tmp_dir / f"{ref.name}.zonew"
		try:
			input_path.write_bytes(read_zone_ref_bytes(ref))
			for neighbor in welded_neighbors:
				(tmp_dir / f"{neighbor.name}.zonew").write_bytes(read_zone_ref_bytes(neighbor))
		except RegionLoadError as exc:
			raise ZoneToolError(str(exc))

		result = ryzom_zone_tools.run_zone_welder(binary_path, input_path, output_path)
		print(f"(IA_AGENT_DEBUG) (zone_tools) (zone_tools.py:87) returncode={result.returncode} stdout={result.stdout!r} stderr={result.stderr!r} output_exists={output_path.is_file()}")
		if not output_path.is_file():
			details = (result.stderr or result.stdout or "no output produced").strip()
			raise ZoneToolError(f"zone_welder failed to produce {ref.name}.zonew: {details}")

		output_bytes = output_path.read_bytes()
		if persist_to is not None:
			persist_to.parent.mkdir(parents=True, exist_ok=True)
			persist_to.write_bytes(output_bytes)

		try:
			return parse_zone(output_bytes)
		except ZoneParseError as exc:
			raise ZoneToolError(f"{ref.name}: produced .zonew failed to parse: {exc}")
