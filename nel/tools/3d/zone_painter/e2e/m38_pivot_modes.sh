#!/usr/bin/env bash
# M38 pivot point: what a transform is anchored on (the pivot-point control).
#
# Irrelevant to a move - a translation is a translation wherever it is anchored - and decisive
# for rotate and scale. Gated on its own because each mode answers a different question and
# three of the five are easy to conflate:
#
# Selection center the centroid of the selected SUB-OBJECTS
# Center of sel objects the centre of the NODES those sub-objects live in
# Center of all objects the centre of every editable node, held DURING an interaction
#
# The session places a second node one cell east, so "all objects" and "selected objects"
# have different answers and a mode that quietly fell back to another would be caught.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m38_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"
FAIL=0

WS="$OUT/ws"
rm -rf "$WS"; mkdir -p "$WS/landscape/ligo/lacustre/max"
cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$WS/landscape/ligo/lacustre/max/"
ln -sfn "$GFX/landscape/_texture_tiles" "$WS/landscape/_texture_tiles"

# Corners 5 and 11 sit at (160,0) and (160,32), both in the FIRST node. The second node covers
# x 160..320, so the two "objects" answers differ by exactly half a cell.
cat > "$OUT/piv.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchVertex(0, 11, 1)
for m = 0, 4 do
  painter.setPivotMode(m)
  local x, y, z = painter.pivotPos()
  if x then print(string.format("PIV%d %.2f %.2f %.2f", m, x, y, z))
  else print("PIV" .. m .. " none") end
end
-- The user pivot has no value until it is placed; placing it uses the SELECTION centre, which
-- is mode 0's answer, so the two must agree afterwards.
painter.setUserPivotToSelection()
painter.setPivotMode(4)
local x, y, z = painter.pivotPos()
print(string.format("PIVSET %.2f %.2f %.2f", x, y, z))
EOF
$XVFB "$ZP" "$WS" --startup-auto "lacustre/material-fond?place=1,0" --no-hint-stamp \
	--no-thumbnail --startup-lua "$OUT/piv.lua" --screenshot /dev/null > "$OUT/piv.log" 2>&1

L="$OUT/piv.log"
say() { grep -a "^$1 " "$L" | head -1 | cut -d' ' -f2-; }
want() { [[ "$3" == "$2" ]] && echo "OK ($1): $3" || { echo "FAIL ($1): got [$3], expected [$2]"; FAIL=1; }; }

want "selection center" "160.00 16.00 -30.00" "$(say PIV0)"
want "world center" "0.00 0.00 0.00" "$(say PIV1)"
# Two nodes spanning x 0..320, y 0..160.
want "center of all objects" "160.00 80.00 -30.28" "$(say PIV2)"
# Only the first node owns the selection: x 0..160.
want "center of sel objects" "80.00 80.00 -30.28" "$(say PIV3)"
grep -qa "^PIV4 none$" "$L" \
	&& echo "OK: the user pivot has no value until it is placed" \
	|| { echo "FAIL: user pivot reported a position before being placed"; FAIL=1; }
want "user pivot after placing" "160.00 16.00 -30.00" "$(say PIVSET)"

echo "===== M38-2: the gizmo is drawn AT the pivot ====="
# The artist has to see what the next rotate will turn around before turning it, so the gizmo
# moves with the pivot rather than staying on the selection. Two renders, same selection.
shot() { # $1 = pivot mode, $2 = tga
	cat > "$OUT/g$1.lua" <<EOF
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchVertex(0, 11, 1)
painter.setPivotMode($1)
EOF
	$XVFB "$ZP" "$WS" --startup-auto "lacustre/material-fond" --no-hint-stamp --no-thumbnail \
		--startup-lua "$OUT/g$1.lua" --screenshot "$2" > "$OUT/g$1.log" 2>&1
}
shot 0 "$OUT/g_sel.tga"
shot 1 "$OUT/g_world.tga"
if command -v compare >/dev/null 2>&1; then
	D=$(compare -metric AE "$OUT/g_sel.tga" "$OUT/g_world.tga" null: 2>&1 || true)
	if awk -v d="$D" 'BEGIN{ exit !(d + 0 > 500) }'; then
		echo "OK: the gizmo moved with the pivot ($D pixels differ)"
	else
		echo "FAIL: the two pivots rendered the same ($D pixels) - the gizmo ignored the pivot"; FAIL=1
	fi
else
	echo "SKIP: ImageMagick missing, gizmo placement not checked"
fi

if [[ $FAIL -ne 0 ]]; then echo "M38 GATES FAILED"; exit 1; fi
echo "ALL M38 GATES PASSED"
