#!/usr/bin/env bash
# M60 Detach with Copy (plan mA7): the legacy Detach dialog's Copy checkbox.
#
# ELEMENT form: topoCopyElements clones the selection's patches and every element they
# reference as a COINCIDENT island - the original stays untouched, paint copies
# verbatim, binds internal to the selection re-establish on the clone. FILE form:
# detachToFile(name, true) writes the new brick and the session changes NOTHING (not
# undoable by design - nothing mutates; a save is never undone).
#
#  1. COPY-ELEMENT (bassin-1, patch 12): +1 patch, +4 verts, clone corners COINCIDE
#     with the originals, the painted marker copies, the original keeps its own paint
#     and geometry; the clone is its own element (Element expand finds exactly it);
#     undo restores the null-edit baseline byte for byte; round-trip.
#  2. WELD-BACK PIN: welding the stacked clone's corners onto the originals at a tiny
#     threshold REFUSES (the merge would put doubled patches on shared edges) and the
#     file stays byte-identical.
#  3. BINDS: copying the ilot_croix island (element of patch 229) re-establishes its
#     internal binds on the clone (bound count grows), round-trips, undoes byte-exact.
#  4. COPY-TO-FILE: the new brick exists and round-trips; the SESSION saves
#     byte-identical to the null-edit baseline.
set -euo pipefail

ZP="${ZONE_PAINTER:-}"
if [[ -z "$ZP" || ! -x "$ZP" ]]; then ZP="/home/kaetemi/ryzomcore/build/nel-pipeline/bin/zone_painter"; fi
PMCT="${PIPELINE_MAX_CORPUS_TEST:-/home/kaetemi/ryzomcore/build/nel-pipeline/bin/pipeline_max_corpus_test}"
GFX="${RYZOMCORE_GRAPHICS:-/home/kaetemi/ryzomcore_graphics}"
OUT=/tmp/zp_ui/m60_out
XVFB="xvfb-run -a"
rm -rf "$OUT"; mkdir -p "$OUT"

seed() { rm -rf "$1"; mkdir -p "$1/landscape/ligo/lacustre/max"
	cp "$GFX/landscape/ligo/lacustre/max/$2.max" "$1/landscape/ligo/lacustre/max/"
	ln -sfn "$GFX/landscape/_texture_tiles" "$1/landscape/_texture_tiles"; }
run_session() { ZONE_PAINTER_BOARD_ACTION="save:$2" $XVFB "$ZP" "$1" --startup-auto "lacustre/$2" \
	--no-hint-stamp --no-thumbnail --startup-lua "$3" --screenshot /dev/null > "$4" 2>&1; }

B=zonematerial-bassin-1
"$ZP" "$GFX/landscape/ligo/lacustre/max/$B.max" --null-edit --out "$OUT/base.null.max" > "$OUT/null.log" 2>&1

echo "===== M60-1: copy-element - coincident clone, paint copies, undo, round-trip ====="
seed "$OUT/ws1" "$B"
cat > "$OUT/s1.lua" <<'EOF'
painter.setMode(0)
painter.rawTile(0, 12, 0, 0, 77, 0)
painter.setMode(4)
painter.setSubObject(3)
local nP0 = painter.patchCount(0)
local nV0 = painter.vertexCount(0)
local orig = {}
for c = 0, 3 do
  local v = painter.patchCornerVert(0, 12, c)
  local x, y, z = painter.patchVertexPos(0, v)
  orig[c] = { x, y, z }
end
painter.selectPatchFace(0, 12, 0)
painter.detachPatchSelection(true)
local nP1 = painter.patchCount(0)
local nV1 = painter.vertexCount(0)
assert(nP1 == nP0 + 1, "expected +1 patch, got +" .. (nP1 - nP0))
assert(nV1 == nV0 + 4, "expected +4 verts, got +" .. (nV1 - nV0))
local clone = nP1 - 1
-- The clone coincides with the original, corner for corner (same local slots).
for c = 0, 3 do
  local v = painter.patchCornerVert(0, clone, c)
  assert(v >= nV0, "clone ring names an original vertex")
  local x, y, z = painter.patchVertexPos(0, v)
  local e = orig[c]
  local d = math.max(math.abs(x-e[1]), math.abs(y-e[2]), math.abs(z-e[3]))
  assert(d < 1e-5, "clone corner " .. c .. " off the original by " .. d)
