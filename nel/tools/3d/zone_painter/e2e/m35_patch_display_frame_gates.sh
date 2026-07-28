#!/usr/bin/env bash
# M35 patch display-frame gates: an edit has to come back to the CAGE in the right place.
#
# M34 proves the .max bytes. It cannot see the display, and the display is where two separate
# frame bugs hid:
#
#  1. The write-target payload is not a position. A PatchMesh slot stores an absolute object
#     position while a mapper record stores a DELTA from its own Original, so a display update
#     that took the stored value literally put mapper-path vertices at roughly the node origin
#     - a whole-zone jump, on a third of the corpus, with byte-perfect file output.
#  2. ObjectTM is not the display frame. A file placed anywhere but the board origin is drawn
#     through its placement, so recomputing a display position from ObjectTM alone dropped the
#     placement and moved the vertex by exactly one board cell. On top of that the live
#     landscape push then refused the patch as out of its packed range, so a multi-file session
#     could not update the surface at all.
#
# Both are invisible to a byte gate and both are caught here by reading the vertex back with
# painter.patchVertexPos, which returns the DISPLAYED world position - the same number the
# marker is drawn at.
#
# Covered: all three write targets single-file, and a second EDITABLE file placed one cell
# away, which is the case that only exists once a session holds more than one file.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m35_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"
FAIL=0

seed() { # $1 = workspace dir, rest = basenames
	local ws="$1"; shift
	rm -rf "$ws"; mkdir -p "$ws/landscape/ligo/lacustre/max"
	for b in "$@"; do cp "$GFX/landscape/ligo/lacustre/max/$b.max" "$ws/landscape/ligo/lacustre/max/"; done
	ln -sfn "$GFX/landscape/_texture_tiles" "$ws/landscape/_texture_tiles"
}

# The script is the same for every case; only the zone id changes. Vertex 5 is a free (unbound)
# corner on all four test files - a bound vertex would legitimately refuse to move.
mkscript() { # $1 = out path, $2 = zone id
	cat > "$1" <<EOF
painter.setMode(4)
painter.setSubObject(1)
local bx, by, bz = painter.patchVertexPos($2, 5)
print(string.format("BEFORE %.3f %.3f %.3f", bx, by, bz))
painter.clearPatchVertexSelection()
painter.selectPatchVertex($2, 5, 1)
painter.movePatchSelection(0, 0, 1.5)
local ax, ay, az = painter.patchVertexPos($2, 5)
print(string.format("AFTER %.3f %.3f %.3f", ax, ay, az))
painter.undo()
local ux, uy, uz = painter.patchVertexPos($2, 5)
print(string.format("UNDONE %.3f %.3f %.3f", ux, uy, uz))
EOF
}

# BEFORE + (0,0,1.5) == AFTER, and UNDONE == BEFORE. Exact equality is the right test: the move
# is a float add on both sides, and the failures this gate exists for are off by 80 to 160
# world units, not by an ulp.
check() { # $1 = label, $2 = log
	local log="$2" b a u
	b=$(grep -a '^BEFORE ' "$log" | head -1 | cut -d' ' -f2-)
	a=$(grep -a '^AFTER ' "$log" | head -1 | cut -d' ' -f2-)
	u=$(grep -a '^UNDONE ' "$log" | head -1 | cut -d' ' -f2-)
	if [[ -z "$b" || -z "$a" || -z "$u" ]]; then
		echo "FAIL ($1): vertex readback missing (log: $log)"; FAIL=1; return
	fi
	local want
	want=$(awk -v s="$b" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2], c[3]+1.5}')
	if [[ "$a" != "$want" ]]; then
		echo "FAIL ($1): moved vertex at [$a], expected [$want] (was [$b])"; FAIL=1; return
	fi
	if [[ "$u" != "$b" ]]; then
		echo "FAIL ($1): undo left the cage at [$u], expected [$b]"; FAIL=1; return
	fi
	# The live landscape push must ACCEPT the patch. It reports rather than writing a wrong
	# surface, so a frame bug shows up here as a message and an unchanged surface.
	if grep -qa "packed range" "$log"; then
		echo "FAIL ($1): live surface push refused the patch"; FAIL=1; return
	fi
	echo "OK ($1): cage followed the edit at [$a], undo restored [$u]"
}

