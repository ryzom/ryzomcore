#!/usr/bin/env bash
# M46 attach gates: merge another open editable zone into this one - the session-level
# inverse of detach. The source zone's mesh is appended onto the target's write-target
# stream reoriented through the two display transforms (world positions keep), paint and
# binds travel (pending paint included - the op flushes first), and the source FILE is
# saved and closed: its brick on disk keeps everything it had, so the merge loses nothing.
#
#  1. THE MERGE (bassin-1 + material-fond at cell -2,0): a marker painted on the SOURCE
#     right before the attach appears in the target at the appended index (paint travels,
#     unsaved paint included); patch and vertex counts add up; a source vertex keeps its
#     WORLD position through the reorientation; the source zone is gone from the session;
#     the saved target persists across reopen and round-trips the encoder; and the closed
#     source file kept the marker on disk (saved on close - nothing lost).
#  2. UNDO: attach, undo, save - the target is byte-identical to its null-edit baseline
#     (the attach is the fresh undo stack's first stroke after the close).
#  3. REFUSALS: attaching a zone to its own object refuses; attaching a scripted INSTANCE
#     zone refuses (not an open editable file); neither changes the target.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m46_out
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

echo "===== M46-1: the merge - paint travels, world positions keep, source closes ====="
cat > "$OUT/merge.lua" <<'EOF'
assert(painter.openZone("material-fond", -2, 0))
local c0 = painter.patchCount(0)
local v0 = painter.vertexCount(0)
local cS = painter.patchCount(1000)
local vS = painter.vertexCount(1000)
painter.setMode(0)
painter.rawTile(1000, 2, 1, 1, 77, 0) -- pending paint on the SOURCE, never saved by hand
local wx, wy, wz = painter.patchVertexPos(1000, 0)
local tx, ty, tz = painter.patchVertexPos(0, 0)
painter.setMode(4)
local n = painter.attachZone(0, 1000)
assert(n == cS, "attach count")
assert(painter.patchCount(0) == c0 + cS, "merged patches")
assert(painter.vertexCount(0) == v0 + vS, "merged verts")
assert(painter.patchCount(1000) == nil, "source zone still present")
local t = painter.tileAt(0, c0 + 2, 1, 1)
assert(t == 77, "source marker did not travel (got " .. tostring(t) .. ")")
-- Both sides keep their WORLD positions: the appended geometry through the reorientation,
-- and the target's own through the anchor-cell fixup (the merged footprint extends the
-- authored origin the board placement re-derives from).
local ax, ay, az = painter.patchVertexPos(0, v0 + 0)
local d = math.abs(ax - wx) + math.abs(ay - wy) + math.abs(az - wz)
assert(d < 0.01, "appended world position drifted " .. d)
local bx, by, bz = painter.patchVertexPos(0, 0)
local dt = math.abs(bx - tx) + math.abs(by - ty) + math.abs(bz - tz)
assert(dt < 0.01, "target world position drifted " .. dt)
print("M46-1 OK c=" .. (c0 + cS) .. " v=" .. (v0 + vS))
EOF
seed "$OUT/ws1"
run_session "$OUT/ws1" "$B" "$OUT/merge.lua" "$OUT/g1.log"
CV=$(grep -a "M46-1 OK" "$OUT/g1.log" | sed 's/.*OK //')
[[ -n "$CV" ]] || { echo "FAIL: merge script"; tail -10 "$OUT/g1.log"; exit 1; }
grep -aq "(base target)" "$OUT/g1.log" || { echo "FAIL: expected the base-stream target"; exit 1; }
C1=$(echo "$CV" | sed 's/c=\([0-9]*\).*/\1/')
cat > "$OUT/merge_v.lua" <<EOF
assert(painter.patchCount(0) == $C1, "reopen count")
print("M46-1 reopen OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/merge_v.lua" "$OUT/g1v.log"
grep -aq "M46-1 reopen OK" "$OUT/g1v.log" || { echo "FAIL: merge did not persist"; tail -5 "$OUT/g1v.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/$MAXDIR/$B.max" > "$OUT/g1.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on merged file"; tail -4 "$OUT/g1.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1.pm.log" || { echo "FAIL: pm identity on merged file"; exit 1; }
# The closed source file kept the pre-attach state INCLUDING the pending marker (saved on close).
cat > "$OUT/src_v.lua" <<'EOF'
assert(painter.tileAt(0, 2, 1, 1) == 77, "source file lost the marker")
print("M46-1 source OK")
EOF
run_verify "$OUT/ws1" "$F" "$OUT/src_v.lua" "$OUT/g1s.log"
grep -aq "M46-1 source OK" "$OUT/g1s.log" || { echo "FAIL: closed source file"; tail -5 "$OUT/g1s.log"; exit 1; }
echo "OK merge (paint travels, world keeps, source closed with its state saved)"

echo "===== M46-2: undo rolls the target back to its null-edit baseline ====="
"$ZP" "$GFX/$MAXDIR/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1
cat > "$OUT/undo.lua" <<'EOF'
assert(painter.openZone("material-fond", -2, 0))
local c0 = painter.patchCount(0)
local tx, ty, tz = painter.patchVertexPos(0, 0)
painter.setMode(4)
assert(painter.attachZone(0, 1000) > 0, "attach failed")
painter.undo()
assert(painter.patchCount(0) == c0, "undo count")
local bx, by, bz = painter.patchVertexPos(0, 0)
local dt = math.abs(bx - tx) + math.abs(by - ty) + math.abs(bz - tz)
assert(dt < 0.01, "target moved across attach+undo by " .. dt)
print("M46-2 OK")
EOF
seed "$OUT/ws2"
run_session "$OUT/ws2" "$B" "$OUT/undo.lua" "$OUT/g2.log"
grep -aq "M46-2 OK" "$OUT/g2.log" || { echo "FAIL: undo script"; tail -8 "$OUT/g2.log"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$OUT/ws2/$MAXDIR/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: attach undo left $U bytes changed"; exit 1; }
echo "OK attach undo byte-identical"

echo "===== M46-3: refusals leave the target alone ====="
cat > "$OUT/refuse.lua" <<'EOF'
local c0 = painter.patchCount(0)
painter.setMode(4)
assert(painter.attachZone(0, 0) == 0, "self-attach did not refuse")
-- an instance ZONE of another file is a viewpoint, not an open editable file
assert(painter.openZone("material-fond", -2, 0))
assert(painter.placeInstance(8, 0, "material-fond"))
assert(painter.attachZone(0, 10000) == 0, "instance attach did not refuse")
assert(painter.patchCount(0) == c0, "target changed")
print("M46-3 OK")
EOF
seed "$OUT/ws3"
run_session "$OUT/ws3" "$B" "$OUT/refuse.lua" "$OUT/g3.log"
grep -aq "M46-3 OK" "$OUT/g3.log" || { echo "FAIL: refusals"; tail -6 "$OUT/g3.log"; exit 1; }
grep -aq "same object" "$OUT/g3.log" || { echo "FAIL: expected the same-object refusal"; exit 1; }
grep -aq "not an open editable file" "$OUT/g3.log" || { echo "FAIL: expected the file refusal"; exit 1; }
echo "OK refusals"

echo "ALL M46 GATES PASSED"
