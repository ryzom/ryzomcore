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

"""Download and install the `build_gamedata` pipeline data Nuno publishes as
`.zip` archives -- too large to version in `ryzom-data` itself -- into the
right place under `<ryzom-data>/pipeline/`.

Three categories, each with its own base URL and target directory under
`<ryzom-data>/pipeline/` (project-todos/forgery/landscape_editor__
zone_render_modes__pipeline_data_installer.md):

	landscape           https://download.ryzom.com/tools/landscape/<name>.zip
	                    -> pipeline/landscape/<name>/
	                    <name> = ecosystem name (desert/jungle/lacustre/primes_racines)

	pipeline_ecosystems  https://download.ryzom.com/tools/pipeline/<name>.zip
	                    -> pipeline/export/ecosystems/<name>/
	                    <name> = ecosystem name (same names, different content)

	pipeline_continents  https://download.ryzom.com/tools/pipeline/<name>.zip
	                    -> pipeline/export/continents/<name>/
	                    <name> = continent name (e.g. "nexus")

Each archive already has a top-level `<name>/` folder and content already
trimmed to what Atyscape needs -- Nuno builds these archives himself, so
this module never filters/sorts their contents. Only `pipeline_continents`
is wired to an automatic trigger so far (see `continent_ecosystem.py` and
`landscape_editor.py`); `landscape`/`pipeline_ecosystems` are installable
through this same module but have no consumer yet.

No new dependency: `urllib.request` (stdlib, streaming) for download,
`zipfile` (stdlib) for extraction -- Nuno produces `.zip` rather than `.7z`
specifically to avoid needing an external extraction dependency.

Usage:
	from ryzom_forgery import pipeline_data_installer as pdi

	if not pdi.is_installed("pipeline_continents", "nexus"):
		progress = {"phase": "downloading", "downloaded_bytes": 0, "total_bytes": None, "error": None, "done": False}
		pdi.download_and_install("pipeline_continents", "nexus", progress)
"""

import shutil
import tempfile
import urllib.error
import urllib.request
import zipfile
from pathlib import Path
from typing import Dict, Optional

from pynel import repository_paths


class PipelineDataInstallError(Exception):
	pass


class _Category:
	def __init__(self, base_url: str, target_subpath: str):
		self.base_url = base_url
		self.target_subpath = target_subpath


CATEGORIES: Dict[str, _Category] = {
	"landscape": _Category("https://download.ryzom.com/tools/landscape/", "pipeline/landscape"),
	"pipeline_ecosystems": _Category("https://download.ryzom.com/tools/pipeline/", "pipeline/export/ecosystems"),
	"pipeline_continents": _Category("https://download.ryzom.com/tools/pipeline/", "pipeline/export/continents"),
}


def _ryzom_data_path() -> Path:
	path = repository_paths.get("ryzom-data")
	if path is None or not Path(path).is_dir():
		raise PipelineDataInstallError("ryzom-data path is not configured (pynel.repository_paths)")
	return Path(path)


def _target_dir(category: str, name: str) -> Path:
	if category not in CATEGORIES:
		raise PipelineDataInstallError(f"unknown pipeline data category {category!r}")
	return _ryzom_data_path() / CATEGORIES[category].target_subpath / name


def is_installed(category: str, name: str) -> bool:
	"""True if `<target>/<name>/` exists and has at least one entry."""
	target = _target_dir(category, name)
	return target.is_dir() and any(target.iterdir())


def download_and_install(category: str, name: str, progress: Dict) -> None:
	"""Downloads `<base_url><name>.zip`, streaming into a temp file while
	updating `progress["phase"]` (`"downloading"`/`"extracting"`) and, while
	downloading, `progress["downloaded_bytes"]`/`progress["total_bytes"]`
	(`total_bytes` stays `None` if the server doesn't send a
	`Content-Length`, for an indeterminate progress bar), then extracts it
	with `zipfile` into the category's target directory, then deletes the
	temp file. Raises `PipelineDataInstallError` on failure (network error,
	404, invalid zip, `ryzom-data` not configured) -- does *not* catch
	exceptions into `progress` itself; a caller driving this from a
	background thread (see `pipeline_data_install_dialog.py`) is
	responsible for catching and recording `progress["error"]`/setting
	`progress["done"]`, the same pattern already used by
	`landscape_editor.py`'s `_run_load_continent`/`_run_load_refs`."""
	target = _target_dir(category, name)
	url = CATEGORIES[category].base_url + name + ".zip"

	progress["phase"] = "downloading"
	progress["downloaded_bytes"] = 0
	progress["total_bytes"] = None

	tmp_fd, tmp_path_str = tempfile.mkstemp(suffix=".zip")
	tmp_path = Path(tmp_path_str)
	try:
		try:
			with urllib.request.urlopen(url) as response:
				content_length = response.headers.get("Content-Length")
				if content_length is not None:
					progress["total_bytes"] = int(content_length)
				with open(tmp_fd, "wb") as tmp_file:
					while True:
						chunk = response.read(1024 * 1024)
						if not chunk:
							break
						tmp_file.write(chunk)
						progress["downloaded_bytes"] += len(chunk)
		except urllib.error.URLError as exc:
			raise PipelineDataInstallError(f"failed to download {url}: {exc}") from exc

		progress["phase"] = "extracting"
		# The zip's own top-level entry is `<name>/...` (see module
		# docstring), so extracting into target.parent produces target
		# itself -- target.mkdir would be wrong here (extractall creates it).
		target.parent.mkdir(parents=True, exist_ok=True)
		try:
			with zipfile.ZipFile(tmp_path) as zf:
				zf.extractall(target.parent)
		except zipfile.BadZipFile as exc:
			raise PipelineDataInstallError(f"{url} is not a valid zip file: {exc}") from exc
	finally:
		tmp_path.unlink(missing_ok=True)
