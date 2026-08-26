#!/usr/bin/env bash
#
# Manual smoke test for the "Auto-export imports/ -> shapes/" chantier's
# Steps 1-4: shape_import.py's centralized IMPORTERS/find_importer(),
# import_watcher.py's sanitize_shape_name()/export_new_shape()/
# update_existing_shape()/_backup_and_reexport(), and ImportWatcher's actual
# filesystem watch on a workspace's imports/ folder. Doesn't cover the
# GUI-only conflict popup (_draw_import_conflict_popup() in object_editor.py)
# -- that needs Patina itself, visual confirmation only.
#
# Usage: ./test.sh (uses the same .venv as dev.sh, creating it first if needed)

set -e
cd "$(dirname "$0")"

VIRTUAL_ENV=~/.cache/venvs/forgery.ryzom.com
PYTHON=$VIRTUAL_ENV/bin/python

if [[ ! -x $PYTHON ]]; then
	echo "venv missing -- run ./dev.sh once first (e.g. ./dev.sh ryzom_forgery/examples/smoke_test.py) to create it"
	exit 1
fi

OUT_DIR=$(mktemp -d)
trap 'rm -rf "$OUT_DIR"' EXIT

echo "== 1/6: shape_importer.py CLI on a real .fbx =="
"$PYTHON" -m ryzom_forgery.apps.shape_importer "examples/marauder_acc_box-flat.fbx" "$OUT_DIR/cli_test.shape"
if [[ -f "$OUT_DIR/cli_test.shape" ]]; then
	echo "PASS: cli_test.shape written"
else
	echo "FAIL: cli_test.shape missing"
	exit 1
fi

echo "== 2/6: shape_importer.py CLI on an unsupported extension =="
if "$PYTHON" -m ryzom_forgery.apps.shape_importer "examples/marauder_acc_boxflat.max" "$OUT_DIR/should_not_exist.shape" 2>"$OUT_DIR/err.txt"; then
	echo "FAIL: expected a non-zero exit for an unsupported format"
	exit 1
fi
if grep -q "Unsupported input format" "$OUT_DIR/err.txt"; then
	echo "PASS: rejected unsupported format"
else
	echo "FAIL: unexpected error message"
	cat "$OUT_DIR/err.txt"
	exit 1
fi

echo "== 3/6: import_watcher.export_new_shape() directly =="
"$PYTHON" <<PYEOF
from pathlib import Path
from ryzom_forgery.import_watcher import export_new_shape, sanitize_shape_name, target_shape_path

sanitized = sanitize_shape_name("Ohne Titel 3")
assert sanitized == "Ohne_Titel_3", f"sanitize_shape_name mismatch: {sanitized!r}"

workspace_dir = Path("$OUT_DIR")
target = target_shape_path(workspace_dir, Path("examples/Ohne Titel 3.fbx"))
assert target == workspace_dir / "shapes" / "Ohne_Titel_3.shape", f"target_shape_path mismatch: {target}"

export_new_shape(Path("examples/Ohne Titel 3.fbx"), target)
assert target.is_file(), f"{target} not written"
print(f"PASS: export_new_shape wrote {target}")
PYEOF

echo "== 4/6: ImportWatcher end-to-end on a real imports/ folder =="
"$PYTHON" <<PYEOF
import shutil
import time
from pathlib import Path
from ryzom_forgery.import_watcher import ImportWatcher
from ryzom_forgery import workspaces

workspace_dir = Path("$OUT_DIR") / "watcher_workspace"
workspaces.ensure_structure(workspace_dir)

watcher = ImportWatcher()
watcher.set_workspace_dir(workspace_dir)

source = workspace_dir / "imports" / "marauder_acc_box-flat.fbx"
shutil.copyfile("examples/marauder_acc_box-flat.fbx", source)

target = workspace_dir / "shapes" / "marauder_acc_box-flat.shape"
deadline = time.monotonic() + 5.0
while time.monotonic() < deadline and not target.is_file():
	time.sleep(0.1)

watcher.set_workspace_dir(None)
assert target.is_file(), f"{target} was not auto-exported within 5s"
print(f"PASS: ImportWatcher auto-exported {target}")
PYEOF

echo "== 5/6: update_existing_shape() preserves material edits, detects mismatch =="
"$PYTHON" <<PYEOF
from pathlib import Path
from pynel.ryzom_shape import parse_shape, save_shape
from ryzom_forgery.import_watcher import MaterialCountMismatch, export_new_shape, update_existing_shape

target = Path("$OUT_DIR") / "update_test.shape"
export_new_shape(Path("examples/marauder_acc_box-flat.fbx"), target)

shape_file = parse_shape(target.read_bytes())
original_material_count = len(shape_file.value.materials)
shape_file.value.materials[0].shininess = 42.0
save_shape(target, shape_file)

# Same source again: geometry re-applied, no texture change, but the
# shininess edit above must survive (only geometry+diffuse-texture are
# touched, per the chantier's Step 3 contract).
update_existing_shape(Path("examples/marauder_acc_box-flat.fbx"), target)
reloaded = parse_shape(target.read_bytes())
assert reloaded.value.materials[0].shininess == 42.0, (
	f"material edit was clobbered: shininess={reloaded.value.materials[0].shininess}")
assert len(reloaded.value.materials) == original_material_count, "material count changed unexpectedly"
print("PASS: update_existing_shape preserved the material edit")

# A source with a different material count (4 vs marauder's own) must raise
# MaterialCountMismatch rather than silently mangling the shape.
try:
	update_existing_shape(Path("examples/Ohne Titel 3.fbx"), target)
except MaterialCountMismatch:
	print("PASS: update_existing_shape raised MaterialCountMismatch as expected")
else:
	raise AssertionError("expected MaterialCountMismatch, none was raised")
PYEOF

echo "== 6/6: ImportWatcher backs up a mismatched target and re-exports under its real name =="
"$PYTHON" <<PYEOF
from pathlib import Path
from ryzom_forgery.import_watcher import ImportWatcher, export_new_shape

target = Path("$OUT_DIR") / "mismatch_test.shape"
export_new_shape(Path("examples/marauder_acc_box-flat.fbx"), target)

watcher = ImportWatcher()
watcher._update_existing_target(Path("examples/Ohne Titel 3.fbx"), target)

backups = sorted(target.parent.glob(f"{target.stem}_backup_*{target.suffix}"))
assert len(backups) == 1, f"expected exactly one backup, found {backups}"
assert target.is_file(), f"{target} missing after re-export"
print(f"PASS: backed up to {backups[0].name}, re-exported {target.name}")
PYEOF

echo "All tests passed."
