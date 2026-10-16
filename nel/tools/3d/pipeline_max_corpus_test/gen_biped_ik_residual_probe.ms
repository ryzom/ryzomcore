-- ============================================================================
-- gen_biped_ik_residual_probe.ms
-- Target: 3ds Max 9 SP2 + Character Studio (corpus era). Units: METERS.
--
-- PURPOSE (pipeline_max_design.md section 10r open items / section 10s designed probes /
--          section 10s-cinq residual LargePath strike after arm-pin shipping)
--   Produce the Max-side ground truth still missing after offline biped.dlc
--   probes settled plant-gate policy (both-keys keep; GetIKActive is look-ahead
--   and must NOT replace the pipeline gate).
--
-- PART A - REAL CORPUS FILES (read-only; never re-saved)
--   Load residual anims under R:\graphics and dump:
--     * quarter-frame biped.getTransform GT over the residual plant windows
--     * full-range sparse GT (1.0 frame) for context
--     * full per-key property dumps (ikBlend / ankleTension / pivots / TCB / ...)
--     * best-effort IK-active / hinge / space oracles if this build exposes them
--   Residual anchors (from corpus A/B + offline notes):
--     fy_hof_co_l2m_coup_fort_03  R arm LargePath plant ~ticks [4640,5600]
--                                  ~ frames 29..35 @ 30fps (160 ticks/frame)
--     fy_hof_co_l2m_coup_fort / _02  same strike class
--     fy_hof_a_course / a1m course   in-plant foot rot + gait plants
--     fy_hom_emot_a_cheer            planted foot vs COM yaw
--     demitour_go / demitour_dr      COM turn residual class
--
-- PART B - SYNTHETIC IN-PLANT FOOT ROTATION DATASET
--   One leg planted (blend 1, ikSpace=Object/world=2) while COM (a) translates,
--   (b) yaws, (c) pitches/lunges, (d) combinations; each x ankleTension 0 / 1
--   (scripted + saved .max for --diff-rig). Quarter-frame GT.
--   First landing used Body space (ikSpace=0) by accident - foot rode COM;
--   this revision forces Object space after setPlantedKey so world plant holds.
--   Discriminates: parallel-transport / world-hold / ankle-cone / tension mix.
--
-- PART C - ikAnkleTension STORAGE ISOLATION
--   Prior a_ik_ankle0/05/1 triplet was byte-identical outside noise: scripted
--   setKey never took. This part retries every known property spelling on a
--   planted roll twin, saves twins for:
--     pipeline_max_export_anim --diff-rig <base>.max <tN>.max out.txt
--   AND leaves a clearly-named baseline for a 2-minute HAND UI pass if script
--   still fails (see MANUAL_ANKLE note at end of Part C).
--
-- PART D - COM TURN V/H/T INTERPOLATION
--   Pure turn keys at 30/90/150/210deg steps, varied TCB; quarter-frame COM GT.
--   Replaces the corpus-arbitrated angle+residual gate with a measured rule.
--
-- PART E - ARM-PIN / LARGE-PATH SYNTHETICS
--   Dual-hand pin while COM yaws / translates (weapon-plant shape); free->plant
--   blend ramps on arms; long multi-key palm plant with large end-effector travel.
--
-- OUTPUT
--   C:\biped_ik_residual_probe\manifest.txt   (TAB-separated SAMPLE/KEY lines)
--   C:\biped_ik_residual_probe\*.max          (synthetic cases only)
--   Copy the whole folder back to the Linux box as ~/biped_ik_residual_probe/
--
-- HOW TO RUN
--   1. System Unit Scale = Meters.
--   2. Mount graphics at R:\graphics (or edit GFX_ROOT). Part A needs the mount.
--   3. Toggle RUN_PART_* below to run subsets (A alone is the highest ROI).
--   4. MAXScript -> Run Script. Long run (Part A+B+C+D+E); expect 15-40 min.
--   5. Copy C:\biped_ik_residual_probe\ -> ~/biped_ik_residual_probe/
--
-- ERA CAVEAT
--   Max 9 live eval is authoritative for Character Studio RULE discovery and
--   for storage-format --diff-rig. Shipping references under ~/pipeline_export
--   were exported under Max 2010 - use Max 9 GT to learn rules, final A/B still
--   goes through pipeline_max_export_anim vs those refs.
-- ============================================================================

global OUT_DIR  = "C:\\biped_ik_residual_probe\\"   -- EDIT ME
global GFX_ROOT = "R:\\graphics\\"                   -- EDIT ME
makeDir OUT_DIR all:true

-- Subset toggles (set false to skip). Part A is the highest residual ROI.
global RUN_PART_A = true   -- real corpus dense GT (read-only)
global RUN_PART_B = true   -- synthetic foot-rot x tension
global RUN_PART_C = true   -- ankleTension storage isolation (+ manual note)
global RUN_PART_D = true   -- COM turn step synthetic
global RUN_PART_E = true   -- arm-pin / large-path synthetic

frameRate = 30   -- corpus: 30 fps, 160 ticks/frame

-- -------------------------------------------------------------------- manifest
global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = (
  if gMan != undefined do ( format "%\n" s to:gMan; flush gMan )
  format "%\n" s
)

fn caseHeader label = (
  mlog ""
  mlog ("================================================================")
  mlog ("CASE\t" + label)
  mlog ("================================================================")
)

fn partHeader title = (
  mlog ""
  mlog ("################################################################")
  mlog ("# " + title)
  mlog ("################################################################")
)

-- ---------------------------------------------------------------- path helpers
fn gfx pathTail = ( GFX_ROOT + pathTail )
fn outPath name = ( OUT_DIR + name )

-- ------------------------------------------------------------- biped discovery
fn findComNodes = (
  local found = #()
  for o in objects do (
    try ( if classof o.transform.controller == Vertical_Horizontal_Turn then append found o ) catch ()
  )
  if found.count == 0 do (
    for o in objects do (
      if (matchPattern o.name pattern:"Bip*" ignoreCase:true) and o.parent == undefined do append found o
    )
  )
  found
)

fn findComNode = (
  local ns = findComNodes()
  if ns.count > 0 then ns[1] else undefined
)

