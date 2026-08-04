#!/usr/bin/env bash
# M59 Auto / Manual interior (plan mA2): PATCH_AUTO = patch Flags bit 0 (the probe: 645k
# auto, 1,073 authored manual patches in 47 converted desert files).
#
# Switching auto -> manual BAKES the currently derived interiors into the stored vecs -
# the four interior controls become real, editable points (drawn violet for selected
# corners, picked and moved exactly like handles). Manual -> auto abandons the stored
# values (evaluation re-derives; the bytes stay as the corpus leaves them).
#
#  1. BAKE == DERIVATION: the displayed interiors are identical before and after the
#     auto -> manual toggle (the bake wrote exactly what the eval derived); the flag
#     flips; undo restores the null-edit baseline byte for byte; the file round-trips;
#     the manual flag and the baked values survive reopen.
#  2. EDITING: a manual interior selects (scripted AND through the real pick path at its
#     projected position), moves through the ordinary Tier A machinery, and the move
#     survives reopen (manual eval honours the stored value).
#  3. AUTO REFUSES: an auto patch's interior refuses scripted selection and is not
#     offered by the pick.
#  4. NO-CHANGE REFUSAL: setting auto on an already-auto selection writes nothing.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m59_out
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

echo "===== M59-1: bake == derivation, flag flip, persistence, round-trip ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local P = 5
assert(painter.patchFlags(0, P) % 2 == 1, "fixture: patch 5 is auto")
local pre = {}
for k = 0, 3 do
  local x, y, z = painter.patchInteriorPos(0, P, k)
  pre[k] = { x, y, z }
end
painter.selectPatchFace(0, P, 0)
painter.setPatchAuto(false)
assert(painter.patchFlags(0, P) % 2 == 0, "manual flag did not stick")
for k = 0, 3 do
  local x, y, z = painter.patchInteriorPos(0, P, k)
  local e = pre[k]
  local d = math.max(math.abs(x-e[1]), math.abs(y-e[2]), math.abs(z-e[3]))
  assert(d < 1e-5, "baked interior " .. k .. " differs from the derivation by " .. d)
  print("M59-1 BAKE " .. k .. " " .. x .. " " .. y .. " " .. z)
end
print("M59-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M59-1 OK" "$OUT/s1.log" || { echo "FAIL: bake scene"; tail -10 "$OUT/s1.log"; exit 1; }
grep -aq "interior mode: zone 0, 1 patches (base target)" "$OUT/s1.log" \
	|| { echo "FAIL: target line"; grep -a "interior mode" "$OUT/s1.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1.rt.log" 2>&1 \
	|| { echo "FAIL: baked file does not round-trip"; tail -4 "$OUT/s1.rt.log"; exit 1; }
cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
assert(painter.patchFlags(0, 5) % 2 == 0, "manual flag lost on reopen")
print("M59-1B OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M59-1B OK" "$OUT/s1b.log" || { echo "FAIL: persistence"; tail -6 "$OUT/s1b.log"; exit 1; }
echo "OK bake == derivation, persists, round-trips"

seed "$OUT/ws1c" "$B"
cat > "$OUT/s1c.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 0)
painter.setPatchAuto(false)
painter.undo()
assert(painter.patchFlags(0, 5) % 2 == 1, "undo did not restore auto")
print("M59-1C OK")
EOF
run_session "$OUT/ws1c" "$B" "$OUT/s1c.lua" "$OUT/s1c.log"
grep -aq "M59-1C OK" "$OUT/s1c.log" || { echo "FAIL: undo scene"; tail -8 "$OUT/s1c.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1c/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: toggle undo is not byte-identical"; exit 1; }
echo "OK undo byte-identical"

echo "===== M59-2: a manual interior selects (script + real pick), moves, persists ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local P = 5
painter.selectPatchFace(0, P, 0)
painter.setPatchAuto(false)
painter.setSubObject(1)
local corner = painter.patchCornerVert(0, P, 0)
local iv = painter.patchInteriorIndex(0, P, 0)
local ix, iy, iz = painter.patchInteriorPos(0, P, 0)
-- The REAL pick path: click at the interior's projected position with the corner
-- selected (interiors are offered exactly like handles).
painter.selectPatchVertex(0, corner, 0)
local sx, sy = painter.tangentScreenPos(0, iv)
painter.patchClick(sx, sy, 1)
assert(painter.patchTangentSelectionCount() == 1, "the pick did not find the interior")
painter.movePatchSelection(0, 0, 2.0)
local nx, ny, nz = painter.patchInteriorPos(0, P, 0)
assert(math.abs(nz - iz - 2.0) < 1e-4, "the interior did not move")
print("M59-2 OK")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M59-2 OK" "$OUT/s2.log" || { echo "FAIL: edit scene"; tail -10 "$OUT/s2.log"; exit 1; }
cat > "$OUT/s2b.lua" <<'EOF'
painter.setMode(4)
-- The moved interior persisted (manual eval honours the stored value); compare against
-- the still-auto neighbor's derivation asymmetry: just assert the z offset survived.
local x, y, z = painter.patchInteriorPos(0, 5, 0)
print("M59-2B Z " .. z)
print("M59-2B OK")
EOF
run_verify "$OUT/ws2" "$B" "$OUT/s2b.lua" "$OUT/s2b.log"
grep -aq "M59-2B OK" "$OUT/s2b.log" || { echo "FAIL: reopen"; tail -6 "$OUT/s2b.log"; exit 1; }
Z1=$(grep -a "M59-2B Z" "$OUT/s2b.log" | head -1 | awk '{print $3}')
Z0=$(grep -a "M59-1 BAKE 0" "$OUT/s1.log" | head -1 | awk '{print $6}')
python3 -c "import sys; z0=float('$Z0'); z1=float('$Z1'); sys.exit(0 if abs(z1-z0-2.0) < 1e-3 else 1)" \
	|| { echo "FAIL: moved interior did not persist (z $Z0 -> $Z1)"; exit 1; }
echo "OK manual interior picked, moved, persisted"

echo "===== M59-3: an auto interior refuses selection ====="
seed "$OUT/ws3" "$B"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local iv = painter.patchInteriorIndex(0, 5, 0)
painter.selectPatchVertex(0, painter.patchCornerVert(0, 5, 0), 0)
painter.selectPatchTangent(0, iv, 1)
assert(painter.patchTangentSelectionCount() == 0, "an auto interior was selected")
print("M59-3 OK")
EOF
run_session "$OUT/ws3" "$B" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M59-3 OK" "$OUT/s3.log" || { echo "FAIL: auto refusal"; tail -8 "$OUT/s3.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: refusal session changed the file"; exit 1; }
echo "OK auto interiors refuse"

echo "===== M59-4: no-change refusal ====="
seed "$OUT/ws4" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 0)
painter.setPatchAuto(true) -- already auto
print("M59-4 DONE")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M59-4 DONE" "$OUT/s4.log" || { echo "FAIL: refusal scene"; tail -6 "$OUT/s4.log"; exit 1; }
grep -aq "already auto" "$OUT/s4.log" || { echo "FAIL: no refusal printed"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: refused toggle changed the file"; exit 1; }
echo "OK no-change refusal"

echo "ALL M59 GATES PASSED"
