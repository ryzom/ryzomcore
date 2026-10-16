#!/usr/bin/env bash
# M52 Surface Properties (patch level): smoothing groups and per-patch tessellation.
#
# Both are carrier-value edits riding the shared topo runner - the SmGroup word lives in
# the PatchMesh stream, the tile orders and their arrays in the rp blob - so they get the
# writeBack/encode/rebuild sequencing and Kind 6 undo like every topological op.
#
#  1. SMOOTHING GROUPS (zonematerial-bassin-1, base target): set a bit over {5, 10},
#     read it back, clear-all clears it; the two-stroke undo chain restores the null-edit
#     baseline byte for byte; the bit survives save + reopen; the file round-trips.
#  2. TESSELLATION: down one axis then back up, undo chain byte-identical; the
#     corner-anchored keep - a marker tile painted at (0,0) survives a shrink and the
#     grow-back, across save + reopen.
#  3. BALANCE: make the pair uneven (patch 5 to 3x3), balance {5, 10} - both land on the
#     max per axis (4x4).
#  4. REFUSAL (ilot_croix): patch 229 is a bind TARGET - tess change refuses, the file
#     stays byte-identical (NeL derives bound-edge tessellation from the target's orders).
#  5. TARGET (material-fond): the ops route to the modifier stream where one exists -
#     the runner's target line is asserted, like m34 does for geometry.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m52_out
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

echo "===== M52-1: smoothing groups - set, read, clear, undo to baseline ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 10, 1)
-- The corpus authors real smoothing words; pick a bit CLEAR on both patches.
local g5, g10 = painter.patchSmGroups(0, 5), painter.patchSmGroups(0, 10)
local bit = nil
for b = 0, 31 do
  local m = 2 ^ b
  if g5 % (2 * m) < m and g10 % (2 * m) < m then bit = b break end
