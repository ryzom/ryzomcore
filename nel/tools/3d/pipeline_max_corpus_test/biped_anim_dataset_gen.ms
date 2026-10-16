-- ============================================================================
-- biped_anim_dataset_gen.ms  —  Biped ANIMATION differential-analysis dataset
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

global OUT_DIR = "C:\\biped_anim_dataset\\"   -- EDIT ME: output folder
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
-- DRIVER
-- ============================================================================
openManifest()
mlog ("BIPED ANIM DATASET  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string + "  fps=" + (frameRate as string))

-- 0) BASELINE: no keys at all (chunk twin for every diff) ----------------------
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  finalize com "a_baseline" "no animation keys" 0 0 1.0
)

-- 1) COM-ONLY: walk forward, bob, turn — validates h/v/t decode + TCB in-betweens
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 15 (p0 + [0,-1.0,0.05]) undefined
  keyCom com 30 (p0 + [0,-2.0,0]) undefined
  keyCom com 10 undefined ((quat 15 [0,0,1]) * (at time 0 com.transform.rotation))
  keyCom com 30 undefined ((quat -10 [0,0,1]) * (at time 0 com.transform.rotation))
  finalize com "a_com_walk" "COM pos keys 0/15/30 + turn keys 10/30, limbs unkeyed" 0 30 0.25
)

-- 2) FK-ONLY per-limb sparse keys (free keys; validates squad in-betweens per track)
fn fkCase name boneSub axis = (
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local nd = keyRot com boneSub 0 0 axis
  keyRot com boneSub 12 25 axis
  keyRot com boneSub 30 -15 axis
  if nd != undefined do (
    try ( for i = 1 to (numKeys nd.controller) do setKeyIK nd.controller i #free ) catch ()
  )
  finalize com ("a_fk_" + name) ("FK keys 0/12/30 on " + boneSub + " about " + (axis as string)) 0 30 0.25
)
fkCase "uparm"  "L UpperArm" [0,0,1]
fkCase "elbow"  "L Forearm"  [0,0,1]
fkCase "hand"   "L Hand"     [1,0,0]
fkCase "thigh"  "L Thigh"    [0,0,1]
fkCase "knee"   "L Calf"     [0,0,1]
fkCase "foot"   "L Foot"     [1,0,0]
fkCase "spine"  "Bip01 Spine" [0,0,1]
fkCase "head"   "Head"       [0,1,0]
fkCase "tail"   "Tail"       [0,0,1]
fkCase "pony"   "Ponytail1"  [1,0,0]
fkCase "clav"   "L Clavicle" [0,1,0]
fkCase "finger" "L Finger1"  [0,0,1]
fkCase "toe"    "L Toe0"     [0,0,1]

-- 3) TCB / EASE parameter variants on the middle key of the same arm case -------
fn tcbCase name propName v = (
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local nd = keyRot com "L UpperArm" 0 0 [0,0,1]
  keyRot com "L UpperArm" 12 25 [0,0,1]
  keyRot com "L UpperArm" 30 -15 [0,0,1]
  if nd != undefined do setKeyProp nd.controller 2 propName v
  finalize com ("a_tcb_" + name) ("FK arm; middle key " + (propName as string) + "=" + (v as string)) 0 30 0.25
)
tcbCase "tens0"  #tension 0.0
tcbCase "tens50" #tension 50.0
tcbCase "cont0"  #continuity 0.0
tcbCase "cont50" #continuity 50.0
tcbCase "bias0"  #bias 0.0
tcbCase "bias50" #bias 50.0
tcbCase "easeto25"   #easeTo 25.0
tcbCase "easefrom25" #easeFrom 25.0

