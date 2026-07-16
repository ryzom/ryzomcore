/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 */
// Cmb export: .max -> .cmb, replicating the NelExportCollision path of the 3ds Max plugin
// (build_gamedata processes/rbank, and the ligo process's own collision-from-zone-brick call)
// without 3ds Max.
//
// Two independent node-selection callers share ONE underlying operation
// (CExportNel::createCollisionMeshBuildList / createCollisionMeshBuild, nel_mesh_lib/
// export_collision.cpp):
//   - direct (rbank/maxscript/cmb_export.ms): every `geometry`-category node flagged
//     NEL3D_APPDATA_COLLISION or NEL3D_APPDATA_COLLISION_EXTERIOR ("1").
//   - --ligo (processes/ligo/maxscript/nel_ligo_export.ms's exportCollisionsFromZone): the same,
//     PLUS every XRefObject node whose resolved source object is geometry-class.
// Selection only gates candidacy; createCollisionMeshBuildList then groups candidates by their
// raw (NOT lowercased, unlike ig names) NEL3D_APPDATA_IGNAME value (default "unknown_ig" when
// absent) and calls createCollisionMeshBuild ONCE PER GROUP — a node whose ONLY flag is
// COLLISION_EXTERIOR (no COLLISION) is silently dropped at this point (selected by the
// maxscript, never grouped, never contributes geometry). One .cmb file is written per distinct
// ig-name group, named "<igname>.cmb" — exactly the ig/zone --ligo precedent
// (pipeline_max_design.md §10g-bis), not lowercased.
//
// Per group, createCollisionMeshBuild (replicated in buildCollisionMesh below):
//   - converts each COLLISION-flagged node's evaluated mesh to world space (object TM: the
//     node's world transform composed with its object-offset, matching every other tri-mesh
//     reader in this project),
//   - per face: Visibility[0..2] from the Mesh face's edge-visibility bits (REMAPPED: index 0
//     reads Max's EDGE_B bit, 1 reads EDGE_C, 2 reads EDGE_A — the reference's own field-order
//     choice, reproduced verbatim), Material = the face's Max material id, Surface = -1 when the
//     NODE's own COLLISION_EXTERIOR flag is set, else a running (totalSurfaces + faceMatId)
//     offset that increments by (this node's own max material id + 1) after EVERY processed
//     node (exterior nodes included, contributing +1 even though their faces all read -1) —
//     totalSurfaces resets to 0 per ig-name group,
//   - welds vertices ACROSS different nodes within the group (never within the same node) using
//     a real NL3D::CQuadGrid (grid size 64, cell width 1m) with a 5mm threshold — replicated
//     exactly, including the ascending-index-only / first-writer-wins semantics of the
//     reference's single welding pass,
//   - drops any face left degenerate (two remapped indices equal) after welding,
//   - runs the real NLPACS::CCollisionMeshBuild::link(false,...)/link(true,...) validation pass
//     (edge-consistency check); a group with link errors is skipped (matching the reference,
//     which returns NULL and drops just that group, not the whole export).
// Output: the real NLPACS::CCollisionMeshBuild, serialized with its own (plain binary, no XML)
// serial() — nel/pacs/collision_mesh_build.h is header-only, no extra linking beyond NeL::pacs
// for the enums/validation this tool also happens to want.

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/aabbox.h>
#include <nel/pacs/collision_mesh_build.h>
#include <nel/3d/quad_grid.h>

#include "../pipeline_max/storage_ole.h"
#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/geom_object.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"

#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/old_param_block.h"
#include "../pipeline_max_export_common/edit_mesh_mod.h"
#include "../pipeline_max_export_common/xref_resolve.h"
#include "../pipeline_max_export_common/parametric_mesh.h"
#include "../pipeline_max_export_common/db_path.h"


#include <algorithm>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace {

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h).
const uint32 NEL3D_APPDATA_COLLISION = 1423062613;
const uint32 NEL3D_APPDATA_COLLISION_EXTERIOR = 1423062614;
const uint32 NEL3D_APPDATA_IGNAME = 1423062564;

// Derived-object wrapper classes and modifier superclasses — the pair the object walk uses to
// unwrap the modifier stack down to the base object; shared with pipeline_max_export_ig.
using XREFRESOLVE::CLASSID_OSM_DERIVED;
using XREFRESOLVE::CLASSID_WSM_DERIVED;
using XREFRESOLVE::isXRefObject;
using XREFRESOLVE::resolveXRefObject;
const TSClassId SCLASS_OSMODIFIER = 0x00000810;
const TSClassId SCLASS_WSMODIFIER = 0x00000820;

const float WELD_THRESHOLD = 0.005f;
const sint GRID_SIZE = 64;
const float GRID_WIDTH = 1.0f;

// One raw (pre-weld, object-space) face carrying everything a collision face needs. The three
// edge-visibility bools are ALREADY index-remapped to Visibility[0..2] (Max's EDGE_A/B/C bits
// come off the face flags in a different order; see extractObjectMesh's edge decode).
struct SRawFace
{
	uint32 V[3];
	bool Vis0, Vis1, Vis2;
	uint32 MatId;
};

