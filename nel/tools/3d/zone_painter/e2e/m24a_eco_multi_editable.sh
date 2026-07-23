#!/usr/bin/env bash
# M24a eco multi-editable E2E: --open-editable places a second EDITABLE brick on the
# board; per-file relative neighbor hints stamp on save-all; paint+undo on BOTH files
# saves byte-identical to the empty edit; reopening either brick auto-loads the other
# as read-only context from its hint. Outputs under /tmp/zp_ui only.
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
WS=/tmp/zp_ui/m24a_ws
OUT=/tmp/zp_ui/m24a_out
LOGDIR=/tmp/zp_ui

rm -rf "$WS" "$OUT"
mkdir -p "$WS/landscape/ligo/lacustre/max" "$OUT"
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
cp "$GFX/landscape/ligo/lacustre/max/material-peek.max" "$WS/landscape/ligo/lacustre/max/"
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"


# Full log to file, key lines echoed, exit code ASSERTED (the old tee|grep pipeline
# swallowed nonzero exits).
run_zp() { # $1 = log file, $2 = display grep pattern, rest = args
	local log="$1" pat="$2"
	shift 2
	if ! "$ZP" "$@" > "$log" 2>&1; then
		echo "FAIL: zone_painter exited nonzero (log: $log)"
		tail -n 20 "$log"
		exit 1
	fi
	grep -aE "$pat" "$log" || true
}

echo "===== M24a: placement + empty-edit save-all (stamps per-file hints) ====="
: > "$LOGDIR/m24a_empty.script"
run_zp "$LOGDIR/m24a_save.log" \
	"open-editable:|neighbor-hints write|OK save-all|OK panel" \
	"$WS" --startup-auto "lacustre/material-fond" --open-editable "1,0:material-peek" \
	--paint-script "$LOGDIR/m24a_empty.script" \
	--panel-save-test overwrite --out "$OUT/unused.max"
grep -q "open-editable: 'material-peek' @ (1,0)" "$LOGDIR/m24a_save.log"
grep -q "neighbor-hints write 'material-fond': v1|1,0:material-peek" "$LOGDIR/m24a_save.log"
grep -q "neighbor-hints write 'material-peek': v1|-1,0:material-fond" "$LOGDIR/m24a_save.log"
grep -q "OK save-all: 2 file(s)" "$LOGDIR/m24a_save.log"
cp "$WS/landscape/ligo/lacustre/max/material-fond.max" "$OUT/fond_empty.max"
cp "$WS/landscape/ligo/lacustre/max/material-peek.max" "$OUT/peek_empty.max"

echo "===== M24a: paint+undo on BOTH files == empty edit (byte gate) ====="
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
cp "$GFX/landscape/ligo/lacustre/max/material-peek.max" "$WS/landscape/ligo/lacustre/max/"
printf 'tile 0 0 1 1 1\ntile 1000 0 1 1 1\ndisplace 1000 1 2 2 3\nundo\nundo\nundo\n' \
	> "$LOGDIR/m24a_pu.script"
"$ZP" "$WS" --startup-auto "lacustre/material-fond" --open-editable "1,0:material-peek" \
	--paint-script "$LOGDIR/m24a_pu.script" \
	--panel-save-test overwrite --out "$OUT/unused.max" > "$LOGDIR/m24a_pu.log" 2>&1
cmp "$OUT/fond_empty.max" "$WS/landscape/ligo/lacustre/max/material-fond.max"
cmp "$OUT/peek_empty.max" "$WS/landscape/ligo/lacustre/max/material-peek.max"

echo "===== M24a: reopen second brick — hint auto-loads the first as RO ====="
run_zp "$LOGDIR/m24a_reopen.log" "neighbor-hints:|context '|^zone" \
	"$WS" --startup-auto "lacustre/material-peek" --dump-zones "$OUT/reopen_zones"
grep -q "neighbor-hints: source=appdata raw=1 resolved=1" "$LOGDIR/m24a_reopen.log"
grep -q "context 'material-fond' @ cell (-1,0)" "$LOGDIR/m24a_reopen.log"
grep -qE "zone 2000 'QuadPatch01' FROZEN: .* bbox \(-160.0,0.0\)-\(0.0,160.0\)" "$LOGDIR/m24a_reopen.log"

echo "PASS m24a eco multi-editable"
