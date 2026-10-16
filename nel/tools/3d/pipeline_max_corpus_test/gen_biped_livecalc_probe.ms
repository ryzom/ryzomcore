-- ============================================================================
-- gen_biped_livecalc_probe.ms — what actually feeds the live-computed COM
-- height on ship_tank_karavan_mort_idle.max? (pipeline_max_design.md §10n)
--
-- BACKGROUND: interactively confirmed (Kaetemi, Max 9) that this file's
-- Animation-Mode COM height (1.327, exactly the shipped reference) is a LIVE
-- computation, not a stored value — it matches neither chunk 0x006c (figure
-- height, 6.0) nor the current-position chunk family (0x0104 etc., 1.275),
-- and it survives an ordinary move + Figure Mode round-trip unchanged (moving
-- the biped and toggling modes still converges back to 1.327). That rules out
-- "reads a stored chunk" but does NOT tell us what the live computation
-- actually reads. A candidate story (Character Studio's "Biped Dynamics"
-- balance/airborne solve, using gravAccel + the file's one real keyed track,
-- Turning) was proposed but is UNVERIFIED — flagged directly by a sharp
-- follow-up ("a live computation based on WHAT?") that this script exists to
-- answer with actual controlled tests instead of another guess.
--
-- PART A: manipulate real candidate inputs on the ACTUAL file and measure
-- whether the live Animation-Mode value moves. Every case is a fresh reload
-- (same figure/animation-mode-instability caution as the other probes).
-- PART B: attempt to REPRODUCE the phenomenon itself (not the exact number)
-- on a fully-controlled synthetic rig — if we can make a fresh, understood
-- biped show the same "live value diverges from every stored chunk" behavior,
-- we can vary each input systematically instead of only having one real,
-- opaque example to stare at.
--
-- Target: 3ds Max 9. EDIT the paths below if needed.
-- ============================================================================

global OUT_DIR = "C:\\biped_livecalc_probe\\"   -- EDIT ME: output folder
makeDir OUT_DIR all:true
global SHIP_FILE = "R:\\graphics\\stuff\\caravan\\agents\\actors\\ship\\animation\\anims\\ship_tank_karavan_mort_idle.max"  -- EDIT ME

global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

fn findComNode = (
  local best = undefined
  for o in objects do ( try ( if classof o.transform.controller == Vertical_Horizontal_Turn then best = o ) catch () )
  if best == undefined do ( for o in objects do ( if best == undefined and (matchPattern o.name pattern:"Bip*" ignoreCase:true) and o.parent == undefined do best = o ) )
  best
)

fn dumpComTM label com = (
  local tm = com.transform
  mlog ("  comTM\t" + label + "\tpos\t" + (tm.pos.x as string) + "," + (tm.pos.y as string) + "," + (tm.pos.z as string) \
    + "\trot\t" + (tm.rotation.x as string) + "," + (tm.rotation.y as string) + "," + (tm.rotation.z as string) + "," + (tm.rotation.w as string))
)

-- ============================================================================
-- PART A — real file, one candidate input varied at a time
-- ============================================================================
fn loadShip = (
  resetMaxFile #noPrompt
  loadMaxFile SHIP_FILE quiet:true
  local com = findComNode()
  if com == undefined do mlog "  ERROR no biped COM node found"
  com
)

fn caseHeader label = ( mlog ""; mlog ("=== " + label + " ===") )

-- A0: untouched baseline — expect z ~1.327 (the mystery value, matching ref).
(
  caseHeader "a00_baseline"
  local com = loadShip()
  if com != undefined do (
    local ctrl = com.transform.controller
    try ( mlog ("  gravAccel\t" + (ctrl.gravAccel as string)) ) catch ()
    try ( mlog ("  dynamicsType\t" + (ctrl.dynamicsType as string)) ) catch ()
    try ( mlog ("  numKeys.turning\t" + ((numKeys ctrl.turning.controller) as string)) ) catch ()
    dumpComTM "asloaded" com
  )
)

