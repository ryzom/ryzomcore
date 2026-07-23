#!/usr/bin/env bash
# M24e review-fix E2E: placement/board defects found in the M24 review pass.
#  1. --place-context / --open-editable are genuinely repeatable (CCmdArgs onlyOnce trap
#     silently kept only the LAST occurrence).
#  2. Copy-drag of a multi-cell HOME grabbed at an interior cell shifts the block origin
#     by the drag delta (was: raw drop cell).
#  3. Moving an open file onto an instance SOURCED from another open brick is refused
#     (was: collision used the home mask for every instance).
#  4. A --place naming the PRIMARY brick is a home instance (was: skipped every rebuild
#     while still claiming board cells).
#  5. Per-file neighbor hints of a NON-primary carrier place context at the rebased
#     board cell (was: raw file-relative offset, landing on the carrier).
#  6. Closing an eco editable demotes it to a board-managed context SPEC (was: invisible
#     resurrect through the hint chain).
# Outputs under /tmp/zp_ui only. Needs a display (xvfb-run) for the drag hooks.
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
WS=/tmp/zp_ui/m24e_ws
OUT=/tmp/zp_ui/m24e_out
LOGDIR=/tmp/zp_ui
XVFB="xvfb-run -a"

rm -rf "$WS" "$OUT"
mkdir -p "$WS/landscape/ligo/lacustre/max" "$OUT"
for f in material-fond material-peek material-bassin zonematerial-bassin-ilot_croix; do
	cp "$GFX/landscape/ligo/lacustre/max/$f.max" "$WS/landscape/ligo/lacustre/max/"
done
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"
: > "$LOGDIR/m24e_empty.script"

# Full log to file, key lines echoed, exit code ASSERTED (the old tee|grep pipeline
# swallowed nonzero exits — a crash after the asserted lines printed still passed).
# Extra env for the drag/board hooks rides via ZP_ENV (name=value pairs).
run_zp() { # $1 = log file, $2 = display grep pattern, rest = args
	local log="$1" pat="$2"
	shift 2
	if ! env $ZP_ENV $ZP_WRAP "$ZP" "$@" > "$log" 2>&1; then
		echo "FAIL: zone_painter exited nonzero (log: $log)"
		tail -n 20 "$log"
		exit 1
	fi
	grep -aE "$pat" "$log" || true
}
ZP_ENV=""
ZP_WRAP=""


echo "===== M24e-1/2: repeatable specs + home copy-drag origin (4x4 ilot_croix) ====="
ZP_ENV='ZONE_PAINTER_BOARD_DRAG=1,1:1,6:copy' ZP_WRAP="$XVFB" \
run_zp "$LOGDIR/m24e_drag_home.log" "board-drag|place\[" "$WS" \
	--startup-auto "lacustre/zonematerial-bassin-ilot_croix" \
	--screenshot "$OUT/drag_home.tga"
# Copy of the 4x4 first-opened file grabbed at (1,1), dropped at (1,6): origin =
# file(0,0)+delta(0,5). M28: sources always print their basename (uniform per-file).
grep -aq "place\[0\].* origin cell (0,5)" "$LOGDIR/m24e_drag_home.log"
grep -aq "board-drag test (1,1)->(1,6) copy: OK" "$LOGDIR/m24e_drag_home.log"

echo "===== M24e-3/4: sourced-instance collision + place-names-primary ====="
ZP_ENV='ZONE_PAINTER_BOARD_DRAG=0,3:3,6' ZP_WRAP="$XVFB" \
run_zp "$LOGDIR/m24e_drag_src.log" "board-drag|open-editable:|place\[" \
	"$WS" --startup-auto "lacustre/material-fond" \
	--open-editable "2,0:zonematerial-bassin-ilot_croix" \
	--place "2,5:zonematerial-bassin-ilot_croix" \
	--open-editable "0,3:material-peek" \
	--screenshot "$OUT/drag_src.tga"
# Both --open-editable specs honored (repeatable), sourced place appended, move refused
grep -aq "open-editable: 'zonematerial-bassin-ilot_croix' @ (2,0)" "$LOGDIR/m24e_drag_src.log"
grep -aq "open-editable: 'material-peek' @ (0,3)" "$LOGDIR/m24e_drag_src.log"
grep -aq "place\[0\] 'zonematerial-bassin-ilot_croix' origin cell (2,5)" "$LOGDIR/m24e_drag_src.log"
grep -aq "board-drag test (0,3)->(3,6): footprint overlaps an instance" "$LOGDIR/m24e_drag_src.log"
# --place naming the PRIMARY renders as a home instance (display zones +N, not skipped)
run_zp "$LOGDIR/m24e_prim_place.log" "place\[|instances:" \
	"$WS" --startup-auto "lacustre/zonematerial-bassin-ilot_croix" \
	--place "0,5:zonematerial-bassin-ilot_croix" \
	--dump-zones "$LOGDIR/m24e_prim_zones"
# Anchored: "+1 (ids from" — a bare "+1" also matched "+10".."+19"
grep -aq "display zones +1 (ids from" "$LOGDIR/m24e_prim_place.log"
! grep -aq "not open — place skipped" "$LOGDIR/m24e_prim_place.log"

echo "===== M24e-5: non-primary hint carrier rebases context placement ====="
run_zp "$LOGDIR/m24e_peek_save.log" "neighbor-hints write|OK save" \
	"$WS" --startup-auto "lacustre/material-peek" --place-context "1,0:material-bassin" \
	--paint-script "$LOGDIR/m24e_empty.script" \
	--panel-save-test overwrite --out "$OUT/material-peek.max"
cp "$OUT/material-peek.max" "$WS/landscape/ligo/lacustre/max/material-peek.max"
run_zp "$LOGDIR/m24e_hint.log" "open-editable:|context '" \
	"$WS" --startup-auto "lacustre/material-fond" --open-editable "2,0:material-peek" \
	--dump-zones "$LOGDIR/m24e_hint_zones"
# peek carries "1,0:material-bassin"; opened at (2,0) → bassin at board (3,0), not (1,0)
grep -aq "context 'material-bassin' @ cell (3,0)" "$LOGDIR/m24e_hint.log"

echo "===== M24e-6: eco close demotes to a context SPEC ====="
ZP_ENV='ZONE_PAINTER_BOARD_ACTION=close:material-peek' ZP_WRAP="$XVFB" \
run_zp "$LOGDIR/m24e_close.log" "session close|place-context: 'material-peek'|board-action" \
	"$WS" --startup-auto "lacustre/material-fond" --open-editable "2,0:material-peek" \
	--screenshot "$OUT/close_spec.tga"
grep -aq "session close: 'material-peek'" "$LOGDIR/m24e_close.log"
grep -aq "place-context: 'material-peek' @ (2,0)" "$LOGDIR/m24e_close.log"

echo "PASS m24e review fixes"
