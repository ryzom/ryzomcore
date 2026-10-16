#!/usr/bin/env bash
# M39 rotate and scale: the transforms that need a pivot.
#
# Move, rotate and scale go through ONE apply path, because everything except the per-element
# world delta is shared - which elements are eligible, the ride and bound rules, the node-space
# conversion, the single undo stroke, the live push. A move hands every element the same delta;
# rotate and scale derive each element's from where it sits relative to the pivot, which is why
# the core op takes a delta per element rather than one for the list.
#
# The numbers here are computed by hand, not read off a first run: rotating (160,0) and
# (160,32) by 90 degrees about Z around their own centroid (160,16) has exactly one answer, and
# a gate that accepted whatever the code produced would prove nothing.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m39_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"
FAIL=0

WS="$OUT/ws"
rm -rf "$WS"; mkdir -p "$WS/landscape/ligo/lacustre/max"
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"

cat > "$OUT/rs.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.setPivotMode(0)          -- selection centre
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchVertex(0, 11, 1)
local px, py, pz = painter.pivotPos()
print(string.format("PIVOT %.2f %.2f %.2f", px, py, pz))
print(string.format("A0 %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
print(string.format("B0 %.2f %.2f %.2f", painter.patchVertexPos(0, 11)))

-- +90 about Z around (160,16): (160,0) -> (176,16), (160,32) -> (144,16).
painter.rotatePatchSelection(2, 90)
print(string.format("A_ROT %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
print(string.format("B_ROT %.2f %.2f %.2f", painter.patchVertexPos(0, 11)))
painter.undo()
print(string.format("A_UNDO %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))

-- 2x in y about the same pivot: (160,0) -> (160,-16), (160,32) -> (160,48).
painter.scalePatchSelection(1, 2, 1)
print(string.format("A_SCL %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
print(string.format("B_SCL %.2f %.2f %.2f", painter.patchVertexPos(0, 11)))
painter.undo()

-- Same rotation about the WORLD origin instead: (160,0) -> (0,160), (160,32) -> (-32,160).
painter.setPivotMode(1)
painter.rotatePatchSelection(2, 90)
print(string.format("A_WROT %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
print(string.format("B_WROT %.2f %.2f %.2f", painter.patchVertexPos(0, 11)))
painter.undo()

-- An arbitrary axis is the form the gizmo records, since a screen ring is not a world axis.
-- 180 degrees about Z reaches the same place either way, so the two forms must agree.
painter.setPivotMode(0)
painter.rotatePatchSelectionAxis(0, 0, 1, 180)
print(string.format("A_AXIS %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
painter.undo()
painter.rotatePatchSelection(2, 180)
print(string.format("A_ENUM %.2f %.2f %.2f", painter.patchVertexPos(0, 5)))
EOF
$XVFB "$ZP" "$WS" --startup-auto "lacustre/material-fond" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/rs.lua" --screenshot /dev/null > "$OUT/rs.log" 2>&1

L="$OUT/rs.log"
say() { grep -a "^$1 " "$L" | head -1 | cut -d' ' -f2-; }
want() { [[ "$3" == "$2" ]] && echo "OK ($1): $3" || { echo "FAIL ($1): got [$3], expected [$2]"; FAIL=1; }; }

echo "===== M39-1: rotate about the selection centre ====="
want "pivot"          "160.00 16.00 -30.00"  "$(say PIVOT)"
want "corner A"       "176.00 16.00 -30.00"  "$(say A_ROT)"
want "corner B"       "144.00 16.00 -30.00"  "$(say B_ROT)"
want "undo"           "$(say A0)"            "$(say A_UNDO)"
grep -qa "rotatePatchSelection: 7 written" "$L" \
	&& echo "OK: the rotate wrote both corners + their handles as one stroke" \
	|| { echo "FAIL: the rotate did not write 7 elements"; FAIL=1; }

echo "===== M39-2: scale about the same pivot ====="
want "corner A" "160.00 -16.00 -30.00" "$(say A_SCL)"
want "corner B" "160.00 48.00 -30.00"  "$(say B_SCL)"

echo "===== M39-3: the pivot decides the result ====="
# The same rotation about the world origin lands somewhere completely different. If the pivot
# were ignored - or silently fell back to the selection centre - this would match M39-1.
want "corner A about world" "0.00 160.00 -30.00"   "$(say A_WROT)"
want "corner B about world" "-32.00 160.00 -30.00" "$(say B_WROT)"

echo "===== M39-4: the arbitrary-axis form agrees with the enumerated one ====="
want "axis form vs enum form" "$(say A_ENUM)" "$(say A_AXIS)"

if [[ $FAIL -ne 0 ]]; then echo "M39 GATES FAILED"; exit 1; fi
echo "ALL M39 GATES PASSED"