// Constructor for the Edit Mesh created-faces path (see EDITMESH::applyEdits): a created SFace's
// FaceFlags carries matID in the high 16 bits and edge-visibility in the low bits, exactly the
// packing extractObjectMesh reads for base-mesh faces — decoded the same way here so a mesh with
// added Edit-Mesh topology exports its added faces with the right material + edge bits.
struct SRawFaceFromCreated
{
	SRawFace operator()(uint32 vBase, const EDITMESH::SFace &cf) const
	{
		SRawFace f;
		f.V[0] = cf.V[0] + vBase; f.V[1] = cf.V[1] + vBase; f.V[2] = cf.V[2] + vBase;
		f.MatId = cf.FaceFlags >> 16;
		f.Vis0 = (cf.FaceFlags & 2) != 0; // EDGE_B → Visibility[0]
		f.Vis1 = (cf.FaceFlags & 4) != 0; // EDGE_C → Visibility[1]
		f.Vis2 = (cf.FaceFlags & 1) != 0; // EDGE_A → Visibility[2]
		return f;
	}
};

struct SModOp
{
	int Type; // 0 = Edit Mesh, 1 = Mirror
	EDITMESH::SEdits Edits;
	MAXMATH::Matrix3M GizmoTM;
	MAXMATH::Matrix3M CtxTM;
	sint MirrorAxis;
	float MirrorOffset;
	bool MirrorCopy;
};

// Mirror modifier (mods.dlm (0xef92aa7c, 0x511bbe75)), ported from pipeline_max_export_ig.
void applyMirror(const SModOp &op, std::vector<NLMISC::CVector> &verts, std::vector<SRawFace> &faces)
{
	MAXMATH::Matrix3M objToGizmo = op.CtxTM * MAXMATH::inverseM3(op.GizmoTM);
	MAXMATH::Matrix3M gizmoToObj = MAXMATH::inverseM3(objToGizmo);
	static const float FLIPS[6][3] = {
		{ -1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 }, { -1, -1, 1 }, { 1, -1, -1 }, { -1, 1, -1 }
	};
	const float *f = FLIPS[op.MirrorAxis >= 0 && op.MirrorAxis < 6 ? op.MirrorAxis : 0];
	uint32 nv = (uint32)verts.size(), nf = (uint32)faces.size();
	std::vector<NLMISC::CVector> mirrored(nv);
	#define M3_XFORM(M, ix, iy, iz, ox, oy, oz) \
		ox = (ix) * (M).m[0][0] + (iy) * (M).m[1][0] + (iz) * (M).m[2][0] + (M).m[3][0]; \
		oy = (ix) * (M).m[0][1] + (iy) * (M).m[1][1] + (iz) * (M).m[2][1] + (M).m[3][1]; \
		oz = (ix) * (M).m[0][2] + (iy) * (M).m[1][2] + (iz) * (M).m[2][2] + (M).m[3][2];
	for (uint32 i = 0; i < nv; ++i)
	{
		float gx, gy, gz, ox, oy, oz;
		M3_XFORM(objToGizmo, verts[i].x, verts[i].y, verts[i].z, gx, gy, gz)
		gx = gx * f[0] + (f[0] < 0 ? op.MirrorOffset : 0.0f);
		gy = gy * f[1] + (f[1] < 0 ? op.MirrorOffset : 0.0f);
		gz = gz * f[2] + (f[2] < 0 ? op.MirrorOffset : 0.0f);
		M3_XFORM(gizmoToObj, gx, gy, gz, ox, oy, oz)
		mirrored[i] = NLMISC::CVector(ox, oy, oz);
	}
	#undef M3_XFORM
	if (op.MirrorCopy)
	{
		verts.insert(verts.end(), mirrored.begin(), mirrored.end());
		for (uint32 i = 0; i < nf; ++i)
		{
			SRawFace t = faces[i];
			t.V[0] = faces[i].V[0] + nv; t.V[1] = faces[i].V[2] + nv; t.V[2] = faces[i].V[1] + nv;
			bool v0 = t.Vis0; t.Vis0 = t.Vis1; t.Vis1 = v0; // edge winding flip: (a,b,c)->(a,c,b)
			faces.push_back(t);
		}
	}
	else
	{
		verts.swap(mirrored);
		for (uint32 i = 0; i < nf; ++i)
		{
			std::swap(faces[i].V[1], faces[i].V[2]);
			bool v0 = faces[i].Vis0; faces[i].Vis0 = faces[i].Vis1; faces[i].Vis1 = v0;
		}
	}
}

