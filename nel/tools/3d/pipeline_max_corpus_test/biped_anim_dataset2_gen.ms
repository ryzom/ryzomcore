-- ============================================================================
-- biped_anim_dataset2_gen.ms — Biped ANIM dataset ROUND 2 (keyless-vertical /
-- IK blend transitions / bend links / pelvis / ease sweep — wiki 10d-bis residues)
-- Target: 3ds Max 9 (character studio / Biped), the era of the Ryzom .max corpus.
-- Companion to biped_dataset_gen.ms (figure-mode structure dataset); same manifest
-- conventions, same canonical rig. Units: set System Unit Scale to METERS first.
--
-- WHAT IT DOES
--   Builds the canonical biped and authors many *single-effect* short animations
--   (one interpolation/IK feature varied per file), saving each to its own .max.
--   For every file it logs to manifest.txt:
--     1. PER-FRAME GROUND TRUTH: every biped node's world pos + rot quat at every
--        frame — exactly what the reference exporter sampled via GetNodeTM. For
--        the IK cases additionally at quarter-frame steps, which the 30 fps
--        reference .anim files can never show: this reveals the true in-between
--        curve shape of Character Studio's IK re-solve (pipeline_max_design.md
--        §10c "IK — the open work").
--     2. KEY TABLES: per biped node, the controller's key count, key times and a
--        full property dump of each key object (whatever this build exposes:
--        tension/continuity/bias/ease, IK blend, ankle tension, pivot/space...).
--        Correlating these against the stored keytrack chunk floats pins the
--        still-unidentified record fields ([11] pivot int, [9]=-0.12, the p0..p4
--        time-record params) and settles the BipedRangeMin/Max rule (what
--        GetKeyControlInterface actually reports per node, incl. SubAnim keys).
--
-- HOW TO USE THE OUTPUT
--   * Diff each variant's 0x9155 keytrack chunks (0x012c..0x0160) against the
--     matching no-effect twin — single-effect diffs locate each property's float.
--   * Fit candidate in-between models against the sub-frame samples; the planted /
--     blend-transition / overextension / ankle-tension cases are constructed to
--     discriminate the ankle-path, foot-roll and alpha-semantics hypotheses.
--   * Era caveat (same as the figure dataset): Max 9 writes fresh-format records
--     (0x0115 == 0); re-validate every decode against the legacy-format corpus
--     direct references before shipping it.
--
-- NOTE: some per-key IK properties may not be scriptable in this build; every
--   uncertain call sits in a try() ladder and logs SKIP so the manifest records
--   exactly which effects were actually applied. The saved .max is authoritative
--   either way (we read the flags back out of the chunks).
-- ============================================================================

global OUT_DIR = "C:\\biped_anim_dataset2\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true

frameRate = 30   -- corpus convention: 30 fps, 160 ticks/frame

-- -------------------------------------------------------------------- manifest
global gMan
fn openManifest = ( gMan = createFile (OUT_DIR + "manifest.txt") )
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

-- --------------------------------------------------------------- baseline rig
struct BP (
  height       = 1.83,
  angle        = -90.0,
  wpos         = [0,0,0],
  arms         = true,
  neckLinks    = 1,
  spineLinks   = 3,
  legLinks     = 3,
  tailLinks    = 3,      -- tail on by default here (tail keytrack coverage)
  pony1Links   = 2,
  pony2Links   = 0,
  fingers      = 5,
  fingerLinks  = 3,
  toes         = 1,
  toeLinks     = 1,
  ankleAttach  = 0.2,
  triPelvis    = false,  -- corpus anim rigs run with trianglePelvis unchecked (anim_export.ms forces it)
  triNeck      = false,
  prop1        = false,
  prop2        = false,
  prop3        = false,
  faTwist      = 0
)

