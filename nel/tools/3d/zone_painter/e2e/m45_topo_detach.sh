#!/usr/bin/env bash
# M45 detach gates: split the selection off into a NEW BRICK FILE - the file-level half of
# the topology story. One editable zone per file means the legacy "detach as a new object"
# maps to "detach as a new file": the new brick is the source with the complement deleted
# (atomic copy-save), the source loses the selection through the normal delete flow.
#
#  1. THE SPLIT (zonematerial-bassin-1): marker tiles on patches 5 and 6, detach {5,6} as
#     "half" - source drops to c0-2, half.max appears next to the source, opens as its own
#     session with 2 patches and BOTH markers at the compacted indices (paint survives on
#     the detached side), and round-trips the encoder. A second detach ("half" again)
#     collision-bumps to half2.max. The source session reopens at c0-3.
#  2. SOURCE == PLAIN DELETE, byte for byte: detaching {5,6} leaves the source file
#     byte-identical to a plain deletePatchSelection of {5,6} - the source-side transform
#     IS the delete, no drift between the two paths.
#  3. UNDO (both write targets: zonematerial-bassin-1 base, material-bassin
#     modifier+mapper): paint + detach, undo twice, save - byte-identical to the null-edit
#     baseline, while the detached file REMAINS on disk (a save is not undone). The
#     modifier fixture asserts the "(modifier target)" stream choice and that the new
#     brick opens at the detached count.
#  4. REFUSALS: whole-zone selection refuses (save a copy instead), empty selection
#     refuses; neither creates a file.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m45_out
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
MAXDIR="landscape/ligo/lacustre/max"

