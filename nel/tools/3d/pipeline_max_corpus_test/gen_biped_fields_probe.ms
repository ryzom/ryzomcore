-- ============================================================================
-- gen_biped_fields_probe.ms — broad single-variable FIELD probe off a REAL
-- skeleton file, aimed at the still-UNDECODED biped (0x9155) chunks
-- (pipeline_max_design.md §10o coverage note: ~38% of chunk ids / 81% of byte
-- weight decoded; the remainder is dominated by 0x014d, the 0x025d/0x025e
-- shadow bank, the 0x01f4..0x01fd structural tables, 0x0110/0x0116/0x0118,
-- 0x0201/0x0202, 0x00c8..0x00ce, 0x015e, 0x0107/0x0108/0x0190).
--
-- COMPLEMENTS gen_biped_skel_animode_probe.ms — that script sweeps MODE
-- toggles (move / figure / dynamicsType / talentFigMode / lockCom / moveAll /
-- footsteps / mirror / clearAll / scale / height). This one sweeps the fields
-- that script does NOT touch:
--   A. Structural figure params (height, ankleAttach, link counts, fingers/
--      toes, triangle pelvis/neck, arms/foreFeet/shortThumb/knuckles, bodyType,
--      rootName) — expected to move 0x000b..0x0010, 0x0100, 0x01f4..0x01fd.
--   B. Per-limb FIGURE poses (upper arm / forearm / clavicle / thigh / calf /
--      foot / toe / spine / head / neck / pelvis / finger, rotated in Figure
--      Mode) — maps each limb's figure-pose storage slot.
--   C. Per-TRACK animation keys (COM pos, vertical / horizontal / turn only,
--      arm, leg, spine, head, tail, pony, finger, toe).
--   D. Key TCB + ease params (tension / continuity / bias / easeTo / easeFrom).
--   E. IK / dynamics per-key (planted / sliding / free key; gravAccel) —
--      candidates for 0x014d / 0x025d/e / 0x00c8..0x00ce.
--   F. Track structure (separate arms/legs/spine/neck/tail/pony; euler on a
--      track; twist bones) — candidates for 0x0110/0x0116/0x0118.
--   G. Adaptation locks; in-place mode.
--
-- Each case RELOADS the skeleton FRESH and commits Figure Mode -> Animation
-- Mode, then applies EXACTLY ONE conceptual change and saves. So
--   pipeline_max_export_anim --diff-rig f00_baseline.max fNN_<field>.max <out>
-- reports precisely the chunk(s) that field owns, with no cross-contamination.
-- Same safety discipline as the sibling script: fresh reload per case (a bad
-- figure/animation-mode transition cannot cascade), every risky call wrapped
-- in try/catch, requested + actual value logged to the manifest.
--
-- Target: 3ds Max 9 (Character Studio 4). EDIT the paths below if needed.
-- ============================================================================

global OUT_DIR = "C:\\biped_fields_probe\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true
global SKEL_FILE = "R:\\graphics\\stuff\\fyros\\agents\\actors\\male\\animation\\skeletons\\fy_hom_skel.max"  -- EDIT ME
-- A second full run with the female skeleton is worthwhile (most corpus
-- disagreement data is from fy_hof_first_* files):
--   R:\graphics\stuff\fyros\agents\actors\female\animation\skeletons\fy_hof_skel.max

global gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

-- ---------------------------------------------------------------- helpers
fn findComNode = (
  local best = undefined
  for o in objects do ( try ( if classof o.transform.controller == Vertical_Horizontal_Turn then best = o ) catch () )
  if best == undefined do ( for o in objects do ( if best == undefined and (matchPattern o.name pattern:"Bip*" ignoreCase:true) and o.parent == undefined do best = o ) )
  best
)
-- First scene node whose name matches a pattern (case-insensitive).
fn findByPat pat = (
  local r = undefined
  for o in objects do ( if r == undefined and (matchPattern o.name pattern:pat ignoreCase:true) do r = o )
  r
)
fn numKeysSafe c = ( local n = 0; try ( n = numKeys c ) catch (); n )

-- Load the skeleton fresh and commit Figure Mode -> Animation Mode. Returns
-- the COM node (or undefined, logged). Shared start point re-derived per case.
fn loadAndCommit = (
  resetMaxFile #noPrompt
  loadMaxFile SKEL_FILE quiet:true
  local com = findComNode()
  if com == undefined then ( mlog "  ERROR no biped COM node found after load"; undefined )
  else (
    try ( com.transform.controller.figureMode = false ) catch ( mlog "  ERROR figureMode=false threw" )
    com
  )
)