echo "===== M35-1: single file, all three write targets ====="
for pair in "material-fond:modPM" "material-bassin:delta" "zonematerial-bassin-1:base"; do
	B="${pair%%:*}"; WANT="${pair##*:}"
	seed "$OUT/ws_$B" "$B"
	mkscript "$OUT/$B.lua" 0
	$XVFB "$ZP" "$OUT/ws_$B" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
		--startup-lua "$OUT/$B.lua" --screenshot /dev/null > "$OUT/$B.log" 2>&1
	# Assert the target too, so a policy regression that routed everything one way cannot
	# pass this gate by accident.
	if ! grep -qa "$WANT 1" "$OUT/$B.log"; then
		echo "FAIL ($B): expected write target $WANT"; FAIL=1
	fi
	check "$B via $WANT" "$OUT/$B.log"
done

echo "===== M35-2: second editable file placed one board cell away ====="
# material-peek at cell (1,0) gets zone ids from 1000 and is DRAWN 160 units along x from where
# its own file authored it. Editing it through the frame it is drawn in is the whole case.
seed "$OUT/ws_multi" material-fond material-peek
mkscript "$OUT/multi.lua" 1000
$XVFB "$ZP" "$OUT/ws_multi" --startup-auto "lacustre/material-fond" \
	--open-editable "1,0:material-peek" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/multi.lua" --screenshot /dev/null > "$OUT/multi.log" 2>&1
grep -qa "open-editable: 'material-peek' @ (1,0)" "$OUT/multi.log" \
	|| { echo "FAIL (multi): material-peek was not placed at (1,0)"; FAIL=1; }
# The placed file's cage must sit in the board frame, not in its own authored frame: its x is
# one cell (160) beyond the first file's, which occupies [0, 160].
BX=$(grep -a '^BEFORE ' "$OUT/multi.log" | head -1 | cut -d' ' -f2)
awk -v x="$BX" 'BEGIN{ exit !(x >= 159.0) }' \
	|| { echo "FAIL (multi): placed cage at x=$BX, expected it beyond the first file's cell"; FAIL=1; }
check "material-peek placed at (1,0)" "$OUT/multi.log"

echo "===== M35-3: two nodes on one object ====="
# --place puts a SECOND NODE on material-fond's object. Nothing about it is a decoration: it
# is editable, an edit through it reaches the object, and every node showing that object
# follows. Rot 1 (90 deg CCW) so the node transform has to actually do something.
seed "$OUT/ws_node" material-fond
cat > "$OUT/node.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local px, py, pz = painter.patchVertexPos(0, 5)
local ix, iy, iz = painter.patchVertexPos(10000, 5)
print(string.format("A_BEFORE %.3f %.3f %.3f", px, py, pz))
print(string.format("B_BEFORE %.3f %.3f %.3f", ix, iy, iz))
-- Drag the SECOND node along world +x. Under a 90 deg rotation that is -y on the object,
-- so the first node must move along -y by the same amount.
painter.clearPatchVertexSelection()
painter.selectPatchVertex(10000, 5, 1)
painter.movePatchSelection(0.5, 0, 0)
local ax, ay, az = painter.patchVertexPos(0, 5)
local bx, by, bz = painter.patchVertexPos(10000, 5)
print(string.format("A_AFTER %.3f %.3f %.3f", ax, ay, az))
print(string.format("B_AFTER %.3f %.3f %.3f", bx, by, bz))
-- One underlying vertex reached through two nodes is ONE thing: selecting it twice would
-- apply the drag to a single storage location twice, with two disagreeing object deltas.
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 1)
painter.selectPatchVertex(10000, 5, 1)
print("SAME_VERTEX " .. painter.patchVertexSelectionCount())
-- DIFFERENT vertices through different nodes share nothing, and authoring an edge from
-- whichever node shows it best is the point of having them.
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 1)
painter.selectPatchVertex(10000, 6, 1)
print("DIFF_VERTEX " .. painter.patchVertexSelectionCount())
EOF
ZONE_PAINTER_BOARD_ACTION="save:material-fond" $XVFB "$ZP" "$OUT/ws_node" \
	--startup-auto "lacustre/material-fond?place=1,0,1" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/node.lua" --screenshot /dev/null > "$OUT/node.log" 2>&1

