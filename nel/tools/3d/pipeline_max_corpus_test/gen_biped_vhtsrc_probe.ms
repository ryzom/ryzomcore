-- ============================================================================
-- gen_biped_vhtsrc_probe.ms — which chunk does the real exporter read for a
-- biped's COM height when the vertical channel has zero keys?
-- (pipeline_max_design.md §10n)
--
-- BACKGROUND
--   For rigs whose whole COM-vertical keytrack is empty (no keys at all, not
--   even one), our exporter falls back to the rig's structural/template COM
--   height (chunk 0x006c[4..6], "figure height"). A corpus scan of the real
--   assets found two OTHER chunks (0x0104, 0x0260) that often hold a
--   DIFFERENT value — the biped's current/live COM position as of last save —
--   and checking against shipped .anim references shows BOTH sources win in
--   different files:
--     ship_tank_karavan_mort_idle, fy_hof_first_decoupe_end/loop -> current position wins
--     fy_hom_first_recruteur_*, fy_hom_first_meca_end/init       -> figure height wins
--   recruteur's disagreement is huge (current position is ~half the figure
--   height) and looks like a stale editing artifact rather than a real pose;
--   decoupe/ship_tank's disagreement looks like a real authored pose (bending
--   to cut, a collapsed creature). No stored bit distinguishes the two cases
--   from the raw bytes alone (checked: node parenting, reserved chunk fields,
--   gender, file timestamps — see design doc). This script gets it straight
--   from Max: dump the real files' live state, and reproduce each candidate
--   cause (ordinary move, figure-mode move, structural height edit, timeline
--   left scrubbed, uniform scale) on a fresh biped to see which one leaves the
--   006c-vs-0104/0260 fingerprint we're chasing, and re-export everything so
--   we get an authoritative, not-reverse-engineered, answer for what actually
--   ends up in the .anim.
--
--   LEADING HYPOTHESIS (from the Character Studio MAXScript/SDK reference,
--   not a guess): <biped_ctrl>.dynamicsType — 0 = "Biped Dynamics" (Character
--   Studio KEEPS LIVE-COMPUTING airborne trajectories and balance, via
--   per-key Balance Factor/Dynamics Blend, even for frames with no key), 1 =
--   "Spline Dynamics" (plain spline interpolation over whatever keys exist,
--   no live physics). If this differs between the FIG-WINS files (recruteur/
--   meca/couture/coup_1stperson) and the BASE-WINS files (decoupe/ship_tank),
--   that's the discriminator — dumpDynamicsState logs it explicitly for every
--   real file in Part A, and B6/B7 A-B test it directly on a synthetic case.
--
-- Target: 3ds Max 9 (character studio / Biped), matching the corpus era.
-- EDIT the paths below if R:\graphics is mounted elsewhere.
-- ============================================================================

global OUT_DIR = "C:\\biped_vhtsrc_probe\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true
global GFX_ROOT = "R:\\graphics\\"           -- EDIT ME: asset root

frameRate = 30

-- -------------------------------------------------------------------- manifest
global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

-- -------------------------------------------------------------- generic dumpers
fn dumpAllProps label obj = (
  if obj == undefined then ( mlog ("  PROPS\t" + label + "\tUNDEFINED") )
  else (
    try (
      for pn in (getPropNames obj) do (
        local pv = "?"
        try ( pv = ((getProperty obj pn) as string) ) catch ()
        mlog ("  PROP\t" + label + "\t" + (pn as string) + "\t" + pv)
      )
    ) catch ( mlog ("  PROPS\t" + label + "\tgetPropNames-failed") )
  )
)

-- Try every plausible spelling for the biped's structural height; logs whatever
-- resolves so we don't have to guess the exact API name up front.
fn dumpHeightCandidates label com ctrl = (
  local tries = #(
    #(com, "com.height"), #(ctrl, "ctrl.height"),
    #(com, "com.bipHeight"), #(ctrl, "ctrl.bipHeight")
  )
  for t in tries do (
    if t[1] != undefined do (
      try ( mlog ("  HEIGHTCAND\t" + label + "\t" + t[2] + "\t" + ((getProperty t[1] (t[2] as name)) as string)) ) catch ()
    )
  )
)

