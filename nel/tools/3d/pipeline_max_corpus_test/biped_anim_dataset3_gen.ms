-- ============================================================================
-- biped_anim_dataset3_gen.ms — Biped ANIM dataset ROUND 3: footstep-driven vertical
-- (the ship_tank class probe via freeform-author + biped.convertToFootSteps)
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

global OUT_DIR = "C:\\biped_anim_dataset3\\"   -- EDIT ME: output folder
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
-- DRIVER (round 3 — footstep-driven vertical, the ship_tank class probe)
-- Route: author a FREEFORM animation (scriptable), then biped.ConvertToFootSteps
-- (documented Max 9 MAXScript) -> saved footstep-mode files + GT of the vertical
-- Character Studio computes. The footsteps node (biped.getNode ctrl 16) rides the
-- normal key dump, so footstep keys land in the manifest automatically.
-- ============================================================================
openManifest()
mlog ("BIPED ANIM DATASET 3  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string + "  fps=" + (frameRate as string))

fn tryConvertFS ctrl = (
  local ok = false
  try ( biped.convertToFootSteps ctrl true true; ok = true ) catch ()
  if not ok do try ( biped.convertToFootSteps ctrl true; ok = true ) catch ()
  if not ok do try ( biped.convertToFootSteps ctrl; ok = true ) catch ()
  if ok then mlog "  APPLIED convertToFootSteps" else mlog "  SKIP convertToFootSteps"
  ok
)
fn tryConvertFF ctrl kpf = (
  local ok = false
  try ( biped.convertToFreeForm ctrl kpf; ok = true ) catch ()
  if not ok do try ( biped.convertToFreeForm ctrl; ok = true ) catch ()
  if ok then mlog "  APPLIED convertToFreeForm" else mlog "  SKIP convertToFreeForm"
  ok
)

-- 0) baseline twin
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  finalize com "c_baseline" "no animation keys" 0 0 1.0
)

-- 1) stationary feet-planted freeform converted to footsteps (closest to the
--    mort_idle ship class: no COM keys at all before conversion)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  for side in #("L Foot", "R Foot") do (
    local nd = findBone com side
    local fw = at time 0 (biped.getTransform nd #pos)
    local fr = at time 0 (biped.getTransform nd #rotation)
    keyAbs com side 0 fw fr
    keyAbs com side 30 fw fr
    if nd != undefined do ( setKeyIK nd.controller 1 #planted; setKeyIK nd.controller 2 #planted )
  )
  tryConvertFS com.transform.controller
  finalize com "c_fs_still" "feet planted 0/30, COM keyless, converted to footsteps" 0 30 0.25
)

-- 2) freeform walk converted to footsteps (alternating plants + advancing COM)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 40 (p0 + [0,-1.6,0]) undefined
  local lf = findBone com "L Foot"
  local rf = findBone com "R Foot"
  local lw = at time 0 (biped.getTransform lf #pos)
  local rw = at time 0 (biped.getTransform rf #pos)
  local lr = at time 0 (biped.getTransform lf #rotation)
  local rr = at time 0 (biped.getTransform rf #rotation)
  -- L plants at 0..15 (in place) and 25..40 (0.8m ahead); R plants 10..30 (0.4m ahead)
  keyAbs com "L Foot" 0  lw lr
  keyAbs com "L Foot" 15 lw lr
  keyAbs com "L Foot" 25 (lw + [0,-0.8,0]) lr
  keyAbs com "L Foot" 40 (lw + [0,-0.8,0]) lr
  keyAbs com "R Foot" 0  rw rr
  keyAbs com "R Foot" 10 rw rr
  keyAbs com "R Foot" 20 (rw + [0,-0.8,0]) rr
  keyAbs com "R Foot" 40 (rw + [0,-0.8,0]) rr
  if lf != undefined do for i = 1 to (numKeys lf.controller) do setKeyIK lf.controller i #planted
  if rf != undefined do for i = 1 to (numKeys rf.controller) do setKeyIK rf.controller i #planted
  tryConvertFS com.transform.controller
  finalize com "c_fs_walk" "freeform 2-step walk converted to footsteps" 0 40 0.25
)

-- 3) converted walk with the vertical keys then DELETED (what does footstep mode
--    evaluate with an empty vertical track? — the suspected ship_tank state)
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 40 (p0 + [0,-1.6,0]) undefined
  local lf = findBone com "L Foot"
  local rf = findBone com "R Foot"
  local lw = at time 0 (biped.getTransform lf #pos)
  local rw = at time 0 (biped.getTransform rf #pos)
  local lr = at time 0 (biped.getTransform lf #rotation)
  local rr = at time 0 (biped.getTransform rf #rotation)
  keyAbs com "L Foot" 0  lw lr
  keyAbs com "L Foot" 15 lw lr
  keyAbs com "L Foot" 25 (lw + [0,-0.8,0]) lr
  keyAbs com "L Foot" 40 (lw + [0,-0.8,0]) lr
  keyAbs com "R Foot" 0  rw rr
  keyAbs com "R Foot" 10 rw rr
  keyAbs com "R Foot" 20 (rw + [0,-0.8,0]) rr
  keyAbs com "R Foot" 40 (rw + [0,-0.8,0]) rr
  if lf != undefined do for i = 1 to (numKeys lf.controller) do setKeyIK lf.controller i #planted
  if rf != undefined do for i = 1 to (numKeys rf.controller) do setKeyIK rf.controller i #planted
  if (tryConvertFS com.transform.controller) do (
    try ( deleteKeys com.transform.controller.vertical.controller #allKeys; mlog "  APPLIED deleteKeys vertical (footstep mode)" ) catch ( mlog "  SKIP deleteKeys vertical (footstep mode)" )
  )
  finalize com "c_fs_vertdel" "footstep-converted walk, vertical keys deleted afterwards" 0 40 0.25
)

-- 4) footsteps converted BACK to freeform (keyPerFrame off) — the record shape the
--    export pipeline sees for footstep-authored-then-freeformed anims
(
  local com = buildFromParams (BP())
  animationRange = interval 0 40
  local p0 = com.transform.pos
  keyCom com 0  p0 undefined
  keyCom com 40 (p0 + [0,-1.6,0]) undefined
  local lf = findBone com "L Foot"
  local rf = findBone com "R Foot"
  local lw = at time 0 (biped.getTransform lf #pos)
  local rw = at time 0 (biped.getTransform rf #pos)
  local lr = at time 0 (biped.getTransform lf #rotation)
  local rr = at time 0 (biped.getTransform rf #rotation)
  keyAbs com "L Foot" 0  lw lr
  keyAbs com "L Foot" 15 lw lr
  keyAbs com "L Foot" 25 (lw + [0,-0.8,0]) lr
  keyAbs com "L Foot" 40 (lw + [0,-0.8,0]) lr
  keyAbs com "R Foot" 0  rw rr
  keyAbs com "R Foot" 10 rw rr
  keyAbs com "R Foot" 20 (rw + [0,-0.8,0]) rr
  keyAbs com "R Foot" 40 (rw + [0,-0.8,0]) rr
  if lf != undefined do for i = 1 to (numKeys lf.controller) do setKeyIK lf.controller i #planted
  if rf != undefined do for i = 1 to (numKeys rf.controller) do setKeyIK rf.controller i #planted
  if (tryConvertFS com.transform.controller) do (
    tryConvertFF com.transform.controller false
  )
  finalize com "c_fs_roundtrip" "walk converted to footsteps then back to freeform (keyPerFrame false)" 0 40 0.25
)

mlog ("")
mlog ("DONE")
close gMan
-- end of file
