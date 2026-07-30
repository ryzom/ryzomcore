#!/usr/bin/env bash
# M50 sculpted + mapper geometry gates: the fixtures the arithmetic cannot hide on.
#
# Two lessons bought by the 2026-07-30 review pass, pinned here so they stay bought:
#
#  - A byte-exact fixture can be EXACTLY the geometry on which an arithmetic bug vanishes.
#    The subdivide grid-corruption bug produced the correct centre on every uniform
#    lattice (the halved tangent controls happen to agree there) and a dented centre on
#    any sculpted patch. So scene 2 subdivides a genuinely curved pair and checks the
#    centre vertex against the true bicubic centre computed independently from the
#    pre-op controls - AND asserts the fixture actually discriminates (the would-be
#    corrupted centre must sit measurably elsewhere, or the check proves nothing).
#
#  - On a mapper mesh the stored positions of mapped outputs are STALE CACHES: evaluation
#    rebuilds them as input + delta, and the tool's own Tier A edits write the delta only.
#    Scenes 3 and 4 move a corner first (the caches go stale) and then run topology ops,
#    asserting the results follow the geometry the artist SEES.
#
# Scene 1 pins the ride-in-file rule directly: a corner move writes its tangent handles
# too, so the ridden handle survives save + reopen (before the fix the display rode and
# the stream did not - the saved surface was pinched and nobody saw it until reload).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m50_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }
run_verify() { $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

# Shared Lua helpers, prepended to every scene script.
cat > "$OUT/lib.lua" <<'EOF'
-- Ring corners of a patch: corner k = the vertex shared by edge slots (k+3)%4 and k.
function zpRing(z, p)
  local prs = {}
  for e = 0, 3 do
    local a, b = painter.patchEdgeVerts(z, p, e)
    prs[e] = { a, b }
  end
  local rv = {}
  for k = 0, 3 do
    for _, x in ipairs(prs[(k + 3) % 4]) do
      for _, y in ipairs(prs[k]) do
        if x == y then rv[k] = x end
      end
    end
  end
  return rv
end
-- The 16-control grid P[b][a] of a patch, from DISPLAY positions (what the artist sees;
-- for auto-interior patches the interiors are the derived ones the surface renders from).
function zpGrid(z, p)
  local rv = zpRing(z, p)
  local function V(i) local x, y, zz = painter.patchVertexPos(z, rv[i]) return { x, y, zz } end
  local function T(s) local vi = painter.patchVecIndex(z, p, s)
    local x, y, zz = painter.patchTangentPos(z, vi) return { x, y, zz } end
  local function I(s) local x, y, zz = painter.patchInteriorPos(z, p, s) return { x, y, zz } end
  local P = {}
  for b = 0, 3 do P[b] = {} end
  P[0][0] = V(0); P[0][3] = V(1); P[3][3] = V(2); P[3][0] = V(3)
  P[0][1] = T(0); P[0][2] = T(1); P[1][3] = T(2); P[2][3] = T(3)
  P[3][2] = T(4); P[3][1] = T(5); P[2][0] = T(6); P[1][0] = T(7)
  P[1][1] = I(0); P[1][2] = I(1); P[2][2] = I(2); P[2][1] = I(3)
  return P, rv
end
function zpCub(p0, p1, p2, p3)
  local r = {}
  for c = 1, 3 do r[c] = (p0[c] + 3 * p1[c] + 3 * p2[c] + p3[c]) / 8 end
  return r
end
function zpCentre(P)
  local q = {}
  for b = 0, 3 do q[b] = zpCub(P[b][0], P[b][1], P[b][2], P[b][3]) end
  return zpCub(q[0], q[1], q[2], q[3])
end
function zpDist(a, b)
  local dx, dy, dz = a[1] - b[1], a[2] - b[2], a[3] - b[3]
  return math.sqrt(dx * dx + dy * dy + dz * dz)
end
function zpAvg(a, b)
  return { (a[1] + b[1]) / 2, (a[2] + b[2]) / 2, (a[3] + b[3]) / 2 }
end
-- Nearest vertex in index range [i0, i1) to a point; returns index, distance.
function zpNearestVert(z, i0, i1, pt)
  local best, bd = -1, 1e30
  for i = i0, i1 - 1 do
    local x, y, zz = painter.patchVertexPos(z, i)
    if x then
      local d = zpDist({ x, y, zz }, pt)
      if d < bd then best, bd = i, d end
    end
  end
  return best, bd
end
-- The patch containing vertex v as a ring corner, or nil.
function zpPatchOf(z, v)
  local n = painter.patchCount(z)
  for p = 0, n - 1 do
    local rv = zpRing(z, p)
    for k = 0, 3 do
      if rv[k] == v then return p, k end
    end
  end
  return nil
end
EOF

echo "===== M50-1: a moved corner's handles survive save + reopen ====="
B=zonematerial-bassin-1
seed "$OUT/ws1" "$B"
cat "$OUT/lib.lua" > "$OUT/s1.lua"; cat >> "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local vi = painter.patchVecIndex(0, 5, 0)
local _, _, z0 = painter.patchTangentPos(0, vi)
painter.selectPatchVertex(0, 6, 1)
painter.movePatchSelection(0, 0, 5)
local _, _, z1 = painter.patchTangentPos(0, vi)
assert(math.abs(z1 - z0 - 5) < 1e-4, "handle did not ride in session")
print("M50-1 SESSION OK")
EOF
cat "$OUT/lib.lua" > "$OUT/s1b.lua"; cat >> "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
local vi = painter.patchVecIndex(0, 5, 0)
local _, _, z = painter.patchTangentPos(0, vi)
local _, _, vz = painter.patchVertexPos(0, 6)
assert(math.abs(vz - 5) < 1e-4, "vertex did not persist")
assert(math.abs(z - 5) < 1e-4, string.format("handle did not persist the ride (z=%.6f)", z))
print("M50-1 REOPEN OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M50-1 SESSION OK" "$OUT/s1.log" || { echo "FAIL: scene 1 session"; tail -6 "$OUT/s1.log"; exit 1; }
run_verify "$OUT/ws1" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M50-1 REOPEN OK" "$OUT/s1b.log" || { echo "FAIL: the ride did not survive reopen"; tail -6 "$OUT/s1b.log"; exit 1; }
echo "OK ride persists across save + reopen"

echo "===== M50-2: sculpted pair subdivide - the centre is the TRUE bicubic centre ====="
seed "$OUT/ws2" "$B"
cat "$OUT/lib.lua" > "$OUT/s2.lua"; cat >> "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
-- The m43 pair: patches 5 and 10 share the {12, 13} edge, and patch 5 is genuinely
-- curved. The shared edge takes the PLAIN split (both selected), which is the path that
-- halves the reused tangent records - the corruption scenario.
local P5 = zpGrid(0, 5)
local true5 = zpCentre(P5)
-- The would-be corrupted grid: the shared edge is patch 5's slot 2 (row b=3); phase 1
-- would have replaced each tangent with its average toward the adjacent corner.
local C = {}
for b = 0, 3 do C[b] = {} for a = 0, 3 do C[b][a] = P5[b][a] end end
C[3][2] = zpAvg(P5[3][3], P5[3][2])
C[3][1] = zpAvg(P5[3][0], P5[3][1])
local corrupt5 = zpCentre(C)
local margin = zpDist(true5, corrupt5)
print(string.format("discrimination margin: %.6f", margin))
assert(margin > 0.01, "fixture does not discriminate - pick a curvier pair")
local v0 = painter.vertexCount(0)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 10, 1)
assert(painter.subdividePatchSelection() == 2, "subdivide failed")
local v1 = painter.vertexCount(0)
local near, d = zpNearestVert(0, v0, v1, true5)
print(string.format("centre vert %d at %.6f from true centre", near, d))
assert(d < 2e-3, "centre vertex is NOT the true bicubic centre")
local _, dc = zpNearestVert(0, v0, v1, corrupt5)
assert(dc > margin / 2, "a new vertex sits at the CORRUPTED centre")
print("M50-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M50-2 OK" "$OUT/s2.log" || { echo "FAIL: sculpted centre"; tail -8 "$OUT/s2.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" > "$OUT/s2.rt.log" 2>&1 \
	|| { echo "FAIL: subdivided file does not round-trip"; tail -4 "$OUT/s2.rt.log"; exit 1; }
echo "OK sculpted centre exact (margin $(grep -a 'discrimination margin' "$OUT/s2.log" | grep -oa '[0-9.]*$')), file round-trips"

echo "===== M50-3: mapper mesh - subdivide AFTER a move follows the moved geometry ====="
M=material-bassin
"$ZP" "$GFX/landscape/ligo/lacustre/max/$M.max" --null-edit --out "$OUT/$M.null.max" > "$OUT/null.log" 2>&1
seed "$OUT/ws3" "$M"
cat "$OUT/lib.lua" > "$OUT/s3.lua"; cat >> "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- Move a corner: the Tier A write lands in the mapper DELTA, so every stored position of
-- the moved elements is now a stale cache. The old transform read those.
local MV = 5
painter.selectPatchVertex(0, MV, 1)
painter.movePatchSelection(0, 0, 7)
local p, k = zpPatchOf(0, MV)
assert(p, "no patch has the moved vertex as a corner")
print(string.format("moved vertex %d is corner %d of patch %d", MV, k, p))
-- Expected midpoint of the ring edge leaving that corner, from POST-move display controls.
local P = zpGrid(0, p)
local edgeRows = {
  [0] = { P[0][0], P[0][1], P[0][2], P[0][3] },
  [1] = { P[0][3], P[1][3], P[2][3], P[3][3] },
  [2] = { P[3][3], P[3][2], P[3][1], P[3][0] },
  [3] = { P[3][0], P[2][0], P[1][0], P[0][0] },
}
local r = edgeRows[k]
local mid = zpCub(r[1], r[2], r[3], r[4])
local v0 = painter.vertexCount(0)
painter.setSubObject(3)
painter.selectPatchFace(0, p, 1)
assert(painter.subdividePatchSelection() == 1, "subdivide failed on the mapper mesh")
local v1 = painter.vertexCount(0)
local near, d = zpNearestVert(0, v0, v1, mid)
print(string.format("edge midpoint vert %d at %.6f from moved-curve midpoint", near, d))
assert(d < 2e-3, "midpoint followed the STALE stored curve, not the moved one")
print("M50-3 OK")
EOF
run_session "$OUT/ws3" "$M" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M50-3 OK" "$OUT/s3.log" || { echo "FAIL: mapper subdivide"; tail -8 "$OUT/s3.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3/landscape/ligo/lacustre/max/$M.max" > "$OUT/s3.rt.log" 2>&1 \
	|| { echo "FAIL: mapper subdivided file does not round-trip"; tail -4 "$OUT/s3.rt.log"; exit 1; }
echo "OK mapper subdivide follows the evaluated geometry, file round-trips"

echo "===== M50-4: mapper mesh - detach duplicates land at the MOVED position + undo chain ====="
seed "$OUT/ws4" "$M"
cat "$OUT/lib.lua" > "$OUT/s4.lua"; cat >> "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- The moved corner must be SHARED with the unselected rest or no duplicate of it exists:
-- pick the first unbound vertex used as a corner by at least two patches.
local usage, firstPatch = {}, {}
local n = painter.patchCount(0)
for p = 0, n - 1 do
  local rv = zpRing(0, p)
  for k = 0, 3 do
    usage[rv[k]] = (usage[rv[k]] or 0) + 1
    if firstPatch[rv[k]] == nil then firstPatch[rv[k]] = p end
  end
end
local MV, p = nil, nil
for v, c in pairs(usage) do
  if c >= 2 then
    local binded = painter.vertexBindInfo(0, v)
    if binded == 0 and (MV == nil or v < MV) then MV, p = v, firstPatch[v] end
  end
end
assert(MV, "no shared unbound corner found")
print(string.format("moving shared vertex %d (patch %d)", MV, p))
painter.selectPatchVertex(0, MV, 1)
painter.movePatchSelection(0, 0, 7)
local mx, my, mz = painter.patchVertexPos(0, MV)
local v0 = painter.vertexCount(0)
painter.setSubObject(3)
painter.selectPatchFace(0, p, 1)
assert(painter.detachPatchSelection() >= 1, "detach refused")
local v1 = painter.vertexCount(0)
assert(v1 > v0, "no boundary duplicates created")
local near, d = zpNearestVert(0, v0, v1, { mx, my, mz })
print(string.format("duplicate %d at %.6f from the moved corner", near, d))
assert(d < 1e-4, "a duplicate copied the STALE stored position - the seam is open")
-- Two strokes back (detach, move) must restore the null-edit baseline byte for byte.
painter.undo()
painter.undo()
print("M50-4 OK")
EOF
run_session "$OUT/ws4" "$M" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M50-4 OK" "$OUT/s4.log" || { echo "FAIL: mapper detach"; tail -8 "$OUT/s4.log"; exit 1; }
cmp -s "$OUT/$M.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$M.max" \
	|| { echo "FAIL: move+detach undo chain is not byte-identical"; exit 1; }
echo "OK mapper detach duplicates follow eval; undo chain byte-identical"

echo "ALL M50 GATES PASSED"
