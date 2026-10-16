-- ============================================================================
-- gen_prim_mesh_dataset.ms — parametric primitive mesh topology ground truth
-- Target: the 3ds Max version used for the pipeline exports.
--
-- Dumps the exact TriObject mesh (vertices, faces with 1-based indices, face
-- material IDs, smoothing groups, edge visibility) of Box and Plane primitives
-- across parameter variations, to manifest.txt. This pins the BuildMesh
-- vertex/face generation order that the ig exporter's cluster volumes and
-- portal poly stitching depend on (pipeline_max_design.md §10g: the corpus
-- accelerators are Box/Plane primitives; the reference cluster planes and
-- portal polys are derived from these meshes' face order and winding).
--
-- EDIT OUT_DIR, run, copy the folder back to ~/prim_mesh_dataset/.
-- ============================================================================

global OUT_DIR = "C:\\prim_mesh_dataset\\"   -- EDIT ME
makeDir OUT_DIR all:true

global gMan
gMan = createFile (OUT_DIR + "manifest.txt")
fn mlog s = ( if gMan != undefined do ( format "%\n" s to:gMan; flush gMan ); format "%\n" s )

fn dumpMesh name node = (
	local snap = snapshot node
	convertToMesh snap
	local m = snap.mesh
	mlog ("MESH\t" + name + "\tverts\t" + (m.numverts as string) + "\tfaces\t" + (m.numfaces as string))
	for i = 1 to m.numverts do (
		local v = getVert m i
		mlog ("  V\t" + (i as string) + "\t" + (v.x as string) + "\t" + (v.y as string) + "\t" + (v.z as string))
	)
	for i = 1 to m.numfaces do (
		local f = getFace m i
		local mid = getFaceMatID m i
		local sg = getFaceSmoothGroup m i
		local e1 = getEdgeVis m i 1
		local e2 = getEdgeVis m i 2
		local e3 = getEdgeVis m i 3
		mlog ("  F\t" + (i as string) + "\t" + ((f.x as integer) as string) + "\t" + ((f.y as integer) as string) + "\t" + ((f.z as integer) as string) + "\tmid\t" + (mid as string) + "\tsg\t" + (sg as string) + "\tev\t" + (e1 as string) + "," + (e2 as string) + "," + (e3 as string))
	)
	delete snap
)

resetMAXFile #noprompt

-- Boxes: the two corpus accelerator shapes + segment variations
b = box length:2.0 width:2.2 height:3.0 lengthsegs:1 widthsegs:1 heightsegs:1
dumpMesh "box_l2_w2.2_h3_s111" b
delete b
b = box length:0.4 width:2.0 height:2.8 lengthsegs:1 widthsegs:1 heightsegs:1
dumpMesh "box_l0.4_w2_h2.8_s111" b
delete b
b = box length:2.0 width:3.0 height:1.0 lengthsegs:2 widthsegs:3 heightsegs:2
dumpMesh "box_l2_w3_h1_s232" b
delete b
b = box length:1.0 width:1.0 height:-1.0 lengthsegs:1 widthsegs:1 heightsegs:1
dumpMesh "box_negheight" b
delete b
b = box length:-0.4 width:2.0 height:2.8 lengthsegs:1 widthsegs:1 heightsegs:1
dumpMesh "box_neglength" b
delete b
b = box length:0.4 width:-2.0 height:2.8 lengthsegs:1 widthsegs:1 heightsegs:1
dumpMesh "box_negwidth" b
delete b

-- Cylinders: corpus cluster shape (cluster_salle_npc class) + variations
c = cylinder radius:7.8 height:7.34 heightsegs:1 capsegs:1 sides:16
dumpMesh "cyl_r7.8_h7.34_s16" c
delete c
c = cylinder radius:2.0 height:3.0 heightsegs:2 capsegs:1 sides:8
dumpMesh "cyl_r2_h3_hs2_s8" c
delete c

-- Sphere: nel_flare delegate
s = sphere radius:1.5 segs:16
dumpMesh "sphere_r1.5_s16" s
delete s

-- Mirror modifier over a box (the TR_hall_reu_vitrine_decors class): default gizmo,
-- z axis, copy on — dumps the EVALUATED mesh so the mirror-plane semantics
-- (gizmo default = mod-context center?) are pinned against the modifier storage
b = box length:2.0 width:3.0 height:1.5 lengthsegs:1 widthsegs:1 heightsegs:1
addModifier b (mirror ())
b.modifiers[#Mirror].mirror_axis = 2
b.modifiers[#Mirror].copy = true
dumpMesh "box_mirror_z_copy_default" b
b.modifiers[#Mirror].mirror_offset = 2.0
dumpMesh "box_mirror_z_copy_offset2" b
delete b

-- Planes: corpus portal shapes + segment variation
p = plane length:3.5 width:2.3 lengthsegs:1 widthsegs:1
dumpMesh "plane_l3.5_w2.3_s11" p
delete p
p = plane length:2.0 width:4.0 lengthsegs:2 widthsegs:3
dumpMesh "plane_l2_w4_s23" p
delete p
p = plane length:4.0 width:4.0 lengthsegs:4 widthsegs:4
dumpMesh "plane_l4_w4_s44" p
delete p

mlog "BYE"
