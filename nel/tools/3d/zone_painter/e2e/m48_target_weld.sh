#!/usr/bin/env bash
# M48 target-weld gates: the directed weld - a vertex merges INTO a named target, which
# keeps its position and identity (the drag-onto-a-vertex gesture's op). The m44 loop
# without the manual placement: delete a patch, add a quad on a rim edge, weld each fresh
# corner INTO its opposite rim corner - the seed snaps onto the rim, the side and far
# edges fuse (the STATIONARY curve wins the seam), and the zone is whole again.
#
#  1. THE SNAP LOOP (zonematerial-bassin-1): delete patch 5, add on a rim edge, two
#     directed welds close the hole - patch and vertex counts back to the originals, the
#     rim corners keep their exact world positions (the targets never move), the file
#     round-trips the encoder, and the FOUR-stroke undo chain (delete, add, weld, weld)
#     restores the null-edit baseline byte for byte.
#  2. REFUSALS: weld into itself; a bound endpoint (either side) refuses; two corners of
#     one patch refuse (degenerate). Nothing changes.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m48_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

B=zonematerial-bassin-1
MAXDIR="landscape/ligo/lacustre/max"

seed() { rm -rf "$1"; mkdir -p "$1/$MAXDIR"
	cp "$GFX/$MAXDIR/$B.max" "$1/$MAXDIR/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

"$ZP" "$GFX/$MAXDIR/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M48-1: the snap loop - two directed welds close the hole ====="
cat > "$OUT/snap.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local pairs4 = {}
for s = 0, 3 do local a, b = painter.patchEdgeVerts(0, 5, s); pairs4[s] = {a, b} end
local corners = {}
for s = 0, 3 do corners[pairs4[s][1]] = true; corners[pairs4[s][2]] = true end
local cpos = {}
for c in pairs(corners) do local x, y, z = painter.patchVertexPos(0, c); cpos[c] = {x, y, z} end
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
painter.selectPatchFace(0, 5, 1)
assert(painter.deletePatchSelection() == 1)
painter.setSubObject(2)
painter.selectPatchEdge(0, pairs4[0][1], pairs4[0][2], 0)
assert(painter.addQuadPatchSelection() == 1, "rim add failed")
-- each fresh corner welds INTO its nearest rim corner: the target keeps its place, the
-- seed snaps onto the rim - no placement step at all. The fresh corners hold the two
-- HIGHEST indices, so after each weld's compaction the remaining one sits at v0.
painter.setSubObject(1)
for i = 1, 2 do
  local x, y, z = painter.patchVertexPos(0, v0)
  local best, bd = nil, nil
  for c, p in pairs(cpos) do
    local d = (p[1]-x)^2 + (p[2]-y)^2 + (p[3]-z)^2
    if not bd or d < bd then bd = d; best = c end
  end
  assert(painter.weldVertexInto(0, v0, best) == 1, "directed weld failed")
end
assert(painter.patchCount(0) == c0, "snapped count")
assert(painter.vertexCount(0) == v0, "snapped verts")
-- the targets never moved
for c, p in pairs(cpos) do
  local x, y, z = painter.patchVertexPos(0, c)
  if x then
    local d = math.abs(x-p[1]) + math.abs(y-p[2]) + math.abs(z-p[3])
    assert(d < 0.0001, "rim corner moved " .. d)
  end
end
print("M48-1 snapped OK")
for i = 1, 4 do painter.undo() end
assert(painter.patchCount(0) == c0)
assert(painter.vertexCount(0) == v0)
print("M48-1 undone OK")
EOF
seed "$OUT/ws1"
run_session "$OUT/ws1" "$B" "$OUT/snap.lua" "$OUT/g1.log"
grep -aq "M48-1 snapped OK" "$OUT/g1.log" || { echo "FAIL: snap loop"; tail -10 "$OUT/g1.log"; exit 1; }
grep -aq "M48-1 undone OK" "$OUT/g1.log" || { echo "FAIL: snap undo"; tail -6 "$OUT/g1.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws1/$MAXDIR/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: snap undo chain left $U bytes"; exit 1; }
echo "OK the snap loop closes and undoes byte-identically"

echo "===== M48-1b: the snapped file persists and round-trips ====="
cat > "$OUT/snap2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local pairs4 = {}
for s = 0, 3 do local a, b = painter.patchEdgeVerts(0, 5, s); pairs4[s] = {a, b} end
local corners = {}
for s = 0, 3 do corners[pairs4[s][1]] = true; corners[pairs4[s][2]] = true end
local cpos = {}
for c in pairs(corners) do local x, y, z = painter.patchVertexPos(0, c); cpos[c] = {x, y, z} end
local v0 = painter.vertexCount(0)
painter.selectPatchFace(0, 5, 1)
painter.deletePatchSelection()
painter.setSubObject(2)
painter.selectPatchEdge(0, pairs4[0][1], pairs4[0][2], 0)
painter.addQuadPatchSelection()
painter.setSubObject(1)
for i = 1, 2 do
  local x, y, z = painter.patchVertexPos(0, v0)
  local best, bd = nil, nil
  for c, p in pairs(cpos) do
    local d = (p[1]-x)^2 + (p[2]-y)^2 + (p[3]-z)^2
    if not bd or d < bd then bd = d; best = c end
  end
  painter.weldVertexInto(0, v0, best)
end
print("M48-1b done c=" .. painter.patchCount(0) .. " v=" .. painter.vertexCount(0))
EOF
seed "$OUT/ws1b"
run_session "$OUT/ws1b" "$B" "$OUT/snap2.lua" "$OUT/g1b.log"
grep -aq "M48-1b done" "$OUT/g1b.log" || { echo "FAIL: snap2 script"; tail -8 "$OUT/g1b.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1b/$MAXDIR/$B.max" > "$OUT/g1b.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on snapped file"; tail -4 "$OUT/g1b.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1b.pm.log" || { echo "FAIL: pm identity on snapped file"; exit 1; }
echo "OK snapped file round-trips"

echo "===== M48-2: refusals ====="
B2=zonematerial-bassin-ilot_croix
cat > "$OUT/refuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local v0 = painter.vertexCount(0)
assert(painter.weldVertexInto(0, 3, 3) == 0, "self weld")
assert(painter.weldVertexInto(0, 445, 371) == 0, "bound source welded")
assert(painter.weldVertexInto(0, 371, 445) == 0, "bound target welded")
local a, b = painter.patchEdgeVerts(0, 0, 0)
assert(painter.weldVertexInto(0, a, b) == 0, "degenerate weld")
assert(painter.vertexCount(0) == v0)
print("M48-2 OK")
EOF
rm -rf "$OUT/ws2"; mkdir -p "$OUT/ws2/$MAXDIR"
cp "$GFX/$MAXDIR/$B2.max" "$OUT/ws2/$MAXDIR/"
ln -sfn "$GFX/landscape/_texture_tiles" "$OUT/ws2/landscape/_texture_tiles"
run_session "$OUT/ws2" "$B2" "$OUT/refuse.lua" "$OUT/g2.log"
grep -aq "M48-2 OK" "$OUT/g2.log" || { echo "FAIL: refusals"; tail -6 "$OUT/g2.log"; exit 1; }
echo "OK refusals"

echo "===== M48-3: the drag gesture reaches the op through the real handlers ====="
cat > "$OUT/drag.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local pairs4 = {}
for s = 0, 3 do local a, b = painter.patchEdgeVerts(0, 5, s); pairs4[s] = {a, b} end
local corners = {}
for s = 0, 3 do corners[pairs4[s][1]] = true; corners[pairs4[s][2]] = true end
local cpos = {}
for c in pairs(corners) do local x, y, z = painter.patchVertexPos(0, c); cpos[c] = {x, y, z} end
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
painter.selectPatchFace(0, 5, 1)
painter.deletePatchSelection()
painter.setSubObject(2)
painter.selectPatchEdge(0, pairs4[0][1], pairs4[0][2], 0)
painter.addQuadPatchSelection()
painter.setSubObject(1)
-- The mirror seed lands the fresh corners EXACTLY on the rim corners, which would make
-- the screen pick ambiguous - remember each one's weld target, then move it into clear
-- space before dragging (the gesture never needs placement; the gate needs an
-- unambiguous pick).
local target = {}
for i = 0, 1 do
  local x, y, z = painter.patchVertexPos(0, v0 + i)
  local best, bd = nil, nil
  for c, p in pairs(cpos) do
    local d = (p[1]-x)^2 + (p[2]-y)^2 + (p[3]-z)^2
    if not bd or d < bd then bd = d; best = c end
  end
  target[i] = best
end
for i = 0, 1 do
  painter.clearPatchVertexSelection()
  painter.selectPatchVertex(0, v0 + i, 1)
  painter.movePatchSelection(25 + i * 9, 21, 12)
end
-- drag each fresh corner ONTO its rim corner by SCREEN coordinates, through the real
-- begin/finish handlers (the drag proves the pick at both ends); after each weld's
-- compaction the remaining fresh corner sits at v0, its target index is unshifted
painter.clearPatchVertexSelection()
for i = 0, 1 do
  local sx0, sy0 = painter.vertexScreenPos(0, v0)
  local sx1, sy1 = painter.vertexScreenPos(0, target[i])
  assert(sx0 and sx1, "projection failed")
  assert(painter.weldDragAt(sx0, sy0, sx1, sy1), "drag did not begin on the vertex")
end
assert(painter.patchCount(0) == c0, "gesture count")
assert(painter.vertexCount(0) == v0, "gesture verts")
print("M48-3 OK")
EOF
seed "$OUT/ws3"
run_session "$OUT/ws3" "$B" "$OUT/drag.lua" "$OUT/g3.log"
grep -aq "M48-3 OK" "$OUT/g3.log" || { echo "FAIL: drag gesture"; tail -8 "$OUT/g3.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws3/$MAXDIR/$B.max" || true; } | wc -l)
[[ "$U" -ne 0 ]] || { echo "FAIL: gesture session saved the null baseline (nothing happened?)"; exit 1; }
echo "OK the drag gesture welds through the real handlers"

echo "ALL M48 GATES PASSED"
