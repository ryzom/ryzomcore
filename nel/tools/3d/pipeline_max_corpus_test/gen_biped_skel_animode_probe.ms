-- ============================================================================
-- gen_biped_skel_animode_probe.ms — broad biped-toggle probe off a REAL
-- skeleton file (pipeline_max_design.md §10n)
--
-- WHY THIS SHAPE (per Kaetemi's direction, 2026-07-08): open an EXISTING
-- skeleton .max (not a synthetic biped.createNew, not one of the animation
-- files under investigation), commit it from Figure Mode into Animation Mode
-- as the baseline, then make MANY minimal single-change (and a few COMBINED-
-- change) saves off that SAME baseline, broadly — not narrowly re-testing one
-- hypothesis at a time. The Linux side now has an unbiased tool for reading
-- the result: `pipeline_max_export_anim --diff-rig <A.max> <B.max> <out.txt>`
-- diffs EVERY chunk on every Biped (0x9155) system object between two files,
-- byte/float-level, no assumption about which chunk matters — so this script
-- only needs to produce the paired files + minimal ground truth; the actual
-- "what changed" analysis happens after, off the real bytes, not off a guess.
--
-- SAFETY (biped figure/animation mode switching is known to misbehave):
--   * Every case RELOADS the skeleton file FRESH — no case is built by editing
--     on top of a previous case's in-memory state. If Max mishandles one
--     transition, it cannot cascade into any other case.
--   * Every figureMode assignment is wrapped in try/catch AND immediately
--     read back (not assumed) — the manifest logs both the requested and the
--     actual resulting state, since a silently-ignored mode switch would
--     otherwise look identical to a successful one until someone diffs bytes.
--   * Every biped-specific API call (ConvertToFootSteps, Mirror, CopyPosture,
--     talentFigMode, ...) is a best-effort try-ladder, matching every other
--     generator script in this project (biped_anim_dataset*_gen.ms) — exact
--     MAXScript spellings for the less common ones aren't all confirmed.
--
-- Target: 3ds Max 9. EDIT the paths below if needed.
-- ============================================================================

global OUT_DIR = "C:\\biped_skel_animode_probe\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true
global SKEL_FILE = "R:\\graphics\\stuff\\fyros\\agents\\actors\\male\\animation\\skeletons\\fy_hom_skel.max"  -- EDIT ME
-- Also worth a second full run with the female skeleton, since most of the
-- corpus disagreement data so far comes from fy_hof_first_* files:
--   R:\graphics\stuff\fyros\agents\actors\female\animation\skeletons\fy_hof_skel.max

global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

-- ---------------------------------------------------------------- load/setup
fn findComNode = (
  local best = undefined
  for o in objects do ( try ( if classof o.transform.controller == Vertical_Horizontal_Turn then best = o ) catch () )
  if best == undefined do ( for o in objects do ( if best == undefined and (matchPattern o.name pattern:"Bip*" ignoreCase:true) and o.parent == undefined do best = o ) )
  best
)

-- Load the skeleton fresh and commit Figure Mode -> Animation Mode. Returns
-- the COM node, or undefined on failure (logged). This is the shared start
-- point every case below re-derives independently.
fn loadAndCommit = (
  resetMaxFile #noPrompt
  loadMaxFile SKEL_FILE quiet:true
  local com = findComNode()
  if com == undefined then ( mlog "  ERROR no biped COM node found after load"; undefined )
  else (
    local ctrl = com.transform.controller
    local wasFig = "?"
    try ( wasFig = (ctrl.figureMode as string) ) catch ()
    try ( ctrl.figureMode = false ) catch ( mlog "  ERROR figureMode=false threw" )
    local isFig = "?"
    try ( isFig = (ctrl.figureMode as string) ) catch ()
    mlog ("  COMMIT figureMode: was=" + wasFig + " requested=false actual=" + isFig)
    com
  )
)

fn numKeysSafe c = ( if c == undefined then -1 else ( local n = -1; try ( n = numKeys c ) catch (); n ) )

fn dumpGroundTruth label com = (
  local ctrl = com.transform.controller
  local v = undefined; local h = undefined
  try ( v = ctrl.vertical.controller ) catch ()
  try ( h = ctrl.horizontal.controller ) catch ()
  mlog ("GT\t" + label)
  try ( mlog ("  figureMode\t" + (ctrl.figureMode as string)) ) catch ( mlog "  figureMode\tUNREADABLE" )
  try ( mlog ("  dynamicsType\t" + (ctrl.dynamicsType as string)) ) catch ( mlog "  dynamicsType\tUNREADABLE" )
  try ( mlog ("  talentFigMode\t" + (ctrl.talentFigMode as string)) ) catch ( mlog "  talentFigMode\tUNREADABLE" )
  try ( mlog ("  lockCom\t" + (ctrl.lockCom as string)) ) catch ( mlog "  lockCom\tUNREADABLE" )
  mlog ("  numKeys.vertical\t" + ((numKeysSafe v) as string))
  mlog ("  numKeys.horizontal\t" + ((numKeysSafe h) as string))
  local tm = com.transform
  mlog ("  comTM\tpos\t" + (tm.pos.x as string) + "," + (tm.pos.y as string) + "," + (tm.pos.z as string) \
    + "\trot\t" + (tm.rotation.x as string) + "," + (tm.rotation.y as string) + "," + (tm.rotation.z as string) + "," + (tm.rotation.w as string))
)

fn finalizeCase com label = (
  dumpGroundTruth label com
  saveMaxFile (OUT_DIR + label + ".max") quiet:true
  mlog ("SAVED\t" + label + ".max")
  mlog ""
)

-- ------------------------------------------------------------- action atoms
-- Each atom applies ONE conceptual change to an already-loaded, animation-
-- mode-committed com/ctrl. Used both standalone (single-variable cases) and
-- chained (combined cases) — always starting from a fresh loadAndCommit.

fn atomMoveNormal com = ( animate off ( move com [0, 0, -0.1] ) )

fn atomMoveFigureMode com = (
  local ctrl = com.transform.controller
  try ( ctrl.figureMode = true; move com [0, 0, -0.1]; ctrl.figureMode = false )
  catch ( mlog "  ERROR atomMoveFigureMode failed" )
)

fn atomDynType1 com = ( try ( com.transform.controller.dynamicsType = 1 ) catch ( mlog "  ERROR atomDynType1 failed" ) )

fn atomTalentFigMode com = ( try ( com.transform.controller.talentFigMode = true ) catch ( mlog "  ERROR atomTalentFigMode failed" ) )

fn atomLockCom com = ( try ( com.transform.controller.lockCom = true ) catch ( mlog "  ERROR atomLockCom failed" ) )

fn atomMoveAllMode com = ( try ( com.transform.controller.moveAllMode = true ) catch ( mlog "  ERROR atomMoveAllMode failed" ) )

fn atomHeightEdit com = (
  local ctrl = com.transform.controller
  local ok = false
  try ( ctrl.figureMode = true; com.height = com.height * 0.9; ctrl.figureMode = false; ok = true ) catch ()
  if not ok do try ( ctrl.figureMode = true; ctrl.height = ctrl.height * 0.9; ctrl.figureMode = false; ok = true ) catch ()
  mlog ("  atomHeightEdit applied: " + (ok as string))
)

fn atomArmKeyScrub com = (
  local ctrl = com.transform.controller
  local nn = biped.maxNumNodes ctrl
  local arm = undefined
  for i = 1 to nn do ( local nd = biped.getNode ctrl i link:1; if nd != undefined and (matchPattern nd.name pattern:"*UpperArm*" ignoreCase:true) do arm = nd )
  if arm != undefined do (
    animate on (
      at time 0 ( biped.setTransform arm #rotation arm.transform.rotation true )
      at time 20 ( biped.setTransform arm #rotation ((quat 30 [1,0,0]) * arm.transform.rotation) true )
    )
  )
  sliderTime = 10f
)

-- Add a COM vertical key at frame 0, then delete it — tests whether "keyed
-- then unkeyed" leaves different storage than "never keyed at all".
fn atomKeyThenDeleteCom com = (
  local ctrl = com.transform.controller
  local v = undefined
  try ( v = ctrl.vertical.controller ) catch ()
  animate on ( at time 0 ( biped.setTransform com #pos com.transform.pos true ) )
  local ok = false
  try ( if v != undefined and (numKeys v) > 0 do ( deleteKey v 1; ok = true ) ) catch ()
  mlog ("  atomKeyThenDeleteCom deleted: " + (ok as string) + " remaining keys: " + ((numKeysSafe v) as string))
)

fn atomCopyPastePosture com = (
  local ok = false
  try (
    local buf = biped.copyPosture com 1 true true true -- copyType 1, hor/ver/turn
    if buf != undefined do ( biped.pastePosture com 1 0 buf; ok = true )
  ) catch ()
  mlog ("  atomCopyPastePosture applied: " + (ok as string))
)

fn atomConvertFootstepsAndBack com = (
  local ctrl = com.transform.controller
  local ok1 = false; local ok2 = false
  try ( biped.convertToFootSteps ctrl; ok1 = true ) catch ( try ( ctrl.convertToFootSteps(); ok1 = true ) catch () )
  try ( biped.convertToFreeForm ctrl; ok2 = true ) catch ( try ( ctrl.convertToFreeForm(); ok2 = true ) catch () )
  mlog ("  atomConvertFootstepsAndBack: toFootsteps=" + (ok1 as string) + " backToFreeform=" + (ok2 as string))
)

fn atomMirror com = (
  local ok = false
  try ( biped.mirror com.transform.controller; ok = true ) catch ()
  mlog ("  atomMirror applied: " + (ok as string))
)

fn atomClearAllAnimation com = (
  local ok = false
  try ( biped.clearAllAnimation com.transform.controller; ok = true ) catch ()
  mlog ("  atomClearAllAnimation applied: " + (ok as string))
)

fn atomScaleHalf com = ( animate off ( scale com [0.5, 0.5, 0.5] ) )

-- ------------------------------------------------------------------- driver
mlog ("BIPED SKEL ANIMODE PROBE  skel=" + SKEL_FILE + "  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string)
mlog ""

-- B00: the reference baseline every other case is compared against.
( local com = loadAndCommit(); if com != undefined do finalizeCase com "b00_baseline" )

-- ----------------------------------------------------- single-variable cases
global SINGLE_CASES = #(
  #("b01_move_normal", atomMoveNormal),
  #("b02_move_figuremode", atomMoveFigureMode),
  #("b03_dyntype1", atomDynType1),
  #("b04_talentfigmode", atomTalentFigMode),
  #("b05_lockcom", atomLockCom),
  #("b06_moveallmode", atomMoveAllMode),
  #("b07_armkey_scrub", atomArmKeyScrub),
  #("b08_key_then_delete", atomKeyThenDeleteCom),
  #("b09_copypaste_posture", atomCopyPastePosture),
  #("b10_footsteps_roundtrip", atomConvertFootstepsAndBack),
  #("b11_mirror", atomMirror),
  #("b12_clear_all_anim", atomClearAllAnimation),
  #("b13_scale_half", atomScaleHalf),
  #("b14_height_edit", atomHeightEdit)
)
for c in SINGLE_CASES do (
  local com = loadAndCommit()
  if com != undefined do (
    c[2] com
    finalizeCase com c[1]
  )
)

-- ---------------------------------------------------------- combined cases
-- Each entry is a list of atoms applied IN ORDER to the same fresh reload —
-- testing whether a combination leaves a signature no single toggle produces.
global COMBINED_CASES = #(
  #("c01_move_dyntype1", #(atomMoveNormal, atomDynType1)),
  #("c02_move_talentfigmode", #(atomMoveNormal, atomTalentFigMode)),
  #("c03_figmove_dyntype1", #(atomMoveFigureMode, atomDynType1)),
  #("c04_move_key_then_delete", #(atomMoveNormal, atomKeyThenDeleteCom)),
  #("c05_move_footsteps_roundtrip", #(atomMoveNormal, atomConvertFootstepsAndBack)),
  #("c06_move_copypaste_posture", #(atomMoveNormal, atomCopyPastePosture)),
  #("c07_figmove_then_move", #(atomMoveFigureMode, atomMoveNormal)),
  #("c08_move_lockcom", #(atomMoveNormal, atomLockCom))
)
for c in COMBINED_CASES do (
  local com = loadAndCommit()
  if com != undefined do (
    for atomFn in c[2] do atomFn com
    finalizeCase com c[1]
  )
)

mlog ""
mlog "BYE"
