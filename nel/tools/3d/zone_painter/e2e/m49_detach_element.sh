#!/usr/bin/env bash
# M49 detach-to-element + element selection: Kaetemi's reframing of legacy detach for the
# brick world. The selection's boundary splits - duplicated shared vertices and edges,
# copied curves - so the selection becomes its own fully-connected ISLAND inside the same
# RklPatch: nothing moves, nothing dies, paint is untouched, and the zone still exports as
# ONE node (the ligo brick export refuses multi-node files). "Object" selection within a
# zone = the Element expand over shared-vertex connectivity.
#
#  1. THE SPLIT (zonematerial-bassin-1): markers on patches 5 and 6, detach {5,6} - patch
#     count UNCHANGED, vertex count grows by the boundary, markers stay at their indices,
#     the split corners' duplicates sit at the SAME world position (the seam is invisible).
#     ELEMENT: selecting just patch 5 and expanding grabs exactly {5,6} - the island; a
#     complement seed expands to the other c0-2. Persists across save+reopen; the file
#     round-trips the encoder.
#  2. WELD-BACK: detach {5,6}, then select every vertex and weld at a tiny threshold -
#     the coincident duplicates merge home, the copied edges fuse away (the UNMOVED side
#     keeps the seam curve), counts return to the originals; the TWO-stroke undo chain
#     restores the null-edit baseline byte for byte.
#  3. BIND RELEASE (zonematerial-bassin-ilot_croix): detaching the bind-target patch 229
#     releases vertex 445's bind group (anchor and target end up on different sides).
#  4. REFUSALS: whole zone; detaching an already-separate element refuses ("already a
#     separate element" - run the same detach twice).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m49_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

B=zonematerial-bassin-1
MAXDIR="landscape/ligo/lacustre/max"