fn enumBiped com = (
  local ctrl = com.transform.controller
  local nn = 0
  local nl = 0
  try ( nn = biped.maxNumNodes ctrl ) catch ()
  try ( nl = biped.maxNumLinks ctrl ) catch ()
  local res = #()
  for i = 1 to nn do
    for j = 1 to nl do (
      local nd = undefined
      try ( nd = biped.getNode ctrl i link:j ) catch ()
      if nd != undefined do append res #(nd, i, j)
    )
  res
)

fn findBone com sub = (
  for e in (enumBiped com) do
    if (matchPattern e[1].name pattern:("*" + sub + "*") ignoreCase:true) do return e[1]
  undefined
)

-- Match first bone whose name contains any of the substrings (priority order).
fn findBoneAny com subs = (
  for s in subs do (
    local nd = findBone com s
    if nd != undefined do return nd
  )
  undefined
)

-- --------------------------------------------------------- transform dump core
-- Prefer biped.getTransform (Character Studio evaluated pose). Fall back to
-- node.transform (GetNodeTM-equivalent) if biped API fails on a node.
fn tmPosRot nd = (
  local p = undefined
  local r = undefined
  try (
    p = biped.getTransform nd #pos
    r = biped.getTransform nd #rotation
  ) catch ()
  if p == undefined or r == undefined do (
    local tm = nd.transform
    p = tm.pos
    r = tm.rotation
  )
  #(p, r)
)

fn fmtPR p r = (
  "pos\t" + (p.x as string) + "," + (p.y as string) + "," + (p.z as string) \
    + "\trot\t" + (r.x as string) + "," + (r.y as string) + "," + (r.z as string) + "," + (r.w as string)
)

fn dumpNodeSample t nd nameOverride = (
  local pr = tmPosRot nd
  local nm = if nameOverride != undefined then nameOverride else nd.name
  mlog ("  SAMPLE\t" + (t as string) + "\t" + nm + "\t" + (fmtPR pr[1] pr[2]))
)

-- Full biped dump at time t (COM + every biped node).
fn dumpFrameFull com t = (
  at time t (
    dumpNodeSample t com "Bip01"
    for e in (enumBiped com) do dumpNodeSample t e[1] undefined
  )
)

-- Focus dump: COM + named bone list (faster for dense residual windows).
fn dumpFrameFocus com t boneList = (
  at time t (
    dumpNodeSample t com "Bip01"
    for nd in boneList do if nd != undefined do dumpNodeSample t nd undefined
  )
)

fn dumpAnimFull com firstF lastF step = (
  local t = firstF as float
  while t <= (lastF as float) + 0.001 do (
    dumpFrameFull com t
    t += step
  )
)

fn dumpAnimFocus com firstF lastF step boneList = (
  local t = firstF as float
  while t <= (lastF as float) + 0.001 do (
    dumpFrameFocus com t boneList
    t += step
  )
)

-- Build the residual-focus bone set: COM already separate; arms + legs + spine.
fn residualBoneList com = (
  local names = #(
    "Pelvis", "Spine", "Spine1", "Spine2", "Neck", "Head",
    "L Thigh", "L Calf", "L Foot", "L Toe0",
    "R Thigh", "R Calf", "R Foot", "R Toe0",
    "L Clavicle", "L UpperArm", "L Forearm", "L Hand",
    "R Clavicle", "R UpperArm", "R Forearm", "R Hand"
  )
  local res = #()
  for n in names do (
    local nd = findBone com n
    if nd != undefined do append res nd
  )
  -- Also grab any Finger0 base if present (hand pin pivot checks).
  local lf = findBone com "L Finger0"
  local rf = findBone com "R Finger0"
  if lf != undefined do append res lf
  if rf != undefined do append res rf
  res
)

-- --------------------------------------------------------------- key dumpers
fn numKeysSafe c = (
  if c == undefined then -1
  else ( local n = -1; try ( n = numKeys c ) catch (); n )
)

fn dumpCtrlKeys nodeName c label = (
  if c == undefined do ( mlog ("  KEYS\t" + nodeName + "\t" + label + "\tUNDEFINED"); return() )
  local n = 0
  try ( n = numKeys c ) catch ( mlog ("  KEYS\t" + nodeName + "\t" + label + "\tnumKeys-unsupported"); return() )
  mlog ("  KEYS\t" + nodeName + "\t" + label + "\tcount\t" + (n as string))
  for i = 1 to n do (
    local kt = "?"
    try ( kt = ((getKeyTime c i) as string) ) catch ()
    -- Also emit ticks if TimeValue is readable as integer.
    local kticks = "?"
    try ( kticks = (((getKeyTime c i) as integer) as string) ) catch ()
    local props = ""
    local got = false
    try (
      local k = biped.getKey c i
      for pn in (getPropNames k) do (
        local pv = "?"
        try ( pv = ((getProperty k pn) as string) ) catch ()
        props += ((pn as string) + "=" + pv + " ")
      )
      got = true
    ) catch ()
    if not got do (
      try (
        local k2 = getKey c i
        for pn in (getPropNames k2) do (
          local pv = "?"
          try ( pv = ((getProperty k2 pn) as string) ) catch ()
          props += ((pn as string) + "=" + pv + " ")
        )
        got = true
      ) catch ()
    )
    if not got do props = "getKey-unsupported"
    mlog ("    KEY\t" + (i as string) + "\ttime\t" + kt + "\tticks\t" + kticks + "\t" + props)
  )
)

fn dumpAllKeys com = (
  local ctrl = com.transform.controller
  try ( dumpCtrlKeys "Bip01" ctrl.vertical.controller   "vertical" )   catch ( mlog "  KEYS\tBip01\tvertical\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl.horizontal.controller "horizontal" ) catch ( mlog "  KEYS\tBip01\thorizontal\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl.turning.controller    "turning" )    catch ( mlog "  KEYS\tBip01\tturning\tSKIP" )
  try ( dumpCtrlKeys "Bip01" ctrl "com-tm" ) catch ()
  for e in (enumBiped com) do (
    try ( dumpCtrlKeys e[1].name e[1].controller "tm" ) catch ()
  )
)