fn buildFromParams p = (
  resetMaxFile #noPrompt
  local com = biped.createNew p.height p.angle p.wpos \
      arms:p.arms neckLinks:p.neckLinks spineLinks:p.spineLinks legLinks:p.legLinks \
      tailLinks:p.tailLinks ponytail1Links:p.pony1Links ponytail2Links:p.pony2Links \
      fingers:p.fingers fingerLinks:p.fingerLinks toes:p.toes toeLinks:p.toeLinks \
      ankleAttach:p.ankleAttach trianglePelvis:p.triPelvis triangleNeck:p.triNeck \
      prop1Exists:p.prop1 prop2Exists:p.prop2 prop3Exists:p.prop3 forearmTwistLinks:p.faTwist
  com.transform.controller.rootName = "Bip01"
  com
)

-- ------------------------------------------------------------- node enumeration
fn enumBiped com = (
  local ctrl = com.transform.controller
  local nn = biped.maxNumNodes ctrl
  local nl = biped.maxNumLinks ctrl
  local res = #()
  for i = 1 to nn do
    for j = 1 to nl do (
      local nd = biped.getNode ctrl i link:j
      if nd != undefined do append res #(nd, i, j)
    )
  res
)

fn findBone com sub = (
  for e in (enumBiped com) do
    if (matchPattern e[1].name pattern:("*" + sub + "*") ignoreCase:true) do return e[1]
  undefined
)

-- ------------------------------------------------------------ ground-truth dump
-- Per-sample world transforms for every biped node (+ the COM node itself).
fn dumpFrame com t = (
  at time t (
    local ctm = com.transform
    mlog ("  SAMPLE\t" + (t as string) + "\tBip01\tpos\t" + (ctm.pos.x as string) + "," + (ctm.pos.y as string) + "," + (ctm.pos.z as string) \
      + "\trot\t" + (ctm.rotation.x as string) + "," + (ctm.rotation.y as string) + "," + (ctm.rotation.z as string) + "," + (ctm.rotation.w as string))
    for e in (enumBiped com) do (
      local nd = e[1]
      local tm = nd.transform
      mlog ("  SAMPLE\t" + (t as string) + "\t" + nd.name \
        + "\tpos\t" + (tm.pos.x as string) + "," + (tm.pos.y as string) + "," + (tm.pos.z as string) \
        + "\trot\t" + (tm.rotation.x as string) + "," + (tm.rotation.y as string) + "," + (tm.rotation.z as string) + "," + (tm.rotation.w as string))
    )
  )
)

fn dumpAnim com firstF lastF step = (
  local t = firstF as float
  while t <= (lastF as float) + 0.001 do (
    dumpFrame com t
    t += step
  )
)

-- ---------------------------------------------------------- key-property probe
-- Dump every key of a controller: time + all scriptable properties (names differ
-- across builds; getPropNames captures whatever exists).
fn dumpCtrlKeys nodeName c label = (
  local n = 0
  try ( n = numKeys c ) catch ( mlog ("  KEYS\t" + nodeName + "\t" + label + "\tnumKeys-unsupported"); return() )
  mlog ("  KEYS\t" + nodeName + "\t" + label + "\tcount\t" + (n as string))
  for i = 1 to n do (
    local kt = "?"
    try ( kt = ((getKeyTime c i) as string) ) catch ()
    local props = "?"
    try (
      local k = biped.getKey c i
      props = ""
      for pn in (getPropNames k) do (
        local pv = "?"
        try ( pv = ((getProperty k pn) as string) ) catch ()
        props += ((pn as string) + "=" + pv + " ")
      )
    ) catch (
      try (
        local k2 = getKey c i
        props = ""
        for pn in (getPropNames k2) do (
          local pv = "?"
          try ( pv = ((getProperty k2 pn) as string) ) catch ()
          props += ((pn as string) + "=" + pv + " ")
        )
      ) catch ( props = "getKey-unsupported" )
    )
    mlog ("    KEY\t" + (i as string) + "\ttime\t" + kt + "\t" + props)
  )
)

