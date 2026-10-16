#!/usr/bin/env bash
# M58 Bevel outline + extrude along the local normal (plan mA5 + mA6).
#
# The extrude op grows two controls: OUTLINE (the legacy Bevel's second stage - the
# island's boundary ring moves in the XY plane along each vertex's outward direction,
# negative = the classic tapered cliff) and NORMAL Local (the extrude vector becomes
# height * the selection's area-weighted eval normal - Group semantics, one selection
# one normal). The shift-drag gesture stays Z-constrained by design.
#
#  1. BEVEL AT ZERO == EXTRUDE: extrudePatchSelection(8) and (8, 0, false) save
#     byte-identical files.
#  2. OUTLINE GEOMETRY (patch 12 of zonematerial-bassin-1, h=8 outline=-2): the four
#     island corners land at pre + (0,0,8) - 2 * outward, where outward is computed
#     INDEPENDENTLY in the scene (average of the two adjacent ring edges' XY normals,
#     oriented away from the patch centroid); 4 walls; undo byte-identity; round-trip.
#  3. LOCAL NORMAL: extrude a freshly built WALL along its own normal - the wall's
#     island moves HORIZONTALLY (|dxy| = h, |dz| small), not up.
#  4. REGRESSION: the whole m54 extrude gate re-runs green (the Z path unchanged).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
E2E="$(cd "$(dirname "$0")" && pwd)"
OUT=/tmp/zp_ui/m58_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M58-1: bevel with no outline == plain extrude, byte for byte ====="
seed "$OUT/wsA" "$B"
cat > "$OUT/sA.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.extrudePatchSelection(8)
print("M58-A OK")
EOF
run_session "$OUT/wsA" "$B" "$OUT/sA.lua" "$OUT/sA.log"
grep -aq "M58-A OK" "$OUT/sA.log" || { echo "FAIL: plain extrude"; tail -6 "$OUT/sA.log"; exit 1; }
seed "$OUT/wsB" "$B"
cat > "$OUT/sB.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.extrudePatchSelection(8, 0, false)
print("M58-B OK")
EOF
run_session "$OUT/wsB" "$B" "$OUT/sB.lua" "$OUT/sB.log"
grep -aq "M58-B OK" "$OUT/sB.log" || { echo "FAIL: zero-outline extrude"; tail -6 "$OUT/sB.log"; exit 1; }
cmp -s "$OUT/wsA/landscape/ligo/lacustre/max/$B.max" "$OUT/wsB/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: outline 0 differs from the plain extrude"; exit 1; }
echo "OK outline 0 == plain"

echo "===== M58-2: outline geometry against an independent computation ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local P = 12
local H, OUTL = 8.0, -2.0
-- Pre-op ring corners and the independent outward rule: per corner, average of its two
-- adjacent ring edges' XY normals, oriented away from the patch's XY centroid.
local cx, cy = 0, 0
local vx, vy, vz = {}, {}, {}
for c = 0, 3 do
  local v = painter.patchCornerVert(0, P, c)
  vx[c], vy[c], vz[c] = painter.patchVertexPos(0, v)
  cx = cx + vx[c] / 4
  cy = cy + vy[c] / 4
end
local function edgeNormal(c)
  -- edge c runs corner c -> (c+1)%4
  local d = (c + 1) % 4
  local ex, ey = vx[d] - vx[c], vy[d] - vy[c]
  local nx, ny = -ey, ex
  local l = math.sqrt(nx*nx + ny*ny)
  nx, ny = nx / l, ny / l
  local mx, my = (vx[c] + vx[d]) / 2, (vy[c] + vy[d]) / 2
  if nx * (cx - mx) + ny * (cy - my) > 0 then nx, ny = -nx, -ny end
  return nx, ny
end
local expect = {}
for c = 0, 3 do
  local n1x, n1y = edgeNormal(c)           -- leaving edge
  local n0x, n0y = edgeNormal((c + 3) % 4) -- arriving edge
  local sx, sy = n1x + n0x, n1y + n0y
  local l = math.sqrt(sx*sx + sy*sy)
  sx, sy = sx / l, sy / l
  expect[c] = { vx[c] + OUTL * sx, vy[c] + OUTL * sy, vz[c] + H }