-- Best-effort IK-active / hinge oracle dump across a time window.
-- Offline probe: GetIKActive is look-ahead on plant weight - log whatever
-- MaxScript exposes; do NOT treat as plant solve gate.
fn dumpIkOracle com firstF lastF step limbNodes = (
  mlog ("  IKORACLE\tstep\t" + (step as string) + "\trange\t" + (firstF as string) + ".." + (lastF as string))
  local t = firstF as float
  while t <= (lastF as float) + 0.001 do (
    at time t (
      for nd in limbNodes do if nd != undefined do (
        local line = "  IKACT\t" + (t as string) + "\t" + nd.name
        -- Common spellings / access paths across CS builds.
        local ok = false
        try ( line += ("\tikBlend\t" + (nd.controller.ikBlend as string)); ok = true ) catch ()
        try (
          local k = biped.getKey nd.controller 1
          -- nearest-key style: not time-correct, just presence
        ) catch ()
        -- Per-key props already dumped; here try controller-level live values.
        try ( line += ("\tgetIK\t" + ((biped.getIKActive nd) as string)); ok = true ) catch ()
        try ( line += ("\tgetIK2\t" + ((biped.getIkActive nd) as string)); ok = true ) catch ()
        try ( line += ("\tikActive\t" + (nd.ikActive as string)); ok = true ) catch ()
        try ( line += ("\thinge\t" + ((biped.getHingeVal nd) as string)); ok = true ) catch ()
        try ( line += ("\thinge2\t" + ((biped.getHinge nd) as string)); ok = true ) catch ()
        if not ok do line += "\t(no-live-ik-api)"
        -- Always add live world TM of the end effector for cross-check.
        local pr = tmPosRot nd
        line += ("\t" + (fmtPR pr[1] pr[2]))
        mlog line
      )
    )
    t += step
  )
)

-- ---------------------------------------------------------------- key helpers
fn keyRot com boneSub f angleDeg axis = (
  local nd = findBone com boneSub
  if nd == undefined then ( mlog ("  SKIP keyRot no bone " + boneSub); undefined )
  else (
    animate on ( at time f (
      local wr = biped.getTransform nd #rotation
      biped.setTransform nd #rotation ((quat angleDeg axis) * wr) true
    ) )
    nd
  )
)

fn keyAbs com boneSub f wp wr = (
  local nd = findBone com boneSub
  if nd == undefined then ( mlog ("  SKIP keyAbs no bone " + boneSub); undefined )
  else (
    animate on ( at time f (
      if wp != undefined do biped.setTransform nd #pos wp true
      if wr != undefined do biped.setTransform nd #rotation wr true
    ) )
    nd
  )
)

fn keyCom com f wp wr = (
  animate on ( at time f (
    if wp != undefined do biped.setTransform com #pos wp true
    if wr != undefined do biped.setTransform com #rotation wr true
  ) )
)

fn setKeyIK c i mode = (
  local ok = false
  if mode == #planted do (
    try ( biped.setPlantedKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 1.0; ok = true ) catch ()
    if not ok do try ( setProperty (biped.getKey c i) #ikBlend 1.0; ok = true ) catch ()
  )
  if mode == #sliding do (
    try ( biped.setSlidingKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 1.0; ok = true ) catch ()
  )
  if mode == #free do (
    try ( biped.setFreeKey c i; ok = true ) catch ()
    if not ok do try ( local k = biped.getKey c i; k.ikBlend = 0.0; ok = true ) catch ()
    if not ok do try ( setProperty (biped.getKey c i) #ikBlend 0.0; ok = true ) catch ()
  )
  if not ok then mlog ("  SKIP setKeyIK " + (mode as string) + " idx " + (i as string))
  else mlog ("  APPLIED setKeyIK " + (mode as string) + " idx " + (i as string))
  ok
)

fn setKeyProp c i propName v = (
  local ok = false
  try ( local k = biped.getKey c i; setProperty k propName v; ok = true ) catch ()
  if not ok do try ( local k = biped.getKey c i; setProperty k (propName as name) v; ok = true ) catch ()
  if not ok then mlog ("  SKIP setKeyProp " + (propName as string) + " idx " + (i as string))
  else mlog ("  APPLIED setKeyProp " + (propName as string) + "=" + (v as string) + " idx " + (i as string))
  ok
)

-- Force Object/world plant space (corpus flag [11] == 2). Default setPlantedKey on Max 9
-- often leaves Body space (0), which makes planted feet ride COM motion - useless for
-- the in-plant foot-rotation probe. Try every known spelling; log what sticks.
fn setKeyObjectSpace c i = (
  local ok = false
  -- Integer 2 = object (matches stored [11] on artist plant keys).
  local vals = #(2, #object, #world, "object", "world")
  local names = #(#ikSpace, #space, #ik_space, #IKSpace, #ikJoinSpace)
  for pn in names do (
    for v in vals do (
      if (not ok) and (setKeyProp c i pn v) do ok = true
    )
  )
  try (
    local k = biped.getKey c i
    k.ikSpace = 2
    ok = true
    mlog ("  APPLIED k.ikSpace=2 idx " + (i as string))
  ) catch ()
  try (
    local k = biped.getKey c i
    k.space = 2
    ok = true
    mlog ("  APPLIED k.space=2 idx " + (i as string))
  ) catch ()
  if not ok then mlog ("  SKIP setKeyObjectSpace idx " + (i as string))
  -- Read-back for the manifest (confirms storage for --diff-rig / KEYS dump).
  try (
    local k = biped.getKey c i
    local sp = "?"
    try ( sp = (k.ikSpace as string) ) catch ()
    try ( if sp == "?" or sp == "undefined" do sp = (k.space as string) ) catch ()
    mlog ("  SPACE\tidx\t" + (i as string) + "\tikSpace\t" + sp)
  ) catch ()
  ok
)

fn plantKeysObjectSpace c = (
  if c == undefined do return false
  local nk = 0
  try ( nk = numKeys c ) catch ( nk = 0 )
  if nk < 1 do return false
  for i = 1 to nk do (
    setKeyIK c i #planted
    setKeyObjectSpace c i
  )
  true
)

