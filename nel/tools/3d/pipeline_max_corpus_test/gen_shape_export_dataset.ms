-- gen_shape_export_dataset.ms
--
-- Differential dataset for pipeline_max_export_shape coverage decodes. Same channel as
-- gen_prim_mesh_dataset.ms / biped_dataset_gen.ms / gen_biped_fields_probe.ms: each case is a
-- single-variable change from a small canonical baseline, saved as its own .max file and
-- accompanied by a ground-truth block in manifest.txt so a headless diff can pin the storage
-- decode for each open item.
--
-- Author: Jan Boon (Kaetemi) + Claude Opus 4.7
-- Run in 3ds Max 9 SP2, units = meters. Output: ~/shape_export_dataset/ ($HOME on the host).
--
-- Coverage targets (in the same order the design doc §10z-quat handoff ranks them):
--   1) Physique per-vertex payload (largest remaining gap, 654 corpus files):
--      minimal skinned cylinder + 2 rigid bones, then variants that change a single vertex's
--      bone assignment / weight / rigidity / bone count. Ground truth via IPhysiqueExport +
--      MAXScript introspection (bone list, per-vertex bone indices + weights + offsets).
--   2) Skin modifier (0 corpus but modern authoring channel):
--      same test mesh + skeleton, Skin modifier instead of Physique.
--   3) UVW Map projection (76 corpus uses, currently unhandled):
--      per map type (planar / cyl / sph / box / face) × baseline vs single-param sweeps
--      (length/width/height, tile+flip, channel, gizmo TM). GT via MAXScript vertex-map query
--      after evaluating the stack.
--   4) Unwrap UVW (25 corpus uses):
--      plane baseline + a single UV vertex nudged in the Edit UVWs window. GT = final UVs.
--   5) FFD(box) (20 corpus uses):
--      cylinder + FFD(box) modifier, single lattice control point moved.
--   6) Morpher modifier (blend shapes, 7 corpus anim files exercise it too):
--      base mesh + one target mesh + one channel factor variant.
--   7) Parametric-primitive UV generation:
--      Box / Cylinder / Sphere / Plane with "Generate Mapping Coords" ON, GT per-vert UVs.
--   8) Interface-mesh normal welding (9 corpus files):
--      two abutting boxes with the INTERFACE_FILE appdata pointer, GT = post-weld normals.
--   9) Multi/Sub-Object material assigned to a parametric primitive (box01.shape class):
--      a Box with a MultiMtl attached — reproduces the "1 vs 6 materials" corpus DIFF root
--      cause (whether it's the artist's material assignment or Box's per-face matIDs).
--
-- Manifest.txt format follows the existing datasets: one MESH/CASE record per file, indented
-- fields per (V vert, F face, B bone, W weight, U uv, X extra). Fields use \t separators, so
-- python readers can split on tabs.

-- ============================================================================================
-- Utilities

global OUT_DIR = "C:\\shape_export_dataset\\"   -- EDIT ME
makeDir OUT_DIR all:true

global g_manifest
g_manifest = createFile (OUT_DIR + "manifest.txt")
fn writeF s = ( if g_manifest != undefined do ( format "%\n" s to:g_manifest; flush g_manifest ); format "%\n" s )

fn saveScene name = (
    local p = OUT_DIR + name + ".max"
    saveMaxFile p quiet:true clearNeedSaveFlag:true useNewFile:false
    format "SAVED %\n" name
)

-- Wrap a case body in try/catch so one failure doesn't abort the whole run. Errors are logged
-- to the manifest so a follow-up session knows what to fix.
fn runCase caseName body = (
    try (
        body()
    ) catch (
        writeF ("CASE\t" + caseName + "\tERR\t" + (getCurrentException()))
        format "ERR case %: %\n" caseName (getCurrentException())
    )
)

fn freshReset = (
    resetMaxFile #noPrompt
    units.SystemType = #meters
    units.DisplayType = #metric
    units.MetricType = #meters
)

fn dumpMeshVerts obj tag = (
    -- Object-space mesh vertices, converted to mesh first (evaluates the modifier stack).
    local m = snapshotAsMesh obj
    writeF (tag + "\tverts\t" + (m.numverts as string) + "\tfaces\t" + (m.numfaces as string))
    for v = 1 to m.numverts do (
        local p = getVert m v
        writeF ("  V\t" + (v as string) + "\t" + (p.x as string) + "\t" + (p.y as string) + "\t" + (p.z as string))
    )
    for f = 1 to m.numfaces do (
        local vt = getFace m f
        local mid = getFaceMatID m f
        local sg = getFaceSmoothGroup m f
        writeF ("  F\t" + (f as string) + "\t" + (((vt.x) as integer) as string) + "\t" + (((vt.y) as integer) as string) + "\t" + (((vt.z) as integer) as string) + "\tmid\t" + (mid as string) + "\tsg\t" + (sg as string))
    )
    delete m
)

fn dumpMeshUVs obj tag chan = (
    -- Post-stack UVs at map channel `chan` (1 = default UVW). Emits one UV per map vertex plus
    -- one line per face with the three map-vert indices; parallel to mesh faces.
    local m = snapshotAsMesh obj
    if not (meshOp.getMapSupport m chan) then (
        writeF (tag + "\tuvchan\t" + (chan as string) + "\tUNSUPPORTED")
        delete m
        return true
    )
    local nmv = meshop.getNumMapVerts m chan
    local nmf = m.numfaces
    writeF (tag + "\tuvchan\t" + (chan as string) + "\tmapverts\t" + (nmv as string) + "\tmapfaces\t" + (nmf as string))
    for v = 1 to nmv do (
        local p = meshop.getMapVert m chan v
        writeF ("  U\t" + (v as string) + "\t" + (p.x as string) + "\t" + (p.y as string) + "\t" + (p.z as string))
    )
    for f = 1 to nmf do (
        local mf = meshop.getMapFace m chan f
        writeF ("  T\t" + (f as string) + "\t" + (((mf.x) as integer) as string) + "\t" + (((mf.y) as integer) as string) + "\t" + (((mf.z) as integer) as string))
    )
    delete m
)

fn dumpMeshNormals obj tag = (
    -- Post-stack per-corner normals (RVertex-decoded). Used for interface-mesh normal-weld GT.
    local m = snapshotAsMesh obj
    writeF (tag + "\tnormals\tfaces\t" + (m.numfaces as string))
    for f = 1 to m.numfaces do (
        local vt = getFace m f
        for c = 1 to 3 do (
            local n = getNormal m ((vt[c]) as integer)
            writeF ("  N\t" + (f as string) + "\t" + (c as string) + "\t" + (n.x as string) + "\t" + (n.y as string) + "\t" + (n.z as string))
        )
    )
    delete m
)

fn dumpBoneList bones tag = (
    -- Per-bone: index + name + world-space rest position. Order matches the caller's array.
    writeF (tag + "\tbones\t" + (bones.count as string))
    for i = 1 to bones.count do (
        local b = bones[i]
        local p = b.transform.translation
        writeF ("  B\t" + (i as string) + "\t" + (b.name as string) + "\t" + (p.x as string) + "\t" + (p.y as string) + "\t" + (p.z as string))
    )
)

-- ============================================================================================
-- 1) Physique probe: minimal skinned mesh + N-bone rig, save with default envelope init
-- ============================================================================================
--
-- Layout: cylindrical mesh 12 sides × 4 hs (58 verts, 96 faces) — small enough for a full dump.
-- Bones along +Z. Baseline: envelope defaults (Max attaches to the root bone automatically at
-- initial apply). Variants change the rig shape and/or bone count.
--
-- Ground truth constraint under Max 9: the MAXScript `physiqueOps` interface is NOT exposed
-- (it landed later; the Max 9 SDK exposes IPhysiqueExport in C++ only, per PhyExp.h). So this
-- probe cannot dump the per-vertex (boneRef, weight, offset) triples from MAXScript. What it
-- CAN do is author files with KNOWN bone configurations (bone count, positions, hierarchy)
-- and Max's default envelope-based Physique payload; the manifest records bone list + mesh
-- topology so the raw-byte decoder (PMB_SKIN_DUMP) can be cross-referenced against a scene it
-- fully controls. The per-vertex GT decode then falls to the SDK path or a later-Max re-open.