end
-- Paint copied; the original keeps its own.
assert(painter.tileAt(0, clone, 0, 0) == 77, "the marker did not copy")
assert(painter.tileAt(0, 12, 0, 0) == 77, "the original lost its marker")
-- The clone is its own element.
painter.setSubObject(3)
painter.selectPatchFace(0, clone, 0)
painter.expandSelectionToElement()
assert(painter.patchFaceSelectionCount() == 1, "the clone is not a separate element")
print("M60-1 OK")
EOF
run_session "$OUT/ws1" "$B" "$OUT/s1.lua" "$OUT/s1.log"
grep -aq "M60-1 OK" "$OUT/s1.log" || { echo "FAIL: copy-element"; tail -10 "$OUT/s1.log"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws1/landscape/ligo/lacustre/max/$B.max" > "$OUT/s1.rt.log" 2>&1 \
	|| { echo "FAIL: copied file does not round-trip"; tail -4 "$OUT/s1.rt.log"; exit 1; }
echo "OK coincident clone + paint copy + element"

seed "$OUT/ws1b" "$B"
cat > "$OUT/s1b.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.detachPatchSelection(true)
painter.undo()
print("M60-1B OK")
EOF
run_session "$OUT/ws1b" "$B" "$OUT/s1b.lua" "$OUT/s1b.log"
grep -aq "M60-1B OK" "$OUT/s1b.log" || { echo "FAIL: undo scene"; tail -8 "$OUT/s1b.log"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws1b/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: copy undo is not byte-identical"; exit 1; }
echo "OK one undo removes the clone"

echo "===== M60-2: weld-back of the stacked clone refuses ====="
seed "$OUT/ws2" "$B"
cat > "$OUT/s2.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local nV0 = painter.vertexCount(0)
painter.selectPatchFace(0, 12, 0)
painter.detachPatchSelection(true)
local clone = painter.patchCount(0) - 1
painter.setSubObject(1)
painter.clearPatchVertexSelection()
for c = 0, 3 do
  painter.selectPatchVertex(0, painter.patchCornerVert(0, 12, c), 1)
  painter.selectPatchVertex(0, painter.patchCornerVert(0, clone, c), 1)
end
painter.weldPatchSelection(0.01)
print("M60-2 DONE")
EOF
run_session "$OUT/ws2" "$B" "$OUT/s2.lua" "$OUT/s2.log"
grep -aq "M60-2 DONE" "$OUT/s2.log" || { echo "FAIL: weld-back scene"; tail -8 "$OUT/s2.log"; exit 1; }
grep -aq "weld:" "$OUT/s2.log" || { echo "FAIL: no weld line at all"; exit 1; }
# The pin: the stacked merge refuses - it would put more than two patches on one edge.
grep -aq "weld would put more than two patches on one edge" "$OUT/s2.log" \
	|| { echo "PIN CHECK: weld outcome:"; grep -a "weld" "$OUT/s2.log" | head -3; exit 1; }
grep -aq "weldPatchSelection: 0 merged" "$OUT/s2.log" \
	|| { echo "FAIL: the refused weld merged something"; exit 1; }
echo "OK stacked weld-back refuses"

echo "===== M60-3: internal binds re-establish on the copied island (ilot_croix) ====="
I=zonematerial-bassin-ilot_croix
"$ZP" "$GFX/landscape/ligo/lacustre/max/$I.max" --null-edit --out "$OUT/ilot.null.max" > "$OUT/ilot.null.log" 2>&1
seed "$OUT/ws3" "$I"
cat > "$OUT/s3.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
local nV0 = painter.vertexCount(0)
local bound0 = 0
for v = 0, nV0 - 1 do
  if painter.vertexBindInfo(0, v) == 1 then bound0 = bound0 + 1 end
end
painter.selectPatchFace(0, 229, 0)
painter.expandSelectionToElement()
local island = painter.patchFaceSelectionCount()
assert(island >= 2, "fixture: patch 229's island has several patches")
painter.detachPatchSelection(true)
local nV1 = painter.vertexCount(0)
local bound1 = 0
for v = 0, nV1 - 1 do
  if painter.vertexBindInfo(0, v) == 1 then bound1 = bound1 + 1 end
end
print("M60-3 BOUND " .. bound0 .. " -> " .. bound1)
assert(bound1 > bound0, "no internal bind re-established on the clone")
painter.undo()
print("M60-3 OK")
EOF
run_session "$OUT/ws3" "$I" "$OUT/s3.lua" "$OUT/s3.log"
grep -aq "M60-3 OK" "$OUT/s3.log" || { echo "FAIL: bind copy scene"; tail -10 "$OUT/s3.log"; exit 1; }
cmp -s "$OUT/ilot.null.max" "$OUT/ws3/landscape/ligo/lacustre/max/$I.max" \
	|| { echo "FAIL: bind-copy undo is not byte-identical"; exit 1; }
echo "OK internal binds copied; undo byte-identical"

echo "===== M60-4: copy-to-file leaves the session byte-identical ====="
seed "$OUT/ws4" "$B"
cat > "$OUT/s4.lua" <<'EOF'
painter.setMode(4)
painter.setSubObject(3)
painter.selectPatchFace(0, 12, 0)
painter.detachToFile("copybrick", true)
print("M60-4 OK")
EOF
run_session "$OUT/ws4" "$B" "$OUT/s4.lua" "$OUT/s4.log"
grep -aq "M60-4 OK" "$OUT/s4.log" || { echo "FAIL: copy-to-file scene"; tail -8 "$OUT/s4.log"; exit 1; }
grep -aq "detach copy: zone 0, 1 patches" "$OUT/s4.log" || { echo "FAIL: copy line missing"; exit 1; }
[[ -f "$OUT/ws4/landscape/ligo/lacustre/max/copybrick.max" ]] \
	|| { echo "FAIL: the new brick was not written"; exit 1; }
cmp -s "$OUT/base.null.max" "$OUT/ws4/landscape/ligo/lacustre/max/$B.max" \
	|| { echo "FAIL: copy-to-file changed the session"; exit 1; }
"$PMCT" --pm-modify-save-test "$OUT/ws4/landscape/ligo/lacustre/max/copybrick.max" > "$OUT/s4.rt.log" 2>&1 \
	|| { echo "FAIL: the new brick does not round-trip"; tail -4 "$OUT/s4.rt.log"; exit 1; }
echo "OK copy-to-file"

echo "ALL M60 GATES PASSED"
