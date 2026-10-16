#!/usr/bin/env bash
# M40 bind / unbind / edge no-smooth gates: the first Tier B ops (pristine-blob writes via
# the carrier, no stream encoder).
#
#  0. Legacy null-edit output is the baseline (as in M31/M34: the OLE container is rewritten
#     on any save, so the source .max is never the reference).
#  1. Unbind of one BIND_SINGLE writes exactly 21 bytes: the Binded bool8 (1 -> 0) plus the
#     five rebuildable cache uint32s (stored indices -> -1, matching the legacy
#     UnBindingVertex). Any other count means the write escaped its record. The release
#     PERSISTS: a fresh session on the saved file reads Binded == 0.
#  2. Unbind then undo saves byte-identical to the baseline - the undo record carries the
#     whole on-disk bind record, caches included, so the original cache indices come back
#     verbatim.
#  3. Re-bind with an explicit target writes the record fields back (BIND_SINGLE onto the
#     original patch/edge, PrimVert = self) and snaps NOTHING (the vertex already sits on
#     its bindWhere point, and the closure-restricted harvest must not touch the rest of
#     the zone) - so the on-disk delta is exactly the 20 cache bytes.
#  4. The selection-driven bind (nearest valid open edge) picks the SAME target the file
#     authored - the vertex sits on that edge, so the nearest-valid rule must find it.
#  5. Moving the freed vertex off the edge and re-binding SNAPS it back: the bind refresh
#     re-derives the bindWhere point, the position returns to the original bits, and the
#     three-op undo chain (unbind, move, bind) restores the baseline byte for byte.
#  6. No-smooth on a two-patch edge writes the flag on BOTH sides (2 flag words, 2 bytes on
#     disk), reads back as flagged, persists across a reload, and undoes to the baseline.
#  7. Ops addressed through a scripted INSTANCE zone hit the shared carrier: unbind via the
#     clone's zone id produces the same saved bytes as unbind via the source.
#  8. The TRIPLE configuration (25/50/75 trio, zonematerial-bassin-landmark_a vertices
#     71/72/73 onto patch 31 edge 0): selecting ONE companion releases the whole group
#     (UnbindRelatedVertex semantics - the trio references its BIND_50 anchor through
#     PrimVert and has no half-released state), undo restores the baseline byte for byte,
#     and re-binding by selecting just the MIDDLE vertex rediscovers the companions and the
#     authored target (CheckBind config 1).
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m40_out
XVFB="xvfb-run -a"
B=zonematerial-bassin-ilot_croix
# Discovered fixture facts (see the discovery loop in the gate's dev notes): vertex 445 is a
# BIND_SINGLE onto patch 229 edge 0 (corner pair 371-372); patch 0 edge slot 0 (corners 6-7)
# is a two-patch interior edge.
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { # $1 = workspace dir
	rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$B.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"
}

run_session() { # $1 = ws, $2 = lua, $3 = log ; saves the file via the board action
	ZONE_PAINTER_BOARD_ACTION="save:$B" $XVFB "$ZP" "$1" --startup-auto "lacustre/$B" \
		--no-hint-stamp --no-thumbnail --startup-lua "$2" --screenshot /dev/null > "$3" 2>&1
}

run_verify() { # $1 = ws (already holding a saved file), $2 = lua, $3 = log ; no save
	$XVFB "$ZP" "$1" --startup-auto "lacustre/$B" \
		--no-hint-stamp --no-thumbnail --startup-lua "$2" --screenshot /dev/null > "$3" 2>&1
}

saved() { echo "$1/landscape/ligo/lacustre/max/$B.max"; }

echo "===== M40-0: null-edit baseline ====="
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1
echo "baseline md5 $(md5sum "$OUT/base.null.max" | cut -d' ' -f1)"

