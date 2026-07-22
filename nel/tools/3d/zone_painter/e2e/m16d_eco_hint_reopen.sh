#!/usr/bin/env bash
# M16d eco E2E: place-context -> paint+undo -> save -> copy back -> reopen
# Asserts session-open neighbor-hints source=appdata and context auto-loads at (1,0).
# Outputs under /tmp/zp_ui only. Never writes into graphics repos.
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
WS=/tmp/zp_ui/m16d_eco_ws
OUT=/tmp/zp_ui/m16d_eco_out
LOGDIR=/tmp/zp_ui

rm -rf "$WS" "$OUT"
mkdir -p "$WS/landscape/ligo/lacustre/max" "$OUT"
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
cp "$GFX/landscape/ligo/lacustre/max/material-bassin.max" "$WS/landscape/ligo/lacustre/max/"
# Bank layout: G/landscape/_texture_tiles (NOT landscape/ligo/_texture_tiles)
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"
printf 'tile 0 0 0 0 1\nundo\n' > "$LOGDIR/m16d_paint.script"

echo "===== ECO E2E: place-context + paint + save ====="
"$ZP" "$WS" --startup-auto "lacustre/material-fond" \
	--place-context "1,0:material-bassin" \
	--paint-script "$LOGDIR/m16d_paint.script" \
	--panel-save-test overwrite --out "$OUT/material-fond.max" \
	2>&1 | tee "$LOGDIR/m16d_eco_save.log" | grep -E "startup-auto:|session open:|neighbor|context |weld:|place-context|neighbor-hints write|OK save|OK panel" || true

echo "===== ECO E2E: dump saved ====="
"$ZP" --dump-neighbor-hints "$OUT/material-fond.max" 2>&1 | tee "$LOGDIR/m16d_eco_dump.log"

echo "===== ECO E2E: copy back + reopen ====="
cp "$OUT/material-fond.max" "$WS/landscape/ligo/lacustre/max/material-fond.max"
"$ZP" "$WS" --startup-auto "lacustre/material-fond" --dump-zones "$LOGDIR/m16d_eco_reopen_zones" \
	2>&1 | tee "$LOGDIR/m16d_eco_reopen.log" | grep -E "startup-auto:|session open:|neighbor|context |weld:|eligibility" || true

echo "===== ECO E2E asserts ====="
grep -q "neighbor-hints: source=appdata" "$LOGDIR/m16d_eco_reopen.log"
grep -q "context 'material-bassin' @ cell (1,0)" "$LOGDIR/m16d_eco_reopen.log"
grep -q "neighbors: loading 1 context" "$LOGDIR/m16d_eco_reopen.log"
# Elevated bind count vs solo (solo fond ≈ 80)
BINDS=$(grep -oE 'binds: [0-9]+' "$LOGDIR/m16d_eco_reopen.log" | head -1 | awk '{print $2}')
test "${BINDS:-0}" -gt 80

echo "PASS m16d eco hint reopen"
echo "--- verbatim key lines ---"
grep -E "startup-auto:|session open:|neighbor-hints:|neighbors:|context '|weld:" "$LOGDIR/m16d_eco_reopen.log"
