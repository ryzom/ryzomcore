"""Ryzom Forgery ai_wmap export: command-line regeneration of a continent's
AI pathfinding walkability maps.

Usage:
	ai_wmap_export.py CONTINENT

Thin CLI wrapper -- the real logic lives in `ryzom_forgery.ai_wmap_export`
(the same library/CLI split as `apps/dds_export.py` and
`ryzom_forgery/dds_export.py`), so any future GUI app can call
`generate_ai_wmap()` directly without going through this script.
"""

import argparse

from pynel import repository_paths

from ryzom_forgery.ai_wmap_export import AiWmapExportError, generate_ai_wmap


def main(argv=None):
	parser = argparse.ArgumentParser(description="Regenerate a continent's ai_wmap.")
	parser.add_argument("continent", help="Continent name (e.g. bagne)")
	args = parser.parse_args(argv)

	ryzom_data_path = repository_paths.get("ryzom-data")
	if ryzom_data_path is None:
		raise SystemExit("Ryzom Data repository path is not configured (Settings > Repository Paths).")

	try:
		result = generate_ai_wmap(ryzom_data_path, args.continent)
	except AiWmapExportError as exc:
		raise SystemExit(str(exc))

	for path in result.wmap_paths:
		print(path)


if __name__ == "__main__":
	main()
