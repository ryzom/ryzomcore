#!/usr/bin/env bash
# M47 move-to-zone gates: the cross-file patch transfer (Kaetemi's op, no legacy
# equivalent - the session shows several brick files at once, the old plugin never did).
# Selected patches leave their file and join the neighbor's, keeping their place in the
# world; both files stay open and the whole transfer is ONE two-snapshot Kind 6 stroke.
#
#  1. THE TRANSFER (bassin-1 + material-fond opened one cell WEST): compass W resolves to
#     the neighbor and E resolves to nothing; a marker painted on the source right before
#     the move (pending, never hand-saved) appears on the transferred patch in the
#     DESTINATION; counts move 2 across; the transferred corner keeps its WORLD position;
#     both saved files persist their counts across reopen and round-trip the encoder.
#  2. UNDO: ONE undo restores BOTH files - counts back on both sides, and both saved
#     files byte-identical to their null-edit baselines (the two snapshots ride one
#     stroke).
#  3. REFUSALS: no selection refuses; whole-zone selection refuses; moving to the same
#     object refuses. Nothing changes.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m47_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

B=zonematerial-bassin-1
F=material-fond
MAXDIR="landscape/ligo/lacustre/max"

seed() { rm -rf "$1"; mkdir -p "$1/$MAXDIR"
	cp "$GFX/$MAXDIR/$B.max" "$1/$MAXDIR/"
	cp "$GFX/$MAXDIR/$F.max" "$1/$MAXDIR/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }
run_verify() { $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

echo "===== M47-1: the transfer - compass, pending paint, world position ====="
cat > "$OUT/move.lua" <<'EOF'
assert(painter.openZone("material-fond", -1, 0))
local c0 = painter.patchCount(0)
local cD = painter.patchCount(1000)
local vD = painter.vertexCount(1000)
painter.setMode(0)
painter.rawTile(0, 5, 2, 3, 77, 0) -- pending paint on a patch about to move
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.moveDirTarget(6) == 1000, "west neighbor did not resolve")
assert(painter.moveDirTarget(2) == nil, "east neighbor should not exist")
local a = painter.patchEdgeVerts(0, 5, 0)
local wx, wy, wz = painter.patchVertexPos(0, a)
assert(painter.movePatchSelectionToZone(1000) == 2, "move failed")
assert(painter.patchCount(0) == c0 - 2, "source count")
assert(painter.patchCount(1000) == cD + 2, "destination count")
assert(painter.tileAt(1000, cD, 2, 3) == 77, "pending marker did not travel")
-- the moved corner keeps its world position: nearest new destination vertex is where
-- the source corner was
local best = nil
for i = vD, painter.vertexCount(1000) - 1 do
  local x, y, z = painter.patchVertexPos(1000, i)
  local d = math.abs(x - wx) + math.abs(y - wy) + math.abs(z - wz)
  if not best or d < best then best = d end
end
assert(best and best < 0.01, "moved geometry drifted " .. tostring(best))
painter.saveZone("material-fond")
print("M47-1 OK src=" .. (c0 - 2) .. " dst=" .. (cD + 2))
EOF
seed "$OUT/ws1"
run_session "$OUT/ws1" "$B" "$OUT/move.lua" "$OUT/g1.log"
CV=$(grep -a "M47-1 OK" "$OUT/g1.log" | sed 's/.*OK //')
[[ -n "$CV" ]] || { echo "FAIL: move script"; tail -10 "$OUT/g1.log"; exit 1; }
SRCC=$(echo "$CV" | sed 's/src=\([0-9]*\).*/\1/')
DSTC=$(echo "$CV" | sed 's/.*dst=//')
cat > "$OUT/move_v.lua" <<EOF
assert(painter.patchCount(0) == $SRCC, "source reopen count")
print("M47-1 src reopen OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/move_v.lua" "$OUT/g1v.log"
grep -aq "M47-1 src reopen OK" "$OUT/g1v.log" || { echo "FAIL: source did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
cat > "$OUT/move_vd.lua" <<EOF
assert(painter.patchCount(0) == $DSTC, "destination reopen count")
assert(painter.tileAt(0, $DSTC - 2, 2, 3) == 77, "marker lost on reopen")
print("M47-1 dst reopen OK")
EOF
run_verify "$OUT/ws1" "$F" "$OUT/move_vd.lua" "$OUT/g1vd.log"
grep -aq "M47-1 dst reopen OK" "$OUT/g1vd.log" || { echo "FAIL: destination did not persist"; tail -5 "$OUT/g1vd.log"; exit 1; }
for FF in "$B" "$F"; do
	"$PMCT" --pm-modify-save-test "$OUT/ws1/$MAXDIR/$FF.max" > "$OUT/g1.$FF.pm.log" 2>&1 \
		|| { echo "FAIL: pm round-trip on $FF"; tail -4 "$OUT/g1.$FF.pm.log"; exit 1; }
	grep -aq "^OK pm-modify-save" "$OUT/g1.$FF.pm.log" || { echo "FAIL: pm identity on $FF"; exit 1; }
done
echo "OK the transfer (compass, pending paint travels, world position keeps, round-trips)"

echo "===== M47-2: ONE undo restores BOTH files byte-exactly ====="
"$ZP" "$GFX/$MAXDIR/$B.max" --null-edit --out "$OUT/$B.null.max" > "$OUT/nb.log" 2>&1
"$ZP" "$GFX/$MAXDIR/$F.max" --null-edit --out "$OUT/$F.null.max" > "$OUT/nf.log" 2>&1
cat > "$OUT/undo.lua" <<'EOF'
assert(painter.openZone("material-fond", -1, 0))
local c0 = painter.patchCount(0)
local cD = painter.patchCount(1000)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
assert(painter.movePatchSelectionToZone(1000) == 1)
painter.undo()
assert(painter.patchCount(0) == c0, "source undo count")
assert(painter.patchCount(1000) == cD, "destination undo count")
painter.saveZone("material-fond")
print("M47-2 OK")
EOF
seed "$OUT/ws2"
run_session "$OUT/ws2" "$B" "$OUT/undo.lua" "$OUT/g2.log"
grep -aq "M47-2 OK" "$OUT/g2.log" || { echo "FAIL: undo script"; tail -8 "$OUT/g2.log"; exit 1; }
for pair in "$B" "$F"; do
	U=$( { cmp -l "$OUT/$pair.null.max" "$OUT/ws2/$MAXDIR/$pair.max" || true; } | wc -l)
	[[ "$U" -eq 0 ]] || { echo "FAIL: undo left $U bytes changed in $pair"; exit 1; }
done
echo "OK one undo restores both files byte-identically"

echo "===== M47-3: refusals ====="
cat > "$OUT/refuse.lua" <<'EOF'
assert(painter.openZone("material-fond", -1, 0))
local c0 = painter.patchCount(0)
painter.setMode(4)
painter.setSubObject(3)
assert(painter.movePatchSelectionToZone(1000) == 0, "empty selection moved")
painter.selectPatchFace(0, 3, 1)
assert(painter.movePatchSelectionToZone(0) == 0, "same-object move did not refuse")
for p = 0, c0 - 1 do painter.selectPatchFace(0, p, 1) end
assert(painter.movePatchSelectionToZone(1000) == 0, "whole zone moved")
assert(painter.patchCount(0) == c0)
print("M47-3 OK")
EOF
seed "$OUT/ws3"
run_session "$OUT/ws3" "$B" "$OUT/refuse.lua" "$OUT/g3.log"
grep -aq "M47-3 OK" "$OUT/g3.log" || { echo "FAIL: refusals"; tail -6 "$OUT/g3.log"; exit 1; }
grep -aq "whole zone" "$OUT/g3.log" || { echo "FAIL: expected the whole-zone refusal"; exit 1; }
echo "OK refusals"

echo "ALL M47 GATES PASSED"