-- A1: gravAccel forced to 0 — if the live value moves, gravity is an input.
(
  caseHeader "a01_gravaccel_zero"
  local com = loadShip()
  if com != undefined do (
    try ( com.transform.controller.gravAccel = 0.0 ) catch ( mlog "  ERROR gravAccel=0 failed" )
    dumpComTM "after_gravaccel_zero" com
  )
)

-- A2: gravAccel doubled — same test, opposite direction (rules out a
-- coincidental zero-crossing masking a real dependency).
(
  caseHeader "a02_gravaccel_double"
  local com = loadShip()
  if com != undefined do (
    local ctrl = com.transform.controller
    try ( ctrl.gravAccel = ctrl.gravAccel * 2.0 ) catch ( mlog "  ERROR gravAccel*2 failed" )
    dumpComTM "after_gravaccel_double" com
  )
)

-- A3: delete every Turning key — reverts the whole VHT controller to fully
-- keyless. If the live value then matches FIGURE height (6.0) or current-
-- position (1.275) instead of 1.327, Turning's keys were load-bearing for
-- the live computation. If it's STILL ~1.327, Turning isn't the input either.
(
  caseHeader "a03_turning_keys_deleted"
  local com = loadShip()
  if com != undefined do (
    local t = com.transform.controller.turning.controller
    local n = 0
    try ( n = numKeys t ) catch ()
    local deleted = 0
    for i = n to 1 by -1 do ( try ( deleteKey t i; deleted += 1 ) catch () )
    mlog ("  deleted " + (deleted as string) + " of " + (n as string) + " turning keys")
    dumpComTM "after_turning_keys_deleted" com
  )
)

