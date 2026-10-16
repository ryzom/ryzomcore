-- ============================================================================
-- gen_biped_linkcom_probe.ms -- what exactly is the rigid local TM of a biped
-- COM that is LINKED (parented) under another node?
-- (pipeline_max_design.md S10m-ter open item (a); max_geometry_formats Part J.4-bis)
--
-- BACKGROUND
--   The 2026-07-09 anim coverage audit decoded that a linked biped COM (the
--   kitin family's Bip02 abdomen rig under Bip01 Spine, the mektoub rider's
--   Bip01male under the saddle nodes) rides its parent RIGIDLY: the reference
--   .anim's exported local TM for the COM is CONSTANT per file, and the COM's
--   own horizontal/vertical/turn keytracks do NOT place the linked rig in
--   world. Corpus-side reconstruction found:
--     - kitin Bip02: local == the FIGURE-mode attach (COM figure world vs the
--       parent bone's figure world) -- POSITION exact to ~2e-4 on every kitin
--       anim, ROTATION only to ~6e-3 (a small twist whose source is unpinned);
--       the atk_stun_init/loop/end trilogy is off by 0.098 (suspected figure
--       re-pose AFTER the reference export -- reference-era class).
--     - mektoub rider Bip01male: ref local ~= POSITION-IDENTITY (0.0004!) +
--       canonical -90Z rotation vs the saddle. Our figure-attach reconstruction
--       is 1.45 m off; recomputing the saddle chain at its PRS DEFAULT values
--       (instead of its t0 keys) fixes the ROTATION to ~3e-3 but not the
--       position. Neither the rider's figure world, its 0x0104 current-position
--       snapshot, nor any saved-pose composition tried reproduces the position.
--   This script gets the answer from Max itself: dump the LIVE local TM of
--   every linked COM in the real files (that value IS what the reference
--   exporter sampled), re-export through NelExportAnimation where available
--   (authoritative today's-answer -- for the stun trilogy this directly tests
--   the reference-era hypothesis: a fresh export should match OUR figure
--   attach, not the shipped 0.098-off reference), and reproduce the linking
--   operation on a fresh biped with one variable per case so --diff-rig can
--   find where (if anywhere) the attach is stored and which operations
--   re-capture it.
--
-- WHAT TO LOOK AT AFTERWARDS (analysis side, headless):
--   - LINKGT/LINKLOCAL lines: is the live local constant? its exact value vs
--     our figure attach (kitin rot residual 6e-3) and vs the shipped refs.
--   - GETXFORM lines: biped.getTransform vs node.transform on a linked COM --
--     if they differ, getTransform reads the V/H/T channels while the node TM
--     is the ridden value, which is the whole mechanism in one contrast.
--   - DEFAULTSTATE lines (mektoub): saddle TMs with all keys deleted = the
--     PRS default values -- is the rider local ~identity against THAT frame?
--   - Part B .max files: pipeline_max_export_anim --diff-rig c00 vs c01/c02/...
--     pins which chunks change when the attach should have re-captured.
--
-- Target: 3ds Max 9 (character studio / Biped), matching the corpus era.
-- Part A never saves the real files (figure-mode toggles are in-memory only).
-- EDIT the paths below if R:\graphics is mounted elsewhere.
-- ============================================================================

global OUT_DIR = "C:\\biped_linkcom_probe\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true
global GFX_ROOT = "R:\\graphics\\"             -- EDIT ME: asset root

frameRate = 30

-- -------------------------------------------------------------------- manifest
global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

fn fmtP3 p = ( (p.x as string) + "," + (p.y as string) + "," + (p.z as string) )
fn fmtQ q = ( (q.x as string) + "," + (q.y as string) + "," + (q.z as string) + "," + (q.w as string) )

-- ------------------------------------------------------------------- utilities
fn findComNodes = (
  local found = #()
  for o in objects do (
    try ( if classof o.transform.controller == Vertical_Horizontal_Turn then append found o ) catch ()
  )
  found
)

fn numKeysSafe c = ( if c == undefined then -1 else ( local n = -1; try ( n = numKeys c ) catch (); n ) )

fn comControllers com = (
  local ctrl = com.transform.controller
  local v = undefined
  local h = undefined
  local t = undefined
  try ( v = ctrl.vertical.controller ) catch ()
  try ( h = ctrl.horizontal.controller ) catch ()
  try ( t = ctrl.turning.controller ) catch ()
  #(ctrl, v, h, t)
)

-- Per-frame world TM of the linked COM and its parent, plus the computed local
-- (com.transform * inverse(parent.transform), Max row-vector convention) -- the
-- LINKLOCAL value IS what the reference exporter's per-frame sampling stored.
fn dumpLinkGT label com par steps = (
  local rng = animationRange
  local f0 = rng.start
  local f1 = rng.end
  local i = 0
  while i <= steps do (
    local t = f0 + ((f1 - f0) * i / steps)
    at time t (
      local ct = com.transform
      local pt = par.transform
      local lt = ct * (inverse pt)
      mlog ("  LINKGT\t" + label + "\tt\t" + (t as string) \
        + "\tcom.pos\t" + (fmtP3 ct.pos) + "\tcom.rot\t" + (fmtQ ct.rotation) \
        + "\tpar.pos\t" + (fmtP3 pt.pos) + "\tpar.rot\t" + (fmtQ pt.rotation))
      mlog ("  LINKLOCAL\t" + label + "\tt\t" + (t as string) \
        + "\tpos\t" + (fmtP3 lt.pos) + "\trot\t" + (fmtQ lt.rotation))
    )
    i += 1
  )
)

-- biped.getTransform vs node.transform at a few frames: if getTransform reads
-- the V/H/T channel state while the node TM is the parent-ridden value, the
-- two diverge on a linked COM -- that contrast is the mechanism in one line.
fn dumpGetXform label com = (
  local ctrl = com.transform.controller
  local rng = animationRange
  local samples = #(rng.start, (rng.start + (rng.end - rng.start) / 2), rng.end)
  for t in samples do (
    at time t (
      local np = com.transform.pos
      local nq = com.transform.rotation
      local bp = undefined
      local bq = undefined
      try ( bp = biped.getTransform com #pos ) catch ()
      try ( bq = biped.getTransform com #rotation ) catch ()
      local bps = "UNREADABLE"
      local bqs = "UNREADABLE"
      if bp != undefined do bps = fmtP3 bp
      if bq != undefined do bqs = fmtQ bq
      mlog ("  GETXFORM\t" + label + "\tt\t" + (t as string) \
        + "\tnode.pos\t" + (fmtP3 np) + "\tbip.pos\t" + bps \
        + "\tnode.rot\t" + (fmtQ nq) + "\tbip.rot\t" + bqs)
    )
  )
)

-- Figure-mode contrast: the live local while figureMode is ON is the figure
-- attach by definition -- comparing it against the animation-mode LINKLOCAL
-- answers "is the attach the figure attach" per rig, including the exact
-- rotation (our corpus reconstruction has a 6e-3 residual to explain).
-- In-memory only; callers never save afterwards (Part A) or save deliberately
-- as their single variable (Part B).
fn dumpFigureAttach label com par = (
  local ctrl = com.transform.controller
  local ok = false
  try ( ctrl.figureMode = true; ok = true ) catch ()
  if not ok then ( mlog ("  FIGATTACH\t" + label + "\tfigureMode-unsettable") )
  else (
    local ct = com.transform
    local pt = par.transform
    local lt = ct * (inverse pt)
    mlog ("  FIGATTACH\t" + label + "\tpos\t" + (fmtP3 lt.pos) + "\trot\t" + (fmtQ lt.rotation) \
      + "\tcom.pos\t" + (fmtP3 ct.pos) + "\tpar.pos\t" + (fmtP3 pt.pos))
    try ( ctrl.figureMode = false ) catch ()
  )
)

fn dumpComState label com = (
  local cc = comControllers com
  local ctrl = cc[1]
  mlog ("COM-STATE\t" + label)
  mlog ("  parent\t" + (if com.parent == undefined then "<scene root>" else com.parent.name))
  try ( mlog ("  figureMode\t" + (ctrl.figureMode as string)) ) catch ( mlog "  figureMode\tUNREADABLE" )
  mlog ("  numKeys.vertical\t" + ((numKeysSafe cc[2]) as string))
  mlog ("  numKeys.horizontal\t" + ((numKeysSafe cc[3]) as string))
  mlog ("  numKeys.turning\t" + ((numKeysSafe cc[4]) as string))
  mlog ("  sliderTime\t" + (sliderTime as string))
)

-- Re-export through the real pipeline where the NelExport plugin is present:
-- the authoritative what-would-export-TODAY answer. Export from the ROOT
-- selection node (parent == scene root), like the anim maxscript does.
fn tryReexport rootnode label = (
  local outPath = OUT_DIR + label + "_reexport.anim"
  local ok = false
  try ( NelExportAnimation #(rootnode) outPath false; ok = true ) catch ()
  if ok then mlog ("  REEXPORT\t" + label + "\tOK\t" + outPath)
  else mlog ("  REEXPORT\t" + label + "\tSKIP (NelExportAnimation unavailable or failed)")
  ok
)

-- ============================================================================
-- PART A -- the real corpus files, read-only (never re-saved)
-- ============================================================================
global REAL_FILES = #(
  -- the clean figure-attach class (position matches our reconstruction to 2e-4)
  #("kitin_run",        GFX_ROOT + "stuff\\tryker\\agents\\monsters\\kitin\\animation\\anims\\tr_mo_kitin_run.max"),
  -- the era-suspect trilogy member (ref attach 0.098 off our figure attach):
  -- if the LIVE local == OUR figure attach and the REEXPORT matches it too,
  -- the shipped reference is stale (figure re-posed after export) -- era class
  -- PROVEN. If the live local matches the shipped ref instead, our figure
  -- reconstruction is missing something real.
  #("kitin_stun_loop",  GFX_ROOT + "stuff\\tryker\\agents\\monsters\\kitin\\animation\\anims\\tr_mo_kitin_atk_stun_loop.max"),
  -- two-biped queen (Bip02 under Bip01 Spine2) for generality
  #("kitin_queen_death", GFX_ROOT + "stuff\\tryker\\agents\\monsters\\kitin\\animation\\anims\\tr_mo_kitin_queen_idle_death.max"),
  -- the unexplained one: rider COM under an ANIMATED non-biped saddle chain
  #("mektoub_straf_dr", GFX_ROOT + "stuff\\tryker\\agents\\monsters\\cheval\\animation\\anims\\tr_mo_mektoub_selle_straf_droite.max")
)

mlog ("BIPED LINKED-COM PROBE  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string)
mlog ""
mlog "=== PART A: real corpus files (read-only) ==="
for rf in REAL_FILES do (
  local label = rf[1]
  local path = rf[2]
  mlog ("")
  mlog ("LOADING\t" + label + "\t" + path)
  if not (doesFileExist path) then ( mlog ("  ERROR file not found: " + path) )
  else (
    loadMaxFile path quiet:true
    local coms = findComNodes()
    if coms.count == 0 then ( mlog "  ERROR no biped COM node found" )
    else (
      local rootCom = undefined
      for com in coms do (
        dumpComState (label + "#" + com.name) com
        if com.parent == undefined then ( rootCom = com )
        else (
          -- this is the linked COM -- the whole point
          dumpLinkGT (label + "#" + com.name) com com.parent 12
          dumpGetXform (label + "#" + com.name) com
        )
      )
      -- authoritative re-export FIRST (before any figure-mode toggle can
      -- disturb in-memory state), from the root selection node like the
      -- reference maxscript
      if rootCom != undefined do tryReexport rootCom label
      -- figure-mode attach contrast LAST (in-memory commit on exit, no save)
      for com in coms do (
        if com.parent != undefined do dumpFigureAttach (label + "#" + com.name) com com.parent
      )
      -- mektoub only: delete the saddle chain's keys and re-read -- the node
      -- TMs then evaluate at the PRS DEFAULT-value chunks, the frame our
      -- headless walk cannot otherwise observe live. Destructive in-memory,
      -- never saved, done dead last.
      if label == "mektoub_straf_dr" do (
        local rider = undefined
        for com in coms do ( if com.parent != undefined do rider = com )
        if rider != undefined do (
          -- collect only the NON-biped saddle nodes (stop at the first biped
          -- bone): deleting keys on the mount's biped tracks would repose the
          -- whole mount and pollute the default-frame read
          local chain = #()
          local n = rider.parent
          while n != undefined do (
            local isBip = false
            try ( local cc = classof n.transform.controller; isBip = (cc == BipSlave_Control or cc == Vertical_Horizontal_Turn) ) catch ()
            if isBip then n = undefined
            else ( append chain n; n = n.parent )
          )
          for nd in chain do ( try ( deleteKeys nd #allKeys ) catch ( mlog ("  DEFAULTSTATE deleteKeys failed on " + nd.name) ) )
          local ct = rider.transform
          local pt = rider.parent.transform
          local lt = ct * (inverse pt)
          mlog ("  DEFAULTSTATE\t" + label + "\trider-local-after-key-delete\tpos\t" + (fmtP3 lt.pos) + "\trot\t" + (fmtQ lt.rotation))
          for nd in chain do (
            mlog ("  DEFAULTSTATE\t" + label + "\tnode\t" + nd.name + "\tpos\t" + (fmtP3 nd.transform.pos) + "\trot\t" + (fmtQ nd.transform.rotation))
          )
        )
      )
    )
  )
)

-- ============================================================================
-- PART B -- synthetic single-variable link cases (fresh scene each; each case
-- saves its .max so --diff-rig can locate the attach storage and each case's
-- manifest GT pins the live rule)
-- ============================================================================
mlog ""
mlog "=== PART B: synthetic link cases ==="

fn buildRig = (
  resetMaxFile #noPrompt
  local com = biped.createNew 1.8 -90.0 [0,0,0] \
      arms:true neckLinks:1 spineLinks:3 legLinks:3 tailLinks:0 \
      ponytail1Links:0 ponytail2Links:0 fingers:5 fingerLinks:3 toes:1 toeLinks:1 \
      ankleAttach:0.2 trianglePelvis:false triangleNeck:false \
      prop1Exists:false prop2Exists:false prop3Exists:false forearmTwistLinks:0
  com.transform.controller.rootName = "Bip01"
  animationRange = interval 0 40
  com
)

-- A keyed parent box: static 30-degree Z rotation, position keyed over the
-- range so "does the linked COM ride it" is visible in the GT sweep.
fn buildParentBox = (
  local b = box name:"linkparent" length:0.4 width:0.4 height:0.2 pos:[1.0, 2.0, 0.5]
  b.rotation = quat 30 [0,0,1]
  animate on (
    at time 0  ( b.pos = [1.0, 2.0, 0.5] )
    at time 40 ( b.pos = [1.6, 2.4, 0.9] )
  )
  b
)

fn tryLink com par label = (
  local ok = false
  try ( com.parent = par; ok = (com.parent == par) ) catch ()
  if ok then mlog ("  LINKED\t" + label + "\tcom.parent=" + com.parent.name)
  else mlog ("  LINK-FAILED\t" + label + "\t(.parent assignment rejected)")
  ok
)

fn finishCase com par label = (
  dumpComState label com
  if com.parent != undefined do (
    dumpLinkGT label com com.parent 8
    dumpGetXform label com
  )
  tryReexport com label
  saveMaxFile (OUT_DIR + label + ".max") quiet:true
  mlog ("  SAVED\t" + OUT_DIR + label + ".max")
)

-- C00: link at the untouched figure pose. Attach should equal the figure
-- attach under EVERY candidate rule -- the baseline for --diff-rig.
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c00_link_at_figure"
  finishCase com par "c00_link_at_figure"
)

-- C01: POSE the COM first (ordinary move + rotation, not figure mode), THEN
-- link. THE discriminator: figure-attach rule predicts the attach ignores the
-- pose; link-capture predicts the attach preserves the posed world TM.
(
  local com = buildRig()
  local par = buildParentBox()
  animate off ( move com [0.0, 0.3, 0.25] )
  -- COM rotation may be restricted outside figure mode on this build; the
  -- position part of the discriminator stands either way
  try ( animate off ( rotate com (quat 20 [0,0,1]) ) ) catch ( mlog "  NOTE c01 COM rotate rejected (position-only pose)" )
  tryLink com par "c01_pose_then_link"
  finishCase com par "c01_pose_then_link"
)

-- C02: link first, then ordinary-move the COM. Does the live local change
-- (attach re-captured / channel-driven) or snap back (rigid)?
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c02_link_then_move"
  animate off ( move com [0.2, 0.0, 0.1] )
  finishCase com par "c02_link_then_move"
)

-- C03: link, then key the COM's V/H/T (setTransform #pos at 0 and 20). Do the
-- channels move the linked node at all? (Corpus says they should NOT.)
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c03_link_then_vht_keys"
  animate on (
    at time 0  ( biped.setTransform com #pos (com.transform.pos) true )
    at time 20 ( biped.setTransform com #pos (com.transform.pos + [0.0, 0.0, 0.3]) true )
  )
  finishCase com par "c03_link_then_vht_keys"
)

-- C04: link, then edit the FIGURE pose (figure-mode move) and exit. Under the
-- figure-attach rule the exported attach must FOLLOW this edit -- this is also
-- the synthetic reproduction of the stun-trilogy era hypothesis.
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c04_link_then_figmove"
  local ctrl = com.transform.controller
  try (
    ctrl.figureMode = true
    move com [0.0, 0.0, -0.2]
    ctrl.figureMode = false
  ) catch ( mlog "  ERROR c04 figure-mode move failed" )
  finishCase com par "c04_link_then_figmove"
)

-- C05: link, then move the PARENT box (ordinary move on top of its keys).
-- Rigid riding predicts the local is unchanged while the world follows.
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c05_link_then_moveparent"
  animate off ( move par [0.0, -0.5, 0.2] )
  finishCase com par "c05_link_then_moveparent"
)

-- C06: link with the slider scrubbed to a mid-range frame (box displaced by
-- its keys) -- is the attach captured at the CURRENT frame's parent pose?
(
  local com = buildRig()
  local par = buildParentBox()
  sliderTime = 20f
  tryLink com par "c06_link_at_frame20"
  finishCase com par "c06_link_at_frame20"
)

-- C07: link, then unlink again (contrast case: which chunks revert).
(
  local com = buildRig()
  local par = buildParentBox()
  tryLink com par "c07_link_unlink"
  try ( com.parent = undefined ) catch ( mlog "  ERROR c07 unlink failed" )
  finishCase com par "c07_link_unlink"
)

mlog ""
mlog "BYE"