-- 4) IK CASES (the decisive experiments) ---------------------------------------
-- 4a. blend transitions 0->1, 1->1, 1->0 with a moving COM: which intervals re-solve
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyCom com 0  p0 undefined
  keyCom com 40 (p0 + [0,-1.2,0]) undefined
  -- foot keys: same world transform at 10/20/30 (planted), free at 0 and 40
  keyAbs com "L Foot" 0  undefined fr
  keyAbs com "L Foot" 10 fw fr
  keyAbs com "L Foot" 20 fw fr
  keyAbs com "L Foot" 30 fw fr
  keyAbs com "L Foot" 40 undefined fr
  if foot != undefined do (
    setKeyIK foot.controller 1 #free
    setKeyIK foot.controller 2 #planted
    setKeyIK foot.controller 3 #planted
    setKeyIK foot.controller 4 #planted
    setKeyIK foot.controller 5 #free
  )
  finalize com "a_ik_blendtrans" "COM moves 1.2m; L Foot planted keys at 10/20/30, free at 0/40 (0->1, 1->1, 1->0 intervals)" 0 40 0.25
)
-- 4b. overextension: foot planted while COM moves out of reach
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyAbs com "L Foot" 0  fw fr
  keyAbs com "L Foot" 20 fw fr
  if foot != undefined do ( setKeyIK foot.controller 1 #planted; setKeyIK foot.controller 2 #planted )
  keyCom com 0  p0 undefined
  keyCom com 20 (p0 + [0,-0.9,0.1]) undefined
  finalize com "a_ik_overext" "L Foot planted 0..20 while COM moves 0.9m away (+0.1 up): overextension response" 0 20 0.25
)
-- 4c. heel-to-toe roll: planted foot re-keyed with a pitch change (pivot roll)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyAbs com "L Foot" 0 fw fr
  animate on ( at time 20 (
    biped.setTransform foot #rotation ((quat 35 [1,0,0]) * fr) true
  ) )
  if foot != undefined do ( setKeyIK foot.controller 1 #planted; setKeyIK foot.controller 2 #planted )
  finalize com "a_ik_roll" "L Foot planted, pitched +35deg between keys 0 and 20 (pivot roll in-betweens)" 0 20 0.25
)
-- 4d. ankle tension sweep on the roll case
fn ankleCase name v = (
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyAbs com "L Foot" 0 fw fr
  animate on ( at time 20 ( biped.setTransform foot #rotation ((quat 35 [1,0,0]) * fr) true ) )
  if foot != undefined do (
    setKeyIK foot.controller 1 #planted; setKeyIK foot.controller 2 #planted
    setKeyProp foot.controller 1 #ankleTension v
    setKeyProp foot.controller 2 #ankleTension v
  )
  finalize com ("a_ik_ankle" + name) ("roll case with ankleTension=" + (v as string)) 0 20 0.25
)
ankleCase "0"  0.0
ankleCase "05" 0.5
ankleCase "1"  1.0
-- 4e. pinned hand (arm IK)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local hand = findBone com "L Hand"
  local p0 = com.transform.pos
  local hw = at time 0 (biped.getTransform hand #pos)
  local hr = at time 0 (biped.getTransform hand #rotation)
  keyAbs com "L Hand" 0  undefined hr
  keyAbs com "L Hand" 10 hw hr
  keyAbs com "L Hand" 25 hw hr
  keyAbs com "L Hand" 30 undefined hr
  if hand != undefined do (
    setKeyIK hand.controller 1 #free
    setKeyIK hand.controller 2 #planted
    setKeyIK hand.controller 3 #planted
    setKeyIK hand.controller 4 #free
  )
  keyCom com 0 p0 undefined
  keyCom com 30 (p0 + [0,-0.35,-0.08]) undefined
  finalize com "a_ik_hand" "L Hand pinned keys 10/25 (free 0/30) while COM shifts: arm IK equivalents" 0 30 0.25
)

-- 5) VERTICAL DYNAMICS: jump arc with dynamics blend / ballistic tension --------
fn dynCase name blendV ballV = (
  local com = buildFromParams (BP())
  animationRange = interval 0 24
  local ctrl = com.transform.controller
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 8  (p0 + [0,0,0.35]) undefined
  keyCom com 16 (p0 + [0,0,0.02]) undefined
  keyCom com 24 p0 undefined
  try (
    local vc = ctrl.vertical.controller
    for i = 1 to (numKeys vc) do (
      if blendV != undefined do setKeyProp vc i #dynamicsBlend blendV
      if ballV != undefined do setKeyProp vc i #ballisticTension ballV
    )
  ) catch ( mlog "  SKIP vertical key props" )
  finalize com ("a_dyn_" + name) ("vertical hop; dynamicsBlend=" + (blendV as string) + " ballisticTension=" + (ballV as string)) 0 24 0.25
)
dynCase "off"    0.0 undefined
dynCase "on"     1.0 undefined
dynCase "ball0"  1.0 0.0
dynCase "ball1"  1.0 1.0

-- 6) RANGE PROBES ---------------------------------------------------------------
-- 6a. one limb keyed far beyond all others: whose keys define the export range?
(
  local com = buildFromParams (BP())
  animationRange = interval 0 60
  keyRot com "L UpperArm" 0 0 [0,0,1]
  keyRot com "L UpperArm" 20 20 [0,0,1]
  keyRot com "Head" 0 0 [0,1,0]
  keyRot com "Head" 60 15 [0,1,0]   -- head keys extend to 60, arm stops at 20
  finalize com "a_range_head60" "arm keys to 20, head keys to 60 (per-node range probe)" 0 60 1.0
)
-- 6b. keys only on the COM (do limb controllers report inherited/derived keys?)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local p0 = com.transform.pos
  keyCom com 0 p0 undefined
  keyCom com 20 (p0 + [0,-0.5,0]) undefined
  finalize com "a_range_comonly" "COM keys only (what does each node's IKeyControl report?)" 0 20 1.0
)

mlog ("")
mlog ("DONE")
close gMan
-- end of file
