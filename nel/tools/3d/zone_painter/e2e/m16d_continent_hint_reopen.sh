#!/usr/bin/env bash
# M16d continent E2E: board session with real neighbors -> save stamps appdata hints
# -> copy back -> reopen asserts source=appdata (not grid fallback) and nonzero welds.
# Outputs under /tmp/zp_ui only. Never writes into graphics repos.
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
SB="${SNOWBALLS_SOURCE:-/home/kaetemi/snowballs_source}"
WS=/tmp/zp_ui/m16d_cont_ws
OUT=/tmp/zp_ui/m16d_cont_out
LOGDIR=/tmp/zp_ui

rm -rf "$WS" "$OUT"
mkdir -p "$WS/max/zones" "$OUT"
for z in 3_AC 3_AD 3_AE 4_AC 4_AD 4_AE 5_AC 5_AD 5_AE; do
	if [[ -f "$SB/max/zones/${z}.max" ]]; then
		cp "$SB/max/zones/${z}.max" "$WS/max/zones/"
	fi
done
ln -sfn "$SB/tilebank" "$WS/tilebank"
# Empty paint-script: board session still rewrites to stamp neighbor hints
: > "$LOGDIR/m16d_empty.script"

echo "===== CONT E2E: board open + save (stamp hints) ====="
"$ZP" "$WS" --startup-auto "m16d_cont_ws/4_AD" \
	--paint-script "$LOGDIR/m16d_empty.script" \
	--panel-save-test overwrite --out "$OUT/4_AD.max" \
	2>&1 | tee "$LOGDIR/m16d_cont_save.log" | grep -E "startup-auto:|session open:|neighbor|context |weld:|neighbor-hints write|OK save|OK panel" || true

echo "===== CONT E2E: dump saved ====="
"$ZP" --dump-neighbor-hints "$OUT/4_AD.max" 2>&1 | tee "$LOGDIR/m16d_cont_dump.log"

echo "===== CONT E2E: copy back + reopen ====="
cp "$OUT/4_AD.max" "$WS/max/zones/4_AD.max"
"$ZP" "$WS" --startup-auto "m16d_cont_ws/4_AD" --dump-zones "$LOGDIR/m16d_cont_reopen_zones" \
	2>&1 | tee "$LOGDIR/m16d_cont_reopen.log" | grep -E "startup-auto:|session open:|neighbor|context |weld:|eligibility" || true

echo "===== CONT E2E asserts ====="
grep -q "neighbor-hints: source=appdata" "$LOGDIR/m16d_cont_reopen.log"
grep -qE "neighbors: loading [1-9]" "$LOGDIR/m16d_cont_reopen.log"
WELDS=$(grep -oE 'weld: [0-9]+' "$LOGDIR/m16d_cont_reopen.log" | head -1 | awk '{print $2}')
test "${WELDS:-0}" -gt 0

echo "PASS m16d continent hint reopen"
echo "--- verbatim key lines ---"
grep -E "startup-auto:|session open:|neighbor-hints:|neighbors:|context '|weld:" "$LOGDIR/m16d_cont_reopen.log"
