#!/usr/bin/env bash
# M36 patch sub-object levels: edge and patch selection, and the picking behind them.
#
# Edge and patch levels MOVE VERTICES - edge move is its two corners, patch move is
# its four - so they are implemented by projecting the level's own selection onto the vertex
# set the move machinery already consumes. That makes two things worth gating separately:
#
# 1. The projection. The level's set stays the authority and the vertex set is recomputed from
# it, which is what makes removing one edge of a pair that shared a corner leave that corner
# selected instead of dropping it. A naive "add two, remove two" gets that wrong.
# 2. The picking. A script that calls selectPatchEdge proves the selection machinery and
# nothing about whether clicking on an edge finds that edge - and a --screenshot run has no
# pointer, so painter.patchClick exists to reach the pick code headlessly at all.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m36_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"
FAIL=0

WS="$OUT/ws"
rm -rf "$WS"; mkdir -p "$WS/landscape/ligo/lacustre/max"
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"

# Vertices 5, 11 and 17 run consecutively along one border of material-fond, so (5,11) and
# (11,17) are adjacent cage edges sharing corner 11 - the case the projection rule is about.
cat > "$OUT/levels.lua" <<'EOF'
painter.setMode(4)

painter.setSubObject(2) -- EP_EDGE
painter.selectPatchEdge(0, 5, 11, 0)
print("E_SEL " .. painter.patchEdgeSelectionCount() .. " " .. painter.patchVertexSelectionCount())
local ax, ay, az = painter.patchVertexPos(0, 5)
local bx, by, bz = painter.patchVertexPos(0, 11)
print(string.format("E_A_BEFORE %.3f %.3f %.3f", ax, ay, az))
print(string.format("E_B_BEFORE %.3f %.3f %.3f", bx, by, bz))
painter.movePatchSelection(0, 0, 1.5)
local cx, cy, cz = painter.patchVertexPos(0, 5)
local dx, dy, dz = painter.patchVertexPos(0, 11)
print(string.format("E_A_AFTER %.3f %.3f %.3f", cx, cy, cz))
print(string.format("E_B_AFTER %.3f %.3f %.3f", dx, dy, dz))
painter.undo()
local ex, ey, ez = painter.patchVertexPos(0, 5)
print(string.format("E_A_UNDONE %.3f %.3f %.3f", ex, ey, ez))

-- Two edges sharing corner 11; dropping one must NOT drop the shared corner.
painter.selectPatchEdge(0, 5, 11, 0)
painter.selectPatchEdge(0, 11, 17, 1)
print("E_TWO " .. painter.patchEdgeSelectionCount() .. " " .. painter.patchVertexSelectionCount())
painter.selectPatchEdge(0, 5, 11, 2)
print("E_ONE " .. painter.patchEdgeSelectionCount() .. " " .. painter.patchVertexSelectionCount())

painter.setSubObject(3) -- EP_PATCH
print("LEVEL_SWITCH " .. painter.patchEdgeSelectionCount() .. " " .. painter.patchVertexSelectionCount())
painter.selectPatchFace(0, 0, 0)
print("F_SEL " .. painter.patchFaceSelectionCount() .. " " .. painter.patchVertexSelectionCount())
painter.movePatchSelection(0, 0, 1.0)

-- Picking. A patch covers a lot of screen, so the centre is a reliable target; edge level
-- gets a horizontal sweep, which must cross the cage several times; and a vertex-level click
-- far off the terrain must clear rather than find something.
painter.setSubObject(3)
painter.patchClick(0.45, 0.55, 0)
print("PICK_FACE " .. painter.patchFaceSelectionCount() .. " " .. painter.patchVertexSelectionCount())
painter.setSubObject(2)
local hits = 0
for i = 0, 40 do
  painter.patchClick(0.30 + i * 0.01, 0.55, 0)
  if painter.patchEdgeSelectionCount() > 0 then hits = hits + 1 end
end
print("PICK_EDGE_HITS " .. hits)
painter.setSubObject(1)
painter.patchClick(0.02, 0.02, 0)
print("PICK_EMPTY " .. painter.patchVertexSelectionCount())
EOF

$XVFB "$ZP" "$WS" --startup-auto "lacustre/material-fond" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/levels.lua" --screenshot /dev/null > "$OUT/levels.log" 2>&1

L="$OUT/levels.log"
say() { grep -a "^$1 " "$L" | head -1 | cut -d' ' -f2-; }
want() { # $1 label, $2 expected, $3 got
	[[ "$3" == "$2" ]] && echo "OK ($1): $3" || { echo "FAIL ($1): got [$3], expected [$2]"; FAIL=1; }
}

echo "===== M36-1: edge level moves its two corners ====="
want "edge selection projects to 2 vertices" "1 2" "$(say E_SEL)"
EA=$(say E_A_BEFORE); EB=$(say E_B_BEFORE)
wa=$(awk -v s="$EA" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2], c[3]+1.5}')
wb=$(awk -v s="$EB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2], c[3]+1.5}')
want "corner A moved" "$wa" "$(say E_A_AFTER)"
want "corner B moved" "$wb" "$(say E_B_AFTER)"
want "undo restored the edge" "$EA" "$(say E_A_UNDONE)"
grep -qa "movePatchSelection: 7 written" "$L" \
	&& echo "OK: the edge move wrote its 2 corners + their 5 handles" \
	|| { echo "FAIL: the edge move did not write 7 elements"; FAIL=1; }

echo "===== M36-2: the level's set is the authority, not the vertex projection ====="
want "two edges sharing a corner" "2 3" "$(say E_TWO)"
want "dropping one keeps the shared corner" "1 2" "$(say E_ONE)"

echo "===== M36-3: patch level moves its four corners ====="
want "switching level clears the selection" "0 0" "$(say LEVEL_SWITCH)"
want "face selection projects to 4 vertices" "1 4" "$(say F_SEL)"
grep -qa "movePatchSelection: 16 written" "$L" \
	&& echo "OK: the patch move wrote its 4 corners + their 12 handles" \
	|| { echo "FAIL: the patch move did not write 16 elements"; FAIL=1; }

echo "===== M36-4: picking ====="
want "a click at patch level finds a face" "1 4" "$(say PICK_FACE)"
PH=$(say PICK_EDGE_HITS)
awk -v h="$PH" 'BEGIN{ exit !(h >= 5) }' \
	&& echo "OK: a sweep at edge level crossed the cage $PH times" \
	|| { echo "FAIL: edge picking found $PH hits across a full sweep"; FAIL=1; }
want "a click on empty space clears" "0" "$(say PICK_EMPTY)"

if [[ $FAIL -ne 0 ]]; then echo "M36 GATES FAILED"; exit 1; fi
echo "ALL M36 GATES PASSED"