seed() { rm -rf "$1"; mkdir -p "$1/$MAXDIR"
	cp "$GFX/$MAXDIR/$2.max" "$1/$MAXDIR/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }
run_verify() { $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

echo "===== M49-1: the split - island born in place, element selection grabs it ====="
cat > "$OUT/split.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 5, 2, 3, 100, 0)
painter.rawTile(0, 6, 1, 1, 101, 0)
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
-- a shared corner of patch 5 and its world position, to find its duplicate after
painter.setMode(4)
painter.setSubObject(3)
local a = painter.patchEdgeVerts(0, 5, 3) -- a corner on the selection boundary
local wx, wy, wz = painter.patchVertexPos(0, a)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection() == 2, "detach failed")
assert(painter.patchCount(0) == c0, "patch count changed")
local v1 = painter.vertexCount(0)
assert(v1 > v0, "no boundary split")
-- a duplicate sits at the same world position as some original (the seam is invisible)
local found = false
for i = v0, v1 - 1 do
  local x, y, z = painter.patchVertexPos(0, i)
  if x and math.abs(x - wx) + math.abs(y - wy) + math.abs(z - wz) < 0.0001 then found = true end
end
assert(found, "no duplicate at the split corner")
assert(painter.tileAt(0, 5, 2, 3) == 100, "marker 5 moved")
assert(painter.tileAt(0, 6, 1, 1) == 101, "marker 6 moved")
-- ELEMENT: one patch of the island expands to exactly the island
painter.clearPatchVertexSelection()
painter.selectPatchFace(0, 5, 0)
assert(painter.expandSelectionToElement() == 1, "island expand")
assert(painter.patchFaceSelectionCount() == 2, "island size")
painter.selectPatchFace(0, 0, 0)
painter.expandSelectionToElement()
assert(painter.patchFaceSelectionCount() == c0 - 2, "mainland size")
print("M49-1 OK v=" .. v1)
EOF
seed "$OUT/ws1" "$B"
run_session "$OUT/ws1" "$B" "$OUT/split.lua" "$OUT/g1.log"
V1=$(grep -a "M49-1 OK" "$OUT/g1.log" | sed 's/.*v=//')
[[ -n "$V1" ]] || { echo "FAIL: split script"; tail -10 "$OUT/g1.log"; exit 1; }
cat > "$OUT/split_v.lua" <<EOF
assert(painter.vertexCount(0) == $V1, "reopen vertex count")
assert(painter.tileAt(0, 5, 2, 3) == 100, "marker lost")
print("M49-1 reopen OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/split_v.lua" "$OUT/g1v.log"
grep -aq "M49-1 reopen OK" "$OUT/g1v.log" || { echo "FAIL: split did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/$MAXDIR/$B.max" > "$OUT/g1.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on split file"; tail -4 "$OUT/g1.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1.pm.log" || { echo "FAIL: pm identity on split file"; exit 1; }
echo "OK the split (island in place, element selection, persists, round-trips)"

echo "===== M49-2: weld-back closes the seam; two-stroke undo is byte-exact ====="
"$ZP" "$GFX/$MAXDIR/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1
cat > "$OUT/weldback.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection() == 2)
assert(painter.vertexCount(0) > v0)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
for i = 0, painter.vertexCount(0) - 1 do painter.selectPatchVertex(0, i, 1) end
assert(painter.weldPatchSelection(0.001) > 0, "weld-back refused")
assert(painter.patchCount(0) == c0, "weld-back patches")
assert(painter.vertexCount(0) == v0, "weld-back verts")
print("M49-2 welded OK")
painter.undo()
painter.undo()
assert(painter.vertexCount(0) == v0, "undo verts")
print("M49-2 undone OK")
EOF
seed "$OUT/ws2" "$B"
run_session "$OUT/ws2" "$B" "$OUT/weldback.lua" "$OUT/g2.log"
grep -aq "M49-2 welded OK" "$OUT/g2.log" || { echo "FAIL: weld-back"; tail -10 "$OUT/g2.log"; exit 1; }
grep -aq "M49-2 undone OK" "$OUT/g2.log" || { echo "FAIL: weld-back undo"; tail -6 "$OUT/g2.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws2/$MAXDIR/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: undo chain left $U bytes"; exit 1; }
echo "OK weld-back and byte-exact undo"

echo "===== M49-3: binds crossing the cut release ====="
B3=zonematerial-bassin-ilot_croix
cat > "$OUT/bind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local b0 = painter.vertexBindInfo(0, 445)
assert(b0 == 1, "fixture: vertex 445 should be bound")
painter.selectPatchFace(0, 229, 1)
assert(painter.detachPatchSelection() == 1, "detach failed")
assert(painter.vertexBindInfo(0, 445) == 0, "bind survived the cut")
print("M49-3 OK")
EOF
seed "$OUT/ws3" "$B3"
run_session "$OUT/ws3" "$B3" "$OUT/bind.lua" "$OUT/g3.log"
grep -aq "M49-3 OK" "$OUT/g3.log" || { echo "FAIL: bind release"; tail -8 "$OUT/g3.log"; exit 1; }
echo "OK binds release across the cut"

echo "===== M49-4: refusals ====="
cat > "$OUT/refuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
for p = 0, c0 - 1 do painter.selectPatchFace(0, p, 1) end
assert(painter.detachPatchSelection() == 0, "whole zone detached")
painter.selectPatchFace(0, 5, 0)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection() == 2, "first detach failed")
painter.selectPatchFace(0, 5, 0)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection() == 0, "already-separate did not refuse")
print("M49-4 OK")
EOF
seed "$OUT/ws4" "$B"
run_session "$OUT/ws4" "$B" "$OUT/refuse.lua" "$OUT/g4.log"
grep -aq "M49-4 OK" "$OUT/g4.log" || { echo "FAIL: refusals"; tail -6 "$OUT/g4.log"; exit 1; }
grep -aq "whole zone" "$OUT/g4.log" || { echo "FAIL: expected the whole-zone refusal"; exit 1; }
grep -aq "already a separate element" "$OUT/g4.log" || { echo "FAIL: expected the already-separate refusal"; exit 1; }
echo "OK refusals"

echo "ALL M49 GATES PASSED"
