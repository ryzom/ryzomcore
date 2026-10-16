#!/usr/bin/env bash
# M56 Hide / Unhide All (plan mA3): session-only display state, patch-keyed.
#
# SESSION-ONLY by the corpus probe's verdict (PATCH_HIDDEN carried by 4 patches over the
# whole corpus; the on-disk bit is preserved verbatim, never interpreted). A hidden patch
# drops from the cage, the pick paths and the arrows; its elements refuse selection; the
# LANDSCAPE keeps rendering it. Cleared by working-set rebuilds (indices shift).
#
#  1. PATCH LEVEL (zonematerial-bassin-1): hide the selected patch; the REAL pick path
#     (patchClick at its projected centre) no longer finds it - the same click that
#     selected it before; scripted selectPatchFace refuses; unhide restores the pick.
#  2. VERTEX RULES: hiding patches makes their interior vertex refuse selection while a
#     RIM vertex shared with a visible patch stays selectable; vertex-level hide takes
#     the patches touching the selected vertex.
#  3. SESSION-ONLY: a hide + save is byte-identical to the null-edit baseline.
#  4. REBUILD CLEAR: a topology op (delete elsewhere) clears the hide set - indices
#     shifted, stale entries would hide the wrong patches.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m56_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M56-1: hidden patch is unpickable through the real path ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local P = 12
-- The patch's projected centre, from its ring corners (the aim rule the weld gates use).
local function centreOf(p)
  local sx, sy = 0, 0
  for c = 0, 3 do
    local v = painter.patchCornerVert(0, p, c)
    local x, y = painter.vertexScreenPos(0, v)
    sx = sx + x / 4
    sy = sy + y / 4
  end
  return sx, sy
end
local cx, cy = centreOf(P)
-- Positive control: the click finds the patch while it is visible.
painter.patchClick(cx, cy, 1)
assert(painter.patchFaceSelectionCount() == 1, "control: the click did not select")
painter.hideSelection()
assert(painter.patchHidden(0, P), "hide did not register")
assert(painter.patchFaceSelectionCount() == 0, "hide left the patch selected")
-- The same click now hits nothing (a plain miss clears, and there is nothing to find).
painter.patchClick(cx, cy, 1)
assert(painter.patchFaceSelectionCount() == 0, "a hidden patch was picked")
-- Scripted selection refuses too.
painter.selectPatchFace(0, P, 1)
assert(painter.patchFaceSelectionCount() == 0, "scripted select of a hidden patch stuck")
-- Unhide restores the pick.
painter.unhideAll()
assert(not painter.patchHidden(0, P), "unhide did not clear")
painter.patchClick(cx, cy, 1)
assert(painter.patchFaceSelectionCount() == 1, "the pick did not come back after unhide")
print("M56-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M56-1 OK" "$OUT/s1.log" || { echo "FAIL: pick scene"; tail -10 "$OUT/s1.log"; exit 1; }
grep -aq "hide: 1 patches" "$OUT/s1.log" || { echo "FAIL: hide count line"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: hide/unhide session changed the file"; exit 1; }
echo "OK hidden = unpickable; session byte-clean"

echo "===== M56-2: vertex rules - interior refuses, rim survives; vertex-level hide ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
-- Hide ALL patches touching vertex V, via vertex-level hide; V refuses afterwards
-- while a rim vertex of the hidden block shared with a visible patch survives.
local V = 7 -- an interior lattice vertex of the 5x5 grid
painter.setSubObject(1)
painter.selectPatchVertex(0, V, 0)
painter.hideSelection()
assert(painter.patchVertexSelectionCount() == 0, "hide left the vertex selected")
-- Count the hidden patches; every patch using V must be hidden.
local hidden = 0
for p = 0, painter.patchCount(0) - 1 do
  local touches = false
  for c = 0, 3 do
    if painter.patchCornerVert(0, p, c) == V then touches = true end
  end
  if touches then
    assert(painter.patchHidden(0, p), "a patch touching the vertex is not hidden")
    hidden = hidden + 1
  end
end
assert(hidden >= 2, "fixture: vertex should touch 2+ patches")
-- The fully-hidden vertex refuses selection...
painter.selectPatchVertex(0, V, 1)
assert(painter.patchVertexSelectionCount() == 0, "a hidden vertex was selected")
-- ...but a rim corner of the hidden block, shared with a visible patch, still selects.
local rim = nil
for p = 0, painter.patchCount(0) - 1 do
  if painter.patchHidden(0, p) then
    for c = 0, 3 do
      local cand = painter.patchCornerVert(0, p, c)
      if cand ~= V then
        -- visible iff some visible patch uses cand
        for q = 0, painter.patchCount(0) - 1 do
          if not painter.patchHidden(0, q) then
            for c2 = 0, 3 do
              if painter.patchCornerVert(0, q, c2) == cand then rim = cand end
            end
          end
        end
      end
    end
  end
end
assert(rim, "no rim vertex found")
painter.selectPatchVertex(0, rim, 0)
assert(painter.patchVertexSelectionCount() == 1, "a rim vertex refused selection")
print("M56-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M56-2 OK" "$OUT/s2.log" || { echo "FAIL: vertex rules"; tail -10 "$OUT/s2.log"; exit 1; }
echo "OK vertex rules"

echo "===== M56-3: a topology rebuild clears the hide set ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.hideSelection()
assert(painter.patchHidden(0, 12), "hide did not register")
-- Delete a DIFFERENT patch: indices shift, the hide set clears with the rebuild.
painter.setSubObject(3)
painter.selectPatchFace(0, 24, 0)
painter.deletePatchSelection()
assert(not painter.patchHidden(0, 12), "stale hide survived the topology rebuild")
painter.undo()
print("M56-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M56-3 OK" "$OUT/s3.log" || { echo "FAIL: rebuild clear"; tail -10 "$OUT/s3.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: delete+undo not byte-identical"; exit 1; }
echo "OK rebuild clears hide; delete+undo byte-identical"

echo "ALL M56 GATES PASSED"
