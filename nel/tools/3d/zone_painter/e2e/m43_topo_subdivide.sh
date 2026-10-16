#!/usr/bin/env bash
# M43 subdivide gates: the exact bicubic 4-way split with paint-quadrant inheritance - the
# Tier B headline op - through the shared topology machinery (Kind 6 undo, keepUndo
# rebuild, byte identity).
#
#  1. QUADRANT INHERITANCE: four raw marker tiles at the corners of 16x16 patch 5 land at
#     the four children's matching corners after subdivide (child grids are 8x8 and keep
#     the parent's orientation - no rotation, plain quadrant copy: (0,0)->q0(0,0),
#     (15,0)->q3(7,0), (0,15)->q1(0,7), (15,15)->q2(7,7)). The subdivide creates exactly
#     the T-junction binds its shared edges need (3 on this patch), every bind is a
#     BIND_SINGLE with itself as primary, everything persists across save+reopen, and the
#     saved file round-trips the encoder byte-identically.
#  2. UNDO: subdivide -> undo restores the topology AND the pre-op marker, and the saved
#     file is byte-identical to the null-edit baseline.
#  3. PAIR: subdividing two adjacent patches together truly splits the shared edge (no
#     bind between them: +6 patches, 4 outer T-binds) and undoes byte-identically.
#  4. MODIFIER TARGET: subdividing on a mapper-carrying file leaves the eval position of
#     vertex 0 bit-stable (the mapper needs no rewrite for a pure addition) and reopens at
#     the new count.
#  5. REFUSAL: subdividing a patch that is a bind TARGET refuses and changes nothing
#     (splitting a T-junction target would strand its bound vertices).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m43_out
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

echo "===== M43-1: quadrant inheritance + T-binds + persistence + encoder round-trip ====="
cat > "$OUT/sub.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 5, 0, 0, 100, 0)
painter.rawTile(0, 5, 15, 0, 101, 0)
painter.rawTile(0, 5, 0, 15, 102, 0)
painter.rawTile(0, 5, 15, 15, 103, 0)
local c0 = painter.patchCount(0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.subdividePatchSelection() == 1)
assert(painter.patchCount(0) == c0 + 3, "count")
assert(painter.tileAt(0, 5, 0, 0) == 100, "q0 marker")
assert(painter.tileAt(0, c0, 0, 7) == 102, "q1 marker")
assert(painter.tileAt(0, c0 + 1, 7, 7) == 103, "q2 marker")
assert(painter.tileAt(0, c0 + 2, 7, 0) == 101, "q3 marker")
local nb = 0
for v = 0, 4999 do
  local b, t, p, e, prim = painter.vertexBindInfo(0, v)
  if b == nil then break end
  if b == 1 then
    nb = nb + 1
    assert(t == 3 and prim == v, "T-bind record shape")
  end