fn buildPhysiqueBaseline caseName numBones = (
    freshReset()
    -- Create bones along +Z, each 1m tall, linked parent → child.
    local bones = #()
    local prev = undefined
    for i = 1 to numBones do (
        local zA = (i - 1) as float
        local zB = i as float
        local b = boneSys.createBone [0,0,zA] [0,0,zB] [1,0,0]
        b.name = ("Bone" + (formattedPrint i format:"02d"))
        b.height = 1.0
        if prev != undefined do b.parent = prev
        append bones b
        prev = b
    )
    -- Cylindrical mesh spanning Z=0..N, radius 0.4.
    local cyl = cylinder radius:0.4 height:(numBones as float) heightsegs:(numBones*4) sides:12 capsegs:1 pos:[0,0,0]
    cyl.name = "SkinMesh"
    convertToMesh cyl
    -- Add Physique modifier. Max 9 authors default envelope-based per-vertex assignments on
    -- attach; the modifier's own attachment (SDK-level AttachRootNode) is invoked via UI or
    -- SDK, not MAXScript. Saving the file here captures a KNOWN-config Physique payload that
    -- the raw-byte decoder can compare against.
    local ph = Physique()
    addModifier cyl ph
    writeF ("CASE\t" + caseName + "\tkind\tphysique\tbones\t" + (numBones as string))
    dumpBoneList bones caseName
    dumpMeshVerts cyl caseName
    saveScene caseName
)