-- Compact, self-describing ground truth: the structural params + COM TM + key
-- counts, so the manifest records what each saved file actually contains (a
-- silently-ignored assignment is caught by comparing requested vs. logged).
-- Explicit try-per-line (MaxScript has no lambdas), matching the sibling script.
fn gt com label = (
  local ctrl = com.transform.controller
  mlog ("GT\t" + label)
  try ( mlog ("  figureMode\t"     + (ctrl.figureMode as string)) )     catch ( mlog "  figureMode\t?" )
  try ( mlog ("  height\t"         + (ctrl.height as string)) )         catch ( try ( mlog ("  height\t" + (com.height as string)) ) catch ( mlog "  height\t?" ) )
  try ( mlog ("  ankleAttach\t"    + (ctrl.ankleAttach as string)) )    catch ( mlog "  ankleAttach\t?" )
  try ( mlog ("  spineLinks\t"     + (ctrl.spineLinks as string)) )     catch ( mlog "  spineLinks\t?" )
  try ( mlog ("  neckLinks\t"      + (ctrl.neckLinks as string)) )      catch ( mlog "  neckLinks\t?" )
  try ( mlog ("  legLinks\t"       + (ctrl.legLinks as string)) )       catch ( mlog "  legLinks\t?" )
  try ( mlog ("  tailLinks\t"      + (ctrl.tailLinks as string)) )      catch ( mlog "  tailLinks\t?" )
  try ( mlog ("  fingerLinks\t"    + (ctrl.fingerLinks as string)) )    catch ( mlog "  fingerLinks\t?" )
  try ( mlog ("  toeLinks\t"       + (ctrl.toeLinks as string)) )       catch ( mlog "  toeLinks\t?" )
  try ( mlog ("  fingers\t"        + (ctrl.fingers as string)) )        catch ( mlog "  fingers\t?" )
  try ( mlog ("  toes\t"           + (ctrl.toes as string)) )           catch ( mlog "  toes\t?" )
  try ( mlog ("  ponytail\t"       + (ctrl.ponytail as string)) )       catch ( mlog "  ponytail\t?" )
  try ( mlog ("  trianglePelvis\t" + (ctrl.trianglePelvis as string)) ) catch ( mlog "  trianglePelvis\t?" )
  try ( mlog ("  triangleNeck\t"   + (ctrl.triangleNeck as string)) )   catch ( mlog "  triangleNeck\t?" )
  try ( mlog ("  arms\t"           + (ctrl.arms as string)) )           catch ( mlog "  arms\t?" )
  try ( mlog ("  foreFeet\t"       + (ctrl.foreFeet as string)) )       catch ( mlog "  foreFeet\t?" )
  try ( mlog ("  shortThumb\t"     + (ctrl.shortThumb as string)) )     catch ( mlog "  shortThumb\t?" )
  try ( mlog ("  knuckles\t"       + (ctrl.knuckles as string)) )       catch ( mlog "  knuckles\t?" )
  try ( mlog ("  bodyType\t"       + (ctrl.bodyType as string)) )       catch ( mlog "  bodyType\t?" )
  try ( mlog ("  rootName\t"       + (ctrl.rootName as string)) )       catch ( mlog "  rootName\t?" )
  try ( mlog ("  dynamicsType\t"   + (ctrl.dynamicsType as string)) )   catch ( mlog "  dynamicsType\t?" )
  try ( mlog ("  gravAccel\t"      + (ctrl.gravAccel as string)) )      catch ( mlog "  gravAccel\t?" )
  try ( mlog ("  sepArmsTracks\t"  + (ctrl.sepArmsTracks as string)) )  catch ( mlog "  sepArmsTracks\t?" )
  try ( mlog ("  sepLegsTracks\t"  + (ctrl.sepLegsTracks as string)) )  catch ( mlog "  sepLegsTracks\t?" )
  local vk = 0; local hk = 0; local tk = 0
  try ( vk = numKeysSafe ctrl.vertical.controller )   catch ()
  try ( hk = numKeysSafe ctrl.horizontal.controller ) catch ()
  try ( tk = numKeysSafe ctrl.turning.controller )    catch ()
  mlog ("  comKeys v/h/t\t" + (vk as string) + "/" + (hk as string) + "/" + (tk as string))
  local tm = com.transform
  mlog ("  comTM\tpos\t" + (tm.pos.x as string) + "," + (tm.pos.y as string) + "," + (tm.pos.z as string) \
    + "\trot\t" + (tm.rotation.x as string) + "," + (tm.rotation.y as string) + "," + (tm.rotation.z as string) + "," + (tm.rotation.w as string))
)

