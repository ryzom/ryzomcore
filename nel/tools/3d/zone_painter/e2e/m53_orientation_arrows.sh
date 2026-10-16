#!/usr/bin/env bash
# M53 orientation arrows: the display that makes Turn CW/CCW visible at all.
#
# Two layers behind one toggle (the legacy painter's ToggleArrows, C in patch scope):
# every bank tile gains an ADDITIVE arrow layer - each painted tile shows its frame,
# rotation included - and patch mode draws a thin per-patch frame arrow on the overlay
# (the legacy patch editor's centred line arrow). A turn moves no geometry, so without
# this display it is invisible; with it, the arrows turn.
#
# Pixel gates on the screenshot pass (the render is deterministic - scene 0 proves it
# with a repeat-run control):
#  1. Arrows ON vs off changes the patch-mode frame (the overlay arrows).
#  2. With arrows on, a CCW turn changes the frame (the arrow turned with the ring).
#  3. Arrows ON vs off changes the TILE-mode frame (the additive tile layer renders).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m53_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

B=zonematerial-bassin-1
seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$B.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
shot() { # $1 = ws, $2 = lua, $3 = tga, $4 = log
	$XVFB "$ZP" "$1" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
		--startup-lua "$2" --screenshot "$3" > "$4" 2>&1; }
ndiff() { { cmp -l "$1" "$2" || true; } | wc -l; }

cat > "$OUT/off.lua" <<'EOF'
painter.setMode(4)
EOF
cat > "$OUT/on.lua" <<'EOF'
painter.setMode(4)
painter.setShowArrows(true)
EOF
cat > "$OUT/onturn.lua" <<'EOF'
painter.setMode(4)
painter.setShowArrows(true)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.turnPatchSelection(true) == 1)
painter.selectPatchFace(0, 5, 2)
EOF
cat > "$OUT/tile_on.lua" <<'EOF'
painter.setShowArrows(true)
EOF
cat > "$OUT/tile_off.lua" <<'EOF'
painter.setMode(0)
EOF

echo "===== M53-0: determinism control (same scene renders byte-identical) ====="
seed "$OUT/ws"
shot "$OUT/ws" "$OUT/off.lua" "$OUT/off_a.tga" "$OUT/off_a.log"
shot "$OUT/ws" "$OUT/off.lua" "$OUT/off_b.tga" "$OUT/off_b.log"
C=$(ndiff "$OUT/off_a.tga" "$OUT/off_b.tga")
[[ "$C" -eq 0 ]] || { echo "FAIL: repeat render differs by $C bytes - pixel gates unsound"; exit 1; }
echo "OK deterministic render"

echo "===== M53-1: arrows change the patch-mode frame ====="
shot "$OUT/ws" "$OUT/on.lua" "$OUT/on.tga" "$OUT/on.log"
D=$(ndiff "$OUT/off_a.tga" "$OUT/on.tga")
[[ "$D" -gt 500 ]] || { echo "FAIL: arrows on/off differ by only $D bytes"; exit 1; }
echo "OK overlay arrows render ($D bytes differ)"

echo "===== M53-2: with arrows on, a TURN is visible ====="
shot "$OUT/ws" "$OUT/onturn.lua" "$OUT/onturn.tga" "$OUT/onturn.log"
T=$(ndiff "$OUT/on.tga" "$OUT/onturn.tga")
[[ "$T" -gt 200 ]] || { echo "FAIL: turn with arrows on differs by only $T bytes"; exit 1; }
echo "OK the turn shows ($T bytes differ)"

echo "===== M53-3: arrows change the TILE-mode frame (the additive layer) ====="
seed "$OUT/wst"
shot "$OUT/wst" "$OUT/tile_off.lua" "$OUT/toff.tga" "$OUT/toff.log"
shot "$OUT/wst" "$OUT/tile_on.lua" "$OUT/ton.tga" "$OUT/ton.log"
A=$(ndiff "$OUT/toff.tga" "$OUT/ton.tga")
[[ "$A" -gt 500 ]] || { echo "FAIL: additive tile arrows differ by only $A bytes"; exit 1; }
echo "OK tile arrows render ($A bytes differ)"

echo "ALL M53 GATES PASSED"