echo "===== M45-1: the split - markers travel, new brick opens, collision bump ====="
cat > "$OUT/split.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 5, 2, 3, 100, 0)
painter.rawTile(0, 6, 1, 1, 101, 0)
local c0 = painter.patchCount(0)
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection("half") == 2, "detach failed")
assert(painter.patchCount(0) == c0 - 2, "source count")
-- second detach: same name must collision-bump
painter.selectPatchFace(0, 3, 1)
assert(painter.detachPatchSelection("half") == 1, "second detach failed")
assert(painter.patchCount(0) == c0 - 3, "source count 2")
print("M45-1 OK count=" .. (c0 - 3))
EOF
seed "$OUT/ws1" "$B"
run_session "$OUT/ws1" "$B" "$OUT/split.lua" "$OUT/g1.log"
C1=$(grep -a "M45-1 OK" "$OUT/g1.log" | sed 's/.*count=//')
[[ -n "$C1" ]] || { echo "FAIL: split script"; tail -8 "$OUT/g1.log"; exit 1; }
grep -aq "(base target)" "$OUT/g1.log" || { echo "FAIL: expected the base-stream target"; exit 1; }
[[ -f "$OUT/ws1/$MAXDIR/half.max" ]] || { echo "FAIL: half.max missing"; exit 1; }
[[ -f "$OUT/ws1/$MAXDIR/half2.max" ]] || { echo "FAIL: half2.max missing (collision bump)"; exit 1; }
cat > "$OUT/half_v.lua" <<'EOF'
assert(painter.patchCount(0) == 2, "detached count")
assert(painter.tileAt(0, 0, 2, 3) == 100, "marker of old patch 5")
assert(painter.tileAt(0, 1, 1, 1) == 101, "marker of old patch 6")
print("M45-1 half OK")
EOF
run_verify "$OUT/ws1" "half" "$OUT/half_v.lua" "$OUT/g1h.log"
grep -aq "M45-1 half OK" "$OUT/g1h.log" || { echo "FAIL: detached brick"; tail -6 "$OUT/g1h.log"; exit 1; }
cat > "$OUT/src_v.lua" <<EOF
assert(painter.patchCount(0) == $C1, "source reopen count")
print("M45-1 source OK")
EOF
run_verify "$OUT/ws1" "$B" "$OUT/src_v.lua" "$OUT/g1s.log"
grep -aq "M45-1 source OK" "$OUT/g1s.log" || { echo "FAIL: source reopen"; tail -5 "$OUT/g1s.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/$MAXDIR/half.max" > "$OUT/g1.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on half.max"; tail -4 "$OUT/g1.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g1.pm.log" || { echo "FAIL: pm identity on half.max"; exit 1; }
echo "OK split (markers travel, brick opens, bump works, round-trips)"

echo "===== M45-2: detach source == plain delete, byte for byte ====="
cat > "$OUT/det.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.detachPatchSelection("cut") == 2)
print("M45-2a OK")
EOF
cat > "$OUT/del.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 5, 1)
painter.selectPatchFace(0, 6, 1)
assert(painter.deletePatchSelection() == 2)
print("M45-2b OK")
EOF
seed "$OUT/ws2a" "$B"
run_session "$OUT/ws2a" "$B" "$OUT/det.lua" "$OUT/g2a.log"
grep -aq "M45-2a OK" "$OUT/g2a.log" || { echo "FAIL: detach for compare"; tail -6 "$OUT/g2a.log"; exit 1; }
seed "$OUT/ws2b" "$B"
run_session "$OUT/ws2b" "$B" "$OUT/del.lua" "$OUT/g2b.log"
grep -aq "M45-2b OK" "$OUT/g2b.log" || { echo "FAIL: delete for compare"; tail -6 "$OUT/g2b.log"; exit 1; }
U=$( { cmp -l "$OUT/ws2a/$MAXDIR/$B.max" "$OUT/ws2b/$MAXDIR/$B.max" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: detach source differs from plain delete by $U bytes"; exit 1; }
echo "OK detach source == plain delete"

echo "===== M45-3: undo restores the source byte-exactly; the new brick stays ====="
cat > "$OUT/undo.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 1, 0, 0, 99, 0)
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
painter.selectPatchFace(0, c0 - 1, 1)
assert(painter.detachPatchSelection("undopart") == 1)
painter.undo() -- the detach's source-side delete
painter.undo() -- the marker
assert(painter.patchCount(0) == c0, "undo count")
print("M45-3 OK")
EOF
for B3 in zonematerial-bassin-1 material-bassin; do
	"$ZP" "$GFX/landscape/ligo/lacustre/max/$B3.max" --null-edit --out "$OUT/$B3.null.max" > "$OUT/$B3.null.log" 2>&1
	seed "$OUT/ws3_$B3" "$B3"
	run_session "$OUT/ws3_$B3" "$B3" "$OUT/undo.lua" "$OUT/g3_$B3.log"
	grep -aq "M45-3 OK" "$OUT/g3_$B3.log" || { echo "FAIL: undo scene on $B3"; tail -8 "$OUT/g3_$B3.log"; exit 1; }
	U=$( { cmp -l "$OUT/$B3.null.max" "$OUT/ws3_$B3/$MAXDIR/$B3.max" || true; } | wc -l)
	[[ "$U" -eq 0 ]] || { echo "FAIL: undo on $B3 left $U bytes changed"; exit 1; }
	[[ -f "$OUT/ws3_$B3/$MAXDIR/undopart.max" ]] || { echo "FAIL: $B3 detached file vanished"; exit 1; }
	echo "OK $B3 undo byte-identical, brick stays"
done
grep -aq "(modifier target)" "$OUT/g3_material-bassin.log" \
	|| { echo "FAIL: expected the modifier-stream target on material-bassin"; exit 1; }
cat > "$OUT/mod_v.lua" <<'EOF'
assert(painter.patchCount(0) == 1, "modifier detached count")
print("M45-3 mod OK")
EOF
run_verify "$OUT/ws3_material-bassin" "undopart" "$OUT/mod_v.lua" "$OUT/g3m.log"
grep -aq "M45-3 mod OK" "$OUT/g3m.log" || { echo "FAIL: modifier-file brick"; tail -6 "$OUT/g3m.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws3_material-bassin/$MAXDIR/undopart.max" > "$OUT/g3m.pm.log" 2>&1 \
	|| { echo "FAIL: pm round-trip on the modifier-file brick"; tail -4 "$OUT/g3m.pm.log"; exit 1; }
grep -aq "^OK pm-modify-save" "$OUT/g3m.pm.log" || { echo "FAIL: pm identity on the modifier-file brick"; exit 1; }
echo "OK modifier-target brick opens and round-trips"

echo "===== M45-4: refusals create nothing ====="
cat > "$OUT/refuse.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local c0 = painter.patchCount(0)
assert(painter.detachPatchSelection("nope") == 0, "empty selection detached")
for p = 0, c0 - 1 do painter.selectPatchFace(0, p, 1) end
assert(painter.detachPatchSelection("nope") == 0, "whole zone detached")
assert(painter.patchCount(0) == c0)
print("M45-4 OK")
EOF
seed "$OUT/ws4" "$B"
run_session "$OUT/ws4" "$B" "$OUT/refuse.lua" "$OUT/g4.log"
grep -aq "M45-4 OK" "$OUT/g4.log" || { echo "FAIL: refusals"; tail -6 "$OUT/g4.log"; exit 1; }
grep -aq "whole zone" "$OUT/g4.log" || { echo "FAIL: expected the whole-zone refusal"; exit 1; }
[[ ! -f "$OUT/ws4/$MAXDIR/nope.max" ]] || { echo "FAIL: refusal created a file"; exit 1; }
echo "OK refusals"

echo "ALL M45 GATES PASSED"