nodepos() { grep -a "^$1 " "$OUT/node.log" | head -1 | cut -d' ' -f2-; }
AB=$(nodepos A_BEFORE); AA=$(nodepos A_AFTER)
BB=$(nodepos B_BEFORE); BA=$(nodepos B_AFTER)
wantA=$(awk -v s="$AB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2]-0.5, c[3]}')
wantB=$(awk -v s="$BB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1]+0.5, c[2], c[3]}')
[[ "$BA" == "$wantB" ]] \
	&& echo "OK (node B): the dragged node moved to [$BA]" \
	|| { echo "FAIL (node B): [$BA], expected [$wantB]"; FAIL=1; }
[[ "$AA" == "$wantA" ]] \
	&& echo "OK (node A): the sibling node followed to [$AA] through its own transform" \
	|| { echo "FAIL (node A): sibling at [$AA], expected [$wantA] - fan-out or transform"; FAIL=1; }
grep -qa "^SAME_VERTEX 1$" "$OUT/node.log" \
	&& echo "OK: one vertex through two nodes stays one selection" \
	|| { echo "FAIL: the same vertex was selectable through both nodes"; FAIL=1; }
grep -qa "^DIFF_VERTEX 2$" "$OUT/node.log" \
	&& echo "OK: different vertices through different nodes both select" \
	|| { echo "FAIL: different vertices through different nodes were refused"; FAIL=1; }
grep -qa "packed range" "$OUT/node.log" \
	&& { echo "FAIL (node): live surface push refused the patch"; FAIL=1; } || true

echo "===== M35-4: the node used to reach the object does not change the bytes ====="
# The same edit expressed in each node's own displayed space must write the same file. This is
# the whole node/object claim in one comparison: if a node transform were wrong anywhere, the
# two saves would differ.
seed "$OUT/ws_eq" material-fond
cat > "$OUT/eq.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 1)
painter.movePatchSelection(0, -0.5, 0)
EOF
ZONE_PAINTER_BOARD_ACTION="save:material-fond" $XVFB "$ZP" "$OUT/ws_eq" \
	--startup-auto "lacustre/material-fond?place=1,0,1" --no-hint-stamp --no-thumbnail \
	--startup-lua "$OUT/eq.lua" --screenshot /dev/null > "$OUT/eq.log" 2>&1
if cmp -s "$OUT/ws_eq/landscape/ligo/lacustre/max/material-fond.max" \
          "$OUT/ws_node/landscape/ligo/lacustre/max/material-fond.max"; then
	echo "OK: edit through the rotated node is byte-identical to the same edit through the first"
else
	echo "FAIL: the two nodes wrote different bytes for the same edit"; FAIL=1
fi

echo "===== M35-5: a move past the packed range rebuilds the zone and keeps its tiles ====="
# Control points are 16-bit fixed point around a bias/scale derived from the bbox at build()
# time, and pack() CLAMPS - so a move that leaves that range cannot be written in place, and
# used to leave the surface silently stale while the .max got the edit.
#
# The rebuild has one trap worth a gate of its own: it must NOT rebuild from the display cage.
# pz.Patches carries the geometry but its tile records are the ones assembly loaded, so
# building from there would revert every tile painted since - correct geometry, reverted
# terrain. The fill below is what makes that visible: a band of tile set 7 reads clearly
# darker than the authored ground, and its mean brightness must survive the rebuild.
seed "$OUT/ws_range" material-fond
cat > "$OUT/range.lua" <<'EOF'
painter.fillTile(0, 0, 7)
painter.fillTile(0, 1, 7)
painter.fillTile(0, 2, 7)
painter.fillTile(0, 3, 7)
painter.screenshot("SHOTDIR/range_before.tga")
painter.setMode(4)
painter.setSubObject(1)
local bx, by, bz = painter.patchVertexPos(0, 5)
print(string.format("R_BEFORE %.3f %.3f %.3f", bx, by, bz))
-- 3 units past a corner that already sits at the edge of the zone's packed range.
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 1)
painter.movePatchSelection(0, -3, 0)
local ax, ay, az = painter.patchVertexPos(0, 5)
print(string.format("R_AFTER %.3f %.3f %.3f", ax, ay, az))
painter.screenshot("SHOTDIR/range_after.tga")
EOF
sed -i "s#SHOTDIR#$OUT#g" "$OUT/range.lua"
$XVFB "$ZP" "$OUT/ws_range" --startup-auto "lacustre/material-fond" --no-hint-stamp \
	--no-thumbnail --verbose --startup-lua "$OUT/range.lua" --screenshot /dev/null \
	> "$OUT/range.log" 2>&1

grep -qa "rebuilt (move left the packed range)" "$OUT/range.log" \
	&& echo "OK: the out-of-range move rebuilt the zone" \
	|| { echo "FAIL: no rebuild - either the move stayed in range or the rebuild is gone"; FAIL=1; }
grep -qa "could not be updated" "$OUT/range.log" \
	&& { echo "FAIL: the live surface was left stale"; FAIL=1; } || true
RB=$(grep -a '^R_BEFORE ' "$OUT/range.log" | head -1 | cut -d' ' -f2-)
RA=$(grep -a '^R_AFTER ' "$OUT/range.log" | head -1 | cut -d' ' -f2-)
wantR=$(awk -v s="$RB" 'BEGIN{split(s,c," "); printf "%.3f %.3f %.3f", c[1], c[2]-3.0, c[3]}')
[[ "$RA" == "$wantR" ]] \
	&& echo "OK: the cage took the full move to [$RA]" \
	|| { echo "FAIL: cage at [$RA], expected [$wantR]"; FAIL=1; }

# Mean brightness of a crop inside the filled band and away from the moved corner. Measured
# values: filled 0.147, authored ground 0.198 - so 0.01 is five times tighter than the gap a
# reverted tile set would open, and the rebuild itself moves it by ~4e-6.
if command -v convert >/dev/null 2>&1 && [[ -f "$OUT/range_before.tga" && -f "$OUT/range_after.tga" ]]; then
	MB=$(convert "$OUT/range_before.tga" -crop 260x60+330+400 +repage -format "%[fx:mean]" info:)
	MA=$(convert "$OUT/range_after.tga" -crop 260x60+330+400 +repage -format "%[fx:mean]" info:)
	if awk -v a="$MA" -v b="$MB" 'BEGIN{ exit !((a-b < 0.01) && (b-a < 0.01)) }'; then
		echo "OK: painted tiles survived the rebuild (band mean $MB -> $MA)"
	else
		echo "FAIL: the rebuild reverted the terrain (band mean $MB -> $MA)"; FAIL=1
	fi
else
	echo "SKIP: ImageMagick or the screenshots are missing, tile-survival not checked"
fi

if [[ $FAIL -ne 0 ]]; then echo "M35 GATES FAILED"; exit 1; fi
echo "ALL M35 GATES PASSED"