-- A4: leave the Turning key(s) in place but change the rotation VALUE
-- substantially (does the settled height depend on what orientation the
-- fall ended at, or just on whether a key exists at all?).
(
  caseHeader "a04_turning_value_changed"
  local com = loadShip()
  if com != undefined do (
    local t = com.transform.controller.turning.controller
    local n = 0
    try ( n = numKeys t ) catch ()
    if n > 0 do (
      try (
        local kt = getKeyTime t 1
        animate on ( at time kt ( biped.setTransform com #rotation ((quat 90 [0,0,1]) * com.transform.rotation) true ) )
        mlog "  turning key rotation perturbed +90deg Z at its own key time"
      ) catch ( mlog "  ERROR turning value change failed" )
    )
    dumpComTM "after_turning_value_changed" com
  )
)

-- A5: dynamicsType forced to 1 (Spline Dynamics — no live physics per the
-- SDK docs). If the live 1.327 value depends on Biped Dynamics specifically,
-- this should make it match a stored chunk instead (figure height or current
-- position) rather than staying at 1.327.
(
  caseHeader "a05_dyntype1"
  local com = loadShip()
  if com != undefined do (
    try ( com.transform.controller.dynamicsType = 1 ) catch ( mlog "  ERROR dynamicsType=1 failed" )
    dumpComTM "after_dyntype1" com
  )
)

-- A6: ordinary move, NO figure-mode toggle — does the live value track an
-- immediate move even without a mode round-trip, or is it already immune?
(
  caseHeader "a06_move_no_figtoggle"
  local com = loadShip()
  if com != undefined do (
    animate off ( move com [0, 0, 2.0] )
    dumpComTM "after_move_no_figtoggle" com
  )
)

-- A7: ordinary move THEN a figure-mode round-trip — script reproduction of
-- Kaetemi's manual test (expect convergence back to ~1.327 regardless).
(
  caseHeader "a07_move_and_figtoggle"
  local com = loadShip()
  if com != undefined do (
    animate off ( move com [0, 0, 2.0] )
    local ctrl = com.transform.controller
    try ( ctrl.figureMode = true ) catch ()
    dumpComTM "infigmode_after_move" com
    try ( ctrl.figureMode = false ) catch ()
    dumpComTM "backtoanim_after_move" com
  )
)

-- A8: dump every scriptable property of the Turning key itself — the SDK
-- docs describe Balance Factor / Dynamics Blend as PER-KEY parameters; if
-- they're exposed at all, they should show up here by name.
(
  caseHeader "a08_turning_key_props"
  local com = loadShip()
  if com != undefined do (
    local t = com.transform.controller.turning.controller
    local n = 0
    try ( n = numKeys t ) catch ()
    mlog ("  turning numKeys: " + (n as string))
    for i = 1 to n do (
      try (
        local k = biped.getKey t i
        mlog ("  KEY " + (i as string) + " props:")
        for pn in (getPropNames k) do (
          local pv = "?"
          try ( pv = ((getProperty k pn) as string) ) catch ()
          mlog ("    " + (pn as string) + " = " + pv)
        )
      ) catch ( mlog "  ERROR reading key props" )
    )
  )
)

-- ============================================================================
-- PART B — attempt to reproduce the PHENOMENON on a controlled synthetic rig
-- ============================================================================
fn buildBaseline = (
  resetMaxFile #noPrompt
  local com = biped.createNew 1.8 -90.0 [0,0,0] \
      arms:true neckLinks:1 spineLinks:3 legLinks:3 tailLinks:0 \
      ponytail1Links:0 ponytail2Links:0 fingers:5 fingerLinks:3 toes:1 toeLinks:1 \
      ankleAttach:0.2 trianglePelvis:false triangleNeck:false \
      prop1Exists:false prop2Exists:false prop3Exists:false forearmTwistLinks:0
  com.transform.controller.rootName = "Bip01"
  animationRange = interval 0 40
  local ctrl = com.transform.controller
  try ( ctrl.figureMode = false ) catch ()
  com
)

fn finalizeCase com label = (
  dumpComTM label com
  saveMaxFile (OUT_DIR + label + ".max") quiet:true
)

-- B0: control — fresh biped, zero keys anywhere, ordinary move. Expect the
-- already-established result: figure height stays put, current-position
-- family tracks the move, no third mystery value (matches gen_biped_
-- vhtsrc_probe's b1). Included here as the baseline this whole part compares
-- against.
(
  caseHeader "b00_keyless_moved"
  local com = buildBaseline()
  animate off ( move com [0, 0, -0.3] )
  finalizeCase com "b00_keyless_moved"
)

-- B1: ONE static key on Turning (not an actual rotation change over time,
-- just a single keyframe existing), vertical/horizontal still keyless,
-- moved. Tests whether merely HAVING a turning key (regardless of motion)
-- changes the moved-position behavior at all.
(
  caseHeader "b01_turning_onekey_moved"
  local com = buildBaseline()
  local t = com.transform.controller.turning.controller
  animate on ( at time 0 ( biped.setTransform com #rotation com.transform.rotation true ) )
  animate off ( move com [0, 0, -0.3] )
  finalizeCase com "b01_turning_onekey_moved"
)

-- B2: a REAL turning rotation (two keys, an actual animated spin) plus
-- moved position — closer to ship_tank's actual shape (a real motion on
-- Turning, nothing on Vertical/Horizontal).
(
  caseHeader "b02_turning_animated_moved"
  local com = buildBaseline()
  animate on (
    at time 0 ( biped.setTransform com #rotation com.transform.rotation true )
    at time 20 ( biped.setTransform com #rotation ((quat 180 [0,0,1]) * com.transform.rotation) true )
  )
  animate off ( move com [0, 0, -0.3] )
  finalizeCase com "b02_turning_animated_moved"
)

-- B3: same as B2 but with dynamicsType explicitly 0 and a large gravAccel —
-- maximizes the chance of triggering any live dynamics path, if one exists.
(
  caseHeader "b03_turning_animated_moved_dyn0_grav"
  local com = buildBaseline()
  local ctrl = com.transform.controller
  try ( ctrl.dynamicsType = 0 ) catch ()
  try ( ctrl.gravAccel = ctrl.gravAccel * 5.0 ) catch ()
  animate on (
    at time 0 ( biped.setTransform com #rotation com.transform.rotation true )
    at time 20 ( biped.setTransform com #rotation ((quat 180 [0,0,1]) * com.transform.rotation) true )
  )
  animate off ( move com [0, 0, -0.3] )
  finalizeCase com "b03_turning_animated_moved_dyn0_grav"
)

mlog ""
mlog "BYE"
