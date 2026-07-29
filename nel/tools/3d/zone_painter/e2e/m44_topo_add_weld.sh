#!/usr/bin/env bash
# M44 add-quad + weld gates: growing surface from an open edge, and the stitch - closing
# the loop on the practical authoring cycle.
#
#  1a. ADD QUAD refusals (zonematerial-bassin-ilot_croix): a SHARED edge (6-7) refuses,
#      and so does a BIND-TARGET edge (7-12 is vertex 503's target - growing it would
#      bury the T-junction).
#  1b. The grow scene (zonematerial-bassin-1, a DISCOVERED open border edge): one new
#      patch with the owner's tile orders and EMPTY tiles (fresh paintable surface), two
#      new vertices; persists across save+reopen; the saved file round-trips the encoder.
#  2. THE FULL AUTHORING LOOP (zonematerial-bassin-1): delete a patch (hole), add a quad
#     on a rim edge (the mirror seed lands roughly in the hole), move the two fresh
#     corners exactly onto the opposite rim corners, weld with a tight threshold - the
#     coincident pairs merge, the side and far edges FUSE with the rim, and the zone is
#     whole again: patch count and vertex count both back to the originals. The stitched
#     file round-trips the encoder, and the whole FIVE-STROKE undo chain (delete, add,
#     move x2, weld) restores the null-edit baseline byte for byte.
#  3. Weld refusal: welding a bound vertex refuses (release the bind first).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m44_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }
run_verify() { $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

echo "===== M44-1a: add-quad refusals (shared edge; bind-target edge) ====="
B1=zonematerial-bassin-ilot_croix
cat > "$OUT/addref.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(2)
local c0 = painter.patchCount(0)
painter.selectPatchEdge(0, 6, 7, 0)
assert(painter.addQuadPatchSelection() == 0, "shared edge was grown")
painter.selectPatchEdge(0, 7, 12, 0)
assert(painter.addQuadPatchSelection() == 0, "bind-target edge was grown")
assert(painter.patchCount(0) == c0)
print("M44-1a OK")
EOF
seed "$OUT/ws1a" "$B1"
run_session "$OUT/ws1a" "$B1" "$OUT/addref.lua" "$OUT/g1a.log"
grep -aq "M44-1a OK" "$OUT/g1a.log" || { echo "FAIL: add refusals"; tail -6 "$OUT/g1a.log"; exit 1; }
grep -aq "not open" "$OUT/g1a.log" || { echo "FAIL: expected the not-open refusal"; exit 1; }
grep -aq "bind target" "$OUT/g1a.log" || { echo "FAIL: expected the bind-target refusal"; exit 1; }
echo "OK add refusals"

echo "===== M44-1b: grow a discovered open border edge ====="
B1B=zonematerial-bassin-1
cat > "$OUT/add.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(2)
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
local grown = false
for p = 0, c0 - 1 do
  for s = 0, 3 do
    local a, b = painter.patchEdgeVerts(0, p, s)
    if a then
      painter.selectPatchEdge(0, a, b, 0)
      if painter.addQuadPatchSelection() == 1 then grown = true; break end
    end
  end
  if grown then break end
end
assert(grown, "no open edge grew")
assert(painter.patchCount(0) == c0 + 1, "count")
assert(painter.vertexCount(0) == v0 + 2, "verts")
local t, r, n = painter.tileAt(0, c0, 0, 0)
assert(n == 0, "new patch tiles not empty")
print("M44-1b OK count=" .. (c0 + 1))
EOF
seed "$OUT/ws1" "$B1B"
run_session "$OUT/ws1" "$B1B" "$OUT/add.lua" "$OUT/g1.log"
C1=$(grep -a "M44-1b OK" "$OUT/g1.log" | sed 's/.*count=//')
[[ -n "$C1" ]] || { echo "FAIL: add script"; tail -8 "$OUT/g1.log"; exit 1; }
cat > "$OUT/add_v.lua" <<EOF
assert(painter.patchCount(0) == $C1, "reopen count")
print("M44-1b reopen OK")
EOF
run_verify "$OUT/ws1" "$B1B" "$OUT/add_v.lua" "$OUT/g1v.log"
grep -aq "M44-1b reopen OK" "$OUT/g1v.log" || { echo "FAIL: add did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B1B.max" > "$OUT/g1.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on grown file"; tail -4 "$OUT/g1.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1.pm.log" || { echo "FAIL: pm identity on grown file"; exit 1; }
echo "OK add quad (fresh surface, persisted, round-trips)"

echo "===== M44-2: the full authoring loop - delete, add, move, weld; undo chain ====="
B2=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B2.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1
cat > "$OUT/loop.lua" <<'EOF'
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
assert(painter.patchCount(0) == c0)
assert(painter.vertexCount(0) == v0 + 2)
local nA, nB = v0, v0 + 1
painter.setSubObject(1)
for _, nv in ipairs({nA, nB}) do
  local x, y, z = painter.patchVertexPos(0, nv)
  local best, bd = nil, nil
  for c, p in pairs(cpos) do
    local d = (p[1]-x)^2 + (p[2]-y)^2 + (p[3]-z)^2
    if not bd or d < bd then bd = d; best = c end
  end
  painter.clearPatchVertexSelection()
  painter.selectPatchVertex(0, nv, 1)
  local p = cpos[best]
  painter.movePatchSelection(p[1] - x, p[2] - y, p[3] - z)
end
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, nA, 1)
painter.selectPatchVertex(0, nB, 1)
for c in pairs(corners) do painter.selectPatchVertex(0, c, 1) end
assert(painter.weldPatchSelection(0.01) > 0, "weld refused")
assert(painter.patchCount(0) == c0, "stitched count")
assert(painter.vertexCount(0) == v0, "stitched verts")
print("M44-2 stitched OK")
-- the whole five-stroke chain back to the baseline
for i = 1, 5 do painter.undo() end
assert(painter.patchCount(0) == c0)
assert(painter.vertexCount(0) == v0)
print("M44-2 undone OK")
EOF
seed "$OUT/ws2" "$B2"
run_session "$OUT/ws2" "$B2" "$OUT/loop.lua" "$OUT/g2.log"
grep -aq "M44-2 stitched OK" "$OUT/g2.log" || { echo "FAIL: loop script"; tail -10 "$OUT/g2.log"; exit 1; }
grep -aq "M44-2 undone OK" "$OUT/g2.log" || { echo "FAIL: loop undo"; tail -6 "$OUT/g2.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B2.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: loop undo chain left $U bytes"; exit 1; }
echo "OK the full loop stitches and the undo chain is byte-identical"

echo "===== M44-2b: the stitched file itself round-trips and persists ====="
cat > "$OUT/loop2.lua" <<'EOF'
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
local nA, nB = v0, v0 + 1
painter.setSubObject(1)
for _, nv in ipairs({nA, nB}) do
  local x, y, z = painter.patchVertexPos(0, nv)
  local best, bd = nil, nil
  for c, p in pairs(cpos) do
    local d = (p[1]-x)^2 + (p[2]-y)^2 + (p[3]-z)^2
    if not bd or d < bd then bd = d; best = c end
  end
  painter.clearPatchVertexSelection()
  painter.selectPatchVertex(0, nv, 1)
  local p = cpos[best]
  painter.movePatchSelection(p[1] - x, p[2] - y, p[3] - z)
end
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, nA, 1)
painter.selectPatchVertex(0, nB, 1)
for c in pairs(corners) do painter.selectPatchVertex(0, c, 1) end
painter.weldPatchSelection(0.01)
print("M44-2b done c=" .. painter.patchCount(0) .. " v=" .. painter.vertexCount(0))
EOF
seed "$OUT/ws2b" "$B2"
run_session "$OUT/ws2b" "$B2" "$OUT/loop2.lua" "$OUT/g2b.log"
CV=$(grep -a "M44-2b done" "$OUT/g2b.log" | sed 's/.*done //')
[[ -n "$CV" ]] || { echo "FAIL: loop2 script"; tail -8 "$OUT/g2b.log"; exit 1; }
cat > "$OUT/loop2_v.lua" <<EOF
print("M44-2b reopen c=" .. painter.patchCount(0) .. " v=" .. painter.vertexCount(0))
EOF
run_verify "$OUT/ws2b" "$B2" "$OUT/loop2_v.lua" "$OUT/g2bv.log"
CV2=$(grep -a "M44-2b reopen" "$OUT/g2bv.log" | sed 's/.*reopen //')
[[ "$CV" == "$CV2" ]] || { echo "FAIL: stitched file changed on reopen ($CV vs $CV2)"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws2b/landscape/ligo/lacustre/max/$B2.max" > "$OUT/g2b.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on stitched file"; tail -4 "$OUT/g2b.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g2b.pm.log" || { echo "FAIL: pm identity on stitched file"; exit 1; }
echo "OK stitched file persists and round-trips"

echo "===== M44-3: welding a bound vertex refuses ====="
cat > "$OUT/wrefuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- vertex 445 is bound; 371 is a target-edge end nearby
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
painter.selectPatchVertex(0, 371, 1)
assert(painter.weldPatchSelection(1000) == 0, "bound vertex was welded")
print("M44-3 OK")
EOF
seed "$OUT/ws3" "$B1"
run_session "$OUT/ws3" "$B1" "$OUT/wrefuse.lua" "$OUT/g3.log"
grep -aq "M44-3 OK" "$OUT/g3.log" || { echo "FAIL: weld refusal"; tail -6 "$OUT/g3.log"; exit 1; }
echo "OK weld refusal"

echo "ALL M44 GATES PASSED"