echo "===== M40-1: unbind releases the record, 21 bytes, persists ====="
cat > "$OUT/unbind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
local n = painter.unbindPatchSelection()
assert(n == 1, "expected 1 released, got " .. tostring(n))
local binded = painter.vertexBindInfo(0, 445)
assert(binded == 0, "record still bound after unbind")
print("unbind OK")
EOF
seed "$OUT/ws1"
run_session "$OUT/ws1" "$OUT/unbind.lua" "$OUT/g1.log"
grep -aq "unbind OK" "$OUT/g1.log" || { echo "FAIL: unbind script"; tail -5 "$OUT/g1.log"; exit 1; }
N=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws1")" || true; } | wc -l)
[[ "$N" -eq 21 ]] || { echo "FAIL: unbind touched $N bytes, expected 21"; exit 1; }
cat > "$OUT/verify_unbound.lua" <<'EOF'
local binded = painter.vertexBindInfo(0, 445)
assert(binded == 0, "reload still bound")
print("verify unbound OK")
EOF
run_verify "$OUT/ws1" "$OUT/verify_unbound.lua" "$OUT/g1v.log"
grep -aq "verify unbound OK" "$OUT/g1v.log" || { echo "FAIL: unbind did not persist"; exit 1; }
echo "OK unbind (21 bytes, persisted)"

echo "===== M40-2: unbind + undo == baseline ====="
cat > "$OUT/unbind_undo.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
painter.unbindPatchSelection()
painter.undo()
local binded = painter.vertexBindInfo(0, 445)
assert(binded == 1, "undo did not restore the bind")
print("unbind undo OK")
EOF
seed "$OUT/ws2"
run_session "$OUT/ws2" "$OUT/unbind_undo.lua" "$OUT/g2.log"
grep -aq "unbind undo OK" "$OUT/g2.log" || { echo "FAIL: unbind-undo script"; exit 1; }
U=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws2")" || true; } | wc -l)
[[ "$U" -eq 0 ]] || { echo "FAIL: undo left $U bytes changed (cache restore not verbatim?)"; exit 1; }
echo "OK undo restores the baseline byte for byte"

echo "===== M40-3: explicit re-bind restores the record, snaps nothing, 20 cache bytes ====="
cat > "$OUT/rebind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
painter.unbindPatchSelection()
assert(painter.bindPatchVertex(0, 445, 229, 0))
local binded, t, p, e, prim = painter.vertexBindInfo(0, 445)
assert(binded == 1 and t == 3 and p == 229 and e == 0 and prim == 445,
       string.format("bad record: %d %d %d %d %d", binded, t, p, e, prim))
print("rebind OK")
EOF
seed "$OUT/ws3"
run_session "$OUT/ws3" "$OUT/rebind.lua" "$OUT/g3.log"
grep -aq "rebind OK" "$OUT/g3.log" || { echo "FAIL: rebind script"; tail -5 "$OUT/g3.log"; exit 1; }
grep -aq "(single, 0 snapped)" "$OUT/g3.log" || { echo "FAIL: rebind snapped something on already-snapped data"; grep -a "bindPatchVertex" "$OUT/g3.log"; exit 1; }
R=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws3")" || true; } | wc -l)
[[ "$R" -eq 20 ]] || { echo "FAIL: rebind touched $R bytes, expected the 20 cache bytes"; exit 1; }
cat > "$OUT/verify_bound.lua" <<'EOF'
local binded, t, p, e, prim = painter.vertexBindInfo(0, 445)
assert(binded == 1 and t == 3 and p == 229 and e == 0, "reload lost the bind")
print("verify bound OK")
EOF
run_verify "$OUT/ws3" "$OUT/verify_bound.lua" "$OUT/g3v.log"
grep -aq "verify bound OK" "$OUT/g3v.log" || { echo "FAIL: rebind did not persist"; exit 1; }
echo "OK explicit re-bind (record + 20 cache bytes)"

echo "===== M40-4: selection bind picks the authored target (nearest valid open edge) ====="
cat > "$OUT/autobind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
painter.unbindPatchSelection()
local n = painter.bindPatchSelection()
assert(n == 1, "expected 1 bound")
local binded, t, p, e, prim = painter.vertexBindInfo(0, 445)
assert(binded == 1 and p == 229 and e == 0,
       string.format("auto target mismatch: patch %d edge %d", p, e))
print("autobind OK")
EOF
seed "$OUT/ws4"
run_session "$OUT/ws4" "$OUT/autobind.lua" "$OUT/g4.log"
grep -aq "autobind OK" "$OUT/g4.log" || { echo "FAIL: selection bind"; tail -5 "$OUT/g4.log"; exit 1; }
echo "OK selection bind found the authored edge"

