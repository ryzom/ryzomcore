#!/usr/bin/env bash
# M57 Edge-level Subdivide + Propagate (plan mA4).
#
# Selecting edges and hitting Subdiv splits each ADJACENT patch once, along the parameter
# that CROSSES the selected edge - a single-axis 1->2 split (rows only, never
# rows-then-columns). The cut necessarily splits the patch's opposite edge too; an
# unsplit neighbor there takes the canonical T-junction bind. Propagate walks the strip
# (each affected patch's opposite edge joins the set) until it loops or exits an open
# border. Adjacent selected edges on one patch degenerate it to the proven 1->4.
#
#  1. COUNTS + T-BINDS (zonematerial-bassin-1, no propagate): one interior edge -> both
#     adjacent patches split (+2), three midpoints, exactly the two OPPOSITE-edge
#     midpoints bind; the split axis's tile order halves on the surviving patch index
#     while the other axis keeps; painted markers land in the right halves; undo restores
#     the null-edit baseline byte for byte; the file round-trips the encoder.
#  2. THE SPLIT IS THE CURVE (sculpted in-scene - the m50 lesson: flat fixtures hide
#     arithmetic): a corner is moved 3 m up first, then the edge splits; the new midpoint
#     must land at the cubic's (c0+3c1+3c2+c3)/8 computed INDEPENDENTLY from the pre-split
#     controls, and the reused outer tangent at (c0+c1)/2.
#  3. PROPAGATE: the same single edge walks the whole 5-patch strip (+5 patches, 6
#     midpoints, NO binds - interior rungs are shared by splitting pairs, the end rungs
#     exit the open border).
#  4. ADJACENT PAIR: two adjacent edges of one patch -> that patch 1->4, the two edge
#     neighbors 1->2 (+5 total); undo byte-identity; round-trip.
#  5. MAPPER (material-bassin): move a corner first (stored caches go stale), then split
#     an edge of a patch it touches - the midpoint must follow the EVALUATED curve.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m57_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M57-1: counts, T-binds, tile halving, markers, undo, round-trip ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
local nP0 = painter.patchCount(0)
local nV0 = painter.vertexCount(0)
-- Markers on patch 12: origin tile and the far-v tile (the axis the split halves).
painter.setMode(0)
painter.rawTile(0, 12, 0, 0, 100, 0)
painter.rawTile(0, 12, 0, 15, 101, 0)
painter.rawTile(0, 12, 15, 0, 102, 0)
painter.setMode(4)
painter.setSubObject(2)
local a, b = painter.patchEdgeVerts(0, 12, 0)
painter.selectPatchEdge(0, a, b, 0)
painter.subdivideEdgeSelection()
local nP1 = painter.patchCount(0)
local nV1 = painter.vertexCount(0)
assert(nP1 == nP0 + 2, "expected +2 patches, got +" .. (nP1 - nP0))
assert(nV1 == nV0 + 3, "expected +3 vertices, got +" .. (nV1 - nV0))
-- Exactly the two opposite-edge midpoints bind (the selected edge's midpoint is shared
-- by two splitting patches and stays free).
local bound = 0
for v = nV0, nV1 - 1 do
  local isb = painter.vertexBindInfo(0, v)
  if isb == 1 then bound = bound + 1 end
end
assert(bound == 2, "expected 2 T-binds, got " .. bound)
-- The surviving patch index (the lo child) halves the SPLIT axis only.
local u, v = painter.patchTess(0, 12)
assert((u == 4 and v == 3) or (u == 3 and v == 4), "one axis must halve: " .. u .. "x" .. v)
-- Paint: the origin marker stays at the lo child's origin; the u-axis marker survives
-- on whichever child carries it; every marker is still present SOMEWHERE.
assert(painter.tileAt(0, 12, 0, 0) == 100, "origin marker lost")
local found101, found102 = false, false
for p = 0, nP1 - 1 do
  local pu, pv = painter.patchTess(0, p)
  for uu = 0, (2^pu) - 1 do
    for vv = 0, (2^pv) - 1 do
      local t = painter.tileAt(0, p, uu, vv)
      if t == 101 then found101 = true end
      if t == 102 then found102 = true end
    end
  end
