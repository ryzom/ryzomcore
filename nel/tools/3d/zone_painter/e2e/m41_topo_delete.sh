#!/usr/bin/env bash
# M41 topological delete gates: the first Tier B op that changes element counts, with PAINT
# SURVIVAL as the acceptance bar (the headline goal: the legacy editor destroyed painted
# tiles on any topological edit; ours must keep them wherever the surface survived).
#
#  1. BASE-stream target (zonematerial-bassin-1, no modifier stack): paint a tile on patch
#     5, delete patch 0. The count drops by one and the SAME tile record reads back at
#     patch index 4 - the paint traveled with its surviving patch through the index shift -
#     both in-session (across the working-set rebuild) and after save + reopen.
#  2. MODIFIER-stream target with a vertex mapper (material-bassin): delete the LAST patch.
#     The eval position of vertex 0 must not move by a single bit (the mapper's
#     input-indexed records survived the output remap), and the file reopens at the new
#     count - the whole eval chain (0x1140 + 0x1130 + 0x4001) parses the mutated streams.
#  3. Bind interaction (zonematerial-bassin-ilot_croix): patch 229 is the bind TARGET of
#     two BIND_SINGLE records (vertices 445 and 496). Deleting it releases exactly those
#     two (94 -> 92 bound), persisted across reopen.
#  4. Refusals (empty selection, delete-everything) leave the topology alone.
#  5. The saved topology-edited file passes the encoder's own whole-file identity proof
#     (--pm-modify-save-test): our output is well-formed input.
#  6. TOPOLOGY UNDO: delete -> undo -> redo -> undo replays through the Kind 6 raw
#     snapshots and the working-set rebuild, and the final save is BYTE-IDENTICAL to the
#     null-edit baseline - on the base-stream file and on the modifier+mapper file alike
#     (fresh chunks inherit their siblings' 64-bit header width; a 32-bit default would
#     shift every later byte).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m41_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { # $1 = workspace dir, $2 = basename
	rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

run_session() { # $1 = ws, $2 = basename, $3 = lua, $4 = log ; saves via the board action
	ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
		--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1
}

run_verify() { # $1 = ws, $2 = basename, $3 = lua, $4 = log ; no save
	$XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
		--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1
}

echo "===== M41-1: base target - delete shifts indices, paint travels ====="
cat > "$OUT/base.lua" <<'EOF'
painter.setMode(0)
painter.paintTile(0, 5, 0.2, 0.2, 2)
local t0, r0, n0 = painter.tileAt(0, 5, 1, 1)
assert(n0 and n0 > 0, "paint did not land")
local c0 = painter.patchCount(0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 0, 1)
local n = painter.deletePatchSelection()
assert(n == 1, "expected 1 deleted")
assert(painter.patchCount(0) == c0 - 1, "count did not drop")
local t1, r1, n1 = painter.tileAt(0, 4, 1, 1)
assert(t1 == t0 and r1 == r0 and n1 == n0,
       string.format("paint did not travel: %s/%s/%s vs %s/%s/%s",
                     tostring(t1), tostring(r1), tostring(n1), tostring(t0), tostring(r0), tostring(n0)))