-- Exhaustive ankle-tension setter: try every spelling known from SDK / UI / docs.
fn setAnkleTensionExhaustive c i v = (
  local names = #(
    #ankleTension, #ikAnkleTension, #AnkleTension, #ik_ankle_ten,
    #Ankle_Tension, #ankle_tension, #ikAnkleTen, #ankleTen
  )
  local any = false
  for pn in names do (
    if setKeyProp c i pn v do any = true
  )
  -- Direct property write ladder.
  try ( local k = biped.getKey c i; k.ankleTension = v; any = true; mlog ("  APPLIED k.ankleTension=" + (v as string)) ) catch ()
  try ( local k = biped.getKey c i; k.ikAnkleTension = v; any = true; mlog ("  APPLIED k.ikAnkleTension=" + (v as string)) ) catch ()
  if not any do mlog ("  SKIP all ankleTension spellings idx " + (i as string) + " v=" + (v as string))
  any
)

-- Read-back whatever ankle-ish props actually exist on key i.
fn dumpAnkleProps c i label = (
  try (
    local k = biped.getKey c i
    mlog ("  ANKLEPROPS\t" + label + "\tidx\t" + (i as string))
    for pn in (getPropNames k) do (
      local pns = (pn as string)
      if (matchPattern pns pattern:"*ankle*" ignoreCase:true) \
        or (matchPattern pns pattern:"*ik*" ignoreCase:true) \
        or (matchPattern pns pattern:"*pivot*" ignoreCase:true) \
        or (matchPattern pns pattern:"*blend*" ignoreCase:true) \
        or (matchPattern pns pattern:"*space*" ignoreCase:true) \
        or (matchPattern pns pattern:"*join*" ignoreCase:true) do (
        local pv = "?"
        try ( pv = ((getProperty k pn) as string) ) catch ()
        mlog ("    " + pns + "=" + pv)
      )
    )
  ) catch ( mlog ("  ANKLEPROPS\t" + label + "\tFAILED") )
)

-- --------------------------------------------------------------- synthetic rig
struct BP (
  height       = 1.83,
  angle        = -90.0,
  wpos         = [0,0,0],
  arms         = true,
  neckLinks    = 1,
  spineLinks   = 3,
  legLinks     = 3,
  tailLinks    = 0,
  pony1Links   = 0,
  pony2Links   = 0,
  fingers      = 5,
  fingerLinks  = 3,
  toes         = 1,
  toeLinks     = 1,
  ankleAttach  = 0.2,
  triPelvis    = false,
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
  try ( com.transform.controller.rootName = "Bip01" ) catch ()
  try ( com.transform.controller.figureMode = false ) catch ()
  com
)

fn finalizeSynthetic com name desc firstF lastF step saveFile:true focusBones:undefined = (
  try ( com.transform.controller.figureMode = false ) catch ()
  mlog ""
  mlog ("FILE\t" + name + ".max\tCHANGE\t" + desc)
  mlog ("  TRUTH\tstep\t" + (step as string) + "\trange\t" + (firstF as string) + ".." + (lastF as string))
  dumpAllKeys com
  if focusBones != undefined then
    dumpAnimFocus com firstF lastF step focusBones
  else
    dumpAnimFull com firstF lastF step
  if saveFile do (
    saveMaxFile (outPath (name + ".max")) quiet:true
    mlog ("  SAVED\t" + (outPath (name + ".max")))
  )
)

-- ------------------------------------------- Part B/C/D/E helpers (top-level)
-- MaxScript is unreliable about `fn` defined inside `if` blocks; keep these global.

fn focusLeg com = (
  local res = #()
  for n in #("Pelvis", "L Thigh", "L Calf", "L Foot", "L Toe0", "R Thigh", "R Calf", "R Foot", "Spine") do (
    local nd = findBone com n
    if nd != undefined do append res nd
  )
  res
)

fn focusArm com = (
  local res = #()
  for n in #("Pelvis", "Spine", "Spine1", "Spine2",
             "L Clavicle", "L UpperArm", "L Forearm", "L Hand",
             "R Clavicle", "R UpperArm", "R Forearm", "R Hand") do (
    local nd = findBone com n
    if nd != undefined do append res nd
  )
  res
)

fn plantFootBase com lastF = (
  local foot = findBone com "L Foot"
  local p0 = com.transform.pos
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  -- Same world foot pose at both plant keys (Object-space plant must pin this).
  keyAbs com "L Foot" 0  fw fr
  keyAbs com "L Foot" lastF fw fr
  if foot != undefined do (
    plantKeysObjectSpace foot.controller
    -- Re-assert world pose after space flip (some builds rewrite EE on space change).
    keyAbs com "L Foot" 0  fw fr
    keyAbs com "L Foot" lastF fw fr
    plantKeysObjectSpace foot.controller
  )
  #(foot, p0, fw, fr)
)

fn comMotionTranslate com p0 = (
  keyCom com 0  p0 undefined
  keyCom com 15 (p0 + [0,-0.6,0]) undefined
  keyCom com 30 (p0 + [0,-1.2,0]) undefined
)

fn comMotionYaw com p0 = (
  local r0 = at time 0 com.transform.rotation
  keyCom com 0  p0 r0
  keyCom com 15 p0 ((quat 90 [0,0,1]) * r0)
  keyCom com 30 p0 ((quat 180 [0,0,1]) * r0)
)

fn comMotionPitch com p0 = (
  local r0 = at time 0 com.transform.rotation
  keyCom com 0  p0 r0
  keyCom com 15 (p0 + [0,-0.15, -0.05]) ((quat 25 [1,0,0]) * r0)
  keyCom com 30 (p0 + [0,-0.30, 0]) r0
)

fn comMotionCombo com p0 = (
  local r0 = at time 0 com.transform.rotation
  keyCom com 0  p0 r0
  keyCom com 15 (p0 + [0,-0.5,0.02]) ((quat 60 [0,0,1]) * r0)
  keyCom com 30 (p0 + [0,-1.0,0]) ((quat 120 [0,0,1]) * r0)
)

-- comMotionFn is one of comMotionTranslate / Yaw / Pitch / Combo (or undefined).
fn footRotCase name desc comMotionFn tensionV = (
  local com = buildFromParams (BP())
  animationRange = interval 0 30
  local base = plantFootBase com 30
  local foot = base[1]
  local p0 = base[2]
  if comMotionFn != undefined do comMotionFn com p0
  if foot != undefined and tensionV != undefined do (
    setAnkleTensionExhaustive foot.controller 1 tensionV
    setAnkleTensionExhaustive foot.controller 2 tensionV
    dumpAnkleProps foot.controller 1 (name + "_k1")
    dumpAnkleProps foot.controller 2 (name + "_k2")
  )
  finalizeSynthetic com name desc 0 30 0.25 focusBones:(focusLeg com)
)