// Apply an Edit Mesh modifier's decoded edit-record to an object-space mesh; delegates to the
// shared EDITMESH::applyEdits (moves → face deletes → vert deletes with face reindexing →
// created verts appended → created faces appended). Cmb passes `appendCreatedFaces=true` because
// its `.cmb` output needs the added topology with the right matID + edge-visibility bits — via
// SRawFaceFromCreated (the field remap matches extractObjectMesh's base-mesh decode). Design-doc
// §10w corrected the created-verts/created-faces chunk-ID mapping (0x0130 verts, 0x0208/0x0210
// faces — the earlier "0x0210 = verts" reading was backwards); shared decode picks up that fix.
// Apply the 0x0220 per-face attribute-change records to the input face set (matID + edge
// visibility rewrites — the mechanism §10w and the earlier §10v had missed). Runs BEFORE the
// shared applyEdits sees the faces because that path is templated on the face type and can't
// reach into cmb-specific matID / edge-vis fields; the shared decode exposes the records via
// SEdits::FaceAttribs. Corpus signal that pinned this: `fy_hall_reunion`'s reference `.cmb` has
// matID 60/59/58/57 across its faces where the raw base mesh has matID 0 uniformly — 0x0220's
// (Values >> 5) & 0xFFFF reproduces the reference's matID column exactly on this file.
void applyFaceAttribs(const EDITMESH::SEdits &e, std::vector<SRawFace> &faces)
{
	for (uint i = 0; i < e.FaceAttribs.size(); ++i)
	{
		const EDITMESH::SFaceAttribChange &fa = e.FaceAttribs[i];
		if (fa.Index >= faces.size()) continue;
		SRawFace &f = faces[fa.Index];
		if (fa.applyMatId()) f.MatId = fa.matId();
		// Edge visibility bits — the mesh face records edge_A/B/C directly (extractObjectMesh
		// remapped them to Visibility[0..2] = (EDGE_B, EDGE_C, EDGE_A), see the field decode);
		// re-apply an update in the SAME mapping so the roundtrip stays consistent.
		if (fa.applyEdge(0)) f.Vis2 = fa.edgeVis(0); // EDGE_A → Visibility[2]
		if (fa.applyEdge(1)) f.Vis0 = fa.edgeVis(1); // EDGE_B → Visibility[0]
		if (fa.applyEdge(2)) f.Vis1 = fa.edgeVis(2); // EDGE_C → Visibility[1]
		// applyHidden: cmb output has no hidden concept; the reference collision extraction
		// doesn't consult it either. Ignore.
	}
}

void applyEditMeshEdits(const EDITMESH::SEdits &e, std::vector<NLMISC::CVector> &verts, std::vector<SRawFace> &faces)
{
	// Apply per-face attribute changes to the input face set first (matID / edge-vis rewrites
	// per 0x0220), then the shared moves/deletes/created-verts/created-faces sequence.
	applyFaceAttribs(e, faces);
	// facesMode=1: append 0x0208 created faces (base-topology). 0x0210 records are decoded but
	// deliberately not applied — they're map-1 texture-face records, not base faces.
	EDITMESH::applyEdits(e, verts, faces, SRawFaceFromCreated(), 1);
}

