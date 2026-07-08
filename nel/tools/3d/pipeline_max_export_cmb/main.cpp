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

const NLMISC::CClassId CLASSID_OSM_DERIVED(0x29263a68, 0x405f22f5);
const NLMISC::CClassId CLASSID_WSM_DERIVED(0x4ec13906, 0x5578130e);
const NLMISC::CClassId CLASSID_XREF_OBJECT(0x92aab38c, 0x00000000); // PartB varies; matched on PartA only, like ig
const TSClassId SCLASS_OSMODIFIER = 0x00000810;
const TSClassId SCLASS_WSMODIFIER = 0x00000820;

const float WELD_THRESHOLD = 0.005f;
const sint GRID_SIZE = 64;
const float GRID_WIDTH = 1.0f;

// Edit Mesh modifier (class 0x50) evaluation: the per-node modifier data lives on the OSM
// Derived wrapper's orphaned 0x2500 containers (one per modifier slot). Decode ported from
// pipeline_max_export_ig/main.cpp (§ design doc, "Edit Mesh modifier decode"), already
// corpus-validated there for cluster/portal meshes; extended here to also carry the raw mesh's
// per-face material id and edge-visibility bits through the edit (ig's own copy only needs
// topology for its containment tests).
struct SEditMeshEdits
{
	std::vector<std::pair<uint32, NLMISC::CVector> > Moves;
	std::vector<NLMISC::CVector> Created; // 0x0210: created vertices (object space; unused, no face data)
	std::vector<bool> DelVerts;
	std::vector<bool> DelFaces;
};

bool readEditMeshBitArray(CStorageContainer *cont, std::vector<bool> &out)
{
	if (!cont) return false;
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != 0x2700) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 4) return false;
		uint32 n;
		memcpy(&n, nlVectorData(raw->Value), 4);
		if (raw->Value.size() < 4 + ((size_t)n + 7) / 8) return false;
		out.resize(n);
		for (uint32 i = 0; i < n; ++i)
			out[i] = (raw->Value[4 + i / 8] >> (i % 8)) & 1;
		return true;
	}
	return false;
}

bool readEditMeshModApp(CStorageContainer *c2500, SEditMeshEdits &out)
{
	for (CStorageContainer::TStorageObjectConstIt it = c2500->chunks().begin(); it != c2500->chunks().end(); ++it)
	{
		if (it->first != 0x2512) continue;
		CStorageContainer *c2512 = dynamic_cast<CStorageContainer *>(it->second);
		if (!c2512) continue;
		for (CStorageContainer::TStorageObjectConstIt jt = c2512->chunks().begin(); jt != c2512->chunks().end(); ++jt)
		{
			if (jt->first != 0x4000) continue;
			CStorageContainer *c4000 = dynamic_cast<CStorageContainer *>(jt->second);
			if (!c4000) continue;
			for (CStorageContainer::TStorageObjectConstIt kt = c4000->chunks().begin(); kt != c4000->chunks().end(); ++kt)
			{
				if (kt->first == 0x0140)
				{
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (raw && raw->Value.size() >= 4)
					{
						uint32 n;
						memcpy(&n, nlVectorData(raw->Value), 4);
						if (raw->Value.size() >= 4 + (size_t)n * 16)
						{
							for (uint32 i = 0; i < n; ++i)
							{
								uint32 idx;
								float v[3];
								memcpy(&idx, nlVectorData(raw->Value) + 4 + i * 16, 4);
								memcpy(v, nlVectorData(raw->Value) + 4 + i * 16 + 4, 12);
								out.Moves.push_back(std::make_pair(idx, NLMISC::CVector(v[0], v[1], v[2])));
							}
						}
					}
				}
				else if (kt->first == 0x0210)
				{
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (raw && raw->Value.size() >= 4)
					{
						uint32 n;
						memcpy(&n, nlVectorData(raw->Value), 4);
						if (raw->Value.size() >= 4 + (size_t)n * 12)
						{
							out.Created.resize(n);
							memcpy(nlVectorData(out.Created), nlVectorData(raw->Value) + 4, (size_t)n * 12);
						}
					}
				}
				else if (kt->first == 0x0170)
					readEditMeshBitArray(dynamic_cast<CStorageContainer *>(kt->second), out.DelVerts);
				else if (kt->first == 0x0270)
					readEditMeshBitArray(dynamic_cast<CStorageContainer *>(kt->second), out.DelFaces);
			}
			return true;
		}
	}
	return false;
}

// One raw (pre-weld, object-space) face carrying everything a collision face needs.
struct SRawFace
{
	uint32 V[3];
	bool Vis0, Vis1, Vis2; // already index-remapped, see extractObjectMesh
	uint32 MatId;
};