end
assert(bit, "no clear smoothing bit on the pair")
local mv = 2 ^ bit
local g0 = g5
painter.setSmoothGroup(bit, true)
assert(painter.patchSmGroups(0, 5) == g5 + mv, "bit not set on patch 5")
assert(painter.patchSmGroups(0, 10) == g10 + mv, "bit not set on patch 10")
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 10, 1)
painter.clearSmoothGroups()
assert(painter.patchSmGroups(0, 5) == 0, "clear-all left bits on patch 5")
painter.undo()
painter.undo()
assert(painter.patchSmGroups(0, 5) == g0, "undo did not restore the word")
print("M52-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M52-1 OK" "$OUT/s1.log" || { echo "FAIL: smoothing groups"; tail -8 "$OUT/s1.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: smoothing undo chain is not byte-identical"; exit 1; }
echo "OK smoothing groups set/clear/undo byte-identical"

echo "===== M52-1b: the set bit survives save + reopen, file round-trips ====="
seed "$OUT/ws1b" "$B"
cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
local g5 = painter.patchSmGroups(0, 5)
local bit = nil
for b = 0, 31 do
  local m = 2 ^ b
  if g5 % (2 * m) < m then bit = b break end
end
assert(bit, "no clear smoothing bit")
painter.setSmoothGroup(bit, true)
print("M52-1B SET " .. bit)
print("M52-1B OK")
EOF
run_session "$OUT/ws1b" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M52-1B OK" "$OUT/s1b.log" || { echo "FAIL: sm set for persistence"; exit 1; }
BIT=$(grep -a "M52-1B SET" "$OUT/s1b.log" | head -1 | grep -oa '[0-9]*$')
cat > "$OUT/s1c.lua" <<EOF
painter.setMode(4)
local m = 2 ^ $BIT
assert(painter.patchSmGroups(0, 5) % (2 * m) >= m, "the set bit did not survive reopen")
print("M52-1C OK")
EOF
run_verify "$OUT/ws1b" "$B" "$OUT/s1c.lua" "$OUT/s1c.log"
grep -aq "M52-1C OK" "$OUT/s1c.log" || { echo "FAIL: sm bit lost on reopen"; tail -6 "$OUT/s1c.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1b/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1b.rt.log" 2>&1 \
	|| { echo "FAIL: sm-edited file does not round-trip"; tail -4 "$OUT/s1b.rt.log"; exit 1; }
echo "OK persistence + round-trip"

echo "===== M52-2: tessellation - corner-anchored keep, undo, persistence ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
local u0, v0 = painter.patchTess(0, 5)
assert(u0 == 4 and v0 == 4, "fixture: patch 5 is 4x4")
painter.setPatchTess(3, 0)
local u1, v1 = painter.patchTess(0, 5)
assert(u1 == 3 and v1 == 4, "u did not shrink")
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.setPatchTess(4, 0)
painter.undo()
painter.undo()
local u2, v2 = painter.patchTess(0, 5)
assert(u2 == 4 and v2 == 4, "undo did not restore the orders")
print("M52-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M52-2 OK" "$OUT/s2.log" || { echo "FAIL: tess"; tail -8 "$OUT/s2.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: tess undo chain is not byte-identical"; exit 1; }
echo "OK tess down/up/undo byte-identical"

echo "===== M52-2b: a painted marker survives shrink + grow, across reopen ====="
seed "$OUT/ws2b" "$B"
cat > "$OUT/s2b.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 5, 0, 0, 100, 0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.setPatchTess(3, 3)
local t = painter.tileAt(0, 5, 0, 0)
assert(t == 100, "marker lost in the shrink")
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.setPatchTess(4, 4)
t = painter.tileAt(0, 5, 0, 0)
assert(t == 100, "marker lost in the grow-back")
local t2 = painter.tileAt(0, 5, 15, 15)
assert(t2 ~= 100, "the grown remainder is not fresh")
print("M52-2B OK")
EOF
cat > "$OUT/s2c.lua" <<'EOF'
painter.setMode(0)
assert(painter.tileAt(0, 5, 0, 0) == 100, "marker did not persist")
print("M52-2C OK")
EOF
run_session "$OUT/ws2b" "$B" "$OUT/s2b.lua" "$OUT/s2b.log"
grep -aq "M52-2B OK" "$OUT/s2b.log" || { echo "FAIL: marker keep"; tail -8 "$OUT/s2b.log"; exit 1; }
run_verify "$OUT/ws2b" "$B" "$OUT/s2c.lua" "$OUT/s2c.log"
grep -aq "M52-2C OK" "$OUT/s2c.log" || { echo "FAIL: marker persistence"; tail -6 "$OUT/s2c.log"; exit 1; }
echo "OK corner-anchored keep + persistence"

echo "===== M52-3: balance onto the max per axis ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.setPatchTess(3, 3)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 10, 1)
painter.balanceTessSelection()
local u, v = painter.patchTess(0, 5)
assert(u == 4 and v == 4, "balance did not land on the max per axis")
print("M52-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M52-3 OK" "$OUT/s3.log" || { echo "FAIL: balance"; tail -8 "$OUT/s3.log"; exit 1; }
echo "OK balance"

echo "===== M52-4: tess refuses on a bind target, file untouched ====="
I=zonematerial-bassin-ilot_croix
"$ZP" "$GFX/landscape/ligo/lacustre/max/$I.max" --null-edit --out "$OUT/ilot.null.max" > "$OUT/ilot.null.log" 2>&1
seed "$OUT/ws4" "$I"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 229, 1)
painter.setPatchTess(3, 0)
print("M52-4 DONE")
EOF
run_session "$OUT/ws4" "$I" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M52-4 DONE" "$OUT/s4.log" || { echo "FAIL: refusal scene"; tail -6 "$OUT/s4.log"; exit 1; }
grep -aq "bind target" "$OUT/s4.log" || { echo "FAIL: no bind-target refusal printed"; exit 1; }
cmp -s "$OUT/ilot.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$I.max" \
	|| { echo "FAIL: refused tess changed the file"; exit 1; }
echo "OK bind-target refusal leaves the file alone"

echo "===== M52-5: modifier-stream target ====="
M=material-fond
seed "$OUT/ws5" "$M"
cat > "$OUT/s5.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.setSmoothGroup(1, true)
print("M52-5 OK")
EOF
run_session "$OUT/ws5" "$M" "$OUT/s5.lua" "$OUT/s5.log"
grep -aq "M52-5 OK" "$OUT/s5.log" || { echo "FAIL: modifier scene"; tail -6 "$OUT/s5.log"; exit 1; }
grep -aq "smoothing group: zone 0, 1 patches (modifier target)" "$OUT/s5.log" \
	|| { echo "FAIL: expected the modifier target"; grep -a "smoothing group" "$OUT/s5.log"; exit 1; }
echo "OK modifier target"

echo "ALL M52 GATES PASSED"
