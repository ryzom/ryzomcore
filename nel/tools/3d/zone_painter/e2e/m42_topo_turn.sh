#!/usr/bin/env bash
# M42 turn gates: the legacy tile-frame quarter turn (TurnPatch port), through the shared
# topology machinery (Kind 6 undo, keepUndo rebuild, byte identity).
#
#  1. PAINT FOLLOWS THE FRAME: a painted tile at (u, v) of a 16x16 patch lands at
#     (OrderT-1-v, u) after one CCW turn; CW turns it back; four CCW turns are the
#     identity; and the whole seven-stroke undo chain (1 paint + 6 turns) saves
#     byte-identical to the null-edit baseline.
#  2. TURN IS A BIJECTION AT THE BYTE LEVEL: four CCW turns with NO undo save
#     byte-identical to the baseline - grid, colors, edge flags and rings all return to
#     their authored bytes through four applications of the transform.
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
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.turnPatchSelection(true) == 1)
-- patch 5 is 16x16: (u, v) lands at (OrderT-1-v, u) = (14, 1)
assert(painter.tileAt(0, 5, 14, 1) == t0, "tile did not transpose")
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

echo "===== M42-2: four CCW turns save byte-identical (bijection proof) ====="
cat > "$OUT/turn4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
for i = 1, 4 do painter.selectPatchFace(0, 5, 0); assert(painter.turnPatchSelection(true) == 1) end
print("M42-2 OK")
EOF
seed "$OUT/ws2" "$B"
run_session "$OUT/ws2" "$B" "$OUT/turn4.lua" "$OUT/g2.log"
grep -aq "M42-2 OK" "$OUT/g2.log" || { echo "FAIL: turn4 script"; tail -6 "$OUT/g2.log"; exit 1; }
T=$( { cmp -l "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$T" -eq 0 ]] || { echo "FAIL: 4x turn left $T bytes"; exit 1; }
echo "OK four turns are the byte identity"

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
painter.paintTile(0, 3, 0.2, 0.2, 2)
local t0 = painter.tileAt(0, 3, 0, 0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 3, 1)
assert(painter.turnPatchSelection(true) == 1)
print("M42-4 t0=" .. tostring(t0))
EOF
seed "$OUT/ws4" "$B4"
run_session "$OUT/ws4" "$B4" "$OUT/modturn.lua" "$OUT/g4.log"
T0=$(grep -a "M42-4 t0=" "$OUT/g4.log" | sed 's/.*t0=//')
[[ -n "$T0" ]] || { echo "FAIL: modifier turn script"; tail -6 "$OUT/g4.log"; exit 1; }
grep -aq "(modifier target)" "$OUT/g4.log" || { echo "FAIL: expected the modifier target"; exit 1; }
# patch 3 of material-bassin: the (0,0) tile lands at (OrderT-1, 0); read the order first
cat > "$OUT/modturn_v.lua" <<EOF
-- after one CCW turn, (0,0) landed at (newOrderS-1, 0); probe along the top row
local found = false
for u = 0, 15 do
  local t = painter.tileAt(0, 3, u, 0)
  if t == nil then break end
  if u > 0 and painter.tileAt(0, 3, u + 1, 0) == nil and t == $T0 then found = true end
end
assert(found, "turned tile not at the top-right corner after reopen")
print("M42-4 reopen OK")
EOF
$XVFB "$ZP" "$OUT/ws4" --startup-auto "lacustre/$B4" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/modturn_v.lua" --screenshot /dev/null > "$OUT/g4v.log" 2>&1
grep -aq "M42-4 reopen OK" "$OUT/g4v.log" || { echo "FAIL: modifier turn did not persist"; tail -5 "$OUT/g4v.log"; exit 1; }
echo "OK modifier target turned and persisted"

echo "ALL M42 GATES PASSED"