-- Topology undo restores through the Kind 6 snapshot, redo reapplies.
painter.undo()
assert(painter.patchCount(0) == c0, "undo did not restore the deleted patch")
local tu = painter.tileAt(0, 5, 1, 1)
assert(tu == t0, "undo lost the painted tile at its old index")
painter.redo()
assert(painter.patchCount(0) == c0 - 1, "redo did not reapply the delete")
print(string.format("M41-1 OK tile=%d count=%d", t0, c0 - 1))
EOF
seed "$OUT/ws1" zonematerial-bassin-1
run_session "$OUT/ws1" zonematerial-bassin-1 "$OUT/base.lua" "$OUT/g1.log"
T=$(grep -a "M41-1 OK" "$OUT/g1.log" | sed 's/.*tile=\([0-9]*\).*/\1/')
C=$(grep -a "M41-1 OK" "$OUT/g1.log" | sed 's/.*count=\([0-9]*\).*/\1/')
[[ -n "$T" && -n "$C" ]] || { echo "FAIL: base delete script"; tail -8 "$OUT/g1.log"; exit 1; }
grep -aq "(base target)" "$OUT/g1.log" || { echo "FAIL: expected the base-stream target"; grep -a "delete:" "$OUT/g1.log"; exit 1; }
cat > "$OUT/base_v.lua" <<EOF
assert(painter.patchCount(0) == $C, "reopen count wrong")
local t1 = painter.tileAt(0, 4, 1, 1)
assert(t1 == $T, "reopen lost the traveled paint")
print("M41-1 reopen OK")
EOF
run_verify "$OUT/ws1" zonematerial-bassin-1 "$OUT/base_v.lua" "$OUT/g1v.log"
grep -aq "M41-1 reopen OK" "$OUT/g1v.log" || { echo "FAIL: base delete did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
echo "OK base target (paint traveled patch 5 -> 4, persisted)"

echo "===== M41-2: modifier + mapper target - eval positions bit-stable ====="
cat > "$OUT/mod.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
local x0, y0, z0 = painter.patchVertexPos(0, 0)
local c0 = painter.patchCount(0)
painter.setSubObject(3)
painter.selectPatchFace(0, c0 - 1, 1)
local n = painter.deletePatchSelection()
assert(n == 1, "expected 1 deleted")
local x1, y1, z1 = painter.patchVertexPos(0, 0)
assert(x1 == x0 and y1 == y0 and z1 == z0, "mapper-driven vertex moved")
print("M41-2 OK count=" .. (c0 - 1))
EOF
seed "$OUT/ws2" material-bassin
run_session "$OUT/ws2" material-bassin "$OUT/mod.lua" "$OUT/g2.log"
C2=$(grep -a "M41-2 OK" "$OUT/g2.log" | sed 's/.*count=\([0-9]*\).*/\1/')
[[ -n "$C2" ]] || { echo "FAIL: modifier delete script"; tail -8 "$OUT/g2.log"; exit 1; }
grep -aq "(modifier target)" "$OUT/g2.log" || { echo "FAIL: expected the modifier-stream target"; grep -a "delete:" "$OUT/g2.log"; exit 1; }
cat > "$OUT/mod_v.lua" <<EOF
assert(painter.patchCount(0) == $C2, "reopen count wrong")
print("M41-2 reopen OK")
EOF
run_verify "$OUT/ws2" material-bassin "$OUT/mod_v.lua" "$OUT/g2v.log"
grep -aq "M41-2 reopen OK" "$OUT/g2v.log" || { echo "FAIL: modifier delete did not persist"; tail -5 "$OUT/g2v.log"; exit 1; }
echo "OK modifier target (mapper remap bit-stable, persisted)"

echo "===== M41-3: deleting a bind target releases its records ====="
cat > "$OUT/bind.lua" <<'EOF'
painter.setMode(4)
local function boundCount()
  local n = 0
  for v = 0, 1999 do
    local b = painter.vertexBindInfo(0, v)
    if b == nil then break end
    if b == 1 then n = n + 1 end
  end
  return n
end
assert(boundCount() == 94, "fixture drifted")
painter.setSubObject(3)
painter.selectPatchFace(0, 229, 1)
assert(painter.deletePatchSelection() == 1)
local a = boundCount()
assert(a == 92, "expected 92 bound after, got " .. a)
print("M41-3 OK")
EOF
seed "$OUT/ws3" zonematerial-bassin-ilot_croix
run_session "$OUT/ws3" zonematerial-bassin-ilot_croix "$OUT/bind.lua" "$OUT/g3.log"
grep -aq "M41-3 OK" "$OUT/g3.log" || { echo "FAIL: bind-release script"; tail -8 "$OUT/g3.log"; exit 1; }
cat > "$OUT/bind_v.lua" <<'EOF'
local n = 0
for v = 0, 1999 do
  local b = painter.vertexBindInfo(0, v)
  if b == nil then break end
  if b == 1 then n = n + 1 end
end
assert(n == 92, "reopen bound count " .. n)
print("M41-3 reopen OK")
EOF
run_verify "$OUT/ws3" zonematerial-bassin-ilot_croix "$OUT/bind_v.lua" "$OUT/g3v.log"
grep -aq "M41-3 reopen OK" "$OUT/g3v.log" || { echo "FAIL: bind release did not persist"; tail -5 "$OUT/g3v.log"; exit 1; }
echo "OK bind release (94 -> 92, persisted)"

echo "===== M41-4: refusals leave the topology alone ====="
cat > "$OUT/refuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
assert(painter.deletePatchSelection() == 0, "empty selection deleted something")
-- Deleting EVERY patch must refuse (an empty object is not a zone).
for p = 0, c0 - 1 do painter.selectPatchFace(0, p, 1) end
assert(painter.deletePatchSelection() == 0, "deleting all patches was not refused")
assert(painter.patchCount(0) == c0, "refusal still changed the topology")
print("M41-4 OK")
EOF
seed "$OUT/ws4" zonematerial-bassin-1
run_session "$OUT/ws4" zonematerial-bassin-1 "$OUT/refuse.lua" "$OUT/g4.log"
grep -aq "M41-4 OK" "$OUT/g4.log" || { echo "FAIL: refusal script"; tail -8 "$OUT/g4.log"; exit 1; }
echo "OK refusals"

echo "===== M41-5: the saved topology-edited files are well-formed encoder input ====="
for pair in "ws1:zonematerial-bassin-1" "ws2:material-bassin" "ws3:zonematerial-bassin-ilot_croix"; do
	W="${pair%%:*}"; B="${pair##*:}"
	"$PMCT" --pm-modify-save-test "$OUT/$W/landscape/ligo/lacustre/max/$B.max" > "$OUT/$W.pm.log" 2>&1 \
		|| { echo "FAIL: pm-modify-save on edited $B"; tail -5 "$OUT/$W.pm.log"; exit 1; }
	grep -aq "^OK pm-modify-save" "$OUT/$W.pm.log" || { echo "FAIL: pm identity on edited $B"; exit 1; }
done
echo "OK edited files round-trip the encoder byte-identically"

echo "===== M41-6: topology undo saves byte-identical to the baseline ====="
cat > "$OUT/undo_cycle.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
painter.selectPatchFace(0, 0, 1)
assert(painter.deletePatchSelection() == 1)
painter.undo()
assert(painter.patchCount(0) == c0, "undo did not restore")
painter.redo()
assert(painter.patchCount(0) == c0 - 1, "redo did not reapply")
painter.undo()
assert(painter.patchCount(0) == c0, "second undo did not restore")
print("M41-6 cycle OK")
EOF
for B6 in zonematerial-bassin-1 material-bassin; do
	"$ZP" "$GFX/landscape/ligo/lacustre/max/$B6.max" --null-edit --out "$OUT/$B6.null.max" > "$OUT/$B6.null.log" 2>&1
	seed "$OUT/ws6_$B6" "$B6"
	run_session "$OUT/ws6_$B6" "$B6" "$OUT/undo_cycle.lua" "$OUT/g6_$B6.log"
	grep -aq "M41-6 cycle OK" "$OUT/g6_$B6.log" || { echo "FAIL: undo cycle on $B6"; tail -8 "$OUT/g6_$B6.log"; exit 1; }
	U=$( { cmp -l "$OUT/$B6.null.max" "$OUT/ws6_$B6/landscape/ligo/lacustre/max/$B6.max" || true; } | wc -l)
	[[ "$U" -eq 0 ]] || { echo "FAIL: undo cycle on $B6 left $U bytes changed"; exit 1; }
	echo "OK $B6 undo cycle byte-identical"
done

echo "ALL M41 GATES PASSED"
