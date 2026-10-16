#!/usr/bin/env bash
# M54 extrude: the legacy Extrude recomposed from the tool's proven pieces - the boundary
# splits exactly as detach-to-element does it, WALLS bridge the two rings (one quad per
# shared boundary edge, vertical edges shared between adjacent walls), and the island
# translates by the height. One Kind 6 stroke, one undo.
#
#  1. THE SHAPE (zonematerial-bassin-1, patch 5: three shared edges + the open west zone
#     border): extrude by 8 adds exactly 3 walls (open border edges rise WITHOUT a wall -
#     the ligo border profile is a cross-file contract); the island ring rises by 8 while
#     the original boundary ring stays; wall tiling = horizontal order 4 (lines up with
#     the top patch) x vertical order 2 (8 m at 2 m/tile); the island keeps its painted
#     interior, the walls start EMPTY; persists across save + reopen; the file
#     round-trips the encoder; ONE undo restores the null-edit baseline byte for byte.
#  2. THE DRAG (extrudeDragAt, the shift-drag's machinery): an upward drag on the
#     selection extrudes by the dragged height - walls appear, count grows.
#  3. MAPPER (material-bassin): extrude follows the Tier A rule - mapped island elements
#     move through their DELTAS; the island rises in eval, undo is byte-identical.
#  4. REFUSALS: zero height; the whole zone; nothing changes on either.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m54_out
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