end
assert(nb == 3, "expected 3 T-binds, got " .. nb)
print("M43-1 OK")
EOF
seed "$OUT/ws1" "$B"
run_session "$OUT/ws1" "$B" "$OUT/sub.lua" "$OUT/g1.log"
grep -aq "M43-1 OK" "$OUT/g1.log" || { echo "FAIL: subdivide script"; tail -8 "$OUT/g1.log"; exit 1; }
grep -aq "(base target)" "$OUT/g1.log" || { echo "FAIL: expected the base target"; exit 1; }
cat > "$OUT/sub_v.lua" <<'EOF'
assert(painter.patchCount(0) == 28)
assert(painter.tileAt(0, 5, 0, 0) == 100)
assert(painter.tileAt(0, 25, 0, 7) == 102)
assert(painter.tileAt(0, 26, 7, 7) == 103)
assert(painter.tileAt(0, 27, 7, 0) == 101)
print("M43-1 reopen OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/sub_v.lua" "$OUT/g1v.log"
grep -aq "M43-1 reopen OK" "$OUT/g1v.log" || { echo "FAIL: subdivide did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/g1.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on subdivided file"; tail -4 "$OUT/g1.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1.pm.log" || { echo "FAIL: pm identity on subdivided file"; exit 1; }
echo "OK quadrant inheritance (paint kept exactly), 3 T-binds, persisted, round-trips"

echo "===== M43-2: subdivide -> undo is byte-identical, marker restored ====="
cat > "$OUT/sub_undo.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 5, 0, 0, 100, 0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.subdividePatchSelection() == 1)
painter.undo()
assert(painter.patchCount(0) == 25, "undo count")
assert(painter.tileAt(0, 5, 0, 0) == 100, "undo marker")
painter.undo()
print("M43-2 OK")
EOF
seed "$OUT/ws2" "$B"
run_session "$OUT/ws2" "$B" "$OUT/sub_undo.lua" "$OUT/g2.log"
grep -aq "M43-2 OK" "$OUT/g2.log" || { echo "FAIL: subdivide-undo script"; tail -6 "$OUT/g2.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: undo left $U bytes"; exit 1; }
echo "OK undo byte-identical"

echo "===== M43-3: adjacent pair - shared edge truly splits, no bind between ====="
cat > "$OUT/pair.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 10, 1)
assert(painter.subdividePatchSelection() == 2)
assert(painter.patchCount(0) == 31, "pair count")
local nb = 0
for v = 0, 4999 do
  local b = painter.vertexBindInfo(0, v)
  if b == nil then break end
  if b == 1 then nb = nb + 1 end
end
assert(nb == 4, "expected 4 outer T-binds, got " .. nb)
painter.undo()
assert(painter.patchCount(0) == 25)
print("M43-3 OK")
EOF
seed "$OUT/ws3" "$B"
run_session "$OUT/ws3" "$B" "$OUT/pair.lua" "$OUT/g3.log"
grep -aq "M43-3 OK" "$OUT/g3.log" || { echo "FAIL: pair script"; tail -6 "$OUT/g3.log"; exit 1; }
P=$( { cmp -l "$OUT/base.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
[[ "$P" -eq 0 ]] || { echo "FAIL: pair undo left $P bytes"; exit 1; }
echo "OK pair split (4 outer binds, shared edge unbound), undo byte-identical"

echo "===== M43-4: modifier + mapper target - eval bit-stable, persists ====="
B4=material-bassin
cat > "$OUT/mod.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local x0, y0, z0 = painter.patchVertexPos(0, 0)
local c0 = painter.patchCount(0)
painter.setSubObject(3)
painter.selectPatchFace(0, c0 - 1, 1)
assert(painter.subdividePatchSelection() == 1)
local x1, y1, z1 = painter.patchVertexPos(0, 0)
assert(x1 == x0 and y1 == y0 and z1 == z0, "mapper vertex moved")
print("M43-4 OK count=" .. (c0 + 3))
EOF
seed "$OUT/ws4" "$B4"
run_session "$OUT/ws4" "$B4" "$OUT/mod.lua" "$OUT/g4.log"
C4=$(grep -a "M43-4 OK" "$OUT/g4.log" | sed 's/.*count=//')
[[ -n "$C4" ]] || { echo "FAIL: modifier subdivide"; tail -6 "$OUT/g4.log"; exit 1; }
grep -aq "(modifier target)" "$OUT/g4.log" || { echo "FAIL: expected the modifier target"; exit 1; }
cat > "$OUT/mod_v.lua" <<EOF
assert(painter.patchCount(0) == $C4, "reopen count")
print("M43-4 reopen OK")
EOF
run_verify "$OUT/ws4" "$B4" "$OUT/mod_v.lua" "$OUT/g4v.log"
grep -aq "M43-4 reopen OK" "$OUT/g4v.log" || { echo "FAIL: modifier subdivide did not persist"; tail -5 "$OUT/g4v.log"; exit 1; }
echo "OK modifier target"

echo "===== M43-5: subdividing a bind target refuses ====="
B5=zonematerial-bassin-ilot_croix
cat > "$OUT/refuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
painter.selectPatchFace(0, 229, 1)
assert(painter.subdividePatchSelection() == 0, "bind target was subdivided")
assert(painter.patchCount(0) == c0, "refusal changed the topology")
print("M43-5 OK")
EOF
seed "$OUT/ws5" "$B5"
run_session "$OUT/ws5" "$B5" "$OUT/refuse.lua" "$OUT/g5.log"
grep -aq "M43-5 OK" "$OUT/g5.log" || { echo "FAIL: refusal script"; tail -6 "$OUT/g5.log"; exit 1; }
echo "OK refusal"

echo "ALL M43 GATES PASSED"
