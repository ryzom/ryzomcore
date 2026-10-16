#!/usr/bin/env bash
# M42 turn gates: the legacy tile-frame quarter turn (TurnPatch port), through the shared
# topology machinery (Kind 6 undo, keepUndo rebuild, byte identity).
#
#  1. PAINT FOLLOWS THE FRAME: a painted tile at (u, v) of a 16x16 patch lands at
#     (OrderT-1-v, u) after one CCW turn; CW turns it back; four CCW turns are the
#     identity ON THE INTERIOR; the SEAM WIPE empties the shared-edge tile rows on the
#     turned side (the rotated paint no longer transitions into the neighbors'); and the
#     whole seven-stroke undo chain (1 paint + 6 turns) saves byte-identical to the
#     null-edit baseline.
#  2. TURN IS A BIJECTION AT THE BYTE LEVEL - on an ISOLATED patch: detach patch 5 to an
#     element first (its edges become open copies, and fully open edges do not wipe),
#     then four CCW turns save byte-identical to the detach-only session. Grid, colors,
#     edge flags and rings all return through four applications of the transform.
#  3. BINDS FOLLOW THE EDGE RING: vertex 445 of zonematerial-bassin-ilot_croix is bound
#     onto patch 229 edge 0; a CCW turn of that patch moves the record to edge 3, CW
#     brings it back, and the turned file round-trips the encoder (--pm-modify-save-test).
#  4. The modifier-stream target turns and persists (material-bassin, reopened count and
#     a turned-grid readback).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m42_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { # $1 = workspace dir, $2 = basename
	rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

run_session() { # $1 = ws, $2 = basename, $3 = lua, $4 = log
	ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
		--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1
}

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M42-1: paint follows the frame; full undo chain is byte-identical ====="
cat > "$OUT/turn.lua" <<'EOF'
painter.setMode(0)
painter.paintTile(0, 5, 0.2, 0.2, 2)
local t0 = painter.tileAt(0, 5, 1, 1)
-- A border tile on the authored paint, for the seam-wipe assertion (patch 5 is interior,
-- so every edge is shared and every border row wipes).
local b0 = painter.tileAt(0, 5, 7, 0)
assert(b0 ~= -1, "fixture: border tile should be painted")
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.turnPatchSelection(true) == 1)
-- patch 5 is 16x16: (u, v) lands at (OrderT-1-v, u) = (14, 1)
assert(painter.tileAt(0, 5, 14, 1) == t0, "tile did not transpose")
-- The seam wipe: patch 5's three SHARED edges wipe their rows; the fourth edge is the
-- zone's open west border, and open edges keep their tiles. Post-turn the ring carried
-- the open edge to slot 2 (the u=15 column), so that row is the KEPT one.
assert(painter.tileAt(0, 5, 7, 0) == -1, "v=0 seam row not wiped")
assert(painter.tileAt(0, 5, 7, 15) == -1, "v=15 seam row not wiped")
assert(painter.tileAt(0, 5, 0, 7) == -1, "u=0 seam row not wiped")
assert(painter.tileAt(0, 5, 15, 7) ~= -1, "the open edge's row should KEEP its tiles")
painter.selectPatchFace(0, 5, 0)
assert(painter.turnPatchSelection(false) == 1)
assert(painter.tileAt(0, 5, 1, 1) == t0, "cw did not restore")
for i = 1, 4 do painter.selectPatchFace(0, 5, 0); assert(painter.turnPatchSelection(true) == 1) end
assert(painter.tileAt(0, 5, 1, 1) == t0, "4x ccw not identity")
for i = 1, 7 do painter.undo() end
print("M42-1 OK")
EOF
seed "$OUT/ws1" "$B"
run_session "$OUT/ws1" "$B" "$OUT/turn.lua" "$OUT/g1.log"
grep -aq "M42-1 OK" "$OUT/g1.log" || { echo "FAIL: turn script"; tail -6 "$OUT/g1.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: undo chain left $U bytes"; exit 1; }
echo "OK paint follows the frame; undo chain byte-identical"