// Extract one node's OBJECT-SPACE mesh, evaluating any Edit Mesh/Mirror modifier stack found on
// the way down (OSM/WSM Derived wrappers). Mirrors pipeline_max_export_ig's nodeWorldMesh chain
// walk, minus the final world-space transform (extractNodeMesh below handles that with the
// node's own TM, matching every other tri-mesh reader in this project).
bool extractObjectMesh(INode &node, CSceneClass *rawObj, std::vector<NLMISC::CVector> &verts,
	std::vector<SRawFace> &faces, std::string &err)
{
	std::vector<SModOp> opStack; // collected outermost-first
	CSceneClass *obj = rawObj;
	int guard = 16;
	while (obj && guard-- > 0)
	{
		// XRefObject anywhere in the walk resolves to the referenced scene's own base object
		// (nested XRefs handled by resolveXRefObject's recursion) — the EvalWorldState semantics
		// the reference exporter gets from Max. --ligo mode's collision nodes routinely land on
		// XRef sources; without this, ~6 of the 1201 brick files produced no output at all
		// (design-doc §10v). Shared with pipeline_max_export_ig via xref_resolve.h.
		if (isXRefObject(obj))
		{
			CSceneClass *resolved = resolveXRefObject(obj, 0);
			if (!resolved) { err = "unresolvable XRefObject"; return false; }
			obj = resolved;
			continue;
		}
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		CSceneClass *base = NULL;
		std::vector<CSceneClass *> mods;
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (!r) continue;
			TSClassId scid = r->classDesc()->superClassId();
			if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER) { mods.push_back(r); continue; }
			base = r;
		}
		std::vector<CStorageContainer *> modApps;
		{
			const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
			for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
				if (it->first == 0x2500)
					modApps.push_back(dynamic_cast<CStorageContainer *>(it->second));
		}
		for (uint m = 0; m < mods.size(); ++m)
		{
			NLMISC::CClassId mcid = mods[m]->classDesc()->classId();
			CStorageContainer *app = m < modApps.size() ? modApps[m] : NULL;
			if (mcid == NLMISC::CClassId(0x00000050, 0x00000000)) // Edit Mesh
			{
				SModOp op;
				op.Type = 0;
				if (app && EDITMESH::readModApp(app, op.Edits))
					opStack.push_back(op);
			}
			else if (mcid == NLMISC::CClassId(0xef92aa7c, 0x511bbe75)) // Mirror
			{
				SModOp op;
				op.Type = 1;
				op.GizmoTM = MAXMATH::Matrix3M::identity();
				op.CtxTM = MAXMATH::Matrix3M::identity();
				op.MirrorAxis = 0;
				op.MirrorOffset = 0.0f;
				op.MirrorCopy = false;
				CReferenceMaker *mrm = dynamic_cast<CReferenceMaker *>(mods[m]);
				for (uint r = 0; mrm && r < mrm->nbReferences(); ++r)
				{
					CSceneClass *ref = dynamic_cast<CSceneClass *>(mrm->getReference(r));
					if (!ref) continue;
					if (ref->classDesc()->superClassId() == 0x8)
					{
						std::map<sint32, OLDPBLOCK::SParam> params;
						OLDPBLOCK::readOldParamBlock(ref, params);
						op.MirrorAxis = (sint)OLDPBLOCK::paramInt(params, 0);
						op.MirrorCopy = OLDPBLOCK::paramInt(params, 1) != 0;
						op.MirrorOffset = OLDPBLOCK::paramFloat(params, 2);
					}
					else if (ref->classDesc()->classId() == NLMISC::CClassId(0x00002005, 0x00000000))
					{
						CReferenceMaker *prm = dynamic_cast<CReferenceMaker *>(ref);
						CSceneClass *pc = dynamic_cast<CSceneClass *>(prm->getReference(0));
						MAXMATH::Point3M gp = MAXSCENE::posValueAt0(pc);
						MAXMATH::QuatM gr = MAXSCENE::rotValueAt0(dynamic_cast<CSceneClass *>(prm->getReference(1)));
						MAXMATH::ScaleValueM gs = MAXSCENE::scaleValueAt0(dynamic_cast<CSceneClass *>(prm->getReference(2)));
						op.GizmoTM = MAXMATH::composePRS(gp, gr, gs);
					}
				}
				if (app)
				{
					for (CStorageContainer::TStorageObjectConstIt it = app->chunks().begin(); it != app->chunks().end(); ++it)
					{
						if (it->first != 0x2510) continue;
						CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
						if (raw && raw->Value.size() >= 48)
							memcpy(op.CtxTM.m, nlVectorData(raw->Value), 48);
					}
				}
				opStack.push_back(op);
			}
			else if (mcid != NLMISC::CClassId(0x000f72b1, 0x00000000)) // UVW Map: geometry-neutral
			{
				fprintf(stderr, "WARNING: node '%s' has unhandled modifier %s; geometry evaluated without it\n",
					ucstring(node.userName()).toUtf8().c_str(), mcid.toString().c_str());
			}
		}
		if (!base) break;
		obj = base;
	}
	if (!obj) { err = "no base object found"; return false; }

	CGeomObject *geom = dynamic_cast<CGeomObject *>(obj);
	if (!geom)
	{
		err = "base object is not a GeomObject (class " + obj->classDesc()->classId().toString()
		    + ", superclass " + NLMISC::toString(obj->classDesc()->superClassId()) + ")";
		return false;
	}
	NLMISC::CClassId baseCid = obj->classDesc()->classId();
	STORAGE::CGeomBuffers *gb = geom->geomBuffers();

	if (gb && gb->triVertices() && gb->triFaces())
	{
		// EditableMesh path: tri buffers (0x0914 vertices / 0x0912 faces). Vis bits + matID pack
		// into the face's flags/smoothingGroups word (matID at high 16 bits).
		verts = *gb->triVertices();
		const std::vector<STORAGE::CGeomTriIndexInfo> &rf = *gb->triFaces();
		faces.resize(rf.size());
		for (size_t i = 0; i < rf.size(); ++i)
		{
			const STORAGE::CGeomTriIndexInfo &f = rf[i];
			faces[i].V[0] = f.a; faces[i].V[1] = f.b; faces[i].V[2] = f.c;
			uint32 faceFlags = f.smoothingGroups;
			faces[i].MatId = faceFlags >> 16;
			faces[i].Vis0 = (faceFlags & 2) != 0; // EDGE_B
			faces[i].Vis1 = (faceFlags & 4) != 0; // EDGE_C
			faces[i].Vis2 = (faceFlags & 1) != 0; // EDGE_A
		}
	}
	else if (gb && gb->polyVertices() && gb->polyFaces())
	{
		// EditablePoly path: poly buffers (0x0100 verts / 0x011a faces) triangulated via
		// CGeomObject::triangulatePolyFace, same code path shape's mesh_eval uses. Poly
		// triangulation subdivides the polygon interior with all-edges-visible (the export path
		// reads the low 3 bits only for shading, so all-visible is fine).
		const std::vector<STORAGE::CGeomPolyVertexInfo> &pv = *gb->polyVertices();
		const std::vector<STORAGE::CGeomPolyFaceInfo> &pf = *gb->polyFaces();
		verts.resize(pv.size());
		for (size_t i = 0; i < pv.size(); ++i)
			verts[i] = pv[i].v;
		std::vector<STORAGE::CGeomTriIndex> tris;
		for (size_t i = 0; i < pf.size(); ++i)
		{
			const STORAGE::CGeomPolyFaceInfo &face = pf[i];
			if (face.Vertices.size() < 3) continue;
			tris.clear();
			CGeomObject::triangulatePolyFace(tris, face);
			for (size_t t = 0; t < tris.size(); ++t)
			{
				SRawFace ff;
				ff.V[0] = tris[t].a; ff.V[1] = tris[t].b; ff.V[2] = tris[t].c;
				ff.MatId = (uint32)face.Material;
				ff.Vis0 = ff.Vis1 = ff.Vis2 = true;
				faces.push_back(ff);
			}
		}
	}
	else if (baseCid == PRIMMESH::CLASSID_BOX || baseCid == PRIMMESH::CLASSID_CYLINDER
	         || baseCid == PRIMMESH::CLASSID_SPHERE || baseCid == PRIMMESH::CLASSID_PLANE)
	{
		// Parametric primitive path: locate the ref-0 old-style ParamBlock (superclass 0x8),
		// drive PRIMMESH::buildParametricMesh for ground-truth-exact topology (see design doc
		// §10g's primitive dataset round). All-edges-visible + matID 0 — matches the default
		// smGroup=1 primitive convention shape's extractParametricPrimitive uses. Base collision
		// nodes are frequently plain Boxes with an Edit Mesh modifier stack on top (§10g), so
		// the modifier evaluation pass below still gets to run.
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		CSceneClass *pblock = NULL;
		for (uint r = 0; rm && r < rm->nbReferences(); ++r)
		{
			CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
			if (ref && ref->classDesc()->superClassId() == 0x8)
			{
				pblock = ref;
				break;
			}
		}
		if (!pblock) { err = "parametric primitive without ref-0 pblock"; return false; }
		std::map<sint32, OLDPBLOCK::SParam> params;
		OLDPBLOCK::readOldParamBlock(pblock, params);

		std::vector<NLMISC::CVector> pverts;
		std::vector<PRIMMESH::SPrimTri> ptris;
		if (!PRIMMESH::buildParametricMesh(baseCid, params, pverts, ptris))
		{
			err = "parametric primitive build failed";
			return false;
		}
		verts.swap(pverts);
		faces.resize(ptris.size());
		for (size_t i = 0; i < ptris.size(); ++i)
		{
			faces[i].V[0] = ptris[i].V[0];
			faces[i].V[1] = ptris[i].V[1];
			faces[i].V[2] = ptris[i].V[2];
			faces[i].MatId = 0;
			faces[i].Vis0 = faces[i].Vis1 = faces[i].Vis2 = true;
		}
	}
	else
	{
		err = "unsupported base object class " + baseCid.toString();
		return false;
	}

	// Apply modifier ops base-upward (opStack was collected outermost-first, so replay back to
	// front — mirrors pipeline_max_export_ig's nodeWorldMesh). Created verts and faces are
	// applied via the shared EDITMESH::applyEdits; §10v's "created geometry missing" warning is
	// retired — §10w decoded 0x0130/0x0208 and §10z-ter corrected 0x0210 (face-vertex remap, not
	// created map-1 faces) so the output carries all three.
	const char *dbgOps = getenv("PMB_DEBUG_MESH");
	// Empty string ("") = debug every collision node; otherwise match by exact node user name.
	bool dbgThis = dbgOps && (dbgOps[0] == 0 || ucstring(node.userName()).toUtf8() == dbgOps);
	if (dbgThis)
		fprintf(stderr, "DEBUG node %s base=%s stack of %u ops (outermost first)\n",
			ucstring(node.userName()).toUtf8().c_str(), obj->classDesc()->classId().toString().c_str(),
			(uint)opStack.size());
	for (uint i = (uint)opStack.size(); i > 0; --i)
	{
		if (dbgThis)
		{
			const SModOp &op = opStack[i - 1];
			if (op.Type == 0)
				fprintf(stderr, "  op[%u] EditMesh: moves=%u remap=%u attribs=%u createdV=%u createdF=%u delV=%u delF=%u\n",
					i - 1, (uint)op.Edits.Moves.size(), (uint)op.Edits.FaceRemap.size(),
					(uint)op.Edits.FaceAttribs.size(), (uint)op.Edits.CreatedVerts.size(),
					(uint)op.Edits.CreatedFacesA.size(), (uint)op.Edits.DelVerts.size(),
					(uint)op.Edits.DelFaces.size());
			else
				fprintf(stderr, "  op[%u] Mirror axis=%d offset=%g copy=%d\n",
					i - 1, op.MirrorAxis, op.MirrorOffset, op.MirrorCopy ? 1 : 0);
		}
		if (opStack[i - 1].Type == 0)
			applyEditMeshEdits(opStack[i - 1].Edits, verts, faces);
		else
			applyMirror(opStack[i - 1], verts, faces);
		if (dbgThis)
			fprintf(stderr, "    -> %u verts, %u faces\n", (uint)verts.size(), (uint)faces.size());
	}
	return true;
}

