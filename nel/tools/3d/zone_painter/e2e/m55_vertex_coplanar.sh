#!/usr/bin/env bash
# M55 Vertex continuity type (the legacy Coplanar / Corner pair, vertex level; plan mA1).
#
# The type is bit 0 of the PatchMesh vertex Flags word (PVERT_COPLANAR; corner = 0),
# pinned EMPIRICALLY over the whole corpus before the op existed (--pm-flags-probe:
# vertex Flags carries only 0/1, and every flagged vertex's tangent directions measure
# coplanar within 1e-6 while unflagged ones spread to ~0.5). The toggle is a value op
# through the shared topo runner; the CONSTRAINT applies at handle-move time - a handle
# moving on a coplanar vertex re-aims the vertex's unmoving handles onto the tilted
# plane (minimal tilt containing the moved direction), each keeping its own length.
#
#  1. TOGGLE (zonematerial-bassin-1, base target): coplanar -> corner on one vertex
#     changes exactly ONE byte; undo restores the null-edit baseline byte for byte;
#     the flag survives save + reopen; the edited file round-trips the encoder.
#  2. REFUSAL: setting the type a vertex already has refuses and writes nothing.
#  3. CONSTRAINT: a handle move on an authored-coplanar vertex re-aims its siblings -
#     asserted against an INDEPENDENT Lua computation of the rule from the pre-move
#     positions (plane from best tangent pair-cross, minimal tilt, length kept).
#     Siblings perpendicular to the tilt stay put (negligible re-aim writes nothing).
#  4. CORNER NEGATIVE: the same move after switching the vertex to corner moves ONLY
#     the picked handle; siblings hold their positions.
#  5. MODIFIER TARGET (material-fond): the value op routes to the modifier stream where
#     one exists (the runner's target line, the m34/m52 discipline).
#  6. BOUND REFUSAL (ilot_croix): a bound vertex's handles refuse to move at all -
#     unchanged by the constraint machinery.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m55_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }
run_verify() { $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M55-1: toggle - one byte, undo to baseline, persistence, round-trip ====="
# Fixture fact (discovered once, pinned): bassin-1 vertex 1 is authored COPLANAR (flag 1)
# with three attached tangents (vecs 1, 70, 2 - west, north, east of a border corner).
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
assert(painter.patchVertFlags(0, 1) == 1, "fixture: vertex 1 is coplanar")
painter.selectPatchVertex(0, 1, 0)
painter.setVertexCoplanar(false)
assert(painter.patchVertFlags(0, 1) == 0, "corner did not stick")
print("M55-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M55-1 OK" "$OUT/s1.log" || { echo "FAIL: toggle scene"; tail -8 "$OUT/s1.log"; exit 1; }
grep -aq "vertex type: zone 0, 1 patches (base target)" "$OUT/s1.log" \
	|| { echo "FAIL: expected the base target line"; grep -a "vertex type" "$OUT/s1.log"; exit 1; }
# cmp -l exits 1 when the files differ, which is the expected case here - brace it.
NDIFF=$( { cmp -l "$OUT/base.null.max" "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$NDIFF" == "1" ]] || { echo "FAIL: expected exactly 1 byte, got $NDIFF"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1.rt.log" 2>&1 \
	|| { echo "FAIL: edited file does not round-trip"; tail -4 "$OUT/s1.rt.log"; exit 1; }
echo "OK toggle = 1 byte + round-trip"

cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
assert(painter.patchVertFlags(0, 1) == 0, "the corner flag did not survive reopen")
print("M55-1B OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M55-1B OK" "$OUT/s1b.log" || { echo "FAIL: persistence"; tail -6 "$OUT/s1b.log"; exit 1; }

seed "$OUT/ws1c" "$B"
cat > "$OUT/s1c.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.selectPatchVertex(0, 1, 0)
painter.setVertexCoplanar(false)
painter.undo()
assert(painter.patchVertFlags(0, 1) == 1, "undo did not restore the flag")
print("M55-1C OK")
EOF
run_session "$OUT/ws1c" "$B" "$OUT/s1c.lua" "$OUT/s1c.log"
grep -aq "M55-1C OK" "$OUT/s1c.log" || { echo "FAIL: undo scene"; tail -8 "$OUT/s1c.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1c/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: toggle undo is not byte-identical"; exit 1; }
echo "OK persistence + undo byte-identical"

echo "===== M55-2: no-change refusal writes nothing ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.selectPatchVertex(0, 1, 0)
painter.setVertexCoplanar(true) -- already coplanar
print("M55-2 DONE")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M55-2 DONE" "$OUT/s2.log" || { echo "FAIL: refusal scene"; tail -6 "$OUT/s2.log"; exit 1; }
grep -aq "already coplanar" "$OUT/s2.log" || { echo "FAIL: no refusal printed"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: refused toggle changed the file"; exit 1; }
echo "OK refusal leaves the file alone"

echo "===== M55-3: the constraint - siblings re-aim onto the tilted plane ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local V = 1
assert(painter.patchVertFlags(0, V) == 1, "fixture: vertex 1 is coplanar")
-- Attached tangents from the patch table, exactly as the tool derives them (the ring
-- corner comes from patchCornerVert - the stored edge records' V1/V2 order is arbitrary).
local function attached(v)
  local out, seen = {}, {}
  for p = 0, painter.patchCount(0) - 1 do
    for j = 0, 7 do
      local corner
      if j % 2 == 1 then corner = (math.floor(j / 2) + 1) % 4 else corner = math.floor(j / 2) end
      if painter.patchCornerVert(0, p, corner) == v then
        local ok, vec = pcall(painter.patchVecIndex, 0, p, j)
        if ok and vec and not seen[vec] then seen[vec] = true; table.insert(out, vec) end
      end
    end
  end
  return out
end
local att = attached(V)
assert(#att >= 3, "fixture: vertex 1 has 3+ handles")
local moveVec = att[1]
local cx, cy, cz = painter.patchVertexPos(0, V)
local pos = {}
for _, t in ipairs(att) do
  local x, y, z = painter.patchTangentPos(0, t)
  pos[t] = { x, y, z }
end
-- INDEPENDENT computation of the rule from the pre-move positions: current plane =
-- largest pair-cross of unit tangent directions; new plane = minimal tilt containing
-- the moved direction; siblings keep length, re-aim into the plane.
local function sub(a, b) return { a[1]-b[1], a[2]-b[2], a[3]-b[3] } end
local function norm(a) return math.sqrt(a[1]^2 + a[2]^2 + a[3]^2) end
local function scale(a, s) return { a[1]*s, a[2]*s, a[3]*s } end
local function dot(a, b) return a[1]*b[1] + a[2]*b[2] + a[3]*b[3] end
local function cross(a, b) return { a[2]*b[3]-a[3]*b[2], a[3]*b[1]-a[1]*b[3], a[1]*b[2]-a[2]*b[1] } end
local C = { cx, cy, cz }
local dirs = {}
for _, t in ipairs(att) do
  local d = sub(pos[t], C)
  dirs[t] = scale(d, 1 / norm(d))
end
local n0, best = nil, -1
for i = 1, #att do
  for j = i + 1, #att do
    local c = cross(dirs[att[i]], dirs[att[j]])
    local m = norm(c)
    if m > best then best = m; n0 = scale(c, 1 / m) end
  end
end
assert(best > 1e-6, "fixture plane is degenerate")
local D = { 0, 0, 2.0 }
local movedNew = { pos[moveVec][1] + D[1], pos[moveVec][2] + D[2], pos[moveVec][3] + D[3] }
local md = sub(movedNew, C)
md = scale(md, 1 / norm(md))
local t = dot(n0, md)
local n1 = { n0[1] - t*md[1], n0[2] - t*md[2], n0[3] - t*md[3] }
n1 = scale(n1, 1 / norm(n1))
local expect = {}
for _, tv in ipairs(att) do
  if tv ~= moveVec then
    local v = sub(pos[tv], C)
    local L = norm(v)
    local d2 = dot(v, n1)
    local u = { v[1] - d2*n1[1], v[2] - d2*n1[2], v[3] - d2*n1[3] }
    local um = norm(u)
    assert(um > 1e-9, "degenerate sibling")
    expect[tv] = { C[1] + u[1]*L/um, C[2] + u[2]*L/um, C[3] + u[3]*L/um }
  end
end
painter.selectPatchVertex(0, V, 0)
painter.selectPatchTangent(0, moveVec, 0)
painter.movePatchSelection(D[1], D[2], D[3])
local reaimed, held = 0, 0
for _, tv in ipairs(att) do
  if tv ~= moveVec then
    local x, y, z = painter.patchTangentPos(0, tv)
    local e = expect[tv]
    local err = math.max(math.abs(x-e[1]), math.abs(y-e[2]), math.abs(z-e[3]))
    assert(err < 2e-3, "sibling " .. tv .. " off the rule by " .. err)
    local o = pos[tv]
    if math.max(math.abs(x-o[1]), math.abs(y-o[2]), math.abs(z-o[3])) > 1e-4 then
      reaimed = reaimed + 1
    else
      held = held + 1
    end
    -- length preserved
    local L0 = norm(sub(o, C))
    local L1 = norm(sub({x,y,z}, C))
    assert(math.abs(L1 - L0) < 2e-3, "sibling " .. tv .. " changed length")
  end
end
assert(reaimed >= 1, "no sibling re-aimed - the constraint did nothing")
print("M55-3 reaimed=" .. reaimed .. " held=" .. held)
print("M55-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M55-3 OK" "$OUT/s3.log" || { echo "FAIL: constraint scene"; tail -10 "$OUT/s3.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" > "$OUT/s3.rt.log" 2>&1 \
	|| { echo "FAIL: constrained-move file does not round-trip"; tail -4 "$OUT/s3.rt.log"; exit 1; }
echo "OK constraint matches the independent rule; file round-trips"

echo "===== M55-3b: constrained move + undo = baseline ====="
seed "$OUT/ws3b" "$B"
cat > "$OUT/s3b.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.selectPatchVertex(0, 1, 0)
painter.selectPatchTangent(0, 1, 0)
painter.movePatchSelection(0, 0, 2.0)
painter.undo()
print("M55-3B OK")
EOF
run_session "$OUT/ws3b" "$B" "$OUT/s3b.lua" "$OUT/s3b.log"
grep -aq "M55-3B OK" "$OUT/s3b.log" || { echo "FAIL: undo scene"; tail -8 "$OUT/s3b.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws3b/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: constrained move undo is not byte-identical"; exit 1; }
echo "OK one undo restores the whole stroke"

echo "===== M55-4: corner vertex - the same move leaves siblings alone ====="
seed "$OUT/ws4" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local V = 1
painter.selectPatchVertex(0, V, 0)
painter.setVertexCoplanar(false)
painter.setSubObject(1)
-- vec 2 is the sibling the COPLANAR constraint re-aims (M55-3); after switching the
-- vertex to corner the same move must leave it exactly in place.
local sx, sy, sz = painter.patchTangentPos(0, 2)
painter.selectPatchVertex(0, V, 0)
painter.selectPatchTangent(0, 1, 0)
painter.movePatchSelection(0, 0, 2.0)
local x, y, z = painter.patchTangentPos(0, 2)
assert(math.abs(x-sx) < 1e-6 and math.abs(y-sy) < 1e-6 and math.abs(z-sz) < 1e-6,
       "a corner vertex's sibling moved")
print("M55-4 OK")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M55-4 OK" "$OUT/s4.log" || { echo "FAIL: corner negative"; tail -8 "$OUT/s4.log"; exit 1; }
grep -aq "patch move: 1 elements" "$OUT/s4.log" \
	|| { echo "FAIL: corner move should write exactly the picked handle"; grep -a "patch move" "$OUT/s4.log"; exit 1; }
echo "OK corner type frees the handles"

echo "===== M55-5: modifier-stream target ====="
M=material-fond
seed "$OUT/ws5" "$M"
cat > "$OUT/s5.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- Flip vertex 0 to the OPPOSITE of its authored type (either direction proves the
-- write; the fixture only has to have a vertex).
local f0 = painter.patchVertFlags(0, 0)
painter.selectPatchVertex(0, 0, 0)
painter.setVertexCoplanar(f0 == 0)
local f1 = painter.patchVertFlags(0, 0)
assert(f1 ~= f0, "flag did not flip on the modifier target")
print("M55-5 OK")
EOF
run_session "$OUT/ws5" "$M" "$OUT/s5.lua" "$OUT/s5.log"
grep -aq "M55-5 OK" "$OUT/s5.log" || { echo "FAIL: modifier scene"; tail -8 "$OUT/s5.log"; exit 1; }
grep -aq "vertex type: zone 0, 1 patches (modifier target)" "$OUT/s5.log" \
	|| { echo "FAIL: expected the modifier target"; grep -a "vertex type" "$OUT/s5.log"; exit 1; }
echo "OK modifier target"

echo "===== M55-6: bound vertex handles still refuse (ilot_croix) ====="
I=zonematerial-bassin-ilot_croix
"$ZP" "$GFX/landscape/ligo/lacustre/max/$I.max" --null-edit --out "$OUT/ilot.null.max" > "$OUT/ilot.null.log" 2>&1
seed "$OUT/ws6" "$I"
cat > "$OUT/s6.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- v445 is bound (the m40 fixture fact). Selecting it and moving must refuse.
painter.selectPatchVertex(0, 445, 0)
painter.movePatchSelection(0, 0, 2.0)
print("M55-6 DONE")
EOF
run_session "$OUT/ws6" "$I" "$OUT/s6.lua" "$OUT/s6.log"
grep -aq "M55-6 DONE" "$OUT/s6.log" || { echo "FAIL: bound scene"; tail -6 "$OUT/s6.log"; exit 1; }
grep -aq "bound vertices follow their edge" "$OUT/s6.log" \
	|| { echo "FAIL: bound refusal missing"; grep -a "patch move" "$OUT/s6.log"; exit 1; }
cmp -s "$OUT/ilot.null.max" "$OUT/ws6/landscape/ligo/lacustre/max/$I.max" \
	|| { echo "FAIL: refused bound move changed the file"; exit 1; }
echo "OK bound refusal unchanged"

echo "ALL M55 GATES PASSED"
