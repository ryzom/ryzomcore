-- ============================================================================
-- gen_ig_selorder_probe.ms — pin the selection-ordering rule for §10w's
-- residual ligo village-bundle diffs (fy_module_village_nb_0x, zo_imm_*,
-- tr_village_*, jungle foret village_*, ilot_butte cases).
--
-- Every node in these files parents to the scene root, so pipeline_max_export_ig
-- 's buildTreeOrder degenerates to storage order and we don't reproduce the
-- reference's ordering. This probe dumps the FOUR relevant MaxScript iteration
-- orders for one representative file so we can identify what Max actually uses:
--
--   * $objects (all scene objects; the ligo maxscript's XRef pre-pass filters
--     this with `where classOf node == XRefObject`)
--   * $geometry
--   * $lights
--   * $helpers
--
-- Plus a simulation of the ligo maxscript's exact `select` / `selectmore`
-- sequence for one representative ig name so we can also dump
-- `$selection as array` directly — this is what the reference exporter
-- receives, and its iteration order is the ground truth.
--
-- EDIT the paths, run in Max 9 (or Max 2010 — either is fine; the probe just
-- reads scene iterator state, no version-sensitive semantics), copy the
-- resulting manifest.txt back to ~/ig_selorder_probe/.
-- ============================================================================

global SRC_FILE = "R:\\graphics\\landscape\\ligo\\desert\\max\\zonematerial-desert-nb01.max"  -- EDIT ME
global OUT_DIR = "C:\\ig_selorder_probe\\"   -- EDIT ME
global IG_NAME = "fy_module_village_nb_01"    -- one of the diffing igs in this file
makeDir OUT_DIR all:true

global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

-- --- getIg replica (same accessor `ig_export.ms` / `nel_ligo_export.ms` use;
--     appdata subid 1423062564 = NEL3D_APPDATA_IGNAME, appdata class ids from
--     the utility script).
global NEL3D_APPDATA_CLASSID_A = 0x04d64858
global NEL3D_APPDATA_CLASSID_B = 0x16d1751d
global NEL3D_APPDATA_SUPERCLASS = 4128
global NEL3D_APPDATA_IGNAME_SUBID = 1423062564

fn getIgOf node = (
    local raw = getAppData node NEL3D_APPDATA_IGNAME_SUBID
    if raw == undefined then "" else raw
)

fn dumpList label nodes = (
    mlog ("== " + label + " ==")
    local idx = 0
    for n in nodes do (
        local ig = getIgOf n
        local objSup = try ( superclassOf n.baseobject as string ) catch "?"
        local objCls = try ( classOf n.baseobject as string ) catch "?"
        local isXRef = classOf n == XRefObject
        mlog ("  " + (idx as string) + "\t" + n.name + "\tig=" + ig + "\tsClass=" + objSup + "\tclass=" + objCls + "\txref=" + (isXRef as string) + "\tpos=" + (n.pos as string))
        idx = idx + 1
    )
)

-- --- Load file, then dump the four iteration orders as MaxScript sees them.
loadMaxFile SRC_FILE quiet:true
mlog ("FILE\t" + SRC_FILE)
mlog ("IG_NAME\t" + IG_NAME)
mlog ("")

-- 1. $objects (order matters for the XRef pre-pass in ligo maxscript).
dumpList "objects" (for n in objects collect n)

-- 2. $geometry (order matters for the geometry pass).
dumpList "geometry" (for n in geometry collect n)

-- 3. $lights (order matters for the lights pass).
dumpList "lights" (for n in lights collect n)

-- 4. $helpers (order matters for the helpers pass).
dumpList "helpers" (for n in helpers collect n)

-- 5. Simulated ligo maxscript selection sequence for IG_NAME. This is the
--    exact code path the reference `exportInstanceGroupFromZone` runs — its
--    `$selection as array` is what actually gets serialized.
max select none
clearSelection()
for node in objects where classOf node == XRefObject do (
    if ((getIgOf node) == IG_NAME) then (
        local src = try ( node.GetSourceObject false ) catch undefined
        if src != undefined and (classOf src) != XRefObject then (
            local ssc = try ( superclassOf src ) catch undefined
            if ssc == GeometryClass or ssc == Helper or ssc == Light then (
                selectmore node
            )
        )
    )
)
for node in geometry do (
    if ((getIgOf node) == IG_NAME) then selectmore node
)
for node in lights do (
    if ((getIgOf node) == IG_NAME) then selectmore node
)
for node in helpers do (
    if ((getIgOf node) == IG_NAME) then selectmore node
)
dumpList "selection_sequence_for_IG_NAME" (selection as array)

-- 6. XRef resolvability classification: for each XRef node in the ig, dump whether
--    `GetSourceObject false` returns a resolved object (i.e. Max sees the source data
--    without needing an external file load). This is what the ligo maxscript XRef
--    pre-pass filters on. Also dump the XRef's stored source-file path so we can
--    correlate which target files are/aren't loaded/embedded.
fn dumpXRefResolvability = (
    mlog ""
    mlog "== xref_resolvability =="
    mlog "  (idx name ig sourceOK sourceClass sourceSClass fileName objName)"
    local xidx = 0
    for node in objects where classOf node == XRefObject do (
        local ig = getIgOf node
        if ig == IG_NAME do (
            local src = try ( node.GetSourceObject false ) catch undefined
            local ok = src != undefined
            local sc = if ok then ( try (classOf src as string) catch "?" ) else "-"
            local ssc = if ok then ( try (superclassOf src as string) catch "?" ) else "-"
            local fnm = try ( node.fileName ) catch "?"
            local onm = try ( node.objectName ) catch "?"
            mlog ("  " + (xidx as string) + "\t" + node.name + "\tig=" + ig + "\tok=" + (ok as string) + "\tclass=" + sc + "\tsClass=" + ssc + "\tfile=" + fnm + "\tobj=" + onm)
            xidx = xidx + 1
        )
    )
)
dumpXRefResolvability()

mlog "BYE"