// One candidate node plus the appdata this tool cares about, resolved once at selection time.
struct SCandidate
{
	INode *Node;
	CSceneClass *Obj; // raw object reference (node.getReference(1)); modifier stack unwrapped later
	bool Collision;
	bool Exterior;
	std::string IgName;
};

// A world-space mesh extracted from one node, ready to be merged into a CCollisionMeshBuild.
struct SNodeMesh
{
	std::vector<NLMISC::CVector> Verts;
	std::vector<uint32> FaceV0, FaceV1, FaceV2;
	std::vector<bool> FaceVis0, FaceVis1, FaceVis2;
	std::vector<uint32> FaceMatId;
};

// Extract a node's evaluated mesh in WORLD space: object-space geometry + modifiers via
// extractObjectMesh, then the node's world transform (objectTM = offsetLocal * nodeTM, same
// composition as every other tri-mesh reader in this project).
bool extractNodeMesh(INode &node, CSceneClass *rawObj, MAXSCENE::SNodeTMCache &tmCache, SNodeMesh &out, std::string &err)
{
	std::vector<NLMISC::CVector> objVerts;
	std::vector<SRawFace> objFaces;
	if (!extractObjectMesh(node, rawObj, objVerts, objFaces, err)) return false;

	MAXMATH::Point3M offPos; MAXMATH::QuatM offRot; MAXMATH::ScaleValueM offScale;
	bool hasOffset = MAXSCENE::readObjectOffset(&node, offPos, offRot, offScale);
	MAXMATH::Matrix3M nodeTM = MAXSCENE::getNodeTM(&node, tmCache);
	MAXMATH::Matrix3M objectTM = hasOffset ? (MAXMATH::composePRS(offPos, offRot, offScale) * nodeTM) : nodeTM;
	NLMISC::CMatrix worldMat;
	MAXSCENE::convertMatrix(worldMat, objectTM);

	out.Verts.resize(objVerts.size());
	for (size_t i = 0; i < objVerts.size(); ++i)
		out.Verts[i] = worldMat * objVerts[i];

	out.FaceV0.resize(objFaces.size()); out.FaceV1.resize(objFaces.size()); out.FaceV2.resize(objFaces.size());
	out.FaceVis0.resize(objFaces.size()); out.FaceVis1.resize(objFaces.size()); out.FaceVis2.resize(objFaces.size());
	out.FaceMatId.resize(objFaces.size());
	for (size_t i = 0; i < objFaces.size(); ++i)
	{
		out.FaceV0[i] = objFaces[i].V[0]; out.FaceV1[i] = objFaces[i].V[1]; out.FaceV2[i] = objFaces[i].V[2];
		out.FaceVis0[i] = objFaces[i].Vis0; out.FaceVis1[i] = objFaces[i].Vis1; out.FaceVis2[i] = objFaces[i].Vis2;
		out.FaceMatId[i] = objFaces[i].MatId;
	}
	return true;
}