echo "===== M54-1: walls, heights, tiling, paint, persistence, round-trip ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local p0 = painter.patchCount(0)
local t0 = painter.tileAt(0, 5, 1, 1)
-- Pre-op ring corners of patch 5 (they become the island's duplicates after the split).
local _, _, z6 = painter.patchVertexPos(0, 6)
local _, _, z12 = painter.patchVertexPos(0, 12)
painter.selectPatchFace(0, 5, 1)
assert(painter.extrudePatchSelection(8) == 1, "extrude failed")
-- Three shared edges wall; the open west border rises without one.
assert(painter.patchCount(0) == p0 + 3, "expected exactly 3 walls")
-- The island: patch 5's ring corners are now the duplicates, risen by 8.
local a, b = painter.patchEdgeVerts(0, 5, 0)
local _, _, za = painter.patchVertexPos(0, a)
assert(math.abs(za - (z6 + 8)) < 2.1, "island did not rise by 8") -- corner heights vary; loose z ref
-- Exact: every ring corner sits exactly 8 above SOME original position; check corner 6's
-- duplicate against its own original, which STAYS at z6.
local _, _, z6b = painter.patchVertexPos(0, 6)
assert(math.abs(z6b - z6) < 1e-5, "the original boundary ring moved")
-- Ring corner z deltas: reconstruct the ring and assert each corner is +8 over its
-- pre-op counterpart by matching x, y.
local function ring(z, p)
  local prs = {}
  for e = 0, 3 do local x, y = painter.patchEdgeVerts(z, p, e); prs[e] = { x, y } end
  local rv = {}
  for k = 0, 3 do
    for _, x in ipairs(prs[(k + 3) % 4]) do
      for _, y in ipairs(prs[k]) do if x == y then rv[k] = x end end
    end
  end
  return rv
end
local rv = ring(0, 5)
for k = 0, 3 do
  local x, y, z = painter.patchVertexPos(0, rv[k])
  local found = false
  for _, ov in ipairs({ 6, 7, 13, 12 }) do
    local ox, oy, oz = painter.patchVertexPos(0, ov)
    if math.abs(x - ox) < 1e-4 and math.abs(y - oy) < 1e-4 then
      assert(math.abs(z - (oz + 8)) < 1e-4, "island corner not +8 over its original")
      found = true
    end
  end
  assert(found, "island corner has no original beneath it")
end
-- Wall tiling: horizontal 4 (matches the top patch), vertical 2 (8 m at 2 m/tile).
for w = p0, p0 + 2 do
  local u, v = painter.patchTess(0, w)
  assert(u == 2 and v == 4, string.format("wall %d tess %dx%d, expected 2x4", w, u, v))
  assert(painter.tileAt(0, w, 0, 0) == -1, "wall did not start empty")
end
-- Paint survival on the island.
assert(painter.tileAt(0, 5, 1, 1) == t0, "island interior lost its paint")
print("M54-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M54-1 OK" "$OUT/s1.log" || { echo "FAIL: extrude shape"; tail -8 "$OUT/s1.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1.rt.log" 2>&1 \
	|| { echo "FAIL: extruded file does not round-trip"; tail -4 "$OUT/s1.rt.log"; exit 1; }
cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
local _, _, z6 = painter.patchVertexPos(0, 6)
local a = select(1, painter.patchEdgeVerts(0, 5, 0))
local _, _, za = painter.patchVertexPos(0, a)
assert(za > z6 + 5, "the extrusion did not persist")
print("M54-1B OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M54-1B OK" "$OUT/s1b.log" || { echo "FAIL: persistence"; tail -6 "$OUT/s1b.log"; exit 1; }
echo "OK shape + tiling + persistence + round-trip"

echo "===== M54-1c: ONE undo restores the baseline byte for byte ====="
seed "$OUT/ws1c" "$B"
cat > "$OUT/s1c.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.extrudePatchSelection(8) == 1)
painter.undo()
print("M54-1C OK")
EOF
run_session "$OUT/ws1c" "$B" "$OUT/s1c.lua" "$OUT/s1c.log"
grep -aq "M54-1C OK" "$OUT/s1c.log" || { echo "FAIL: undo script"; tail -6 "$OUT/s1c.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1c/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: extrude undo is not byte-identical"; exit 1; }
echo "OK one undo, byte-identical"

echo "===== M54-2: the drag machinery extrudes by the dragged height ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local p0 = painter.patchCount(0)
painter.selectPatchFace(0, 5, 1)
-- Press at the pivot's screen spot, drag upward; the Z-constrained drag commits extrude.
local px, py, pz = painter.pivotPos()
local sx, sy = painter.vertexScreenPos(0, 6) -- any on-screen anchor for the press
assert(painter.extrudeDragAt(sx, sy, sx, sy + 0.2))
assert(painter.patchCount(0) == p0 + 3, "drag did not extrude (walls missing)")
local a = select(1, painter.patchEdgeVerts(0, 5, 0))
local _, _, za = painter.patchVertexPos(0, a)
local _, _, z6 = painter.patchVertexPos(0, 6)
assert(za > z6 + 0.5, "drag height did not apply upward")
print("M54-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M54-2 OK" "$OUT/s2.log" || { echo "FAIL: extrude drag"; tail -8 "$OUT/s2.log"; exit 1; }
echo "OK drag extrudes"

echo "===== M54-3: mapper mesh - the island rises in eval, undo byte-identical ====="
M=material-bassin
"$ZP" "$GFX/landscape/ligo/lacustre/max/$M.max" --null-edit --out "$OUT/$M.null.max" > "$OUT/null2.log" 2>&1
seed "$OUT/ws3" "$M"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
-- First patch that extrudes (some refuse: whole-element islands, bind involvement).
local n = painter.patchCount(0)
local done = false
for p = 0, n - 1 do
  painter.setSubObject(3)
  painter.selectPatchFace(0, p, 1)
  local a, b = painter.patchEdgeVerts(0, p, 0)
  local _, _, zPre = painter.patchVertexPos(0, a)
  if painter.extrudePatchSelection(5) == 1 then
    local a2 = select(1, painter.patchEdgeVerts(0, p, 0))
    local _, _, zPost = painter.patchVertexPos(0, a2)
    assert(math.abs(zPost - (zPre + 5)) < 1e-3, "mapper island did not rise in eval")
    done = true
    break
  end
end
assert(done, "no patch of the mapper mesh extruded")
painter.undo()
print("M54-3 OK")
EOF
run_session "$OUT/ws3" "$M" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M54-3 OK" "$OUT/s3.log" || { echo "FAIL: mapper extrude"; tail -8 "$OUT/s3.log"; exit 1; }
cmp -s "$OUT/$M.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$M.max" \
	|| { echo "FAIL: mapper extrude undo not byte-identical"; exit 1; }
echo "OK mapper deltas + undo"

echo "===== M54-4: refusals change nothing ====="
seed "$OUT/ws4" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.extrudePatchSelection(0) == 0, "zero height must refuse")
painter.setSubObject(3)
local n = painter.patchCount(0)
for p = 0, n - 1 do painter.selectPatchFace(0, p, 1) end
assert(painter.extrudePatchSelection(8) == 0, "whole zone must refuse")
print("M54-4 OK")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M54-4 OK" "$OUT/s4.log" || { echo "FAIL: refusal script"; tail -6 "$OUT/s4.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: a refusal changed the file"; exit 1; }
echo "OK refusals leave the file alone"

echo "ALL M54 GATES PASSED"