fn finalizeCase com label = (
  gt com label
  saveMaxFile (OUT_DIR + label + ".max") quiet:true
  mlog ("SAVED\t" + label + ".max\n")
)
fn beginCase label = ( mlog ("==== CASE " + label + " ===="); loadAndCommit() )

-- Set a controller property inside Figure Mode (structural params require it),
-- logging requested vs. actual read-back. Best-effort: prop may live on the
-- node or the controller depending on the build, so both are tried.
fn setFig com propName val = (
  local ctrl = com.transform.controller
  local okC = false; local okN = false
  try ( ctrl.figureMode = true ) catch ()
  try ( setProperty ctrl propName val; okC = true ) catch ()
  if not okC do ( try ( setProperty com propName val; okN = true ) catch () )
  local actual = "?"
  try ( actual = (getProperty ctrl propName) as string ) catch ( try ( actual = (getProperty com propName) as string ) catch () )
  try ( ctrl.figureMode = false ) catch ()
  mlog ("  setFig " + propName + " requested=" + (val as string) + " ctrl=" + (okC as string) + " node=" + (okN as string) + " actual=" + actual)
)
-- Set a controller property in Animation Mode (modes / tracks / dynamics).
fn setAnim com propName val = (
  local ctrl = com.transform.controller
  local ok = false
  try ( setProperty ctrl propName val; ok = true ) catch ()
  local actual = "?"
  try ( actual = (getProperty ctrl propName) as string ) catch ()
  mlog ("  setAnim " + propName + " requested=" + (val as string) + " ok=" + (ok as string) + " actual=" + actual)
)
-- Rotate a bone by name in Figure Mode (sets its FIGURE pose, not a key).
fn figPoseRotate com pat degZ = (
  local ctrl = com.transform.controller
  local nd = findByPat pat
  if nd == undefined then mlog ("  figPoseRotate MISS " + pat)
  else ( try ( ctrl.figureMode = true; in coordsys local rotate nd (angleaxis degZ [0,0,1]); ctrl.figureMode = false;
               mlog ("  figPoseRotate " + nd.name + " +" + (degZ as string) + "degZ") )
         catch ( mlog ("  figPoseRotate FAIL " + pat) ) )
)
-- Add a rotation animation key on a bone by name (biped-aware setTransform).
fn animKeyRot com pat = (
  local nd = findByPat pat
  if nd == undefined then mlog ("  animKeyRot MISS " + pat)
  else ( try ( animate on ( at time 0 ( biped.setTransform nd #rotation nd.transform.rotation true )
                            at time 20 ( biped.setTransform nd #rotation ((quat 25 [1,0,0]) * nd.transform.rotation) true ) )
               mlog ("  animKeyRot " + nd.name) )
         catch ( mlog ("  animKeyRot FAIL " + pat) ) )
)
-- Return a COM sub-channel controller (#vert / #horz / #turn), or undefined.
fn comSub com which = (
  local ctrl = com.transform.controller
  local sub = undefined
  case which of (
    #vert: ( try ( sub = ctrl.vertical.controller )   catch () )
    #horz: ( try ( sub = ctrl.horizontal.controller ) catch () )
    #turn: ( try ( sub = ctrl.turning.controller )    catch () )
  )
  sub
)
-- Add a key on ONE COM sub-channel (isolates 0x0130/1 vs 0x012c/d vs 0x012e/f).
fn comChannelKey com which = (
  local sub = comSub com which
  if sub == undefined then mlog ("  comChannelKey MISS " + (which as string))
  else ( try ( animate on ( at time 0 ( addNewKey sub 0 ); at time 20 ( addNewKey sub (20 * ticksPerFrame) ) )
               mlog ("  comChannelKey " + (which as string) + " keys=" + ((numKeysSafe sub) as string)) )
         catch ( mlog ("  comChannelKey FAIL " + (which as string)) ) )
)
-- Set a TCB / ease field on the first key of a COM sub-channel.
fn comKeyTCB com which field val = (
  local sub = comSub com which
  if sub == undefined then mlog ("  comKeyTCB MISS " + (which as string))
  else ( try ( animate on ( at time 0 ( addNewKey sub 0 ) ) ) catch ()
         local ok = false
         try ( setProperty sub.keys[1] field val; ok = true ) catch ()
         mlog ("  comKeyTCB " + (which as string) + "." + field + " requested=" + (val as string) + " ok=" + (ok as string)) )
)
-- Best-effort IK-state key on the left foot (planted / sliding / free).
fn ikKey com kind = (
  local foot = findByPat "*L Foot*"
  if foot == undefined then mlog "  ikKey MISS L Foot"
  else (
    local ctrl = com.transform.controller
    try ( animate on ( at time 0 ( biped.setTransform foot #pos foot.transform.pos true ) ) ) catch ()
    local ok = false
    case kind of (
      #planted: ( try ( biped.setPlantedKey ctrl 0f foot; ok = true ) catch ( try ( biped.setPlantedKey ctrl 0f; ok = true ) catch () ) )
      #sliding: ( try ( biped.setSlidingKey ctrl 0f foot; ok = true ) catch ( try ( biped.setSlidingKey ctrl 0f; ok = true ) catch () ) )
      #free:    ( try ( biped.setFreeKey    ctrl 0f foot; ok = true ) catch ( try ( biped.setFreeKey    ctrl 0f; ok = true ) catch () ) )
    )
    mlog ("  ikKey " + (kind as string) + " ok=" + (ok as string))
  )
)

mlog ("PROBE gen_biped_fields_probe  skel=" + SKEL_FILE)
( local c = loadAndCommit(); if c != undefined do try ( mlog ("  maxNumNodes=" + ((biped.maxNumNodes c.transform.controller) as string) + " maxNumLinks=" + ((biped.maxNumLinks c.transform.controller) as string)) ) catch () )

-- baseline (no change) — the reference for every --diff-rig
( local com = beginCase "f00_baseline"; if com != undefined do finalizeCase com "f00_baseline" )

-- --------------------------------------------------------- A. STRUCTURAL
( local com = beginCase "f01_height";         if com != undefined do ( local h = 1.0; try ( h = com.transform.controller.height ) catch ( try ( h = com.height ) catch () ); setFig com "height" (h * 0.9); finalizeCase com "f01_height" ) )
( local com = beginCase "f02_ankleAttach";    if com != undefined do ( setFig com "ankleAttach" 0.3;   finalizeCase com "f02_ankleAttach" ) )
( local com = beginCase "f03_spineLinks";     if com != undefined do ( setFig com "spineLinks" 5;      finalizeCase com "f03_spineLinks" ) )
( local com = beginCase "f04_neckLinks";      if com != undefined do ( setFig com "neckLinks" 2;       finalizeCase com "f04_neckLinks" ) )
( local com = beginCase "f05_legLinks4";      if com != undefined do ( setFig com "legLinks" 4;        finalizeCase com "f05_legLinks4" ) )
( local com = beginCase "f06_tailLinks";      if com != undefined do ( setFig com "tailLinks" 3;       finalizeCase com "f06_tailLinks" ) )
( local com = beginCase "f07_fingerLinks";    if com != undefined do ( setFig com "fingerLinks" 2;     finalizeCase com "f07_fingerLinks" ) )
( local com = beginCase "f08_toeLinks";       if com != undefined do ( setFig com "toeLinks" 2;        finalizeCase com "f08_toeLinks" ) )
( local com = beginCase "f09_fingers";        if com != undefined do ( setFig com "fingers" 4;         finalizeCase com "f09_fingers" ) )
( local com = beginCase "f10_toes";           if com != undefined do ( setFig com "toes" 2;            finalizeCase com "f10_toes" ) )
( local com = beginCase "f11_ponytail";       if com != undefined do ( setFig com "ponytail" 1;        finalizeCase com "f11_ponytail" ) )
( local com = beginCase "f12_trianglePelvis"; if com != undefined do ( setFig com "trianglePelvis" true; finalizeCase com "f12_trianglePelvis" ) )
( local com = beginCase "f13_triangleNeck";   if com != undefined do ( setFig com "triangleNeck" true; finalizeCase com "f13_triangleNeck" ) )
( local com = beginCase "f14_arms_off";       if com != undefined do ( setFig com "arms" false;        finalizeCase com "f14_arms_off" ) )
( local com = beginCase "f15_foreFeet";       if com != undefined do ( setFig com "foreFeet" true;     finalizeCase com "f15_foreFeet" ) )
( local com = beginCase "f16_shortThumb";     if com != undefined do ( setFig com "shortThumb" true;   finalizeCase com "f16_shortThumb" ) )
( local com = beginCase "f17_knuckles";       if com != undefined do ( setFig com "knuckles" true;     finalizeCase com "f17_knuckles" ) )
( local com = beginCase "f18_bodyType";       if com != undefined do ( setFig com "bodyType" 1;        finalizeCase com "f18_bodyType" ) )
( local com = beginCase "f19_rootName";       if com != undefined do ( setAnim com "rootName" "BipProbe"; finalizeCase com "f19_rootName" ) )

-- --------------------------------------------------------- B. FIGURE POSE
( local com = beginCase "f30_fig_upperarm"; if com != undefined do ( figPoseRotate com "*UpperArm*" 15; finalizeCase com "f30_fig_upperarm" ) )
( local com = beginCase "f31_fig_forearm";  if com != undefined do ( figPoseRotate com "*Forearm*" 15;  finalizeCase com "f31_fig_forearm" ) )
( local com = beginCase "f32_fig_clavicle"; if com != undefined do ( figPoseRotate com "*Clavicle*" 15; finalizeCase com "f32_fig_clavicle" ) )
( local com = beginCase "f33_fig_thigh";    if com != undefined do ( figPoseRotate com "*Thigh*" 15;    finalizeCase com "f33_fig_thigh" ) )
( local com = beginCase "f34_fig_calf";     if com != undefined do ( figPoseRotate com "*Calf*" 15;     finalizeCase com "f34_fig_calf" ) )
( local com = beginCase "f35_fig_foot";     if com != undefined do ( figPoseRotate com "*Foot*" 15;     finalizeCase com "f35_fig_foot" ) )
( local com = beginCase "f36_fig_toe";      if com != undefined do ( figPoseRotate com "*Toe0*" 15;     finalizeCase com "f36_fig_toe" ) )
( local com = beginCase "f37_fig_spine";    if com != undefined do ( figPoseRotate com "*Spine1*" 15;   finalizeCase com "f37_fig_spine" ) )
( local com = beginCase "f38_fig_head";     if com != undefined do ( figPoseRotate com "*Head*" 15;     finalizeCase com "f38_fig_head" ) )
( local com = beginCase "f39_fig_neck";     if com != undefined do ( figPoseRotate com "*Neck*" 15;     finalizeCase com "f39_fig_neck" ) )
( local com = beginCase "f40_fig_pelvis";   if com != undefined do ( figPoseRotate com "*Pelvis*" 15;   finalizeCase com "f40_fig_pelvis" ) )
( local com = beginCase "f41_fig_finger";   if com != undefined do ( figPoseRotate com "*Finger0*" 15;  finalizeCase com "f41_fig_finger" ) )

-- --------------------------------------------------------- C. PER-TRACK KEYS
( local com = beginCase "f50_key_com_pos"; if com != undefined do ( try ( animate on ( at time 0 ( biped.setTransform com #pos com.transform.pos true )
                                                                                       at time 20 ( biped.setTransform com #pos (com.transform.pos + [0,0,-0.2]) true ) ) ) catch (); finalizeCase com "f50_key_com_pos" ) )
( local com = beginCase "f51_key_vert";  if com != undefined do ( comChannelKey com #vert; finalizeCase com "f51_key_vert" ) )
( local com = beginCase "f52_key_horz";  if com != undefined do ( comChannelKey com #horz; finalizeCase com "f52_key_horz" ) )
( local com = beginCase "f53_key_turn";  if com != undefined do ( comChannelKey com #turn; finalizeCase com "f53_key_turn" ) )
( local com = beginCase "f54_key_arm";   if com != undefined do ( animKeyRot com "*UpperArm*"; finalizeCase com "f54_key_arm" ) )
( local com = beginCase "f55_key_leg";   if com != undefined do ( animKeyRot com "*Thigh*";    finalizeCase com "f55_key_leg" ) )
( local com = beginCase "f56_key_spine"; if com != undefined do ( animKeyRot com "*Spine1*";   finalizeCase com "f56_key_spine" ) )
( local com = beginCase "f57_key_head";  if com != undefined do ( animKeyRot com "*Head*";     finalizeCase com "f57_key_head" ) )
( local com = beginCase "f58_key_tail";  if com != undefined do ( animKeyRot com "*Tail*";     finalizeCase com "f58_key_tail" ) )
( local com = beginCase "f59_key_pony";  if com != undefined do ( animKeyRot com "*Ponytail*"; finalizeCase com "f59_key_pony" ) )
( local com = beginCase "f60_key_finger";if com != undefined do ( animKeyRot com "*Finger0*";  finalizeCase com "f60_key_finger" ) )
( local com = beginCase "f61_key_toe";   if com != undefined do ( animKeyRot com "*Toe0*";     finalizeCase com "f61_key_toe" ) )

-- --------------------------------------------------------- D. KEY TCB / EASE
( local com = beginCase "f70_tcb_tension";    if com != undefined do ( comKeyTCB com #vert "tension" 10.0;    finalizeCase com "f70_tcb_tension" ) )
( local com = beginCase "f71_tcb_continuity"; if com != undefined do ( comKeyTCB com #vert "continuity" 10.0; finalizeCase com "f71_tcb_continuity" ) )
( local com = beginCase "f72_tcb_bias";       if com != undefined do ( comKeyTCB com #vert "bias" 10.0;       finalizeCase com "f72_tcb_bias" ) )
( local com = beginCase "f73_tcb_easeTo";     if com != undefined do ( comKeyTCB com #vert "easeTo" 25.0;     finalizeCase com "f73_tcb_easeTo" ) )
( local com = beginCase "f74_tcb_easeFrom";   if com != undefined do ( comKeyTCB com #vert "easeFrom" 25.0;   finalizeCase com "f74_tcb_easeFrom" ) )

-- --------------------------------------------------------- E. IK / DYNAMICS
( local com = beginCase "f80_gravAccel";   if com != undefined do ( setAnim com "gravAccel" 500.0; finalizeCase com "f80_gravAccel" ) )
( local com = beginCase "f81_planted_key"; if com != undefined do ( ikKey com #planted; finalizeCase com "f81_planted_key" ) )
( local com = beginCase "f82_sliding_key"; if com != undefined do ( ikKey com #sliding; finalizeCase com "f82_sliding_key" ) )
( local com = beginCase "f83_free_key";    if com != undefined do ( ikKey com #free;    finalizeCase com "f83_free_key" ) )

-- --------------------------------------------------------- F. TRACK STRUCTURE
( local com = beginCase "f90_sep_arms";  if com != undefined do ( setAnim com "sepArmsTracks" true; finalizeCase com "f90_sep_arms" ) )
( local com = beginCase "f91_sep_legs";  if com != undefined do ( setAnim com "sepLegsTracks" true; finalizeCase com "f91_sep_legs" ) )
( local com = beginCase "f92_sep_spine"; if com != undefined do ( setAnim com "sepSpineTracks" true; finalizeCase com "f92_sep_spine" ) )
( local com = beginCase "f93_sep_neck";  if com != undefined do ( setAnim com "sepNeckTracks" true; finalizeCase com "f93_sep_neck" ) )
( local com = beginCase "f94_sep_tail";  if com != undefined do ( setAnim com "sepTailTracks" true; finalizeCase com "f94_sep_tail" ) )
( local com = beginCase "f95_sep_pony";  if com != undefined do ( setAnim com "sepPonytail" true;   finalizeCase com "f95_sep_pony" ) )
( local com = beginCase "f96_euler_spine"; if com != undefined do ( local ctrl = com.transform.controller; local ok = false; try ( biped.setEulerActive ctrl 8 0; ok = true ) catch (); mlog ("  euler spine(id8) ok=" + (ok as string)); finalizeCase com "f96_euler_spine" ) )
( local com = beginCase "f97_twist_on";    if com != undefined do ( local ctrl = com.transform.controller; local ok = false; try ( ctrl.figureMode = true; biped.setTwist ctrl true; ctrl.figureMode = false; ok = true ) catch (); mlog ("  twist on ok=" + (ok as string)); finalizeCase com "f97_twist_on" ) )

-- --------------------------------------------------------- G. ADAPT / INPLACE
( local com = beginCase "fA0_adaptLockHorz"; if com != undefined do ( setAnim com "adaptLockHorz" true; finalizeCase com "fA0_adaptLockHorz" ) )
( local com = beginCase "fA1_adaptLockVert"; if com != undefined do ( setAnim com "adaptLockVert" true; finalizeCase com "fA1_adaptLockVert" ) )
( local com = beginCase "fA2_adaptLockTurn"; if com != undefined do ( setAnim com "adaptLockTurn" true; finalizeCase com "fA2_adaptLockTurn" ) )
( local com = beginCase "fA3_inPlaceMode";   if com != undefined do ( setAnim com "inPlaceMode" true;   finalizeCase com "fA3_inPlaceMode" ) )

mlog "DONE gen_biped_fields_probe"
if gMan != undefined do close gMan