fn makeAnkleTwin name tensionV desc = (
  local com = buildFromParams (BP())
  animationRange = interval 0 20
  local foot = findBone com "L Foot"
  local fw = at time 0 (biped.getTransform foot #pos)
  local fr = at time 0 (biped.getTransform foot #rotation)
  keyAbs com "L Foot" 0 fw fr
  animate on ( at time 20 (
    biped.setTransform foot #rotation ((quat 35 [1,0,0]) * fr) true
    biped.setTransform foot #pos fw true
  ) )
  if foot != undefined do (
    plantKeysObjectSpace foot.controller
    if tensionV != undefined do (
      setAnkleTensionExhaustive foot.controller 1 tensionV
      setAnkleTensionExhaustive foot.controller 2 tensionV
    )
    dumpAnkleProps foot.controller 1 (name + "_k1")
    dumpAnkleProps foot.controller 2 (name + "_k2")
  )
  finalizeSynthetic com name desc 0 20 0.25
)

fn turnCase name angles tcbTension = (
  local com = buildFromParams (BP())
  local nAng = angles.count
  local lastF = (nAng - 1) * 10
  animationRange = interval 0 lastF
  local p0 = com.transform.pos
  local r0 = at time 0 com.transform.rotation
  for i = 1 to nAng do (
    local f = (i - 1) * 10
    local cum = angles[i]
    keyCom com f p0 ((quat cum [0,0,1]) * r0)
  )
  if tcbTension != undefined do (
    try (
      local tc = com.transform.controller.turning.controller
      local nk = numKeys tc
      for i = 1 to nk do setKeyProp tc i #tension tcbTension
    ) catch ( mlog "  SKIP turn TCB set" )
  )
  mlog ("  TURN_ANGLES\t" + (angles as string) + "\ttension\t" + (tcbTension as string))
  finalizeSynthetic com name ("pure COM yaw keys; angles=" + (angles as string) + " tens=" + (tcbTension as string)) 0 lastF 0.25
)

fn max2 a b = ( if a > b then a else b )
fn min2 a b = ( if a < b then a else b )

-- ===========================================================================
-- DRIVER
-- ===========================================================================
mlog ("BIPED IK RESIDUAL PROBE")
mlog ("  out=" + OUT_DIR)
mlog ("  gfx=" + GFX_ROOT)
mlog ("  maxver=" + ((maxVersion())[1] as string))
mlog ("  fps=" + (frameRate as string))
mlog ("  parts A=" + (RUN_PART_A as string) \
  + " B=" + (RUN_PART_B as string) \
  + " C=" + (RUN_PART_C as string) \
  + " D=" + (RUN_PART_D as string) \
  + " E=" + (RUN_PART_E as string))
mlog ("  units note: set System Unit Scale to METERS before running")

-- ============================================================================
-- PART A - real corpus residual files (READ-ONLY)
-- ============================================================================
if RUN_PART_A do (
  partHeader "PART A - real corpus residual GT (read-only, never re-saved)"

  -- Each entry: #(label, path, focusFirstFrame, focusLastFrame, denseStep, sparseWhole:true/false)
  -- Tick residual [4640,5600] @ 160 tpf -> frames 29..35. Widen slightly for context.
  -- "whole" sparse dumps cover animationRange at 1.0 when sparseWhole is true.
  global REAL_CASES = #(
    -- === highest residual ROI: LargePath weapon plants ===
    #(
      "hof_l2m_coup_fort_03",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_l2m_coup_fort_03.max"),
      28.0, 36.0, 0.25, true
    ),
    #(
      "hof_l2m_coup_fort",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_l2m_coup_fort.max"),
      0.0, 40.0, 0.25, true
    ),
    #(
      "hof_l2m_coup_fort_02",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_l2m_coup_fort_02.max"),
      0.0, 40.0, 0.25, true
    ),
    #(
      "hom_l2m_coup_fort_03",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_co_l2m_coup_fort_03.max"),
      28.0, 36.0, 0.25, true
    ),
    #(
      "hom_l2m_coup_fort",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_co_l2m_coup_fort.max"),
      0.0, 40.0, 0.25, true
    ),
    -- === course gait: in-plant foot rot phase asymmetry ===
    #(
      "hof_a_course",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_a_course.max"),
      0.0, 30.0, 0.25, true
    ),
    #(
      "hom_a_course",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_a_course.max"),
      0.0, 30.0, 0.25, true
    ),
    #(
      "hof_co_a1m_course",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_a1m_course.max"),
      0.0, 30.0, 0.25, true
    ),
    #(
      "hom_co_a1m_course",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_co_a1m_course.max"),
      0.0, 30.0, 0.25, true
    ),
    -- === planted foot vs body motion (foot rot open item) ===
    #(
      "hom_emot_a_cheer",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_emot_a_cheer.max"),
      0.0, 60.0, 0.25, true
    ),
    #(
      "hof_emot_a_cheer",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_emot_a_cheer.max"),
      0.0, 60.0, 0.25, true
    ),
    #(
      "hom_emot_a_kneel",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_emot_a_kneel.max"),
      0.0, 40.0, 0.25, true
    ),
    -- === COM turn residual class ===
    #(
      "hom_a_demitour_go",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_a_demitour_go.max"),
      0.0, 40.0, 0.25, true
    ),
    #(
      "hom_a_demitour_dr",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_a_demitour_dr.max"),
      0.0, 40.0, 0.25, true
    ),
    #(
      "hof_l2m_demitour_dr",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_l2m_demitour_dr.max"),
      0.0, 40.0, 0.25, true
    ),
    -- === a1m coup (foot-worst family often hides behind arm noise) ===
    #(
      "hom_co_a1m_coup1",
      (gfx "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_co_a1m_coup1.max"),
      0.0, 40.0, 0.25, true
    ),
    #(
      "hof_co_a1m_course_frappe",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_a1m_course_frappe.max"),
      0.0, 30.0, 0.25, true
    ),
    -- === engarde plant (static multi-key held) ===
    #(
      "hof_l2m_engarde",
      (gfx "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_co_l2m_engarde.max"),
      0.0, 40.0, 0.25, true
    )
  )

  for rc in REAL_CASES do (
    local label = rc[1]
    local path  = rc[2]
    local f0    = rc[3]
    local f1    = rc[4]
    local step  = rc[5]
    local sparseWhole = rc[6]

    caseHeader ("A_" + label)
    mlog ("  PATH\t" + path)

    if not (doesFileExist path) then (
      mlog ("  ERROR\tfile-missing\t" + path)
    ) else (
      resetMaxFile #noPrompt
      local loaded = false
      try ( loaded = loadMaxFile path quiet:true ) catch ( loaded = false )
      if not loaded then mlog ("  ERROR\tload-failed\t" + path)
      else (
        local coms = findComNodes()
        mlog ("  COMBIPEDS\t" + (coms.count as string))
        if coms.count == 0 then mlog "  ERROR\tno-biped-COM"
        else (
          -- Multi-biped aware (kitin-style); dump each COM separately.
          for ci = 1 to coms.count do (
            local com = coms[ci]
            local clabel = label + "_bip" + (ci as string) + "_" + com.name
            mlog ("  BIPED\t" + clabel)
            try ( mlog ("  figureMode\t" + (com.transform.controller.figureMode as string)) ) catch ()
            try ( com.transform.controller.figureMode = false ) catch ()

            local rng = animationRange
            mlog ("  animationRange\t" + (rng.start as string) + ".." + (rng.end as string))
            mlog ("  sliderTime\t" + (sliderTime as string))

            -- Key tables (the storage-side oracle for blend/pivot/tension).
            dumpAllKeys com

            local bones = residualBoneList com
            mlog ("  FOCUS_BONES\t" + (bones.count as string))
            for b in bones do mlog ("    BONE\t" + b.name)

            -- Dense residual / focus window.
            mlog ("  TRUTH_DENSE\tstep\t" + (step as string) + "\trange\t" + (f0 as string) + ".." + (f1 as string))
            dumpAnimFocus com f0 f1 step bones

            -- Sparse whole-range context (integer frames).
            if sparseWhole do (
              local w0 = (rng.start as float)
              local w1 = (rng.end as float)
              -- Cap very long ranges to keep manifest tractable.
              if (w1 - w0) > 200.0 do (
                mlog ("  TRUTH_SPARSE_CAP\toriginal\t" + (w0 as string) + ".." + (w1 as string) + "\tcapped-to-focus+/-20")
                w0 = max2 w0 (f0 - 20.0)
                w1 = min2 w1 (f1 + 20.0)
              )
              mlog ("  TRUTH_SPARSE\tstep\t1.0\trange\t" + (w0 as string) + ".." + (w1 as string))
              dumpAnimFocus com w0 w1 1.0 bones
            )

            -- Live IK oracle on limbs (best-effort; may be no-op on Max 9 MS).
            local limbEE = #()
            for nm in #("L Foot", "R Foot", "L Hand", "R Hand", "L Calf", "R Calf", "L Forearm", "R Forearm") do (
              local nd = findBone com nm
              if nd != undefined do append limbEE nd
            )
            dumpIkOracle com f0 f1 (max2 step 0.5) limbEE

            -- Optional re-export if Nel plugin is present (authoritative Max-side .anim).
            try (
              local rePath = outPath (clabel + "_reexport.anim")
              NelExportAnimation #(com) rePath false
              mlog ("  REEXPORT\tOK\t" + rePath)
            ) catch ( mlog "  REEXPORT\tSKIP (NelExportAnimation unavailable)" )
          )
        )
      )
    )
  )
)