-- CONFIRMED via the Character Studio SDK/MAXScript reference (not a guess): a
-- biped's <biped_ctrl>.dynamicsType (0 = "Biped Dynamics" — Character Studio
-- KEEPS CALCULATING airborne trajectories and balance live, via Balance
-- Factor/Dynamics Blend, even off-key; 1 = "Spline Dynamics" — plain spline
-- interpolation over whatever keys exist, no live physics) is the single
-- strongest candidate discriminator for the whole 0x006c-vs-0x0104/0x0260
-- question: if it differs between the FIG-WINS files (recruteur/meca/
-- couture/coup_1stperson) and the BASE-WINS files (decoupe/ship_tank), that's
-- the answer. gravAccel is dumped alongside for completeness (its documented
-- default, 5.63855*height, ties gravity strength to structural height, which
-- may explain how a live dynamics solve could reference height at all).
fn dumpDynamicsState label ctrl = (
  try ( mlog ("  DYNAMICSTYPE\t" + label + "\t" + (ctrl.dynamicsType as string)) ) catch ( mlog ("  DYNAMICSTYPE\t" + label + "\tUNREADABLE") )
  try ( mlog ("  GRAVACCEL\t" + label + "\t" + (ctrl.gravAccel as string)) ) catch ( mlog ("  GRAVACCEL\t" + label + "\tUNREADABLE") )
)

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

fn numKeysSafe c = ( if c == undefined then -1 else ( local n = -1; try ( n = numKeys c ) catch (); n ) )

