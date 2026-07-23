#!/usr/bin/env bash
# M31: byte-equivalence gates anchored on the SESSION path (the actual application),
# not the legacy CLI. The codec corpus gates (--rpo-selftest / ctest #25) stay on the
# minimal legacy chain by design — these gates prove the SAME guarantee holds through
# session assembly, board translation, and the interactive save flow.
#  1. Workspace-session no-op save (--no-hint-stamp --no-thumbnail) is BYTE-IDENTICAL
#     to the legacy null-edit output of the same file (cross-path identity).
#  2. Session paint + undo + save-all == the same null-edit output (paint core through
#     the session is byte-clean).
#  3. Synthetic session (M31b: interactive direct-.max open == single-file session,
#     hint stamping off by construction) no-op save == the same null-edit output.
#  4. Eco board ops are scriptable end-to-end (openZone(cx,cy)/placeInstance/rotate/
#     mirror/remove/closeZone), and painting THROUGH a scripted instance == painting
#     the source directly (shared-carrier guarantee via the script surface).
# Outputs under /tmp/zp_ui only. Needs a display (xvfb-run).
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
B=zonematerial-bassin-ilot_croix
SRC="$GFX/landscape/ligo/lacustre/max/$B.max"
BANK="$GFX/landscape/_texture_tiles/lacustre/lacustre.bank"
OUT=/tmp/zp_ui/m31
XVFB="xvfb-run -a"

rm -rf "$OUT"
mkdir -p "$OUT"

seed_ws() { # $1 = ws dir
	rm -rf "$1"
	mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$SRC" "$1/landscape/ligo/lacustre/max/"
	cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

echo "===== M31-0: legacy null-edit baseline ====="
"$ZP" "$SRC" --null-edit --out "$OUT/null.max" > "$OUT/null.log" 2>&1
REF=$(md5sum "$OUT/null.max" | cut -d' ' -f1)
echo "baseline md5 $REF"

echo "===== M31-1: workspace session no-op save == null-edit output ====="
seed_ws "$OUT/ws1"
ZONE_PAINTER_BOARD_ACTION="save:$B" $XVFB "$ZP" "$OUT/ws1" \
	--startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--screenshot "$OUT/s1.tga" > "$OUT/g1.log" 2>&1
GOT=$(md5sum "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
[[ "$GOT" == "$REF" ]] || { echo "FAIL: session no-op save != null-edit ($GOT)"; exit 1; }
echo "OK cross-path identity"

echo "===== M31-2: session paint + undo + save-all == null-edit output ====="
seed_ws "$OUT/ws2"
cat > "$OUT/paint_undo.lua" <<'EOF'
assert(painter.paintTile(0, 0, 0.5, 0.5, 1))
assert(painter.undo())
assert(painter.saveAll())
print("paint+undo+save OK")
EOF
$XVFB "$ZP" "$OUT/ws2" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--lua-script "$OUT/paint_undo.lua" --screenshot "$OUT/s2.tga" > "$OUT/g2.log" 2>&1
grep -aq "paint+undo+save OK" "$OUT/g2.log" || { echo "FAIL: paint+undo script did not complete"; exit 1; }
GOT=$(md5sum "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
[[ "$GOT" == "$REF" ]] || { echo "FAIL: session paint+undo save != null-edit ($GOT)"; exit 1; }
echo "OK session paint+undo byte-clean"

echo "===== M31-3: synthetic session (direct .max) no-op save == null-edit output ====="
seed_ws "$OUT/ws3"
ZONE_PAINTER_BOARD_ACTION="save:$B" $XVFB "$ZP" \
	"$OUT/ws3/landscape/ligo/lacustre/max/$B.max" --bank "$BANK" --no-thumbnail \
	--screenshot "$OUT/s3.tga" > "$OUT/g3.log" 2>&1
grep -aq "synthesized session" "$OUT/g3.log" || { echo "FAIL: direct open did not synthesize a session"; exit 1; }
GOT=$(md5sum "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
[[ "$GOT" == "$REF" ]] || { echo "FAIL: synthetic no-op save != null-edit ($GOT)"; exit 1; }
echo "OK synthetic session identity (stamp off by construction)"

echo "===== M31-4: scripted eco board ops + paint-through-instance == paint-source ====="
seed_ws "$OUT/ws4a"
seed_ws "$OUT/ws4b"
cat > "$OUT/ops_smoke.lua" <<'EOF'
assert(painter.placeInstance(8, 0))
assert(painter.rotateInstance(8, 0))
assert(painter.mirrorInstance(8, 0))
assert(painter.removeInstance(8, 0))
assert(painter.openZone("material-fond", -2, 0))
assert(painter.closeZone("material-fond", false, true))
print("ops smoke OK")
EOF
$XVFB "$ZP" "$OUT/ws4a" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--lua-script "$OUT/ops_smoke.lua" --screenshot "$OUT/s4a.tga" > "$OUT/g4a.log" 2>&1
grep -aq "ops smoke OK" "$OUT/g4a.log" || { echo "FAIL: ops smoke"; exit 1; }
# paint THROUGH a scripted instance: interior tile of the clone (zone >= 10000)
cat > "$OUT/via_inst.lua" <<'EOF'
assert(painter.placeInstance(8, 0))
assert(painter.paintTile(10000, 0, 0.5, 0.5, 1))
assert(painter.saveAll())
print("via-instance OK")
EOF
cat > "$OUT/via_src.lua" <<'EOF'
assert(painter.paintTile(0, 0, 0.5, 0.5, 1))
assert(painter.saveAll())
print("via-source OK")
EOF
$XVFB "$ZP" "$OUT/ws4a" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--lua-script "$OUT/via_inst.lua" --screenshot "$OUT/s4b.tga" > "$OUT/g4b.log" 2>&1
grep -aq "via-instance OK" "$OUT/g4b.log" || { echo "FAIL: via-instance script"; exit 1; }
$XVFB "$ZP" "$OUT/ws4b" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--lua-script "$OUT/via_src.lua" --screenshot "$OUT/s4c.tga" > "$OUT/g4c.log" 2>&1
grep -aq "via-source OK" "$OUT/g4c.log" || { echo "FAIL: via-source script"; exit 1; }
A=$(md5sum "$OUT/ws4a/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
Bmd=$(md5sum "$OUT/ws4b/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
[[ "$A" == "$Bmd" ]] || { echo "FAIL: paint via instance != paint via source ($A vs $Bmd)"; exit 1; }
[[ "$A" != "$REF" ]] || { echo "FAIL: painted save unexpectedly equals null-edit"; exit 1; }
echo "OK paint-through-instance == paint-source (and both differ from no-op)"

echo "ALL M31 SESSION BYTE GATES PASSED"