fn dumpAllKeys com = (
  local ctrl = com.transform.controller
  -- COM sub-tracks: vertical / horizontal / turning
  try ( dumpCtrlKeys "Bip01" ctrl.vertical.controller "vertical" ) catch ( mlog "  KEYS\tBip01\tvertical\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl.horizontal.controller "horizontal" ) catch ( mlog "  KEYS\tBip01\thorizontal\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl.turning.controller "turning" ) catch ( mlog "  KEYS\tBip01\tturning\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl "com-tm" ) catch ()
  for e in (enumBiped com) do (
    local nd = e[1]
    try ( dumpCtrlKeys nd.name nd.controller "tm" ) catch ()
  )
)

-- ------------------------------------------------------------------- save helper
fn finalize com name desc firstF lastF step = (
  local ctrl = com.transform.controller
  ctrl.figureMode = false
  mlog ("")
  mlog ("FILE\t" + name + ".max\tCHANGE\t" + desc)
  dumpAllKeys com
  mlog ("  TRUTH\tstep\t" + (step as string))
  dumpAnim com firstF lastF step
  saveMaxFile (OUT_DIR + name + ".max") quiet:true
)

-- -------------------------------------------------------------- keying helpers
-- World-space delta rotation key on one biped bone at frame f (creates a biped key).
fn keyRot com boneSub f angleDeg axis = (
  local nd = findBone com boneSub
  if nd == undefined then ( mlog ("SKIP keyRot no bone " + boneSub); undefined )
  else (
    animate on ( at time f (
      local wr = biped.getTransform nd #rotation
      biped.setTransform nd #rotation ((quat angleDeg axis) * wr) true
    ) )
    nd
  )
)

-- Absolute world-position + rotation key on one bone (for planted feet / pinned hands).
fn keyAbs com boneSub f wp wr = (
  local nd = findBone com boneSub
  if nd == undefined then ( mlog ("SKIP keyAbs no bone " + boneSub); undefined )
  else (
    animate on ( at time f (
      if wp != undefined do biped.setTransform nd #pos wp true
      if wr != undefined do biped.setTransform nd #rotation wr true
    ) )
    nd
  )
)

-- COM keys: position (vertical+horizontal tracks) and/or rotation (turning track).
fn keyCom com f wp wr = (
  animate on ( at time f (
    if wp != undefined do biped.setTransform com #pos wp true
    if wr != undefined do biped.setTransform com #rotation wr true
  ) )
)

-- Try-ladder: flag the key at (1-based) index i on a limb controller as planted /
-- sliding / free. Logs which API worked so the manifest records the actual effect.
fn setKeyIK c i mode = (
  local ok = false
  if mode == #planted do (
    try ( biped.setPlantedKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 1.0; ok = true ) catch ()
  )
  if mode == #sliding do (
    try ( biped.setSlidingKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 1.0; ok = true ) catch ()
  )
  if mode == #free do (
    try ( biped.setFreeKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 0.0; ok = true ) catch ()
  )
  if not ok then mlog ("  SKIP setKeyIK " + (mode as string) + " idx " + (i as string))
  else mlog ("  APPLIED setKeyIK " + (mode as string) + " idx " + (i as string))
  ok
)

-- Try-ladder: set a TCB/ease property on key i of controller c.
fn setKeyProp c i propName v = (
  local ok = false
  try ( local k = biped.getKey c i; setProperty k propName v; ok = true ) catch ()
  if not ok then mlog ("  SKIP setKeyProp " + (propName as string) + " idx " + (i as string))
  else mlog ("  APPLIED setKeyProp " + (propName as string) + "=" + (v as string) + " idx " + (i as string))
  ok
)