// CExportNel::createCollisionMeshBuild (nel_mesh_lib/export_collision.cpp), replicated field-
// for-field: merge every candidate's world-space mesh into one CCollisionMeshBuild, weld
// cross-node duplicate vertices, drop degenerate faces, and validate with the real link().
// Returns false (mesh left as-is, caller must not write it) on any hard failure.
bool buildCollisionMesh(const std::vector<SCandidate *> &group, MAXSCENE::SNodeTMCache &tmCache,
	NLPACS::CCollisionMeshBuild &cmb, std::string &err)
{
	cmb.Vertices.clear();
	cmb.Faces.clear();

	std::vector<uint32> rootMeshOfVertex; // which node (index within group) each vertex came from
	uint32 totalSurfaces = 0;

	for (size_t n = 0; n < group.size(); ++n)
	{
		SNodeMesh mesh;
		std::string nerr;
		if (!extractNodeMesh(*group[n]->Node, group[n]->Obj, tmCache, mesh, nerr))
		{
			// Non-fatal — one bad node drops that node, the group keeps building. Typical cases:
			// XRef source resolves to a parametric primitive (Box/Cylinder/Sphere) or an
			// EditablePoly that we don't yet extract to a tri mesh for cmb (ig's own
			// buildParametricMesh + poly triangulation still lives in ig, not shared). Log the
			// warning and continue so the other collision nodes in the ig group still ship.
			fprintf(stderr, "WARNING: cmb: skipping \"%s\": %s\n",
				ucstring(group[n]->Node->userName()).toUtf8().c_str(), nerr.c_str());
			continue;
		}

		uint32 vertBase = (uint32)cmb.Vertices.size();
		for (size_t i = 0; i < mesh.Verts.size(); ++i)
		{
			cmb.Vertices.push_back(mesh.Verts[i]);
			rootMeshOfVertex.push_back((uint32)n);
		}

		uint32 maxMatId = 0;
		bool exterior = group[n]->Exterior;
		for (size_t i = 0; i < mesh.FaceV0.size(); ++i)
		{
			NLPACS::CCollisionFace face;
			face.V[0] = mesh.FaceV0[i] + vertBase;
			face.V[1] = mesh.FaceV1[i] + vertBase;
			face.V[2] = mesh.FaceV2[i] + vertBase;
			face.Visibility[0] = mesh.FaceVis0[i];
			face.Visibility[1] = mesh.FaceVis1[i];
			face.Visibility[2] = mesh.FaceVis2[i];
			uint32 matId = mesh.FaceMatId[i];
			if (!exterior && matId > maxMatId) maxMatId = matId;
			face.Surface = exterior ? (sint32)-1 : (sint32)(totalSurfaces + matId);
			face.Material = (sint32)matId;
			cmb.Faces.push_back(face);
		}
		totalSurfaces += maxMatId + 1;
	}

	// Weld cross-node vertices (real NL3D::CQuadGrid, exactly as the reference: 64 cells, 1m
	// width, insert every vertex's own ±5mm/±5mm/0 box, then a single ascending-index pass that
	// welds a LATER vertex j onto an EARLIER root i when they're within threshold and from
	// different nodes).
	uint32 totalVertices = (uint32)cmb.Vertices.size();
	NL3D::CQuadGrid<uint32> grid;
	grid.create(GRID_SIZE, GRID_WIDTH);
	std::vector<uint32> remapIds(totalVertices);
	for (uint32 i = 0; i < totalVertices; ++i)
	{
		remapIds[i] = i;
		NLMISC::CAABBox box;
		box.setCenter(cmb.Vertices[i]);
		box.setHalfSize(NLMISC::CVector(WELD_THRESHOLD, WELD_THRESHOLD, 0.0f));
		grid.insert(box.getMin(), box.getMax(), i);
	}
	for (uint32 i = 0; i < totalVertices; ++i)
	{
		if (remapIds[i] != i) continue;
		NLMISC::CVector weldTo = cmb.Vertices[i];
		grid.select(cmb.Vertices[i], cmb.Vertices[i]);
		for (NL3D::CQuadGrid<uint32>::CIterator it = grid.begin(); it != grid.end(); ++it)
		{
			uint32 weldedId = *it;
			if (weldedId <= i || rootMeshOfVertex[i] == rootMeshOfVertex[weldedId]
				|| remapIds[weldedId] != weldedId
				|| (cmb.Vertices[weldedId] - weldTo).norm() > WELD_THRESHOLD)
				continue;
			remapIds[weldedId] = i;
		}
	}
	std::vector<NLMISC::CVector> remapVertices;
	for (uint32 i = 0; i < totalVertices; ++i)
	{
		if (remapIds[i] == i)
		{
			uint32 newId = (uint32)remapVertices.size();
			remapVertices.push_back(cmb.Vertices[i]);
			remapIds[i] = newId;
		}
		else
		{
			remapIds[i] = remapIds[remapIds[i]];
		}
	}
	for (size_t i = 0; i < cmb.Faces.size(); ++i)
		for (int j = 0; j < 3; ++j)
			cmb.Faces[i].V[j] = remapIds[cmb.Faces[i].V[j]];
	cmb.Vertices = remapVertices;

	// Drop degenerate faces (any two indices equal after welding).
	std::vector<NLPACS::CCollisionFace> cleaned;
	cleaned.reserve(cmb.Faces.size());
	for (size_t i = 0; i < cmb.Faces.size(); ++i)
	{
		const NLPACS::CCollisionFace &f = cmb.Faces[i];
		if (f.V[0] == f.V[1] || f.V[1] == f.V[2] || f.V[2] == f.V[0]) continue;
		cleaned.push_back(f);
	}
	cmb.Faces = cleaned;

	// Validate (edge-consistency). The reference plugin skips a group on link errors — but the
	// corpus shows it shipped `.cmb` files with edge inconsistencies anyway (fy_hall_reunion:
	// applying only 0x0208 produces the reference's exact face-count 248 but its topology
	// triggers "left face already found" in `link`, and the reference file exists). So we run
	// `link` diagnostically only, log the first error on stderr, and write the file regardless
	// — matching what the reference actually shipped rather than what its documented failure
	// path says. Design-doc §10w's own "the earlier 'created faces are missing' warning is
	// retired" applies here too.
	std::vector<std::string> linkErrors;
	cmb.link(false, linkErrors);
	cmb.link(true, linkErrors);
	if (!linkErrors.empty())
		fprintf(stderr, "WARNING: cmb link diagnostic: %s\n", linkErrors[0].c_str());
	return true;
}

} /* anonymous namespace */

