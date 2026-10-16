-- ============================================================================
-- gen_decors_probe.ms — evaluated-mesh probe for TR_hall_reu_vitrine_decors
--
-- The one remaining ig-corpus deviation (pipeline_max_design.md §10g): the
-- reference links this instance to clusters two stories below the geometry our
-- Edit Mesh + Mirror evaluation produces. This dumps the node's EVALUATED mesh
-- (full modifier stack) in object space plus its node TM, so the mirror-plane
-- semantics can be read off directly. Also dumps the mesh with the Mirror
-- modifier disabled, and the Mirror gizmo transform.
--
-- EDIT the paths, run in Max 9 with the tryker construction db available,
-- copy manifest back to ~/decors_probe/.
-- ============================================================================

global SRC_FILE = "R:\\graphics\\stuff\\tryker\\decors\\constructions\\hall_vitrine_reunion\\tr_hall_vitrine_reunion.max"  -- EDIT ME
global OUT_DIR = "C:\\decors_probe\\"   -- EDIT ME
makeDir OUT_DIR all:true

global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

fn dumpMeshOS name node = (
	local snap = snapshot node
	snap.transform = matrix3 1  -- object space
	convertToMesh snap
	local m = snap.mesh
	mlog ("MESH\t" + name + "\tverts\t" + (m.numverts as string) + "\tfaces\t" + (m.numfaces as string))
	for i = 1 to m.numverts do (
		local v = getVert m i
		mlog ("  V\t" + (i as string) + "\t" + (v.x as string) + "\t" + (v.y as string) + "\t" + (v.z as string))
	)
	for i = 1 to m.numfaces do (
		local f = getFace m i
		mlog ("  F\t" + (i as string) + "\t" + ((f.x as integer) as string) + "\t" + ((f.y as integer) as string) + "\t" + ((f.z as integer) as string))
	)
	delete snap
)

loadMaxFile SRC_FILE quiet:true
n = getNodeByName "TR_hall_reu_vitrine_decors"
if n == undefined then (
	mlog "ERROR node not found"
) else (
	mlog ("NODETM\t" + (n.transform as string))
	mlog ("OBJECTTM\t" + (n.objectTransform as string))
	-- evaluated mesh, full stack
	dumpMeshOS "decors_full" n
	-- gizmo/context info for each modifier
	for i = 1 to n.modifiers.count do (
		local md = n.modifiers[i]
		mlog ("MOD\t" + (i as string) + "\t" + (classof md as string))
		try ( mlog ("  PROPS\t" + (getPropNames md as string)) ) catch ()
		if classof md == mirror then (
			try ( mlog ("  MIRROR axis\t" + (md.mirror_axis as string) + "\toffset\t" + (md.mirror_offset as string) + "\tcopy\t" + (md.copy as string)) ) catch ()
			try ( mlog ("  MIRROR center\t" + (md.center as string)) ) catch ()
			try ( mlog ("  MIRROR gizmo tm\t" + (getModContextTM n md as string)) ) catch ()
			try ( mlog ("  MIRROR gizmo bbox\t" + (getModContextBBoxMin n md as string) + " " + (getModContextBBoxMax n md as string)) ) catch ()
		)
	)
	-- disable the mirror, dump again
	for i = 1 to n.modifiers.count do (
		if classof n.modifiers[i] == mirror do n.modifiers[i].enabled = false
	)
	dumpMeshOS "decors_nomirror" n
)
mlog "BYE"