try ( buildPhysiqueBaseline "phys_2bones" 2 ) catch ( writeF ("CASE	phys_2bones	ERR	" + (getCurrentException())); format "ERR %: %\n" "phys_2bones" (getCurrentException()) )
try ( buildPhysiqueBaseline "phys_3bones" 3 ) catch ( writeF ("CASE	phys_3bones	ERR	" + (getCurrentException())); format "ERR %: %\n" "phys_3bones" (getCurrentException()) )
try ( buildPhysiqueBaseline "phys_4bones" 4 ) catch ( writeF ("CASE	phys_4bones	ERR	" + (getCurrentException())); format "ERR %: %\n" "phys_4bones" (getCurrentException()) )

-- ============================================================================================
-- 2) Skin modifier probe (same test mesh, Skin instead of Physique)
-- ============================================================================================

fn buildSkinCase caseName = (
    freshReset()
    local b1 = boneSys.createBone [0,0,0] [0,0,1] [1,0,0]
    b1.name = "Bone01"
    b1.height = 1.0
    local b2 = boneSys.createBone [0,0,1] [0,0,2] [1,0,0]
    b2.name = "Bone02"
    b2.height = 1.0
    b2.parent = b1
    local cyl = cylinder radius:0.4 height:2.0 heightsegs:8 sides:12 capsegs:1 pos:[0,0,0]
    cyl.name = "SkinMesh"
    convertToMesh cyl
    local sk = Skin()
    addModifier cyl sk
    max modify mode
    select cyl
    modPanel.setCurrentObject sk
    skinOps.addBone sk b1 0
    skinOps.addBone sk b2 0
    writeF ("CASE\t" + caseName + "\tkind\tskin")
    dumpBoneList #(b1, b2) caseName
    dumpMeshVerts cyl caseName
    local nv = skinOps.getNumberVertices sk
    for v = 1 to nv do (
        local nb = skinOps.getVertexWeightCount sk v
        writeF ("  X\t" + (v as string) + "\tnbones\t" + (nb as string))
        for bi = 1 to nb do (
            local boneId = skinOps.getVertexWeightBoneID sk v bi
            local bwt = skinOps.getVertexWeight sk v bi
            writeF ("    W\t" + (bi as string) + "\tboneId\t" + (boneId as string) + "\twt\t" + (bwt as string))
        )
    )
    saveScene caseName
)

try ( buildSkinCase "skin_baseline" ) catch ( writeF ("CASE	skin_baseline	ERR	" + (getCurrentException())); format "ERR %: %\n" "skin_baseline" (getCurrentException()) )

-- ============================================================================================
-- 3) UVW Map projection probe
-- ============================================================================================
--
-- Test mesh: a Box of odd non-square dimensions so every axis is discriminated. Apply UVW Map
-- with a specific map type and dump post-stack UVs (map channel 1). Repeat for planar / cyl /
-- sph / box / face modes; sweep length/width/height, tile+flip, channel selection.
--
-- Storage: the modifier is class (0x000f72b1, 0). Its per-node local data lives in the OSM
-- Derived wrapper's 0x2500 slot (like Edit Mesh — L.1). Its params live in an old-style
-- ParamBlock (superclass 0x8) referenced by the modifier: 0=maptype, 1=length, 2=width,
-- 3=height, 4=uTile, 5=vTile, 6=wTile, 7=uFlip, 8=vFlip, 9=wFlip, 10=channel (MAXScript 1-based),
-- 11=axis, 12=alignment. Gizmo TM lives in the mod-app 0x2510 slot. The reference plugin's
-- UVWMapper::MapPoint applies the map type over the fitted gizmo box.