-- Full state dump for one biped: current scene time, figure-mode flag, key
-- counts on the three COM sub-tracks, the COM transform at whatever time the
-- file is currently sitting at AND explicitly at frame 0, plus a broad prop
-- dump on both the COM node and its controller (dumpAllProps below finds
-- whatever this build actually exposes, including anything named *height*/
-- *scale*/*dynamic* we didn't think to special-case).
fn dumpBipedState label com = (
  local cc = comControllers com
  local ctrl = cc[1]
  mlog ("FILE-STATE\t" + label)
  mlog ("  sliderTime\t" + (sliderTime as string))
  try ( mlog ("  figureMode\t" + (ctrl.figureMode as string)) ) catch ( mlog "  figureMode\tUNREADABLE" )
  mlog ("  numKeys.vertical\t" + ((numKeysSafe cc[2]) as string))
  mlog ("  numKeys.horizontal\t" + ((numKeysSafe cc[3]) as string))
  mlog ("  numKeys.turning\t" + ((numKeysSafe cc[4]) as string))
  local tmNow = com.transform
  mlog ("  comTM.now\tpos\t" + (tmNow.pos.x as string) + "," + (tmNow.pos.y as string) + "," + (tmNow.pos.z as string) \
    + "\trot\t" + (tmNow.rotation.x as string) + "," + (tmNow.rotation.y as string) + "," + (tmNow.rotation.z as string) + "," + (tmNow.rotation.w as string))
  at time 0 (
    local tm0 = com.transform
    mlog ("  comTM.t0\tpos\t" + (tm0.pos.x as string) + "," + (tm0.pos.y as string) + "," + (tm0.pos.z as string) \
      + "\trot\t" + (tm0.rotation.x as string) + "," + (tm0.rotation.y as string) + "," + (tm0.rotation.z as string) + "," + (tm0.rotation.w as string))
  )
  dumpHeightCandidates label com ctrl
  dumpDynamicsState label ctrl
  dumpAllProps (label + ".com") com
  dumpAllProps (label + ".ctrl") ctrl
)

-- Re-export through the real pipeline so we get an authoritative answer, not
-- a guess: if this file's shipped reference height is reproducible TODAY from
-- this exact .max, NelExportAnimation will write it out again right here.
-- Signature per pipeline_max_design.md / anim_export.ms convention.
fn tryReexport com label = (
  local outPath = OUT_DIR + label + "_reexport.anim"
  local ok = false
  try ( NelExportAnimation #(com) outPath false; ok = true ) catch ()
  if ok then mlog ("  REEXPORT\t" + label + "\tOK\t" + outPath)
  else mlog ("  REEXPORT\t" + label + "\tSKIP (NelExportAnimation unavailable or failed)")
  ok
)

fn findComNode = (
  local best = undefined
  for o in objects do (
    try ( if classof o.transform.controller == Vertical_Horizontal_Turn then best = o ) catch ()
  )
  if best == undefined do (
    -- fallback: name-match, in case the VHT classof check didn't resolve in this build
    for o in objects do ( if best == undefined and (matchPattern o.name pattern:"Bip*" ignoreCase:true) and o.parent == undefined do best = o )
  )
  best
)

-- If dynamicsType==0 really does keep a live balance/trajectory solve running,
-- the COM height on one of these "keyless vertical" files might not even be
-- perfectly flat across frames — a plain stored-default read would be. Sample
-- the COM z across the file's own animation range and log every value so a
-- flat line vs. a drifting/settling curve is visible directly in the numbers.
fn dumpComHeightSweep label com = (
  local rng = animationRange
  local f0 = rng.start
  local f1 = rng.end
  local steps = 8
  local i = 0
  while i <= steps do (
    local t = f0 + ((f1 - f0) * i / steps)
    at time t (
      mlog ("  SWEEP\t" + label + "\tt\t" + (t as string) + "\tz\t" + (com.transform.pos.z as string))
    )
    i += 1
  )
)

-- ============================================================================
-- PART A — probe the real corpus files, read-only (never re-saved)
-- ============================================================================
global REAL_FILES = #(
  -- disagreement cases, FIG-WINS (figure/template height was correct)
  #("recruteur_end",       GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_first_recruteur_end.max"),
  #("recruteur_init",      GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_first_recruteur_init.max"),
  #("meca_end",            GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_first_meca_end.max"),
  #("couture_end",         GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_first_couture_end.max"),
  #("coup_1stperson_a1md", GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_co_a1md_coup_1stperson.max"),
  -- disagreement cases, BASE-WINS (current/live position was correct)
  #("decoupe_end",         GFX_ROOT + "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_first_decoupe_end.max"),
  #("ship_tank_mort_idle", GFX_ROOT + "stuff\\caravan\\agents\\actors\\ship\\animation\\anims\\ship_tank_karavan_mort_idle.max"),
  #("ship_tank_mort",      GFX_ROOT + "stuff\\caravan\\agents\\actors\\ship\\animation\\anims\\ship_tank_karavan_mort.max"),
  -- agreeing controls (same characters, for direct A/B comparison)
  #("decoupe_init",        GFX_ROOT + "stuff\\fyros\\agents\\actors\\female\\animation\\anims\\fy_hof_first_decoupe_init.max"),
  #("ecriture_init",       GFX_ROOT + "stuff\\fyros\\agents\\actors\\male\\animation\\anims\\fy_hom_first_ecriture_init.max"),
  -- no shipped reference at all, but same zero-vertical-key condition, worth a look
  #("kitin_queen_idle_death", GFX_ROOT + "stuff\\tryker\\agents\\monsters\\kitin\\animation\\anims\\tr_mo_kitin_queen_idle_death.max"),
  #("kitin_queen_staticidle", GFX_ROOT + "stuff\\tryker\\agents\\monsters\\kitin\\animation\\anims\\tr_mo_kitin_queen_staticidle.max")
)

mlog ("BIPED VHTSRC PROBE  out=" + OUT_DIR + "  maxver=" + (maxVersion())[1] as string)
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
    local com = findComNode()
    if com == undefined then ( mlog "  ERROR no biped COM node found" )
    else (
      dumpBipedState label com
      dumpComHeightSweep label com
      tryReexport com label
    )
  )
)

-- ============================================================================
-- PART B — synthetic reproductions on a fresh biped: which manipulation
-- leaves the 0x006c-vs-0x0104/0x0260 disagreement fingerprint?
-- ============================================================================
mlog ""
mlog "=== PART B: synthetic reproductions ==="

fn buildBaseline = (
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

fn finalizeCase com label = (
  local ctrl = com.transform.controller
  try ( ctrl.figureMode = false ) catch ()
  dumpBipedState label com
  tryReexport com label
  saveMaxFile (OUT_DIR + label + ".max") quiet:true
)

-- B0: untouched baseline (expect figure height == current position, the
-- overwhelmingly common case).
(
  local com = buildBaseline()
  finalizeCase com "b0_baseline"
)

-- B1: ordinary move (NOT figure mode, NOT animate/autokey) — nudges wherever
-- the "current position" default sits without touching structure.
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  animate off ( move com [0, 0, -0.5] )
  finalizeCase com "b1_move_normal"
)

-- B2: figure-mode move — same nudge, but performed while figureMode is on,
-- which is Character Studio's dedicated "edit the rest pose" state.
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  try (
    ctrl.figureMode = true
    move com [0, 0, -0.5]
    ctrl.figureMode = false
  ) catch ( mlog "  ERROR B2 figureMode move failed" )
  finalizeCase com "b2_move_figuremode"
)

-- B3: structural height edit via whatever property this build exposes
-- (best-effort; dumpHeightCandidates in the baseline logs what's available so
-- we know which spelling actually worked, if any).
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  local ok = false
  try ( ctrl.figureMode = true; com.height = 1.2; ctrl.figureMode = false; ok = true ) catch ()
  if not ok do try ( ctrl.figureMode = true; ctrl.height = 1.2; ctrl.figureMode = false; ok = true ) catch ()
  mlog ("  B3 height-edit applied: " + (ok as string))
  finalizeCase com "b3_height_edit"
)

-- B4: timeline left scrubbed to a non-zero frame at save time, with an arm-
-- only key (COM itself stays keyless) — tests whether "wherever the artist
-- left the scrubber" alone perturbs the current-position snapshot even when
-- nothing that should affect COM height is keyed.
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  local nn = biped.maxNumNodes ctrl
  local arm = undefined
  for i = 1 to nn do ( local nd = biped.getNode ctrl i link:1; if nd != undefined and (matchPattern nd.name pattern:"*UpperArm*" ignoreCase:true) do arm = nd )
  if arm != undefined do (
    animate on (
      at time 0 ( biped.setTransform arm #rotation arm.transform.rotation true )
      at time 20 ( biped.setTransform arm #rotation ((quat 45 [1,0,0]) * arm.transform.rotation) true )
    )
  )
  sliderTime = 10f
  finalizeCase com "b4_scrubbed_midframe"
)

-- B5: uniform scale of the whole biped (distinct from a plain Z move — tests
-- whether recruteur's ~half-height anomaly looks like a scale operation
-- rather than a translate).
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  animate off ( scale com [0.5, 0.5, 0.5] )
  finalizeCase com "b5_scale_half"
)

-- B6/B7: the DIRECT test of the leading hypothesis (Character Studio SDK docs
-- confirm <biped_ctrl>.dynamicsType: 0 = "Biped Dynamics", Character Studio
-- keeps live-computing airborne trajectories/balance even off-key; 1 =
-- "Spline Dynamics", plain interpolation, no live physics). Same move-with-
-- zero-vertical-keys setup as B1, just with dynamicsType forced to each
-- value explicitly, so the ONLY variable between B6 and B7 is this flag. If
-- the re-exported height differs between them, this is the discriminator.
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  try ( ctrl.dynamicsType = 0 ) catch ( mlog "  ERROR B6 dynamicsType=0 unsettable" )
  animate off ( move com [0, 0, -0.5] )
  finalizeCase com "b6_move_dyntype0"
)
(
  local com = buildBaseline()
  local ctrl = com.transform.controller
  try ( ctrl.dynamicsType = 1 ) catch ( mlog "  ERROR B7 dynamicsType=1 unsettable" )
  animate off ( move com [0, 0, -0.5] )
  finalizeCase com "b7_move_dyntype1"
)

mlog ""
mlog "BYE"