-- ============================================================================
-- ============================================================================
-- DRIVER (round 2 — targets the residues left open by round 1, wiki 10d-bis)
-- ============================================================================
openManifest()
mlog ("BIPED ANIM DATASET 2  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string + "  fps=" + (frameRate as string))

-- 0) BASELINE twin --------------------------------------------------------------
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  finalize com "b_baseline" "no animation keys" 0 0 1.0
)

-- 1) KEYLESS-VERTICAL family (the ship_tank class: COM z off ~4.7m, vertical track
--    has no keys — which signal drives the runtime height solve?) ----------------
-- 1a. horizontal keys, vertical keys DELETED after keying
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local ctrl = com.transform.controller
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 15 (p0 + [0,-1.0,0.3]) undefined
  keyCom com 30 (p0 + [0,-2.0,0]) undefined
  try ( deleteKeys ctrl.vertical.controller #allKeys; mlog "  APPLIED deleteKeys vertical" ) catch ( mlog "  SKIP deleteKeys vertical" )
  finalize com "b_dynv_hor" "horizontal keys 0/15/30 (z requested but vertical keys deleted): keyless-vertical response" 0 30 0.25
)
-- 1b. turn keys only, vertical deleted
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local ctrl = com.transform.controller
  local r0 = at time 0 com.transform.rotation
  keyCom com 0  undefined r0
  keyCom com 15 undefined ((quat 40 [0,0,1]) * r0)
  keyCom com 30 undefined ((quat -20 [0,0,1]) * r0)
  try ( deleteKeys ctrl.vertical.controller #allKeys; mlog "  APPLIED deleteKeys vertical" ) catch ( mlog "  SKIP deleteKeys vertical" )
  try ( deleteKeys ctrl.horizontal.controller #allKeys; mlog "  APPLIED deleteKeys horizontal" ) catch ( mlog "  SKIP deleteKeys horizontal" )
  finalize com "b_dynv_turn" "turn keys only, vertical+horizontal keys deleted" 0 30 0.25
)
-- 1c. limbs keyed + feet planted, COM fully keyless: does balance/dynamics move z?
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local foot = findBone com "L Foot"
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyAbs com "L Foot" 0 fw fr
  keyAbs com "L Foot" 30 fw fr
  if foot != undefined do ( setKeyIK foot.controller 1 #planted; setKeyIK foot.controller 2 #planted )
  keyRot com "Bip01 Spine" 0 0 [0,1,0]
  keyRot com "Bip01 Spine" 15 35 [0,1,0]
  keyRot com "Bip01 Spine" 30 0 [0,1,0]
  finalize com "b_dynv_bend" "spine bends 35deg with L foot planted, COM keyless: balance response probe" 0 30 0.25
)
-- 1d. gravity: keyless vertical + gravAccel changed (controller-level property)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local ctrl = com.transform.controller
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 30 (p0 + [0,-2.0,0]) undefined
  try ( deleteKeys ctrl.vertical.controller #allKeys; mlog "  APPLIED deleteKeys vertical" ) catch ( mlog "  SKIP deleteKeys vertical" )
  local okg = false
  try ( ctrl.gravAccel = 2.0; okg = true ) catch ()
  if not okg do try ( setProperty ctrl #gravAccel 2.0; okg = true ) catch ()
  if okg then mlog "  APPLIED gravAccel=2.0" else mlog "  SKIP gravAccel"
  finalize com "b_dynv_grav" "keyless vertical + gravAccel 2.0 (if scriptable)" 0 30 0.25
)
-- NOTE: if none of the b_dynv_* cases reproduce the ship_tank constant-offset class,
-- the mechanism is likely FOOTSTEP-driven vertical (footstep mode isn't reliably
-- scriptable in this build) — in that case: author one manual case in the UI
-- (Footstep Mode > create 2-3 footsteps > Create Keys for Inactive Footsteps),
-- save as b_fs_manual.max in OUT_DIR, then run:
--   com = $Bip01.transform.controller.rootNode -- select the biped COM
--   finalize <com> "b_fs_manual_dump" "manual footstep case" 0 30 0.25

-- 2) IK BLEND-TRANSITION family (round 1: ~3cm on thigh/calf during alpha ramps) --
-- 2a. blend ramp with a STATIC COM: isolates the blend curve itself (FK==IK at the
--     boundary keys; any in-between deviation is the transition math alone)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  -- lift the foot at the free key so FK and IK poses differ mid-interval
  keyAbs com "L Foot" 0  (fw + [0,0,0.25]) fr
  keyAbs com "L Foot" 10 fw fr
  keyAbs com "L Foot" 20 fw fr
  if foot != undefined do (
    setKeyIK foot.controller 1 #free
    setKeyIK foot.controller 2 #planted
    setKeyIK foot.controller 3 #planted
  )
  finalize com "b_ikb_static" "static COM; foot free(lifted)->planted->planted: pure blend-in curve" 0 20 0.25
)
-- 2b. long ramp: free at 0, planted at 30 (one 30-frame transition interval)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyCom com 0  p0 undefined
  keyCom com 30 (p0 + [0,-0.9,0]) undefined
  keyAbs com "L Foot" 0  undefined fr
  keyAbs com "L Foot" 30 fw fr
  if foot != undefined do (
    setKeyIK foot.controller 1 #free
    setKeyIK foot.controller 2 #planted
  )
  finalize com "b_ikb_long" "single 30-frame free->planted ramp while COM moves 0.9m" 0 30 0.25
)
-- 2c. ramp-out only: planted at 0/10, free at 25 (1->0 over 15 frames)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 25
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyCom com 0  p0 undefined
  keyCom com 25 (p0 + [0,-0.7,0]) undefined
  keyAbs com "L Foot" 0  fw fr
  keyAbs com "L Foot" 10 fw fr
  keyAbs com "L Foot" 25 undefined fr
  if foot != undefined do (
    setKeyIK foot.controller 1 #planted
    setKeyIK foot.controller 2 #planted
    setKeyIK foot.controller 3 #free
  )
  finalize com "b_ikb_out" "planted 0/10 -> free 25 (1->0 ramp over 15 frames) while COM moves" 0 25 0.25
)
-- 2d. TCB on the blend-boundary keys (does the alpha ramp follow the keys' TCB?)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyCom com 0  p0 undefined
  keyCom com 20 (p0 + [0,-0.6,0]) undefined
  keyAbs com "L Foot" 0  undefined fr
  keyAbs com "L Foot" 20 fw fr
  if foot != undefined do (
    setKeyIK foot.controller 1 #free
    setKeyIK foot.controller 2 #planted
    setKeyProp foot.controller 1 #tension 0.0
    setKeyProp foot.controller 2 #tension 0.0
  )
  finalize com "b_ikb_tens0" "free->planted ramp with tension 0 on both keys (does alpha follow TCB?)" 0 20 0.25
)
-- 2e. arm blend transition (hand pin ramp)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local hand = findBone com "L Hand"
  local p0 = com.transform.pos
  local hw = at time 0 (biped.getTransform hand #pos)
  local hr = at time 0 (biped.getTransform hand #rotation)
  keyCom com 0  p0 undefined
  keyCom com 20 (p0 + [0,-0.3,0]) undefined
  keyAbs com "L Hand" 0  undefined hr
  keyAbs com "L Hand" 20 hw hr
  if hand != undefined do (
    setKeyIK hand.controller 1 #free
    setKeyIK hand.controller 2 #planted
  )
  finalize com "b_ikb_arm" "single free->pinned hand ramp while COM moves 0.3m" 0 20 0.25
)

-- 3) BEND-LINK channels (round 1 keyed only the BASE links; toe bases turned out
--    cosine-eased while finger bases are TCB — are the BEND scalars TCB or cosine?)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  keyRot com "L Finger11" 0 0 [0,0,1]
  keyRot com "L Finger11" 12 30 [0,0,1]
  keyRot com "L Finger11" 30 -10 [0,0,1]
  finalize com "b_fk_fingerbend" "FK keys 0/12/30 on L Finger11 (bend link) about [0,0,1]" 0 30 0.25
)
(
  local p = BP()
  p.toeLinks = 3
  local com = buildFromParams p
  animationRange = interval 0 30
  keyRot com "L Toe01" 0 0 [0,0,1]
  keyRot com "L Toe01" 12 30 [0,0,1]
  keyRot com "L Toe01" 30 -10 [0,0,1]
  finalize com "b_fk_toebend" "toeLinks=3 rig; FK keys 0/12/30 on L Toe01 (bend link) about [0,0,1]" 0 30 0.25
)