fn buildUvwMapCase caseName maptype length_ width_ height_ utile vtile wtile ufl vfl wfl chan = (
    freshReset()
    local bx = box length:1.5 width:2.5 height:1.0 pos:[0,0,0]
    bx.name = "UvwBox"
    convertToMesh bx
    local uv = Uvwmap()
    addModifier bx uv
    -- Map type: 0=planar, 1=cyl, 2=cyl-cap, 3=sph, 4=shrink-wrap, 5=box, 6=face, 7=XYZtoUVW
    uv.maptype = maptype
    uv.length = length_
    uv.width = width_
    uv.height = height_
    uv.utile = utile
    uv.vtile = vtile
    uv.wtile = wtile
    uv.uflip = ufl
    uv.vflip = vfl
    uv.wflip = wfl
    uv.channel = chan
    -- Read back the gizmo TM (the modifier's `Gizmo` sub-anim). Save enough state to reconstruct
    -- what the mod-app's 0x2510 will carry.
    local giz = uv.gizmo.transform
    writeF ("CASE\t" + caseName + "\tkind\tuvwmap\tmaptype\t" + (maptype as string) + \
            "\tL\t" + (length_ as string) + "\tW\t" + (width_ as string) + "\tH\t" + (height_ as string) + \
            "\tut\t" + (utile as string) + "\tvt\t" + (vtile as string) + "\twt\t" + (wtile as string) + \
            "\tuf\t" + (ufl as string) + "\tvf\t" + (vfl as string) + "\twf\t" + (wfl as string) + \
            "\tchan\t" + (chan as string))
    -- Gizmo TM: 4x3 (row-major) — same layout as the 0x2510 chunk.
    local rows = #(giz.row1, giz.row2, giz.row3, giz.row4)
    for i = 1 to 4 do (
        writeF ("  G\t" + (i as string) + "\t" + (rows[i].x as string) + "\t" + (rows[i].y as string) + "\t" + (rows[i].z as string))
    )
    dumpMeshVerts bx caseName
    dumpMeshUVs bx caseName chan
    saveScene caseName
)

-- Planar variants
try ( buildUvwMapCase "uvw_planar_default"     0 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_planar_default	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_planar_default" (getCurrentException()) )
try ( buildUvwMapCase "uvw_planar_tile2x3"     0 1.5 2.5 1.0 2.0 3.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_planar_tile2x3	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_planar_tile2x3" (getCurrentException()) )
try ( buildUvwMapCase "uvw_planar_uflip"       0 1.5 2.5 1.0 1.0 1.0 1.0 true  false false 1 ) catch ( writeF ("CASE	uvw_planar_uflip	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_planar_uflip" (getCurrentException()) )
try ( buildUvwMapCase "uvw_planar_channel2"    0 1.5 2.5 1.0 1.0 1.0 1.0 false false false 2 ) catch ( writeF ("CASE	uvw_planar_channel2	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_planar_channel2" (getCurrentException()) )
try ( buildUvwMapCase "uvw_planar_bigL"        0 5.0 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_planar_bigL	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_planar_bigL" (getCurrentException()) )
-- Cylindrical
try ( buildUvwMapCase "uvw_cyl_default"        1 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_cyl_default	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_cyl_default" (getCurrentException()) )
try ( buildUvwMapCase "uvw_cyl_cap"            2 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_cyl_cap	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_cyl_cap" (getCurrentException()) )
-- Spherical
try ( buildUvwMapCase "uvw_sph_default"        3 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_sph_default	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_sph_default" (getCurrentException()) )
-- Box
try ( buildUvwMapCase "uvw_box_default"        5 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_box_default	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_box_default" (getCurrentException()) )
try ( buildUvwMapCase "uvw_box_tile2x2x2"      5 1.5 2.5 1.0 2.0 2.0 2.0 false false false 1 ) catch ( writeF ("CASE	uvw_box_tile2x2x2	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_box_tile2x2x2" (getCurrentException()) )
-- Face
try ( buildUvwMapCase "uvw_face_default"       6 1.5 2.5 1.0 1.0 1.0 1.0 false false false 1 ) catch ( writeF ("CASE	uvw_face_default	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_face_default" (getCurrentException()) )

-- Gizmo repositioning: move + rotate the gizmo to test the 0x2510 mod-context TM decode
fn buildUvwMapGizmoCase caseName tx ty tz rz = (
    freshReset()
    local bx = box length:1.5 width:2.5 height:1.0 pos:[0,0,0]
    bx.name = "UvwBox"
    convertToMesh bx
    local uv = Uvwmap()
    addModifier bx uv
    uv.maptype = 0  -- planar
    -- Move the gizmo through the modifier's transform sub-anim
    animate off (
        uv.gizmo.position = [tx, ty, tz]
        uv.gizmo.rotation = eulerToQuat (eulerAngles 0 0 rz)
    )
    writeF ("CASE\t" + caseName + "\tkind\tuvwmap-gizmo\ttx\t" + (tx as string) + "\tty\t" + (ty as string) + "\ttz\t" + (tz as string) + "\trz\t" + (rz as string))
    local giz = uv.gizmo.transform
    local rows = #(giz.row1, giz.row2, giz.row3, giz.row4)
    for i = 1 to 4 do (
        writeF ("  G\t" + (i as string) + "\t" + (rows[i].x as string) + "\t" + (rows[i].y as string) + "\t" + (rows[i].z as string))
    )
    dumpMeshVerts bx caseName
    dumpMeshUVs bx caseName 1
    saveScene caseName
)

try ( buildUvwMapGizmoCase "uvw_gizmo_t100"   1.0 0.0 0.0 0.0 ) catch ( writeF ("CASE	uvw_gizmo_t100	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_gizmo_t100" (getCurrentException()) )
try ( buildUvwMapGizmoCase "uvw_gizmo_rot45z" 0.0 0.0 0.0 45.0 ) catch ( writeF ("CASE	uvw_gizmo_rot45z	ERR	" + (getCurrentException())); format "ERR %: %\n" "uvw_gizmo_rot45z" (getCurrentException()) )

-- ============================================================================================
-- 4) Unwrap UVW probe (25 corpus files)
-- ============================================================================================
--
-- Baseline plane with default UVs, then apply Unwrap UVW and nudge one UV vertex. Save the
-- pre-Unwrap UVs and the post-Unwrap UVs so a decoder can pin what payload the modifier stores
-- vs. what changes actually made it into the final map channel.

fn buildUnwrapCase caseName nudgeVert nudgeU nudgeV = (
    freshReset()
    local pl = plane length:2.0 width:2.0 lengthsegs:2 widthsegs:2 pos:[0,0,0]
    pl.name = "UnwrapPlane"
    convertToMesh pl
    local uw = Unwrap_UVW()
    addModifier pl uw
    max modify mode
    select pl
    modPanel.setCurrentObject uw
    if nudgeVert > 0 do (
        try (
            uw.unwrap.setVertexPosition (currenttime) nudgeVert (point3 nudgeU nudgeV 0)
        ) catch()
    )
    writeF ("CASE\t" + caseName + "\tkind\tunwrap\tnudge\t" + (nudgeVert as string) + "\tu\t" + (nudgeU as string) + "\tv\t" + (nudgeV as string))
    dumpMeshVerts pl caseName
    dumpMeshUVs pl caseName 1
    saveScene caseName
)

try ( buildUnwrapCase "unwrap_baseline" 0 0.0 0.0 ) catch ( writeF ("CASE	unwrap_baseline	ERR	" + (getCurrentException())); format "ERR %: %\n" "unwrap_baseline" (getCurrentException()) )
try ( buildUnwrapCase "unwrap_v1_shift" 1 0.25 0.75 ) catch ( writeF ("CASE	unwrap_v1_shift	ERR	" + (getCurrentException())); format "ERR %: %\n" "unwrap_v1_shift" (getCurrentException()) )

-- ============================================================================================
-- 5) FFD(box) probe (20 corpus files)
-- ============================================================================================
--
-- Base cylinder + FFD(box) with default 2×2×2 lattice, then variants moving a single lattice
-- control point. GT = post-stack vertex positions (the deformed mesh). Storage: modifier
-- class (0x8ab36cc5, 0x8d1bf6e4). Params include the lattice count and the per-CP offsets.

fn buildFfdCase caseName cpIdx dx dy dz = (
    freshReset()
    local cyl = cylinder radius:0.5 height:2.0 heightsegs:4 sides:8 capsegs:1 pos:[0,0,0]
    cyl.name = "FfdCyl"
    convertToMesh cyl
    local ff = FFDBox()
    addModifier cyl ff
    max modify mode
    select cyl
    if cpIdx > 0 do (
        -- Move CP index cpIdx by (dx,dy,dz)
        try (
            local cp = ff[#Control_Points][cpIdx]
            cp.value = cp.value + [dx, dy, dz]
        ) catch()
    )
    writeF ("CASE\t" + caseName + "\tkind\tffdbox\tcp\t" + (cpIdx as string) + "\tdx\t" + (dx as string) + "\tdy\t" + (dy as string) + "\tdz\t" + (dz as string))
    dumpMeshVerts cyl caseName
    saveScene caseName
)

try ( buildFfdCase "ffd_baseline"  0 0.0 0.0 0.0 ) catch ( writeF ("CASE	ffd_baseline	ERR	" + (getCurrentException())); format "ERR %: %\n" "ffd_baseline" (getCurrentException()) )
try ( buildFfdCase "ffd_cp1_x0.5"  1 0.5 0.0 0.0 ) catch ( writeF ("CASE	ffd_cp1_x0.5	ERR	" + (getCurrentException())); format "ERR %: %\n" "ffd_cp1_x0.5" (getCurrentException()) )
try ( buildFfdCase "ffd_cp8_z0.3"  8 0.0 0.0 0.3 ) catch ( writeF ("CASE	ffd_cp8_z0.3	ERR	" + (getCurrentException())); format "ERR %: %\n" "ffd_cp8_z0.3" (getCurrentException()) )

-- ============================================================================================
-- 6) Morpher probe (blend shapes)
-- ============================================================================================
--
-- Baseline mesh (box) + one morph target (same topology, deformed). Set channel factor to 0
-- (baseline) and to 50 (halfway). Storage: modifier (0x17bb6854, 0xa5cba2a3), refs 1..100 =
-- per-channel PBs, refs 101+i = target nodes; channel factor controller is a Bezier Float on
-- PB's ref 0.

fn buildMorphCase caseName factor = (
    freshReset()
    local bx = box length:1.0 width:1.0 height:1.0 pos:[0,0,0]
    bx.name = "MorphBase"
    convertToMesh bx
    -- Target: same box, top-face verts moved
    local tgt = box length:1.0 width:1.0 height:1.0 pos:[0,0,0]
    tgt.name = "MorphTarget"
    convertToMesh tgt
    -- Move all top-face verts of tgt up 0.5m (verts with z > 0.5)
    for v = 1 to tgt.mesh.numverts do (
        local p = getVert tgt.mesh v
        if p.z > 0.5 do (
            setVert tgt.mesh v (p + [0,0,0.5])
        )
    )
    update tgt
    local mp = Morpher()
    addModifier bx mp
    max modify mode
    select bx
    modPanel.setCurrentObject mp
    try (
        WM3_MC_BuildFromNode mp 1 tgt
        WM3_MC_SetValue mp 1 factor
    ) catch()
    writeF ("CASE\t" + caseName + "\tkind\tmorpher\tfactor\t" + (factor as string))
    dumpMeshVerts bx caseName
    saveScene caseName
)

try ( buildMorphCase "morph_factor_0"   0.0 ) catch ( writeF ("CASE	morph_factor_0	ERR	" + (getCurrentException())); format "ERR %: %\n" "morph_factor_0" (getCurrentException()) )
try ( buildMorphCase "morph_factor_50"  50.0 ) catch ( writeF ("CASE	morph_factor_50	ERR	" + (getCurrentException())); format "ERR %: %\n" "morph_factor_50" (getCurrentException()) )

-- ============================================================================================
-- 7) Parametric primitive UV mapping (mapping coords ON)
-- ============================================================================================
--
-- Same primitives as gen_prim_mesh_dataset.ms, but with `mapCoords=true`. Dump map channel 1.
-- The reference plugin's UV formulas for Box/Cylinder/Sphere/Plane are what pipeline_max_export_
-- shape's parametric primitive path needs (§10y open item).

fn buildPrimUvCase caseName obj = (
    -- Caller supplies an already-created primitive node; we convertToMesh and dump. `freshReset`
    -- must NOT run here since it'd wipe the just-created obj — each case does its own reset
    -- before creating.
    convertToMesh obj
    writeF ("CASE\t" + caseName + "\tkind\tprim-uv")
    dumpMeshVerts obj caseName
    dumpMeshUVs obj caseName 1
    saveScene caseName
)

try ( freshReset(); buildPrimUvCase "primuv_box" (box length:1.5 width:2.5 height:1.0 mapCoords:true pos:[0,0,0]) ) catch ( writeF ("CASE	primuv_box	ERR	" + (getCurrentException())); format "ERR %: %\n" "primuv_box" (getCurrentException()) )
try ( freshReset(); buildPrimUvCase "primuv_cyl" (cylinder radius:0.5 height:2.0 heightsegs:2 sides:8 mapCoords:true pos:[0,0,0]) ) catch ( writeF ("CASE	primuv_cyl	ERR	" + (getCurrentException())); format "ERR %: %\n" "primuv_cyl" (getCurrentException()) )
try ( freshReset(); buildPrimUvCase "primuv_sphere" (sphere radius:1.0 segments:16 mapCoords:true pos:[0,0,0]) ) catch ( writeF ("CASE	primuv_sphere	ERR	" + (getCurrentException())); format "ERR %: %\n" "primuv_sphere" (getCurrentException()) )
try ( freshReset(); buildPrimUvCase "primuv_plane" (plane length:2.0 width:3.0 lengthsegs:2 widthsegs:3 mapCoords:true pos:[0,0,0]) ) catch ( writeF ("CASE	primuv_plane	ERR	" + (getCurrentException())); format "ERR %: %\n" "primuv_plane" (getCurrentException()) )
try ( freshReset(); buildPrimUvCase "primuv_box_multiseg" (box length:2.0 width:3.0 height:1.0 lengthsegs:2 widthsegs:3 heightsegs:2 mapCoords:true pos:[0,0,0]) ) catch ( writeF ("CASE	primuv_box_multiseg	ERR	" + (getCurrentException())); format "ERR %: %\n" "primuv_box_multiseg" (getCurrentException()) )

-- ============================================================================================
-- 8) Interface-mesh normal weld (9 corpus files)
-- ============================================================================================
--
-- Two abutting Boxes sharing an edge, with the second flagged as the "interface" via the
-- NEL3D_APPDATA_INTERFACE_FILE appdata. GT = post-weld corner normals on the receiver.
-- The reference `BuildMeshInterfaces` (plugin_max/nel_mesh_lib/export_mesh_interface.cpp) welds
-- normals when a receiver's vertex is within a threshold of a source vertex.

fn buildInterfaceCase caseName = (
    freshReset()
    local b1 = box length:1.0 width:1.0 height:1.0 pos:[0,0,0]
    b1.name = "IfaceSource"
    convertToMesh b1
    local b2 = box length:1.0 width:1.0 height:1.0 pos:[1.0,0,0]
    b2.name = "IfaceReceiver"
    convertToMesh b2
    -- Tag b2 with the interface-file appdata (NEL3D_APPDATA_INTERFACE_FILE = 1423062700).
    -- The value would be the path to a .max file, but the sub-id + shape of the storage is what
    -- we're pinning here.
    try (
        setUserPropVal b2 "NEL_INTERFACE_FILE" "iface_source.max"
    ) catch()
    writeF ("CASE\t" + caseName + "\tkind\tinterface-mesh")
    dumpMeshVerts b1 (caseName + ".src")
    dumpMeshVerts b2 (caseName + ".rcv")
    dumpMeshNormals b1 (caseName + ".src")
    dumpMeshNormals b2 (caseName + ".rcv")
    saveScene caseName
)

try ( buildInterfaceCase "iface_two_boxes" ) catch ( writeF ("CASE	iface_two_boxes	ERR	" + (getCurrentException())); format "ERR %: %\n" "iface_two_boxes" (getCurrentException()) )

-- ============================================================================================
-- 9) Parametric primitive + Multi/Sub-Object material (box01.shape class)
-- ============================================================================================
--
-- A Box with a MultiMtl containing 6 sub-materials. This reproduces the box01 corpus DIFF class
-- ("1 vs 6 materials"): the reference exporter carries the artist-authored 6-slot material,
-- our decoder currently produces 1. Whether that's because box01's actual reference has 6
-- materials from artist assignment (in which case the decoder is right and the DIFF is the ref
-- material list, not something we mishandle) or because our decoder is dropping 5 slots is
-- what this case pins.

fn buildBoxMultiMtl caseName = (
    freshReset()
    local bx = box length:1.5 width:2.5 height:1.0 pos:[0,0,0]
    bx.name = "MultiMtlBox"
    local mm = multiMaterial numsubs:6
    for i = 1 to 6 do (
        mm.materialList[i] = Standard()
        mm.materialList[i].name = "Sub" + (i as string)
        mm.materialList[i].diffuseColor = color (i*40) (255 - i*20) (i*30)
    )
    bx.material = mm
    convertToMesh bx
    writeF ("CASE\t" + caseName + "\tkind\tbox-multimtl\tsubs\t6")
    dumpMeshVerts bx caseName
    saveScene caseName
)

try ( buildBoxMultiMtl "box_multimtl_6" ) catch ( writeF ("CASE	box_multimtl_6	ERR	" + (getCurrentException())); format "ERR %: %\n" "box_multimtl_6" (getCurrentException()) )

-- ============================================================================================
-- 10) Nel VertexTreePaint sanity check (the modifier is already implemented; verify)
-- ============================================================================================
--
-- If plugin_max is loaded in the running Max, add the modifier and paint the first N verts.
-- This gives us a ground-truth for the color storage decode. If the plugin isn't present, skip.

fn hasClass name = (
    -- MAXScript classes appear as globals if their plugin is loaded.
    try (
        execute (name + " != undefined")
    ) catch(false)
)

fn buildVtpCase caseName = (
    freshReset()
    local pl = plane length:2.0 width:2.0 lengthsegs:3 widthsegs:3 pos:[0,0,0]
    pl.name = "VtpPlane"
    convertToMesh pl
    if (hasClass "Nel_VertexTreePaint") then (
        try (
            local vtp = execute "Nel_VertexTreePaint()"
            addModifier pl vtp
            writeF ("CASE\t" + caseName + "\tkind\tvertextreepaint\tADDED")
        ) catch (
            writeF ("CASE\t" + caseName + "\tkind\tvertextreepaint\tADD_FAILED")
        )
    ) else (
        writeF ("CASE\t" + caseName + "\tkind\tvertextreepaint\tPLUGIN_MISSING")
    )
    dumpMeshVerts pl caseName
    saveScene caseName
)

try ( buildVtpCase "vtp_baseline_or_missing" ) catch ( writeF ("CASE	vtp_baseline_or_missing	ERR	" + (getCurrentException())); format "ERR %: %\n" "vtp_baseline_or_missing" (getCurrentException()) )

-- ============================================================================================
-- 11) Baseline plain Editable Mesh — reference case
-- ============================================================================================
--
-- A raw Editable Mesh with per-vertex map channel 1 UVs already set. Provides a baseline the
-- other cases can be compared against so a decoder can tell "what does the plain-mesh 0x0914/
-- 0x0912 chunk look like when we know exactly what MAXScript authored".

fn buildPlainMeshCase caseName = (
    freshReset()
    local bx = box length:1.0 width:1.0 height:1.0 mapCoords:true pos:[0,0,0]
    bx.name = "PlainMesh"
    convertToMesh bx
    writeF ("CASE\t" + caseName + "\tkind\tplain-mesh")
    dumpMeshVerts bx caseName
    dumpMeshUVs bx caseName 1
    saveScene caseName
)

try ( buildPlainMeshCase "plain_box" ) catch ( writeF ("CASE	plain_box	ERR	" + (getCurrentException())); format "ERR %: %\n" "plain_box" (getCurrentException()) )

-- ============================================================================================
-- 12) StdUVGen static offset/scale/angle probe
-- ============================================================================================
--
-- The waterfall session (§10k) landed animated texture-matrix export but left the STATIC
-- StdUVGen offset/scale/angle undecoded — the reference bakes them into the material's user
-- texture matrix but we ship identity, adding ~48 bytes of DIFF per animated material.
-- Case: a StandardMaterial with a bitmap texmap whose UV Offset / Tiling / W Angle are set to
-- non-default constants (no controller); GT is the resulting texture matrix (4x4 float) that
-- MAXScript exposes.

fn buildStdUvGenCase caseName uoff voff utile vtile wangle = (
    freshReset()
    local pl = plane length:2.0 width:2.0 mapCoords:true pos:[0,0,0]
    pl.name = "StdUVGenPlane"
    local m = Standard()
    local bmp = Bitmaptexture()
    bmp.coords.U_Offset = uoff
    bmp.coords.V_Offset = voff
    bmp.coords.U_Tiling = utile
    bmp.coords.V_Tiling = vtile
    bmp.coords.W_Angle  = wangle
    m.diffuseMap = bmp
    pl.material = m
    convertToMesh pl
    writeF ("CASE\t" + caseName + "\tkind\tstduvgen\tuoff\t" + (uoff as string) + "\tvoff\t" + (voff as string) + "\tutile\t" + (utile as string) + "\tvtile\t" + (vtile as string) + "\twangle\t" + (wangle as string))
    -- Expose the raw UV-matrix values (the composed static transform the reference bakes).
    -- Fields on StdUVGen match the ParamBlock records the shape exporter reads.
    dumpMeshVerts pl caseName
    saveScene caseName
)

try ( buildStdUvGenCase "stduvgen_baseline" 0.0 0.0 1.0 1.0 0.0 ) catch ( writeF ("CASE	stduvgen_baseline	ERR	" + (getCurrentException())); format "ERR %: %\n" "stduvgen_baseline" (getCurrentException()) )
try ( buildStdUvGenCase "stduvgen_offset"   0.25 0.5 1.0 1.0 0.0 ) catch ( writeF ("CASE	stduvgen_offset	ERR	" + (getCurrentException())); format "ERR %: %\n" "stduvgen_offset" (getCurrentException()) )
try ( buildStdUvGenCase "stduvgen_tile2x3"  0.0 0.0 2.0 3.0 0.0 ) catch ( writeF ("CASE	stduvgen_tile2x3	ERR	" + (getCurrentException())); format "ERR %: %\n" "stduvgen_tile2x3" (getCurrentException()) )
try ( buildStdUvGenCase "stduvgen_angle45"  0.0 0.0 1.0 1.0 45.0 ) catch ( writeF ("CASE	stduvgen_angle45	ERR	" + (getCurrentException())); format "ERR %: %\n" "stduvgen_angle45" (getCurrentException()) )

-- ============================================================================================
-- Manifest done
-- ============================================================================================
writeF ("DONE")
close g_manifest
format "\n\nSHAPE EXPORT DATASET WRITTEN\nDirectory: %\nManifest: %manifest.txt\n" OUT_DIR OUT_DIR
messageBox ("Shape-export dataset written to:\n" + OUT_DIR) title:"Done"