echo "===== M40-5: bind snaps a moved vertex back onto the edge; 3 undos == baseline ====="
cat > "$OUT/move_rebind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 445, 1)
painter.unbindPatchSelection()
local x0, y0, z0 = painter.patchVertexPos(0, 445)
painter.movePatchSelection(0.5, 0, 0)
assert(painter.bindPatchVertex(0, 445, 229, 0))
local x1, y1, z1 = painter.patchVertexPos(0, 445)
assert(math.abs(x1 - x0) < 1e-4 and math.abs(y1 - y0) < 1e-4 and math.abs(z1 - z0) < 1e-4,
       string.format("snap missed: moved %.6f %.6f %.6f", x1 - x0, y1 - y0, z1 - z0))
painter.undo()
painter.undo()
painter.undo()
local binded = painter.vertexBindInfo(0, 445)
assert(binded == 1, "undo chain lost the bind")
print("move rebind OK")
EOF
seed "$OUT/ws5"
run_session "$OUT/ws5" "$OUT/move_rebind.lua" "$OUT/g5.log"
grep -aq "move rebind OK" "$OUT/g5.log" || { echo "FAIL: move-rebind script"; tail -5 "$OUT/g5.log"; exit 1; }
# The bind must have written a real snap (vertex + tangent cache + fed interiors).
grep -aq "(single, 0 snapped)" "$OUT/g5.log" && { echo "FAIL: moved vertex was not snapped"; exit 1; }
M=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws5")" || true; } | wc -l)
[[ "$M" -eq 0 ]] || { echo "FAIL: undo chain left $M bytes changed"; exit 1; }
echo "OK snap + undo chain"

echo "===== M40-6: no-smooth writes BOTH sides of a shared edge, persists, undoes ====="
cat > "$OUT/nosmooth.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(2)
painter.selectPatchEdge(0, 6, 7, 0)
assert(painter.edgeNoSmooth(0, 6, 7) == 0, "fixture edge already flagged")
local n = painter.setEdgeNoSmooth(true)
assert(n == 2, "expected both patch sides written, got " .. tostring(n))
assert(painter.edgeNoSmooth(0, 6, 7) == 1, "flag did not read back")
print("nosmooth OK")
EOF
seed "$OUT/ws6"
run_session "$OUT/ws6" "$OUT/nosmooth.lua" "$OUT/g6.log"
grep -aq "nosmooth OK" "$OUT/g6.log" || { echo "FAIL: nosmooth script"; tail -5 "$OUT/g6.log"; exit 1; }
S=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws6")" || true; } | wc -l)
[[ "$S" -eq 2 ]] || { echo "FAIL: no-smooth touched $S bytes, expected 2"; exit 1; }
cat > "$OUT/verify_nosmooth.lua" <<'EOF'
assert(painter.edgeNoSmooth(0, 6, 7) == 1, "reload lost the flag")
print("verify nosmooth OK")
EOF
run_verify "$OUT/ws6" "$OUT/verify_nosmooth.lua" "$OUT/g6v.log"
grep -aq "verify nosmooth OK" "$OUT/g6v.log" || { echo "FAIL: no-smooth did not persist"; exit 1; }
cat > "$OUT/nosmooth_undo.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(2)
painter.selectPatchEdge(0, 6, 7, 0)
painter.setEdgeNoSmooth(true)
painter.undo()
assert(painter.edgeNoSmooth(0, 6, 7) == 0, "undo did not clear the flag")
print("nosmooth undo OK")
EOF
seed "$OUT/ws6u"
run_session "$OUT/ws6u" "$OUT/nosmooth_undo.lua" "$OUT/g6u.log"
grep -aq "nosmooth undo OK" "$OUT/g6u.log" || { echo "FAIL: nosmooth-undo script"; exit 1; }
SU=$( { cmp -l "$OUT/base.null.max" "$(saved "$OUT/ws6u")" || true; } | wc -l)
[[ "$SU" -eq 0 ]] || { echo "FAIL: nosmooth undo left $SU bytes changed"; exit 1; }
echo "OK no-smooth (2 flag bytes, persisted, undone)"

