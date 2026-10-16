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

# Run a zone_painter invocation: full log to file, key lines echoed, exit code ASSERTED
# (the old tee|grep pipeline swallowed nonzero exits - a crash after the asserted
# lines printed still passed).
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

echo "===== CONT E2E: board open + save (stamp hints) ====="
run_zp "$LOGDIR/m16d_cont_save.log" \
	"startup-auto:|session open:|neighbor|context |weld:|neighbor-hints write|OK save|OK panel" \
	"$WS" --startup-auto "m16d_cont_ws/4_AD" \
	--paint-script "$LOGDIR/m16d_empty.script" \
	--panel-save-test overwrite --out "$OUT/4_AD.max"

echo "===== CONT E2E: dump saved ====="
run_zp "$LOGDIR/m16d_cont_dump.log" "." --dump-neighbor-hints "$OUT/4_AD.max"

echo "===== CONT E2E: copy back + reopen ====="
cp "$OUT/4_AD.max" "$WS/max/zones/4_AD.max"
run_zp "$LOGDIR/m16d_cont_reopen.log" \
	"startup-auto:|session open:|neighbor|context |weld:|eligibility" \
	"$WS" --startup-auto "m16d_cont_ws/4_AD" --dump-zones "$LOGDIR/m16d_cont_reopen_zones"

echo "===== CONT E2E asserts ====="
grep -q "neighbor-hints: source=appdata" "$LOGDIR/m16d_cont_reopen.log"
grep -qE "neighbors: loading [1-9]" "$LOGDIR/m16d_cont_reopen.log"
# Anchor on the session weld summary line specifically (not any future "weld:" line)
WELDS=$(grep -aoE 'weld: [0-9]+ cross-zone edges' "$LOGDIR/m16d_cont_reopen.log" | tail -1 | awk '{print $2}')
test "${WELDS:-0}" -gt 0

echo "PASS m16d continent hint reopen"
echo "--- verbatim key lines ---"
grep -E "startup-auto:|session open:|neighbor-hints:|neighbors:|context '|weld:" "$LOGDIR/m16d_cont_reopen.log"
