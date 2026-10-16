#!/usr/bin/env bash
# M51 Selection block: the Filter Vertices / Vectors pair and Lock Handles.
#
# The filters gate what a vertex-level click can PICK - never what is drawn - and both
# cannot be off at once (the setter refuses; the panel freezes the other box). Lock
# Handles expands a handle move over the corner's owner group, every consumer (preview,
# live push, commit) through one predicate, so the drawn cage and the written file cannot
# disagree about what moved.
#
#  1. FILTERS: with Vertices off, a click on a vertex picks nothing; back on, it picks.
#     With Vectors off, a click on a drawn handle does not pick it (handles are drawn for
#     selected corners, so the corner is selected first). Both-off is refused: the second
#     uncheck leaves the filter on and the click still picks.
#  2. LOCK: a handle move with lock OFF writes exactly the handle; with lock ON it writes
#     the handle AND its corner companion, the companion moving through its own delta.
#     The whole chain undoes to the null-edit baseline byte for byte.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m51_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$B.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$B" $XVFB "$ZP" "$1" --startup-auto "lacustre/$B" \
	--no-hint-stamp --no-thumbnail --startup-lua "$2" --screenshot /dev/null > "$3" 2>&1; }

echo "===== M51-1: pick filters ====="
seed "$OUT/ws1"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local sx, sy = painter.vertexScreenPos(0, 6)
-- Vertices filtered off: the click lands on the vertex and picks NOTHING.
painter.setFilterVertices(false)
painter.patchClick(sx, sy, 0)
assert(painter.patchVertexSelectionCount() == 0, "filtered-off vertex was picked")
painter.setFilterVertices(true)
painter.patchClick(sx, sy, 0)
assert(painter.patchVertexSelectionCount() == 1, "vertex not picked with the filter on")
-- Vectors filtered off: the corner is selected (its handles draw), and a click on the
-- handle's own screen spot must NOT pick the handle.
local tx, ty = painter.tangentScreenPos(0, 10) -- corner 6's handle on edge 0 of patch 5
painter.setFilterVectors(false)
painter.patchClick(tx, ty, 9) -- left+ctrl: an add, so a miss does not clear the corner
assert(painter.patchTangentSelectionCount() == 0, "filtered-off handle was picked")
painter.setFilterVectors(true)
painter.patchClick(tx, ty, 9)
assert(painter.patchTangentSelectionCount() == 1, "handle not picked with the filter on")
painter.selectPatchTangent(0, 10, 2) -- drop it again for the both-off check
-- Both-off refusal: with Vertices off, turning Vectors off must refuse - the vertex
-- filter comes back on and a handle click still picks.
painter.setFilterVertices(false)
painter.setFilterVectors(false)
painter.patchClick(tx, ty, 9)
assert(painter.patchTangentSelectionCount() == 1, "both filters went off at once")
print("M51-1 OK")
EOF
run_session "$OUT/ws1" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M51-1 OK" "$OUT/s1.log" || { echo "FAIL: filters"; tail -8 "$OUT/s1.log"; exit 1; }
echo "OK filters gate picking, both-off refused"

echo "===== M51-2: lock handles ====="
seed "$OUT/ws2"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- Corner 6 owns THREE handles - 10 (edge 0 of patch 5), 62 (edge 3), and one on its
-- third incident edge. Select the corner (so handles show) then handle 10; handle mode -
-- the handles are the transform target.
painter.selectPatchVertex(0, 6, 1)
painter.selectPatchTangent(0, 10, 1)
local _, _, a0 = painter.patchTangentPos(0, 10)
local _, _, b0 = painter.patchTangentPos(0, 62)
painter.setLockHandles(false)
painter.movePatchSelection(0, 0, 2)
local _, _, b1 = painter.patchTangentPos(0, 62)
assert(math.abs(b1 - b0) < 1e-6, "companion moved with lock OFF")
painter.setLockHandles(true)
painter.movePatchSelection(0, 0, 2)
local _, _, a2 = painter.patchTangentPos(0, 10)
local _, _, b2 = painter.patchTangentPos(0, 62)
assert(math.abs(a2 - a0 - 4) < 1e-4, "selected handle did not take both moves")
assert(math.abs(b2 - b0 - 2) < 1e-4, "companion did not move with lock ON")
painter.undo()
painter.undo()
print("M51-2 OK")
EOF
run_session "$OUT/ws2" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M51-2 OK" "$OUT/s2.log" || { echo "FAIL: lock handles"; tail -8 "$OUT/s2.log"; exit 1; }
grep -aq "movePatchSelection: 1 written" "$OUT/s2.log" || {
	echo "FAIL: lock-off handle move did not write exactly the handle"; exit 1; }
grep -aq "movePatchSelection: 3 written" "$OUT/s2.log" || {
	echo "FAIL: lock-on handle move did not write the corner's three handles"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: lock-handle undo chain is not byte-identical"; exit 1; }
echo "OK lock expands the write (1 then 3 elements), undo byte-identical"

echo "ALL M51 GATES PASSED"
