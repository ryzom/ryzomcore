#!/usr/bin/env bash
# M61 Reset paint (plan mA8): the painter's ResetPatch - clear a zone's paint wholesale.
#
# One paint-core op (opResetZone): every patch fillTile(-1) + fillColor white +
# fillDisplace 0, as ONE undo stroke (the per-op endStroke is suspended for the sweep).
# The panel button confirms through the reset modal; scripts call it bare.
#
#  1. UNDO IDENTITY on the authored zone: reset -> ONE undo -> save is byte-identical
#     to the null-edit baseline (the richest paint state the fixture has).
#  2. IN-SESSION STATE: after the reset every sampled tile is empty (layer count 0).
#  3. NEVER-PAINTED EQUALITY on a SYNTHETIC workspace (the plan's rule - authored zones
#     have authored paint): reset+save gives emptyA; painting markers over emptyA and
#     resetting again saves byte-identical to null-edit(emptyA).
#  4. REFUSAL: an unknown zone refuses; nothing changes.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m61_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$2" "$1/landscape/ligo/lacustre/max/$3.max"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
SRC="$GFX/landscape/ligo/lacustre/max/$B.max"
"$ZP" "$SRC" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M61-1: reset -> one undo -> save == baseline; in-session state ====="
seed "$OUT/ws1" "$SRC" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.resetZonePaint(0)
-- Every sampled tile is EMPTY after the reset (layer count 0).
for _, p in ipairs({ 0, 5, 12, 24 }) do
  local t, r, n = painter.tileAt(0, p, 0, 0)
  assert(n == 0, "patch " .. p .. " still carries a tile after reset")
end
painter.undo()
-- The authored paint is back (patch 5 carries authored layers).
local t, r, n = painter.tileAt(0, 5, 0, 0)
assert(n > 0, "one undo did not restore the authored paint")
print("M61-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M61-1 OK" "$OUT/s1.log" || { echo "FAIL: undo scene"; tail -10 "$OUT/s1.log"; exit 1; }
grep -aq "reset paint: zone 0" "$OUT/s1.log" || { echo "FAIL: reset line missing"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: reset+undo is not byte-identical"; exit 1; }
echo "OK one undo restores the whole reset"

echo "===== M61-2: never-painted equality on a synthetic workspace ====="
seed "$OUT/ws2" "$SRC" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.resetZonePaint(0)
print("M61-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M61-2 OK" "$OUT/s2.log" || { echo "FAIL: empty-make scene"; tail -8 "$OUT/s2.log"; exit 1; }
EMPTY="$OUT/ws2/landscape/ligo/lacustre/max/$B.max"
"$ZP" "$EMPTY" --null-edit --out "$OUT/empty.null.max" > "$OUT/empty.null.log" 2>&1

seed "$OUT/ws3" "$EMPTY" "$B"
cat > "$OUT/s3.lua" <<'EOF'
-- Paint over the empty zone, then reset: the save must be a never-painted null-edit.
painter.setMode(0)
painter.rawTile(0, 5, 0, 0, 100, 0)
painter.rawTile(0, 12, 3, 3, 101, 2)
painter.fillColor(0, 7, "ff4040")
painter.fillDisplace(0, 9, 3)
painter.resetZonePaint(0)
for _, p in ipairs({ 5, 12 }) do
  local t, r, n = painter.tileAt(0, p, 0, 0)
  assert(n == 0, "patch " .. p .. " kept paint through the reset")
end
print("M61-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M61-3 OK" "$OUT/s3.log" || { echo "FAIL: paint+reset scene"; tail -10 "$OUT/s3.log"; exit 1; }
cmp -s "$OUT/empty.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: paint+reset differs from the never-painted state"; exit 1; }
echo "OK reset == never painted, byte for byte"

echo "===== M61-3: unknown zone refuses ====="
seed "$OUT/ws4" "$SRC" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.resetZonePaint(99)
print("M61-4 DONE")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M61-4 DONE" "$OUT/s4.log" || { echo "FAIL: refusal scene"; tail -6 "$OUT/s4.log"; exit 1; }
grep -aq "reset paint: unknown zone id" "$OUT/s4.log" || { echo "FAIL: no refusal printed"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: refused reset changed the file"; exit 1; }
echo "OK refusal"

echo "ALL M61 GATES PASSED"
