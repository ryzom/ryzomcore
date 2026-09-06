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

"""Drive the native `zone_welder`/`zone_lighter` tools (nel/tools/3d/
zone_welder/, nel/tools/3d/zone_lighter/) as subprocesses.

Thin wrappers only -- no automatic binary discovery (the caller passes an
explicit path to each built binary) and no automatic parameter/dependency
file generation (the caller supplies real `.cfg` files, following the
`zone_lighter_properties.cfg`/`<name>.depend` examples in docs/zone_tools.md).

Unlike `land_export` (see ryzom_land_tools.py), neither tool has a
meaningful, checkable exit code:
  - `zone_welder` exits 0 both for a real run and for the usage-only path
    (`argc < 3` or `argv[1] == "/?"`) -- a bind conflict between adjacent
    zones is reported only via `nlwarning`, with nothing written, still
    under exit code 0.
  - `zone_lighter` **always** exits 0, even on internal failure (missing
    input, parse exceptions -- all caught and logged, never propagated as
    a process exit code).

So these wrappers never raise on the subprocess result: they just run it
and return the `CompletedProcess` (stdout/stderr captured as text) for the
caller to log. The caller is responsible for validating the actual outcome
-- e.g. via `pynel.ryzom_zone.load_zone()` on the expected output file(s).

`zone_welder` may write more files than its literal `output_path` argument:
on success it rewrites every modified adjacent zone too, as
`<dir of output_path>/<neighbor name><ext of output_path>` -- the neighbors
must already exist in that directory for welding to do anything (see
docs/zone_tools.md §1).

Usage:
	from pynel import ryzom_zone_tools as rzt
	rzt.run_zone_welder("/path/to/zone_welder", "zone_exported/12_ab.zone", "zone_welded/12_ab.zonew")
	rzt.run_zone_lighter(
		"/path/to/zone_lighter", "zone_welded/12_ab.zonew", "zone_lighted/12_ab.zonel",
		"zone_lighter_properties.cfg", "zone_depend/12_ab.depend",
	)
"""

import subprocess
from pathlib import Path
from typing import Union


def run_zone_welder(
	binary_path: Union[str, Path],
	input_path: Union[str, Path],
	output_path: Union[str, Path],
	weld_threshold: float = 1.1,
) -> subprocess.CompletedProcess:
	"""Runs `zone_welder <input_path> <output_path> <weld_threshold>`.
	Neighbor zones for `input_path` (see getAdjacentZonesName() naming
	convention, docs/zone_tools.md §1) are loaded from -- and, if modified,
	rewritten to -- the directory of `output_path`, not `input_path`."""
	return subprocess.run(
		[str(binary_path), str(input_path), str(output_path), str(weld_threshold)],
		capture_output=True,
		text=True,
	)


def run_zone_lighter(
	binary_path: Union[str, Path],
	input_path: Union[str, Path],
	output_path: Union[str, Path],
	parameter_file: Union[str, Path],
	dependency_file: Union[str, Path],
) -> subprocess.CompletedProcess:
	"""Runs `zone_lighter <input_path> <output_path> <parameter_file>
	<dependency_file>`. `dependency_file` names are resolved against the
	directory/extension of `input_path` (docs/zone_tools.md §2) -- an empty
	`dependencies = {};` file is valid when no cross-zone shadow analysis is
	needed. The `-waterpatch` mode is out of scope, not exposed here."""
	return subprocess.run(
		[str(binary_path), str(input_path), str(output_path), str(parameter_file), str(dependency_file)],
		capture_output=True,
		text=True,
	)
