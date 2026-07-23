#!/usr/bin/env bash
# M33: board-op recorder fidelity. UI-path board ops (env hooks = the same shared op
# functions the board menus call) recorded with REC on, the recording extracted and
# replayed in a FRESH session, and the replayed save must be BYTE-IDENTICAL to the
# recorded run's save. Hint stamping stays ON — the stamp encodes the board layout
# (moved home cell, context placement), so layout drift between record and replay is
# byte-visible in the saved file. Outputs under /tmp/zp_ui only. Needs xvfb-run.
set -euo pipefail

ZP="${ZONE_PAINTER:-$(command -v zone_painter || true)}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then
	ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"
fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
B=zonematerial-bassin-ilot_croix
OUT=/tmp/zp_ui/m33
XVFB="xvfb-run -a"

rm -rf "$OUT"
mkdir -p "$OUT"

seed_ws() { # $1 = ws dir
	rm -rf "$1"
	mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$B.max" "$1/landscape/ligo/lacustre/max/"
	cp "$GFX/landscape/ligo/lacustre/max/material-fond.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

echo "===== M33-1: record UI-path board ops ====="
seed_ws "$OUT/ws1"
cat > "$OUT/rec_on.lua" <<'EOF'
painter.setRecording(true)
print("recording ON")
EOF
# Hook order in the viewer: lua pre-pass -> BOARD_DRAG -> BOARD_ACTION -> recorder dump.
ZONE_PAINTER_BOARD_DRAG="0,0:0,5" \
ZONE_PAINTER_BOARD_ACTION="place:8,0;rotate:8,0;place-context:-4,0:material-fond;save:$B" \
ZONE_PAINTER_DUMP_RECORDER=1 \
$XVFB "$ZP" "$OUT/ws1" --startup-auto "lacustre/$B" --no-thumbnail \
	--lua-script "$OUT/rec_on.lua" --screenshot "$OUT/s1.tga" > "$OUT/g1.log" 2>&1
grep -aq "recording ON" "$OUT/g1.log" || { echo "FAIL: recording pre-pass"; exit 1; }
grep -aq "board-drag test (0,0)->(0,5): OK" "$OUT/g1.log" || { echo "FAIL: drag hook"; exit 1; }
grep -aq "board-action save:$B: OK" "$OUT/g1.log" || { echo "FAIL: save action"; exit 1; }
# Extract the recorder dump into a replayable script
sed -n '/^recorder dump:$/,/^--- end recorder ---$/p' "$OUT/g1.log" \
	| sed '1d;$d' > "$OUT/rec.lua"
test -s "$OUT/rec.lua" || { echo "FAIL: empty recorder dump"; exit 1; }
echo "--- recorded board ops ---"
grep -aE "moveCell|copyCell|placeInstance|rotateInstance|placeContext|saveZone" "$OUT/rec.lua"
grep -aq 'painter.moveCell(0, 0, 0, 5)' "$OUT/rec.lua" || { echo "FAIL: moveCell not recorded"; exit 1; }
grep -aq 'painter.placeInstance(8, 0, ' "$OUT/rec.lua" || { echo "FAIL: placeInstance not recorded"; exit 1; }
grep -aq 'painter.rotateInstance(8, 0, 1)' "$OUT/rec.lua" || { echo "FAIL: rotateInstance not recorded"; exit 1; }
grep -aq 'painter.placeContext(-4, 0, "material-fond")' "$OUT/rec.lua" || { echo "FAIL: placeContext not recorded"; exit 1; }
grep -aq "painter.saveZone(\"$B\")" "$OUT/rec.lua" || { echo "FAIL: saveZone not recorded"; exit 1; }
REC_MD5=$(md5sum "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
# The recorded save must have stamped the layout (backstop: the byte compare below is
# not comparing two files that both silently skipped stamping)
"$ZP" --dump-neighbor-hints "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/hints1.log" 2>&1
grep -aq "count=0" "$OUT/hints1.log" && { echo "FAIL: recorded save has no hint stamp"; exit 1; }

echo "===== M33-2: replay the recording in a fresh session ====="
seed_ws "$OUT/ws2"
$XVFB "$ZP" "$OUT/ws2" --startup-auto "lacustre/$B" --no-thumbnail \
	--lua-script "$OUT/rec.lua" --screenshot "$OUT/s2.tga" > "$OUT/g2.log" 2>&1
grep -aq "session save: '$B'" "$OUT/g2.log" || grep -aq "OK save" "$OUT/g2.log" \
	|| { echo "FAIL: replay did not save"; tail -5 "$OUT/g2.log"; exit 1; }
PLAY_MD5=$(md5sum "$OUT/ws2/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
[[ "$REC_MD5" == "$PLAY_MD5" ]] || { echo "FAIL: replay save != recorded save ($PLAY_MD5 vs $REC_MD5)"; exit 1; }
# Instance state equality (rotate/place are not byte-visible in the stamp): the last
# rebuild's display-zone summary must match between record and replay
I1=$(grep -aoE "display zones \+[0-9]+" "$OUT/g1.log" | tail -1)
I2=$(grep -aoE "display zones \+[0-9]+" "$OUT/g2.log" | tail -1)
[[ -n "$I1" && "$I1" == "$I2" ]] || { echo "FAIL: instance state differs ('$I1' vs '$I2')"; exit 1; }
echo "OK record==replay ($REC_MD5, $I1)"

echo "ALL M33 RECORDER BOARD-OP GATES PASSED"