struct SModOp
{
	int Type; // 0 = Edit Mesh, 1 = Mirror
	SEditMeshEdits Edits;
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

// Apply Edit Mesh edits: moves, then face deletes, then vertex deletes with face reindexing.
// Created vertices (0x0210) are deliberately NOT appended: this decode has no matching "created
// faces" record (0x0410-family, undecoded — see readEditMeshModApp), so a Created vertex is by
// construction unreferenced by any face here; appending it would add a dangling point to the
// output (and, on the one corpus file where this occurs, the tail of the 0x0210 payload reads as
// non-finite garbage once the real vertex data runs out — see the caller's warning). Dropping
// them means a node with genuine Edit-Mesh-added topology exports with its added faces missing
// rather than with corrupt data; the caller warns so this is visible, not silent.
void applyEditMeshEdits(const SEditMeshEdits &e, std::vector<NLMISC::CVector> &verts, std::vector<SRawFace> &faces)
{
	for (uint i = 0; i < e.Moves.size(); ++i)
		if (e.Moves[i].first < verts.size())
			verts[e.Moves[i].first] += e.Moves[i].second;
	if (!e.DelFaces.empty())
	{
		std::vector<SRawFace> kept;
		for (uint i = 0; i < faces.size(); ++i)
			if (i >= e.DelFaces.size() || !e.DelFaces[i])
				kept.push_back(faces[i]);
		faces.swap(kept);
	}
	if (!e.DelVerts.empty())
	{
		std::vector<uint32> remap(verts.size());
		std::vector<NLMISC::CVector> kept;
		for (uint i = 0; i < verts.size(); ++i)
		{
			remap[i] = (uint32)kept.size();
			if (i >= e.DelVerts.size() || !e.DelVerts[i])
				kept.push_back(verts[i]);
		}
		verts.swap(kept);
		for (uint i = 0; i < faces.size(); ++i)
			for (int j = 0; j < 3; ++j)
				faces[i].V[j] = remap[faces[i].V[j]];
	}
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
				if (app && readEditMeshModApp(app, op.Edits))
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
	if (!geom) { err = "base object is not a GeomObject"; return false; }
	STORAGE::CGeomBuffers *gb = geom->geomBuffers();
	if (!gb) { err = "no GeomBuffers chunk"; return false; }
	const std::vector<NLMISC::CVector> *rawVerts = gb->triVertices();
	const std::vector<STORAGE::CGeomTriIndexInfo> *rawFaces = gb->triFaces();
	if (!rawVerts || !rawFaces) { err = "no tri buffers (EditablePoly or unsupported object?)"; return false; }

	verts = *rawVerts;
	faces.resize(rawFaces->size());
	for (size_t i = 0; i < rawFaces->size(); ++i)
	{
		const STORAGE::CGeomTriIndexInfo &f = (*rawFaces)[i];
		faces[i].V[0] = f.a; faces[i].V[1] = f.b; faces[i].V[2] = f.c;
		uint32 faceFlags = f.smoothingGroups;
		faces[i].MatId = faceFlags >> 16;
		faces[i].Vis0 = (faceFlags & 2) != 0; // EDGE_B
		faces[i].Vis1 = (faceFlags & 4) != 0; // EDGE_C
		faces[i].Vis2 = (faceFlags & 1) != 0; // EDGE_A
	}

	// Apply modifier ops base-upward (opStack was collected outermost-first, so replay back to
	// front — mirrors pipeline_max_export_ig's nodeWorldMesh).
	for (uint i = (uint)opStack.size(); i > 0; --i)
	{
		if (opStack[i - 1].Type == 0)
		{
			if (!opStack[i - 1].Edits.Created.empty())
				fprintf(stderr, "WARNING: node '%s' has Edit Mesh created geometry (created faces are not "
					"decoded); those faces are missing from the export\n", ucstring(node.userName()).toUtf8().c_str());
			applyEditMeshEdits(opStack[i - 1].Edits, verts, faces);
		}
		else
			applyMirror(opStack[i - 1], verts, faces);
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
		if (!extractNodeMesh(*group[n]->Node, group[n]->Obj, tmCache, mesh, err))
		{
			err = "\"" + ucstring(group[n]->Node->userName()).toUtf8() + "\": " + err;
			return false;
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

	// Validate (edge-consistency); a group with link errors is dropped entirely, matching the
	// reference (createCollisionMeshBuild returns NULL, createCollisionMeshBuildList just skips
	// this group).
	std::vector<std::string> linkErrors;
	cmb.link(false, linkErrors);
	cmb.link(true, linkErrors);
	if (!linkErrors.empty())
	{
		err = linkErrors[0];
		return false;
	}
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
		else args.push_back(a);
	}
	if (args.size() < 2)
	{
		std::cerr << "usage: pipeline_max_export_cmb [--ligo] <input.max> <output_dir>\n";
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
	// as every other tool in this family). --ligo additionally walks XRefObject nodes, but this
	// corpus's collision-flagged nodes are always plain EditableMesh (no XRef seen so far), so
	// XRef resolution is intentionally not implemented yet — an XRef candidate is reported and
	// skipped rather than silently mishandled (§12.2: type only what you can prove).
	std::vector<SCandidate> candidates;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		CSceneClass *rawObj = dynamic_cast<CSceneClass *>(node->getReference(1));
		if (!rawObj) continue;

		bool isXRef = (rawObj->classDesc()->classId().a() == CLASSID_XREF_OBJECT.a());
		if (isXRef && !ligoMode) continue; // direct mode never selects XRefObject nodes
		if (isXRef)
		{
			std::cerr << "WARNING: \"" << ucstring(node->userName()).toUtf8()
				<< "\": XRefObject collision nodes are not yet supported, skipped\n";
			continue;
		}
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