int main(int argc, char **argv)
{
	bool ligoMode = false;
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
	{
		std::string a = argv[i];
		if (a == "--ligo") ligoMode = true;
		else if (a == "--db" && i + 1 < argc) { DBPATH::setDefaultRoot(argv[++i]); }
		else if (a == "--path-alias" && i + 1 < argc)
		{
			// --path-alias <windows-prefix>=<root>, same shape as pipeline_max_export_ig — for
			// corpus content authored under a different drive/root than R:\graphics\...
			std::string kv = argv[++i];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n", kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
		}
		else args.push_back(a);
	}
	if (args.size() < 2)
	{
		std::cerr << "usage: pipeline_max_export_cmb [--ligo] [--db <root>] [--path-alias <prefix>=<root>] <input.max> <output_dir>\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export (no output written)\n";
		return 1;
	}
	std::string inputPath = args[0];
	std::string outputDir = args[1];
	while (!outputDir.empty() && (outputDir[outputDir.size() - 1] == '/' || outputDir[outputDir.size() - 1] == '\\'))
		outputDir.resize(outputDir.size() - 1);

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);

	// XRef resolution: shared with pipeline_max_export_ig. --ligo mode's collision nodes are
	// routinely XRefs into other .max files (§10v open); the registry drives every scene we
	// pull in as an XRef source. Deduce the DB root from the input path when --db was not
	// passed — same convention as pipeline_max_export_ig.
	XREFRESOLVE::configure(&reg);
	if (DBPATH::defaultRoot().empty())
	{
		// Strip "/stuff/..." or "/landscape/..." tail to find the DB root the input lives under.
		std::string abs = inputPath;
		std::string::size_type slash = abs.find_last_of("/\\");
		while (slash != std::string::npos)
		{
			abs.resize(slash);
			slash = abs.find_last_of("/\\");
			if (slash != std::string::npos)
			{
				std::string tail = abs.substr(slash + 1);
				if (tail == "stuff" || tail == "landscape" || tail == "graphics" || tail == "database")
				{
					DBPATH::setDefaultRoot(abs.substr(0, slash));
					break;
				}
			}
		}
	}

	CStorageOleIn in;
	if (!in.open(inputPath)) { std::cerr << "ERROR: not an OLE compound file: " << inputPath << "\n"; return 1; }

	CDllDirectory dll;
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(b); dll.serial(st); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(b); cd.serial(st); }
	cd.parse(VersionUnknown);
	CScene scene(&reg, &dll, &cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(b); scene.serial(st); }
	scene.parse(VersionUnknown);

	// Select candidates: `geometry`-category nodes flagged COLLISION or COLLISION_EXTERIOR,
	// in scene-container order (the maxscript's `for m in geometry` enumeration, same precedent
	// as every other tool in this family). --ligo mode additionally accepts XRefObject nodes,
	// unwrapped through XREFRESOLVE::resolveXRefObject (shared with pipeline_max_export_ig)
	// exactly as the reference plugin's EvalWorldState resolves them live; this closes design-
	// doc §10v's "6 of 1201 brick files export nothing" gap.
	std::vector<SCandidate> candidates;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		CSceneClass *rawObj = dynamic_cast<CSceneClass *>(node->getReference(1));
		if (!rawObj) continue;

		if (isXRefObject(rawObj) && !ligoMode) continue; // direct mode never selects XRefs
		CSceneClass *obj = rawObj; // modifier stack (if any) is unwrapped in extractObjectMesh

		bool collision = APPDATA::getScriptAppDataInt(node, NEL3D_APPDATA_COLLISION, 0) == 1;
		bool exterior = APPDATA::getScriptAppDataInt(node, NEL3D_APPDATA_COLLISION_EXTERIOR, 0) == 1;
		if (!collision && !exterior) continue; // not selected by the maxscript at all

		SCandidate c;
		c.Node = node;
		c.Obj = obj;
		c.Collision = collision;
		c.Exterior = exterior;
		c.IgName = APPDATA::getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
		if (c.IgName.empty()) c.IgName = "unknown_ig";
		candidates.push_back(c);
	}

	// Group by igname (first-seen order), keeping only COLLISION==1 nodes in each group's node
	// list — a COLLISION_EXTERIOR-only node was selected above but contributes no geometry
	// (matches createCollisionMeshBuildList's own bCol-only gate exactly).
	std::vector<std::string> groupOrder;
	std::map<std::string, std::vector<SCandidate *> > groups;
	for (size_t i = 0; i < candidates.size(); ++i)
	{
		if (!candidates[i].Collision) continue;
		std::map<std::string, std::vector<SCandidate *> >::iterator git = groups.find(candidates[i].IgName);
		if (git == groups.end())
		{
			groupOrder.push_back(candidates[i].IgName);
			git = groups.insert(std::make_pair(candidates[i].IgName, std::vector<SCandidate *>())).first;
		}
		git->second.push_back(&candidates[i]);
	}

	if (groupOrder.empty())
	{
		std::cerr << "WARNING: no collision geometry in " << inputPath << "\n";
		return 3;
	}

	MAXSCENE::SNodeTMCache tmCache;
	int written = 0;
	bool hadError = false;
	for (size_t g = 0; g < groupOrder.size(); ++g)
	{
		const std::string &igname = groupOrder[g];
		NLPACS::CCollisionMeshBuild cmb;
		std::string err;
		if (!buildCollisionMesh(groups[igname], tmCache, cmb, err))
		{
			std::cerr << "ERROR: group \"" << igname << "\": " << err << "\n";
			hadError = true;
			continue;
		}
		std::string outPath = outputDir + "/" + igname + ".cmb";
		NLMISC::COFile file;
		if (!file.open(outPath)) { std::cerr << "ERROR: cannot open output " << outPath << "\n"; hadError = true; continue; }
		cmb.serial(file);
		file.close();
		++written;
	}

	if (hadError && written == 0) return 1;
	return 0;
}

/* end of file */