echo "===== M42-2: four CCW turns on an ISOLATED patch save byte-identical ====="
# Shared edges wipe, so the bijection is proven where the wipe cannot reach: detach
# patch 5 to an element (open boundary copies), then four turns == the detach alone.
cat > "$OUT/det.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.detachPatchSelection() == 1)
print("M42-2 DETACH OK")
EOF
cat > "$OUT/turn4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.detachPatchSelection() == 1)
painter.setSubObject(3)
for i = 1, 4 do painter.selectPatchFace(0, 5, 0); assert(painter.turnPatchSelection(true) == 1) end
print("M42-2 OK")
EOF
seed "$OUT/ws2a" "$B"
run_session "$OUT/ws2a" "$B" "$OUT/det.lua" "$OUT/g2a.log"
grep -aq "M42-2 DETACH OK" "$OUT/g2a.log" || { echo "FAIL: detach-only script"; tail -6 "$OUT/g2a.log"; exit 1; }
seed "$OUT/ws2" "$B"
run_session "$OUT/ws2" "$B" "$OUT/turn4.lua" "$OUT/g2.log"
grep -aq "M42-2 OK" "$OUT/g2.log" || { echo "FAIL: turn4 script"; tail -6 "$OUT/g2.log"; exit 1; }
T=$( { cmp -l "$OUT/ws2a/landscape/ligo/lacustre/max/$B.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$T" -eq 0 ]] || { echo "FAIL: 4x turn on the island left $T bytes"; exit 1; }
echo "OK four turns are the byte identity on the open-edged island"

echo "===== M42-3: binds follow the edge ring; turned file round-trips the encoder ====="
B3=zonematerial-bassin-ilot_croix
cat > "$OUT/bindturn.lua" <<'EOF'
painter.setMode(4)
local _, _, p0, e0 = select(1, painter.vertexBindInfo(0, 445))
local b, t, p, e = painter.vertexBindInfo(0, 445)
assert(b == 1 and p == 229 and e == 0, "fixture drifted")
painter.setSubObject(3)
painter.selectPatchFace(0, 229, 1)
assert(painter.turnPatchSelection(true) == 1)
local b1, t1, p1, e1 = painter.vertexBindInfo(0, 445)
assert(b1 == 1 and p1 == 229 and e1 == 3, "bind did not follow the ring: edge " .. e1)
painter.selectPatchFace(0, 229, 0)
assert(painter.turnPatchSelection(false) == 1)
local b2, t2, p2, e2 = painter.vertexBindInfo(0, 445)
assert(e2 == 0, "cw did not bring the bind back")
-- leave ONE ccw turn in the saved file for the encoder round-trip check
painter.selectPatchFace(0, 229, 0)
assert(painter.turnPatchSelection(true) == 1)
print("M42-3 OK")
EOF
seed "$OUT/ws3" "$B3"
run_session "$OUT/ws3" "$B3" "$OUT/bindturn.lua" "$OUT/g3.log"
grep -aq "M42-3 OK" "$OUT/g3.log" || { echo "FAIL: bind-turn script"; tail -6 "$OUT/g3.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3/landscape/ligo/lacustre/max/$B3.max" > "$OUT/g3.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on turned file"; tail -4 "$OUT/g3.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g3.pm.log" || { echo "FAIL: pm identity on turned file"; exit 1; }
echo "OK binds follow the ring; turned file round-trips"

echo "===== M42-4: modifier-stream target turns and persists ====="
B4=material-bassin
cat > "$OUT/modturn.lua" <<'EOF'
painter.setMode(0)
-- An INTERIOR marker: the seam wipe empties the border rows of a turned shared-edge
-- patch, so the persistence probe must ride a tile the wipe cannot reach.
painter.rawTile(0, 3, 1, 1, 102, 0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 3, 1)
assert(painter.turnPatchSelection(true) == 1)
print("M42-4 t0=102")
EOF
seed "$OUT/ws4" "$B4"
run_session "$OUT/ws4" "$B4" "$OUT/modturn.lua" "$OUT/g4.log"
T0=$(grep -a "M42-4 t0=" "$OUT/g4.log" | sed 's/.*t0=//')
[[ -n "$T0" ]] || { echo "FAIL: modifier turn script"; tail -6 "$OUT/g4.log"; exit 1; }
grep -aq "(modifier target)" "$OUT/g4.log" || { echo "FAIL: expected the modifier target"; exit 1; }
# patch 3 of material-bassin: the (0,0) tile lands at (OrderT-1, 0); read the order first
cat > "$OUT/modturn_v.lua" <<EOF
-- after one CCW turn, (1,1) landed at ((1 << newOrderU) - 2, 1)
local nu, nv = painter.patchTess(0, 3)
local t = painter.tileAt(0, 3, (2 ^ nu) - 2, 1)
assert(t == $T0, "turned tile not at the transposed spot after reopen: " .. tostring(t))
print("M42-4 reopen OK")
EOF
$XVFB "$ZP" "$OUT/ws4" --startup-auto "lacustre/$B4" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/modturn_v.lua" --screenshot /dev/null > "$OUT/g4v.log" 2>&1
grep -aq "M42-4 reopen OK" "$OUT/g4v.log" || { echo "FAIL: modifier turn did not persist"; tail -5 "$OUT/g4v.log"; exit 1; }
echo "OK modifier target turned and persisted"

echo "ALL M42 GATES PASSED"