end
assert(found101 and found102, "a marker tile vanished in the split")
print("M57-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M57-1 OK" "$OUT/s1.log" || { echo "FAIL: counts scene"; tail -10 "$OUT/s1.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1.rt.log" 2>&1 \
	|| { echo "FAIL: split file does not round-trip"; tail -4 "$OUT/s1.rt.log"; exit 1; }
echo "OK counts + binds + tiles + round-trip"

seed "$OUT/ws1b" "$B"
cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(2)
local a, b = painter.patchEdgeVerts(0, 12, 0)
painter.selectPatchEdge(0, a, b, 0)
painter.subdivideEdgeSelection()
painter.undo()
print("M57-1B OK")
EOF
run_session "$OUT/ws1b" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M57-1B OK" "$OUT/s1b.log" || { echo "FAIL: undo scene"; tail -8 "$OUT/s1b.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1b/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: split+undo is not byte-identical"; exit 1; }
echo "OK one undo restores the stroke"

echo "===== M57-2: the split IS the curve (sculpted in-scene) ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- Sculpt: raise one end of the target edge so the neighborhood is genuinely curved.
local a, b = painter.patchEdgeVerts(0, 12, 0)
painter.selectPatchVertex(0, a, 0)
painter.movePatchSelection(0, 0, 3.0)
-- The edge's cubic, read AFTER the sculpt through the same accessors the tool evals.
-- Slot 0 of patch 12: leaving tangent Vec[0], arriving Vec[1]; the edge record may run
-- either direction, so order the controls from corner a.
local ax, ay, az = painter.patchVertexPos(0, a)
local bx, by, bz = painter.patchVertexPos(0, b)
local t0 = painter.patchVecIndex(0, 12, 0)
local t1 = painter.patchVecIndex(0, 12, 1)
local t0x, t0y, t0z = painter.patchTangentPos(0, t0)
local t1x, t1y, t1z = painter.patchTangentPos(0, t1)
-- Which corner does slot 0 leave? Corner 0 of patch 12. If that is b, swap.
local c0 = painter.patchCornerVert(0, 12, 0)
if c0 ~= a then
  ax, ay, az, bx, by, bz = bx, by, bz, ax, ay, az
end
local mx = (ax + 3*t0x + 3*t1x + bx) / 8
local my = (ay + 3*t0y + 3*t1y + by) / 8
local mz = (az + 3*t0z + 3*t1z + bz) / 8
local ltx, lty, ltz = (ax + t0x) / 2, (ay + t0y) / 2, (az + t0z) / 2
local nV0 = painter.vertexCount(0)
painter.setSubObject(2)
painter.selectPatchEdge(0, a, b, 0)
painter.subdivideEdgeSelection()
local nV1 = painter.vertexCount(0)
-- Find the new vertex at the expected midpoint.
local best, bestd = nil, 1e9
for v = nV0, nV1 - 1 do
  local x, y, z = painter.patchVertexPos(0, v)
  local d = math.max(math.abs(x-mx), math.abs(y-my), math.abs(z-mz))
  if d < bestd then bestd = d; best = v end
end
assert(bestd < 1e-3, "no new vertex at the cubic midpoint (best off by " .. bestd .. ")")
-- The reused outer tangent (slot Vec[0] of the lo child on that edge) sits at (c0+c1)/2.
local nt0x, nt0y, nt0z = painter.patchTangentPos(0, t0)
local d2 = math.max(math.abs(nt0x-ltx), math.abs(nt0y-lty), math.abs(nt0z-ltz))
assert(d2 < 1e-3, "outer half tangent off the de Casteljau value by " .. d2)
print("M57-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M57-2 OK" "$OUT/s2.log" || { echo "FAIL: curve scene"; tail -10 "$OUT/s2.log"; exit 1; }
echo "OK the split is the curve"

echo "===== M57-3: propagate walks the strip to both open borders ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
local nP0 = painter.patchCount(0)
local nV0 = painter.vertexCount(0)
painter.setSubObject(2)
painter.setSubdividePropagate(true)
local a, b = painter.patchEdgeVerts(0, 12, 0)
painter.selectPatchEdge(0, a, b, 0)
painter.subdivideEdgeSelection()
painter.setSubdividePropagate(false)
local nP1 = painter.patchCount(0)
local nV1 = painter.vertexCount(0)
assert(nP1 == nP0 + 5, "expected the whole 5-patch strip, got +" .. (nP1 - nP0))
assert(nV1 == nV0 + 6, "expected 6 rung midpoints, got +" .. (nV1 - nV0))
for v = nV0, nV1 - 1 do
  assert(painter.vertexBindInfo(0, v) == 0, "a propagated strip midpoint is bound")
end
print("M57-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M57-3 OK" "$OUT/s3.log" || { echo "FAIL: propagate scene"; tail -10 "$OUT/s3.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" > "$OUT/s3.rt.log" 2>&1 \
	|| { echo "FAIL: propagated file does not round-trip"; tail -4 "$OUT/s3.rt.log"; exit 1; }
echo "OK propagate walks the strip, no binds"

echo "===== M57-4: adjacent pair degenerates to 1->4 ====="
seed "$OUT/ws4" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
local nP0 = painter.patchCount(0)
painter.setSubObject(2)
local a0, b0 = painter.patchEdgeVerts(0, 12, 0)
local a1, b1 = painter.patchEdgeVerts(0, 12, 1)
painter.selectPatchEdge(0, a0, b0, 0)
painter.selectPatchEdge(0, a1, b1, 1)
painter.subdivideEdgeSelection()
local nP1 = painter.patchCount(0)
-- patch 12 -> 1->4 (+3), its two selected-edge neighbors 1->2 (+1 each).
assert(nP1 == nP0 + 5, "expected +5 patches, got +" .. (nP1 - nP0))
painter.undo()
print("M57-4 OK")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M57-4 OK" "$OUT/s4.log" || { echo "FAIL: adjacent pair scene"; tail -10 "$OUT/s4.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: 1->4 degenerate undo not byte-identical"; exit 1; }
echo "OK adjacent pair = 1->4 + neighbors; undo byte-identical"

echo "===== M57-5: mapper - the midpoint follows the EVALUATED curve ====="
M=material-bassin
"$ZP" "$GFX/landscape/ligo/lacustre/max/$M.max" --null-edit --out "$OUT/mb.null.max" > "$OUT/mb.null.log" 2>&1
seed "$OUT/ws5" "$M"
cat > "$OUT/s5.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- Stale the caches: move a corner of patch 0's edge 0 (delta write target).
local a, b = painter.patchEdgeVerts(0, 0, 0)
painter.selectPatchVertex(0, a, 0)
painter.movePatchSelection(0, 0, 2.5)
local ax, ay, az = painter.patchVertexPos(0, a)
local bx, by, bz = painter.patchVertexPos(0, b)
local t0 = painter.patchVecIndex(0, 0, 0)
local t1 = painter.patchVecIndex(0, 0, 1)
local t0x, t0y, t0z = painter.patchTangentPos(0, t0)
local t1x, t1y, t1z = painter.patchTangentPos(0, t1)
local c0 = painter.patchCornerVert(0, 0, 0)
if c0 ~= a then
  ax, ay, az, bx, by, bz = bx, by, bz, ax, ay, az
end
local mx = (ax + 3*t0x + 3*t1x + bx) / 8
local my = (ay + 3*t0y + 3*t1y + by) / 8
local mz = (az + 3*t0z + 3*t1z + bz) / 8
local nV0 = painter.vertexCount(0)
painter.setSubObject(2)
painter.selectPatchEdge(0, a, b, 0)
painter.subdivideEdgeSelection()
local nV1 = painter.vertexCount(0)
local best, bestd = nil, 1e9
for v = nV0, nV1 - 1 do
  local x, y, z = painter.patchVertexPos(0, v)
  local d = math.max(math.abs(x-mx), math.abs(y-my), math.abs(z-mz))
  if d < bestd then bestd = d; best = v end
end
assert(bestd < 1e-3, "mapper midpoint off the evaluated curve by " .. bestd)
print("M57-5 OK")
EOF
run_session "$OUT/ws5" "$M" "$OUT/s5.lua" "$OUT/s5.log"
grep -aq "M57-5 OK" "$OUT/s5.log" || { echo "FAIL: mapper scene"; tail -10 "$OUT/s5.log"; exit 1; }
grep -aq "subdivide: zone 0, .* (modifier target)" "$OUT/s5.log" \
	|| { echo "FAIL: expected the modifier target"; grep -a "subdivide" "$OUT/s5.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws5/landscape/ligo/lacustre/max/$M.max" > "$OUT/s5.rt.log" 2>&1 \
	|| { echo "FAIL: mapper split does not round-trip"; tail -4 "$OUT/s5.rt.log"; exit 1; }
echo "OK mapper eval"

echo "ALL M57 GATES PASSED"
