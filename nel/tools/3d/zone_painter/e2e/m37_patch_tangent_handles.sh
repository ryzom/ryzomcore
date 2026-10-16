#!/usr/bin/env bash
# M37 tangent handles: shaping the Bezier, not just moving its corners.
#
# A patch's corners index the PatchMesh's Verts table and its handles index Vecs
# (SPmPatch::V[4] and Vec[8]). The two are addressed identically all the way down - their own
# element container in the stream, their own half of the vertex mapper - so the whole write
# policy is shared and only the table changes. This gate proves that: the same three write
# targets, the same byte discipline, on handles.
#
# The interaction rule is the part worth stating. Handles are drawn only for SELECTED corners,
# so the corner selection has to survive picking a handle - drop it and the handle vanishes
# under the click. Both sets are then non-empty at once, and the handles win: clicking a handle
# is the artist saying "this one now", and the vertex stays selected while you drag
# its handle. So a handle move must leave its corner where it was.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m37_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"
FAIL=0

seed() { # $1 = ws, $2 = basename
	rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

echo "===== M37-1: a handle moves, its corner does not, undo restores both ====="
# Vec 9 is a handle of corner 5 on material-fond (corner at x=160, handle at x=149.33 along the
# same edge). Both are selected: the corner because handles are only shown for selected
# corners, the handle because it is the target.
seed "$OUT/ws" material-fond
cat > "$OUT/one.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchTangent(0, 9, 0)
print("SEL " .. painter.patchVertexSelectionCount() .. " " .. painter.patchTangentSelectionCount())
local tx, ty, tz = painter.patchTangentPos(0, 9)
local vx, vy, vz = painter.patchVertexPos(0, 5)
print(string.format("T_BEFORE %.3f %.3f %.3f", tx, ty, tz))
print(string.format("V_BEFORE %.3f %.3f %.3f", vx, vy, vz))
painter.movePatchSelection(0, 0, 5)
local ax, ay, az = painter.patchTangentPos(0, 9)
local bx, by, bz = painter.patchVertexPos(0, 5)
print(string.format("T_AFTER %.3f %.3f %.3f", ax, ay, az))
print(string.format("V_AFTER %.3f %.3f %.3f", bx, by, bz))
painter.undo()
local ux, uy, uz = painter.patchTangentPos(0, 9)
print(string.format("T_UNDONE %.3f %.3f %.3f", ux, uy, uz))
EOF
$XVFB "$ZP" "$OUT/ws" --startup-auto "lacustre/material-fond" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/one.lua" --screenshot /dev/null > "$OUT/one.log" 2>&1

L="$OUT/one.log"
say() { grep -a "^$1 " "$L" | head -1 | cut -d' ' -f2-; }
want() { [[ "$3" == "$2" ]] && echo "OK ($1): $3" || { echo "FAIL ($1): got [$3], expected [$2]"; FAIL=1; }; }

want "corner and handle both selected" "1 1" "$(say SEL)"
TB=$(say T_BEFORE); VB=$(say V_BEFORE)
wt=$(awk -v s="$TB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2], c[3]+5}')
want "the handle moved" "$wt" "$(say T_AFTER)"
want "its corner held still" "$VB" "$(say V_AFTER)"
want "undo restored the handle" "$TB" "$(say T_UNDONE)"
grep -qa "movePatchSelection: 1 written" "$L" \
	&& echo "OK: exactly one element written - the corner was not written too" \
	|| { echo "FAIL: the handle move did not write exactly one element"; FAIL=1; }

echo "===== M37-2: handles reach all three write targets, byte for byte ====="
# Same discipline as M34: the baseline is the --null-edit output, never the source .max, and
# moving one handle 1.5 along Z alters a single float, of which 2 bytes differ. The target is
# asserted, so a policy that silently routed every handle to one place would fail here.
cat > "$OUT/none.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
EOF
cat > "$OUT/move.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchTangent(0, 9, 0)
painter.movePatchSelection(0, 0, 1.5)
EOF
cat > "$OUT/undo.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
painter.selectPatchTangent(0, 9, 0)
painter.selectPatchTangent(0, 8, 1)
painter.movePatchSelection(0, 0, 1.5)
painter.undo()
EOF

run() { # $1 = ws, $2 = basename, $3 = lua, $4 = log
	ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
		--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1
}

for pair in "material-fond:modPM" "material-bassin:delta" "zonematerial-bassin-1:base"; do
	B="${pair%%:*}"; WANT="${pair##*:}"
	SRC="$GFX/landscape/ligo/lacustre/max/$B.max"
	"$ZP" "$SRC" --null-edit --out "$OUT/$B.null.max" > "$OUT/$B.null.log" 2>&1
	REF=$(md5sum "$OUT/$B.null.max" | cut -d' ' -f1)

	seed "$OUT/ws_$B" "$B"
	run "$OUT/ws_$B" "$B" "$OUT/none.lua" "$OUT/$B.none.log"
	if [[ "$(md5sum "$OUT/ws_$B/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)" == "$REF" ]]; then
		echo "OK ($B): patch mode with nothing moved is byte-identical"
	else
		echo "FAIL ($B): entering patch mode changed the file"; FAIL=1
	fi

	seed "$OUT/ws_$B" "$B"
	run "$OUT/ws_$B" "$B" "$OUT/move.lua" "$OUT/$B.move.log"
	N=$({ cmp -l "$OUT/$B.null.max" "$OUT/ws_$B/landscape/ligo/lacustre/max/$B.max" | wc -l; } || true)
	if [[ "$N" == "2" ]] && grep -qa "$WANT 1" "$OUT/$B.move.log"; then
		echo "OK ($B): one handle moved changed 2 bytes via $WANT"
	else
		echo "FAIL ($B): $N bytes changed, expected 2 via $WANT"
		grep -a "movePatchSelection:" "$OUT/$B.move.log" || true
		FAIL=1
	fi

	seed "$OUT/ws_$B" "$B"
	run "$OUT/ws_$B" "$B" "$OUT/undo.lua" "$OUT/$B.undo.log"
	if cmp -s "$OUT/$B.null.max" "$OUT/ws_$B/landscape/ligo/lacustre/max/$B.max"; then
		echo "OK ($B): two handles moved then undone is back to the baseline"
	else
		echo "FAIL ($B): undo did not restore the baseline"; FAIL=1
	fi
done

echo "===== M37-3: a handle rides its corner when the CORNER is the target ====="
# With no handle selected the corner is the target, and its handles must follow it - a corner
# whose handles stayed behind would shear the surface. They must follow exactly once: the
# write moves the corner only, and the handles are absolute points that take the same shift.
seed "$OUT/ws_ride" material-fond
cat > "$OUT/ride.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 0)
local tx, ty, tz = painter.patchTangentPos(0, 9)
print(string.format("R_T_BEFORE %.3f %.3f %.3f", tx, ty, tz))
painter.movePatchSelection(0, 0, 3)
local ax, ay, az = painter.patchTangentPos(0, 9)
print(string.format("R_T_AFTER %.3f %.3f %.3f", ax, ay, az))
EOF
$XVFB "$ZP" "$OUT/ws_ride" --startup-auto "lacustre/material-fond" --no-hint-stamp \
	--no-thumbnail --startup-lua "$OUT/ride.lua" --screenshot /dev/null > "$OUT/ride.log" 2>&1
L="$OUT/ride.log"
RB=$(say R_T_BEFORE)
wr=$(awk -v s="$RB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2], c[3]+3}')
want "the handle rode its corner exactly once" "$wr" "$(say R_T_AFTER)"
grep -qa "movePatchSelection: 3 written" "$L" \
	&& echo "OK: the corner move wrote the corner + its 2 handles (the ride is in the file)" \
	|| { echo "FAIL: the corner move did not write corner + handles"; FAIL=1; }

if [[ $FAIL -ne 0 ]]; then echo "M37 GATES FAILED"; exit 1; fi
echo "ALL M37 GATES PASSED"