echo "===== M40-7: unbind through a scripted instance == unbind through the source ====="
cat > "$OUT/via_inst.lua" <<'EOF'
assert(painter.placeInstance(8, 0))
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(10000, 445, 1)
local n = painter.unbindPatchSelection()
assert(n == 1, "instance unbind released " .. tostring(n))
local b0 = painter.vertexBindInfo(0, 445)
local b1 = painter.vertexBindInfo(10000, 445)
assert(b0 == 0 and b1 == 0, "carrier not shared: " .. b0 .. "/" .. b1)
assert(painter.saveAll())
print("via-instance OK")
EOF
seed "$OUT/ws7"
$XVFB "$ZP" "$OUT/ws7" --startup-auto "lacustre/$B" --no-hint-stamp --no-thumbnail \
	--lua-script "$OUT/via_inst.lua" --screenshot /dev/null > "$OUT/g7.log" 2>&1
grep -aq "via-instance OK" "$OUT/g7.log" || { echo "FAIL: via-instance script"; tail -5 "$OUT/g7.log"; exit 1; }
cmp -s "$(saved "$OUT/ws1")" "$(saved "$OUT/ws7")" || { echo "FAIL: instance unbind != source unbind"; exit 1; }
echo "OK instance unbind hits the shared carrier"

echo "===== M40-8: triple bind group - companion release, undo, middle-vertex re-bind ====="
B2=zonematerial-bassin-landmark_a
seed2() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$B2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B2.max" --null-edit --out "$OUT/base2.null.max" > "$OUT/null2.log" 2>&1
cat > "$OUT/triple_undo.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
-- select ONE companion (the BIND_25 member, vertex 72): the whole trio must release
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 72, 1)
local n = painter.unbindPatchSelection()
assert(n == 3, "expected the whole trio released, got " .. tostring(n))
local b71 = painter.vertexBindInfo(0, 71)
local b73 = painter.vertexBindInfo(0, 73)
assert(b71 == 0 and b73 == 0, "siblings survived a group release")
painter.undo()
print("triple undo OK")
EOF
seed2 "$OUT/ws8"
ZONE_PAINTER_BOARD_ACTION="save:$B2" $XVFB "$ZP" "$OUT/ws8" --startup-auto "lacustre/$B2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$OUT/triple_undo.lua" --screenshot /dev/null > "$OUT/g8.log" 2>&1
grep -aq "triple undo OK" "$OUT/g8.log" || { echo "FAIL: triple release/undo"; tail -5 "$OUT/g8.log"; exit 1; }
T=$( { cmp -l "$OUT/base2.null.max" "$OUT/ws8/landscape/ligo/lacustre/max/$B2.max" || true; } | wc -l)
[[ "$T" -eq 0 ]] || { echo "FAIL: triple undo left $T bytes changed"; exit 1; }
cat > "$OUT/triple_rebind.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(1)
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 72, 1)
painter.unbindPatchSelection()
-- re-bind by selecting just the middle vertex: config 1 rediscovers the companions
painter.clearPatchVertexSelection()
painter.selectPatchVertex(0, 71, 1)
local n = painter.bindPatchSelection()
assert(n == 1, "expected 1 middle vertex bound")
local checks = { {71, 2}, {72, 0}, {73, 1} }
for _, c in ipairs(checks) do
  local binded, t, p, e, prim = painter.vertexBindInfo(0, c[1])
  assert(binded == 1 and t == c[2] and p == 31 and e == 0 and prim == 71,
         string.format("v%d record wrong: %d %d %d %d %d", c[1], binded, t, p, e, prim))
end
print("triple rebind OK")
EOF
seed2 "$OUT/ws8r"
ZONE_PAINTER_BOARD_ACTION="save:$B2" $XVFB "$ZP" "$OUT/ws8r" --startup-auto "lacustre/$B2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$OUT/triple_rebind.lua" --screenshot /dev/null > "$OUT/g8r.log" 2>&1
grep -aq "triple rebind OK" "$OUT/g8r.log" || { echo "FAIL: triple rebind"; tail -5 "$OUT/g8r.log"; exit 1; }
echo "OK triple group (release via companion, undo identity, middle-vertex rediscovery)"

echo "ALL M40 GATES PASSED"
