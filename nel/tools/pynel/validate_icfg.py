#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Opus 5.5 (Anthropic) <noreply@anthropic.com>
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

"""Round-trip validation of pynel.ryzom_icfg against real .icfg files
(project-todos/pynel/icfg_read_write.md) -- NOT part of the pynel package.

For every .icfg in the given folder: parse, re-serialize, then
- version 11 files must be byte-identical to the original;
- older files must re-parse from the re-serialized (upgraded) bytes.

Usage:
	python3 validate_icfg.py <folder>
"""

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from pynel import ryzom_icfg as ri


def _first_diff(a: bytes, b: bytes) -> int:
	for i, (x, y) in enumerate(zip(a, b)):
		if x != y:
			return i
	return min(len(a), len(b))


def _validate(path: Path) -> bool:
	data = path.read_bytes()
	try:
		cfg = ri.parse_icfg(data)
		out = ri.dumps(cfg)
	except ri.IcfgParseError as exc:
		print(f"KO  {path.name}: {exc}")
		return False

	widgets = [w for d in cfg.desktops if d is not None and d.extra is not None for w in d.extra.widgets]
	raw = sum(1 for w in widgets if isinstance(w.config, ri.RawWidgetConfig))
	summary = f"v{cfg.version}, {len(cfg.desktops)} modes, {len(widgets) - raw} widgets decoded, {raw} raw"

	if cfg.version == ri.ICFG_STREAM_VERSION:
		if out != data:
			print(f"KO  {path.name}: {summary}, differs at offset {_first_diff(data, out)} "
				f"(sizes {len(data)} / {len(out)})")
			return False
	else:
		try:
			ri.parse_icfg(out)
		except ri.IcfgParseError as exc:
			print(f"KO  {path.name}: {summary}, upgraded file does not re-parse: {exc}")
			return False

	print(f"OK  {path.name}: {summary}")
	return True


def main() -> int:
	parser = argparse.ArgumentParser(description="Round-trip validation of pynel.ryzom_icfg")
	parser.add_argument("folder", type=Path)
	args = parser.parse_args()

	files = sorted(args.folder.glob("*.icfg"))
	if not files:
		print(f"no .icfg file in {args.folder}")
		return 1
	failed = sum(1 for path in files if not _validate(path))
	print(f"{len(files) - failed}/{len(files)} OK")
	return 1 if failed else 0


if __name__ == "__main__":
	sys.exit(main())