-- 4) PELVIS rotation keys (round 1 had no pelvis case; pins the pelvis channel and
--    the tail/leg anchoring under an animated pelvis) ----------------------------
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  keyRot com "Pelvis" 0 0 [0,1,0]
  keyRot com "Pelvis" 15 25 [0,1,0]
  keyRot com "Pelvis" 30 0 [0,1,0]
  finalize com "b_fk_pelvis" "FK keys 0/15/30 on Pelvis about [0,1,0]" 0 30 0.25
)

-- 5) EASE sweep (round 1 measured the UI-25 warp deviating from the classic curve:
--    linear slope 1.2222 vs 1.3333 with a smooth tail — sweep values + a scalar
--    channel case to fit the true family) ----------------------------------------
fn easeCase name propName v = (
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local nd = keyRot com "L UpperArm" 0 0 [0,0,1]
  keyRot com "L UpperArm" 12 25 [0,0,1]
  keyRot com "L UpperArm" 30 -15 [0,0,1]
  if nd != undefined do setKeyProp nd.controller 2 propName v
  finalize com ("b_ease_" + name) ("FK arm; middle key " + (propName as string) + "=" + (v as string)) 0 30 0.25
)
easeCase "to10"   #easeTo 10.0
easeCase "to40"   #easeTo 40.0
easeCase "to50"   #easeTo 50.0
easeCase "from50" #easeFrom 50.0
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local nd = keyRot com "L UpperArm" 0 0 [0,0,1]
  keyRot com "L UpperArm" 12 25 [0,0,1]
  keyRot com "L UpperArm" 30 -15 [0,0,1]
  if nd != undefined do (
    setKeyProp nd.controller 2 #easeTo 25.0
    setKeyProp nd.controller 2 #easeFrom 25.0
  )
  finalize com "b_ease_both25" "FK arm; middle key easeTo=25 AND easeFrom=25" 0 30 0.25
)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local nd = keyRot com "L Calf" 0 0 [0,0,1]
  keyRot com "L Calf" 12 30 [0,0,1]
  keyRot com "L Calf" 30 -10 [0,0,1]
  if nd != undefined do (
    try ( for i = 1 to (numKeys nd.controller) do setKeyIK nd.controller i #free ) catch ()
    setKeyProp nd.controller 2 #easeTo 25.0
  )
  finalize com "b_ease_knee25" "knee (scalar hinge channel); middle key easeTo=25" 0 30 0.25
)

-- 6) MULTI-LINK NECK (neck angle channels under animation; round 1 rig had 1 link)
(
  local p = BP()
  p.neckLinks = 3
  local com = buildFromParams p
  animationRange = interval 0 30
  keyRot com "Neck1" 0 0 [0,0,1]
  keyRot com "Neck1" 15 25 [0,0,1]
  keyRot com "Neck1" 30 0 [0,0,1]
  finalize com "b_fk_neck3" "neckLinks=3 rig; FK keys 0/15/30 on Neck1 about [0,0,1]" 0 30 0.25
)

mlog ("")
mlog ("DONE")
close gMan
-- end of file