-- ============================================================================
-- PART B - synthetic in-plant foot rotation dataset
-- ============================================================================
if RUN_PART_B do (
  partHeader "PART B - synthetic in-plant foot rotation (COM motion x ankleTension)"

  -- (a) COM translates forward (body walks over planted foot)
  footRotCase "b_frot_trans_tU" "plant L foot; COM translate -1.2 Y; tension unset" comMotionTranslate undefined
  footRotCase "b_frot_trans_t0" "plant L foot; COM translate -1.2 Y; ankleTension=0"   comMotionTranslate 0.0
  footRotCase "b_frot_trans_t1" "plant L foot; COM translate -1.2 Y; ankleTension=1"   comMotionTranslate 1.0

  -- (b) COM yaws in place (half-turn class)
  footRotCase "b_frot_yaw_tU" "plant L foot; COM yaw 0->180; tension unset" comMotionYaw undefined
  footRotCase "b_frot_yaw_t0" "plant L foot; COM yaw 0->180; ankleTension=0" comMotionYaw 0.0
  footRotCase "b_frot_yaw_t1" "plant L foot; COM yaw 0->180; ankleTension=1" comMotionYaw 1.0

  -- (c) COM pitches / lunges
  footRotCase "b_frot_pitch_tU" "plant L foot; COM pitch+translate lunge; tension unset" comMotionPitch undefined
  footRotCase "b_frot_pitch_t0" "plant L foot; COM pitch+translate lunge; ankleTension=0" comMotionPitch 0.0
  footRotCase "b_frot_pitch_t1" "plant L foot; COM pitch+translate lunge; ankleTension=1" comMotionPitch 1.0

  -- (d) combination: translate + yaw (cheer / demitour-plant hybrid)
  footRotCase "b_frot_combo_tU" "plant L foot; COM translate+yaw; tension unset" comMotionCombo undefined
  footRotCase "b_frot_combo_t0" "plant L foot; COM translate+yaw; ankleTension=0" comMotionCombo 0.0
  footRotCase "b_frot_combo_t1" "plant L foot; COM translate+yaw; ankleTension=1" comMotionCombo 1.0

  -- (e) BOTH feet planted, COM yaws (double-support plant)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local lf = findBone com "L Foot"
    local rf = findBone com "R Foot"
    local p0 = com.transform.pos
    local r0 = at time 0 com.transform.rotation
    local lfw = at time 0 (biped.getTransform lf #pos)
    local lfr = at time 0 (biped.getTransform lf #rotation)
    local rfw = at time 0 (biped.getTransform rf #pos)
    local rfr = at time 0 (biped.getTransform rf #rotation)
    keyAbs com "L Foot" 0 lfw lfr
    keyAbs com "L Foot" 30 lfw lfr
    keyAbs com "R Foot" 0 rfw rfr
    keyAbs com "R Foot" 30 rfw rfr
    if lf != undefined do plantKeysObjectSpace lf.controller
    if rf != undefined do plantKeysObjectSpace rf.controller
    keyCom com 0  p0 r0
    keyCom com 30 p0 ((quat 90 [0,0,1]) * r0)
    finalizeSynthetic com "b_frot_both_yaw90" "both feet Object-space planted; COM yaw 90; tension unset" 0 30 0.25 focusBones:(focusLeg com)
  )

  -- (f) planted foot with deliberate heel->ball roll (pivot handoff GT)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 20
    local foot = findBone com "L Foot"
    local fw = at time 0 (biped.getTransform foot #pos)
    local fr = at time 0 (biped.getTransform foot #rotation)
    keyAbs com "L Foot" 0 fw fr
    animate on ( at time 20 (
      biped.setTransform foot #rotation ((quat 35 [1,0,0]) * fr) true
      biped.setTransform foot #pos fw true
    ) )
    if foot != undefined do (
      plantKeysObjectSpace foot.controller
      dumpAnkleProps foot.controller 1 "b_frot_roll_k1"
      dumpAnkleProps foot.controller 2 "b_frot_roll_k2"
    )
    finalizeSynthetic com "b_frot_roll" "Object-space planted L foot heel->ball roll +35deg pitch" 0 20 0.25 focusBones:(focusLeg com)
  )

  -- (g) static plant, no COM motion (identity control)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 20
    plantFootBase com 20
    finalizeSynthetic com "b_frot_static" "planted L foot; COM unkeyed (static control)" 0 20 0.25 focusBones:(focusLeg com)
  )
)

-- ============================================================================
-- PART C - ikAnkleTension storage isolation
-- ============================================================================
if RUN_PART_C do (
  partHeader "PART C - ikAnkleTension storage isolation (for --diff-rig)"

  makeAnkleTwin "c_ankle_base" undefined "planted roll; NO ankleTension write (diff baseline)"
  makeAnkleTwin "c_ankle_t0"   0.0       "planted roll; ankleTension scripted 0.0"
  makeAnkleTwin "c_ankle_t05"  0.5       "planted roll; ankleTension scripted 0.5"
  makeAnkleTwin "c_ankle_t1"   1.0       "planted roll; ankleTension scripted 1.0"

  -- Also try tension on a FREE key (rules out plant-only storage).
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 20
    local foot = findBone com "L Foot"
    keyRot com "L Foot" 0 0 [1,0,0]
    keyRot com "L Foot" 20 20 [1,0,0]
    if foot != undefined do (
      setKeyIK foot.controller 1 #free
      setKeyIK foot.controller 2 #free
      setAnkleTensionExhaustive foot.controller 1 1.0
      setAnkleTensionExhaustive foot.controller 2 1.0
      dumpAnkleProps foot.controller 1 "c_ankle_free_t1_k1"
    )
    finalizeSynthetic com "c_ankle_free_t1" "FREE foot keys + ankleTension=1 (storage only if free keys hold it)" 0 20 1.0
  )

  mlog ""
  mlog "MANUAL_ANKLE"
  mlog "  If c_ankle_t0/t1 are byte-identical to c_ankle_base outside noise chunks"
  mlog "  (same failure as a_ik_ankle* in biped_anim_dataset), do this 2-minute UI pass:"
  mlog "    1. File > Open  C:\\biped_ik_residual_probe\\c_ankle_base.max"
  mlog "    2. Select L Foot, open Motion panel > Key Info / IK"
  mlog "    3. At frame 0 set Ankle Tension = 0, at frame 20 set Ankle Tension = 0"
  mlog "    4. Save As c_ankle_hand_t0.max  (same folder)"
  mlog "    5. Re-open c_ankle_base.max, set Ankle Tension = 1 on both plant keys"
  mlog "    6. Save As c_ankle_hand_t1.max"
  mlog "  Then on Linux:"
  mlog "    pipeline_max_export_anim --diff-rig c_ankle_base.max c_ankle_hand_t0.max out0.txt"
  mlog "    pipeline_max_export_anim --diff-rig c_ankle_base.max c_ankle_hand_t1.max out1.txt"
  mlog "    pipeline_max_export_anim --diff-rig c_ankle_hand_t0.max c_ankle_hand_t1.max out01.txt"
  mlog "  The third diff pins the storage field in one shot."
)

-- ============================================================================
-- PART D - COM turn V/H/T interpolation
-- ============================================================================
if RUN_PART_D do (
  partHeader "PART D - COM turn step interpolation (angle+residual rule)"

  turnCase "d_turn_30"  #(0, 30, 60, 90)                          undefined
  turnCase "d_turn_90"  #(0, 90, 180)                             undefined
  turnCase "d_turn_150" #(0, 150)                                 undefined
  turnCase "d_turn_210" #(0, 210)                                 undefined
  turnCase "d_turn_90_tens0"  #(0, 90, 180)                       0.0
  turnCase "d_turn_90_tens50" #(0, 90, 180)                       50.0
  -- Pathological large single step (demitour-class).
  turnCase "d_turn_162" #(0, 162)                                 undefined
  -- Mixed pitch+yaw (mort-class; forces residual-frame questions).
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local p0 = com.transform.pos
    local r0 = at time 0 com.transform.rotation
    keyCom com 0  p0 r0
    keyCom com 15 p0 ((quat 40 [1,0,0]) * (quat 90 [0,0,1]) * r0)
    keyCom com 30 p0 ((quat 0 [1,0,0]) * (quat 180 [0,0,1]) * r0)
    finalizeSynthetic com "d_turn_pitchyaw" "COM combined pitch+yaw (mort-class residual-frame probe)" 0 30 0.25
  )
)

-- ============================================================================
-- PART E - arm-pin / large-path synthetics
-- ============================================================================
if RUN_PART_E do (
  partHeader "PART E - arm-pin / large-path synthetics"

  -- E1: single hand pin, static COM (gesture-class wrist pin)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local hand = findBone com "R Hand"
    local hw = at time 0 (biped.getTransform hand #pos)
    local hr = at time 0 (biped.getTransform hand #rotation)
    keyAbs com "R Hand" 0  hw hr
    keyAbs com "R Hand" 15 hw hr
    keyAbs com "R Hand" 30 hw hr
    if hand != undefined do plantKeysObjectSpace hand.controller
    finalizeSynthetic com "e_arm_static_pin" "R Hand Object-space planted 0/15/30; COM static (gesture wrist-pin)" 0 30 0.25 focusBones:(focusArm com)
  )

  -- E2: single hand pin, COM translates (body moves under pinned hand)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local hand = findBone com "R Hand"
    local p0 = com.transform.pos
    local hw = at time 0 (biped.getTransform hand #pos)
    local hr = at time 0 (biped.getTransform hand #rotation)
    keyAbs com "R Hand" 0  hw hr
    keyAbs com "R Hand" 30 hw hr
    if hand != undefined do plantKeysObjectSpace hand.controller
    keyCom com 0  p0 undefined
    keyCom com 30 (p0 + [0,-0.5,0]) undefined
    finalizeSynthetic com "e_arm_pin_trans" "R Hand Object-space planted; COM translate -0.5 Y" 0 30 0.25 focusBones:(focusArm com)
  )

  -- E3: dual hand pin (weapon-hold shape), COM yaws
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local lh = findBone com "L Hand"
    local rh = findBone com "R Hand"
    local p0 = com.transform.pos
    local r0 = at time 0 com.transform.rotation
    local lhw = at time 0 (biped.getTransform lh #pos)
    local lhr = at time 0 (biped.getTransform lh #rotation)
    local rhw = at time 0 (biped.getTransform rh #pos)
    local rhr = at time 0 (biped.getTransform rh #rotation)
    keyAbs com "L Hand" 0  lhw lhr
    keyAbs com "L Hand" 30 lhw lhr
    keyAbs com "R Hand" 0  rhw rhr
    keyAbs com "R Hand" 30 rhw rhr
    if lh != undefined do plantKeysObjectSpace lh.controller
    if rh != undefined do plantKeysObjectSpace rh.controller
    keyCom com 0  p0 r0
    keyCom com 30 p0 ((quat 45 [0,0,1]) * r0)
    finalizeSynthetic com "e_arm_dual_yaw" "both hands Object-space planted; COM yaw 45 (weapon-hold LargePath-ish)" 0 30 0.25 focusBones:(focusArm com)
  )

  -- E4: multi-key palm plant with LARGE end-effector travel (LargePath gate shape)
  --     Hands re-keyed to a moving world target while planted (artist sliding a weapon).
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 40
    local rh = findBone com "R Hand"
    local p0 = com.transform.pos
    local rhw0 = at time 0 (biped.getTransform rh #pos)
    local rhr0 = at time 0 (biped.getTransform rh #rotation)
    -- Four planted keys; world pos travels ~0.9 m (storedD LargePath-class).
    keyAbs com "R Hand" 0  rhw0 rhr0
    keyAbs com "R Hand" 10 (rhw0 + [0.1, -0.25, 0.05]) rhr0
    keyAbs com "R Hand" 25 (rhw0 + [0.2, -0.55, 0.08]) ((quat 15 [1,0,0]) * rhr0)
    keyAbs com "R Hand" 40 (rhw0 + [0.15, -0.90, 0.02]) ((quat 25 [1,0,0]) * rhr0)
    if rh != undefined do plantKeysObjectSpace rh.controller
    keyCom com 0  p0 undefined
    keyCom com 40 (p0 + [0,-0.2,0]) undefined
    finalizeSynthetic com "e_arm_largepath" "R Hand 4-key Object-space planted; ~0.9m EE travel (LargePath dual-fold GT)" 0 40 0.25 focusBones:(focusArm com)
  )

  -- E5: free->plant->free arm blend ramps (transition class)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 30
    local rh = findBone com "R Hand"
    local p0 = com.transform.pos
    local rhw = at time 0 (biped.getTransform rh #pos)
    local rhr = at time 0 (biped.getTransform rh #rotation)
    keyAbs com "R Hand" 0  undefined rhr
    keyAbs com "R Hand" 10 rhw rhr
    keyAbs com "R Hand" 20 rhw rhr
    keyAbs com "R Hand" 30 undefined rhr
    if rh != undefined do (
      setKeyIK rh.controller 1 #free
      setKeyIK rh.controller 2 #planted
      setKeyObjectSpace rh.controller 2
      setKeyIK rh.controller 3 #planted
      setKeyObjectSpace rh.controller 3
      setKeyIK rh.controller 4 #free
    )
    keyCom com 0  p0 undefined
    keyCom com 30 (p0 + [0,-0.3,0]) undefined
    finalizeSynthetic com "e_arm_blendtrans" "R Hand free->Object-plant->plant->free; COM translate (arm blend ramp)" 0 30 0.25 focusBones:(focusArm com)
  )

  -- E6: 2-knot static plant (should stay FK under section 10s-quat static-2-knot gate)
  (
    local com = buildFromParams (BP())
    animationRange = interval 0 20
    local rh = findBone com "R Hand"
    local p0 = com.transform.pos
    local rhw = at time 0 (biped.getTransform rh #pos)
    local rhr = at time 0 (biped.getTransform rh #rotation)
    keyAbs com "R Hand" 0  rhw rhr
    keyAbs com "R Hand" 20 rhw rhr
    if rh != undefined do plantKeysObjectSpace rh.controller
    keyCom com 0  p0 undefined
    keyCom com 20 (p0 + [0,-0.15,0]) undefined
    finalizeSynthetic com "e_arm_static2" "R Hand 2-knot Object-space plant + small COM move (static-2-knot FK-gate control)" 0 20 0.25 focusBones:(focusArm com)
  )
)

-- ============================================================================
mlog ""
mlog "DONE gen_biped_ik_residual_probe"
mlog ("  manifest: " + OUT_DIR + "manifest.txt")
mlog "  next (Linux):"
mlog "    1. copy C:\\biped_ik_residual_probe -> ~/biped_ik_residual_probe/"
mlog "    2. --diff-rig c_ankle_base.max c_ankle_t1.max  (and hand-authored twins if needed)"
mlog "    3. pipeline_max_export_anim --dump-samples on residual .max vs SAMPLE lines"
mlog "    4. cmp live SAMPLE mid-plant vs our eval and vs ~/pipeline_export .anim"
mlog "BYE"

try ( close gMan ) catch ()
gMan = undefined