end
local nP0 = painter.patchCount(0)
local nV0 = painter.vertexCount(0)
painter.selectPatchFace(0, P, 0)
painter.extrudePatchSelection(H, OUTL, false)
assert(painter.patchCount(0) == nP0 + 4, "an interior island grows 4 walls")
local nV1 = painter.vertexCount(0)
assert(nV1 == nV0 + 4, "an interior island duplicates its 4 ring corners")
for c = 0, 3 do
  local best = 1e9
  for i = nV0, nV1 - 1 do
    local x, y, z = painter.patchVertexPos(0, i)
    local e = expect[c]
    local d = math.max(math.abs(x-e[1]), math.abs(y-e[2]), math.abs(z-e[3]))
    if d < best then best = d end
  end
  assert(best < 2e-3, "outlined corner " .. c .. " off the rule by " .. best)
end
print("M58-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M58-2 OK" "$OUT/s2.log" || { echo "FAIL: outline geometry"; tail -10 "$OUT/s2.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" > "$OUT/s2.rt.log" 2>&1 \
	|| { echo "FAIL: bevel file does not round-trip"; tail -4 "$OUT/s2.rt.log"; exit 1; }
echo "OK outlined ring at the exact in-plane offset"

seed "$OUT/ws2b" "$B"
cat > "$OUT/s2b.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.extrudePatchSelection(8, -2, false)
painter.undo()
print("M58-2B OK")
EOF
run_session "$OUT/ws2b" "$B" "$OUT/s2b.lua" "$OUT/s2b.log"
grep -aq "M58-2B OK" "$OUT/s2b.log" || { echo "FAIL: bevel undo"; tail -6 "$OUT/s2b.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws2b/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: bevel undo is not byte-identical"; exit 1; }
echo "OK one undo restores the bevel"

echo "===== M58-3: local normal - a wall extrudes sideways ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local nP0 = painter.patchCount(0)
painter.selectPatchFace(0, 12, 0)
painter.extrudePatchSelection(8)
-- The first wall patch built by the extrude.
local W = nP0
local wx, wy, wz = {}, {}, {}
for c = 0, 3 do
  local v = painter.patchCornerVert(0, W, c)
  wx[c], wy[c], wz[c] = painter.patchVertexPos(0, v)
end
local nV0 = painter.vertexCount(0)
painter.setSubObject(3)
painter.selectPatchFace(0, W, 0)
painter.extrudePatchSelection(4, 0, true)
local nV1 = painter.vertexCount(0)
assert(nV1 > nV0, "the wall extrude created no ring copies")
-- Every new corner sits 4 m from SOME pre-op wall corner, nearly horizontally.
for i = nV0, nV1 - 1 do
  local x, y, z = painter.patchVertexPos(0, i)
  local ok = false
  for c = 0, 3 do
    local dx, dy, dz = x - wx[c], y - wy[c], z - wz[c]
    local d = math.sqrt(dx*dx + dy*dy + dz*dz)
    if math.abs(d - 4) < 0.3 and math.abs(dz) < 1.0 then ok = true end
  end
  assert(ok, "wall ring copy " .. i .. " did not move 4 m sideways")
end
print("M58-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M58-3 OK" "$OUT/s3.log" || { echo "FAIL: local normal"; tail -10 "$OUT/s3.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" > "$OUT/s3.rt.log" 2>&1 \
	|| { echo "FAIL: wall-extrude file does not round-trip"; tail -4 "$OUT/s3.rt.log"; exit 1; }
echo "OK the wall's island moved along its normal"

echo "===== M58-4: the m54 extrude gate re-runs green (Z path unchanged) ====="
bash "$E2E/m54_topo_extrude.sh" > "$OUT/m54.log" 2>&1 \
	|| { echo "FAIL: m54 regression"; tail -10 "$OUT/m54.log"; exit 1; }
echo "OK m54 regression"

echo "ALL M58 GATES PASSED"
