#!/usr/bin/env bash
# M34 patch-move byte gates: the Tier A geometry write, proven on BOTH write targets.
#
# 0. Legacy null-edit output is the baseline. As in M31 the source .max is NOT the
# reference - the OLE container is rewritten on any save - so cross-path identity is
# always measured against this file.
# 1. Entering patch mode, picking a sub-object level and drawing the cage writes NOTHING:
# a no-op save is byte-identical to the baseline.
# 2. Moving one vertex changes exactly that vertex's bytes. Moving along Z by 1.5 alters a
# single float, of which 2 bytes actually differ - so the expected delta is 2, and any
# larger number means the write escaped its target.
# 3. Both write targets are covered, which is the point of using two files:
# material-fond - no mapper record for the vertex -> modifier PatchMesh (0x1140)
# material-bassin - mapped vertex -> mapper Delta (0x1130)
# The log line is asserted, so a policy regression that silently routed everything to
# one target would fail here rather than pass by luck.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m34_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { # $1 = workspace dir, $2 = basename
	rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

cat > "$OUT/none.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
EOF
cat > "$OUT/move.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 5, 1)
painter.movePatchSelection(0, 0, 1.5)
EOF

run_session() { # $1 = ws, $2 = basename, $3 = lua, $4 = log
	ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
		--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1
}

for pair in "material-fond:modPM" "material-bassin:delta"; do
	B="${pair%%:*}"; WANT="${pair##*:}"
	SRC="$GFX/landscape/ligo/lacustre/max/$B.max"

	echo "===== M34-0 ($B): null-edit baseline ====="
	"$ZP" "$SRC" --null-edit --out "$OUT/$B.null.max" > "$OUT/$B.null.log" 2>&1
	REF=$(md5sum "$OUT/$B.null.max" | cut -d' ' -f1)
	echo "baseline md5 $REF"

	echo "===== M34-1 ($B): patch mode entered, nothing moved == baseline ====="
	seed "$OUT/ws_none_$B" "$B"
	run_session "$OUT/ws_none_$B" "$B" "$OUT/none.lua" "$OUT/$B.none.log"
	GOT=$(md5sum "$OUT/ws_none_$B/landscape/ligo/lacustre/max/$B.max" | cut -d' ' -f1)
	[[ "$GOT" == "$REF" ]] || { echo "FAIL: patch-mode no-op save != null-edit ($GOT)"; exit 1; }
	echo "OK no-op identity"

	echo "===== M34-2/3 ($B): one vertex moved, target $WANT ====="
	seed "$OUT/ws_move_$B" "$B"
	run_session "$OUT/ws_move_$B" "$B" "$OUT/move.lua" "$OUT/$B.move.log"
	grep -aq "movePatchSelection: 1 written" "$OUT/$B.move.log" || {
		echo "FAIL: move did not write 1 vertex"; grep -a "movePatchSelection" "$OUT/$B.move.log"; exit 1; }
	case "$WANT" in
		modPM) grep -aq "modPM 1, delta 0" "$OUT/$B.move.log" || {
			echo "FAIL: expected the modifier-PatchMesh target"; grep -a "movePatchSelection" "$OUT/$B.move.log"; exit 1; } ;;
		delta) grep -aq "modPM 0, delta 1" "$OUT/$B.move.log" || {
			echo "FAIL: expected the mapper-delta target"; grep -a "movePatchSelection" "$OUT/$B.move.log"; exit 1; } ;;
	esac
	# cmp -l exits 1 when the files differ, which is the expected case here - brace it so
	# pipefail does not turn a successful comparison into a script abort.
	N=$( { cmp -l "$OUT/$B.null.max" "$OUT/ws_move_$B/landscape/ligo/lacustre/max/$B.max" || true; } | wc -l)
	[[ "$N" -eq 2 ]] || { echo "FAIL: move touched $N bytes, expected 2"; exit 1; }
	echo "OK move wrote 2 bytes via $WANT"
done

echo "ALL M34 GATES PASSED"
