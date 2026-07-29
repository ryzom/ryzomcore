/**
 * \file interface_build.cpp
 * \brief See interface_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

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

#include "interface_build.h"

#include <algorithm>
#include <cstdio>
#include <list>
#include <map>
#include <set>

#include <nel/misc/aabbox.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/3d/mesh_mrm_skinned.h>

#include "mesh_eval.h"
#include "../pipeline_max_export_common/parametric_mesh.h"
#include "../pipeline_max_export_common/biped_rig.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/scene.h"

using namespace NLMISC;
using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_INTERFACE_FILE 1423062700
#define NEL3D_APPDATA_INTERFACE_THRESHOLD 1423062701
#define NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS 1423062702

namespace IFACEBUILD {

// ---------------------------------------------------------------------------------------------

// One interface polygon: WORLD-space ordered border vertices + per-vertex edge normals — the
// exact structure the reference's CMeshInterface holds (export_mesh_interface.cpp:48-130).
struct SInterfaceVert
{
	CVector Pos;
	CVector Normal;
};

struct SMeshInterface
{
	std::vector<SInterfaceVert> Verts;

	bool canWeld(const CVector &pos, float threshold, uint &snapTo) const
	{
		for (uint k = 0; k < Verts.size(); ++k)
		{
			if ((pos - Verts[k].Pos).norm() < threshold)
			{
				snapTo = k;
				return true;
			}
		}
		return false;
	}
	bool canWeld(const CVector &pos, float threshold) const
	{
		uint dummy;
		return canWeld(pos, threshold, dummy);
	}
	void buildBBox(CAABBox &dest) const
	{
		CVector minV = Verts[0].Pos, maxV = Verts[0].Pos;
		for (uint k = 1; k < Verts.size(); ++k)
		{
			minV.minof(minV, Verts[k].Pos);
			maxV.maxof(maxV, Verts[k].Pos);
		}
		dest.setMinMax(minV, maxV);
	}
};

// ---------------------------------------------------------------------------------------------
// Border-polygon extraction — CExportNel::maxPolygonMeshToOrderedPoly (export_misc.cpp:1020):
// segments referenced by exactly one triangle are border segments; order them into a loop
// starting from the FIRST border segment's V0 (std::map order = (minV, maxV) lexicographic),
// advancing with find_if from the list head. avgNormal = normalized sum of face normals.

struct SSeg
{
	uint V0, V1;
	SSeg(uint v0, uint v1) : V0(v0), V1(v1) { }
	bool operator<(const SSeg &o) const
	{
		uint lv0 = std::min(V0, V1), lv1 = std::max(V0, V1);
		uint rv0 = std::min(o.V0, o.V1), rv1 = std::max(o.V0, o.V1);
		if (lv0 != rv0) return lv0 < rv0;
		return lv1 < rv1;
	}
};

// World-space face corner fetch helper over an evaluated mesh.
static inline CVector worldVert(const SEvalMesh &mesh, uint vi, const CMatrix &toWorld)
{
	const MAXMATH::Point3M &p = mesh.Verts[vi];
	return toWorld * CVector(p.x, p.y, p.z);
}

static void meshToOrderedPoly(const SEvalMesh &mesh, const CMatrix &toWorld,
                              std::vector<CVector> &dest, CVector &avgNormal)
{
	avgNormal.set(0, 0, 0);
	typedef std::map<SSeg, uint> TSegMap;
	TSegMap segs;
	for (uint k = 0; k < mesh.Faces.size(); ++k)
	{
		// face normal in world (reference getMaxFaceNormal: (c1-c0)^(c2-c1), normalized)
		CVector c0 = worldVert(mesh, mesh.Faces[k].V[0], toWorld);
		CVector c1 = worldVert(mesh, mesh.Faces[k].V[1], toWorld);
		CVector c2 = worldVert(mesh, mesh.Faces[k].V[2], toWorld);
		CVector n = (c1 - c0) ^ (c2 - c1);
		n.normalize();
		avgNormal += n;
		for (uint l = 0; l < 3; ++l)
		{
			SSeg seg(mesh.Faces[k].V[l], mesh.Faces[k].V[(l + 1) % 3]);
			TSegMap::iterator it = segs.find(seg);
			if (it != segs.end())
				++it->second;
			else
				segs[seg] = 1;
		}
	}
	avgNormal.normalize();

	std::list<SSeg> borderSegs;
	for (TSegMap::const_iterator it = segs.begin(); it != segs.end(); ++it)
		if (it->second == 1)
			borderSegs.push_back(it->first);

	dest.clear();
	if (borderSegs.empty()) return;

	dest.push_back(worldVert(mesh, borderSegs.begin()->V0, toWorld));
	uint nextToFind = borderSegs.begin()->V1;
	borderSegs.pop_front();
	for (;;)
	{
		std::list<SSeg>::iterator nextSeg = borderSegs.begin();
		for (; nextSeg != borderSegs.end(); ++nextSeg)
			if (nextSeg->V0 == nextToFind || nextSeg->V1 == nextToFind)
				break;
		if (nextSeg == borderSegs.end()) return;
		dest.push_back(worldVert(mesh, nextSeg->V0 == nextToFind ? nextSeg->V0 : nextSeg->V1, toWorld));
		nextToFind = (nextSeg->V0 == nextToFind) ? nextSeg->V1 : nextSeg->V0;
		borderSegs.erase(nextSeg);
	}
}

// ---------------------------------------------------------------------------------------------
// Interface-file loading (cached): every geometry node of the interface .max contributes one
// border polygon, verts in WORLD space of the INTERFACE scene, per-vertex normals from the
// prev/next border edges × the polygon's average normal (export_mesh_interface.cpp:134-189).

typedef std::map<std::string, std::vector<SMeshInterface> > TInterfaceCache;
static TInterfaceCache s_interfaceCache;

void clearInterfaceCache()
{
	s_interfaceCache.clear();
}

static const std::vector<SMeshInterface> *loadInterfaces(const std::string &authoredPath)
{
	// Strip at NUL (some appdata strings carry uninitialized tail bytes past the terminator),
	// append .max when the extension is missing, resolve through the database root.
	std::string path = authoredPath.c_str(); // NUL strip
	if (NLMISC::CFile::getExtension(path).empty())
		path += ".max";
	std::string resolved;
	if (!resolveDbPath(path, resolved))
	{
		fprintf(stderr, "WARNING: interface file '%s' does not resolve under the database root\n",
		        path.c_str());
		return nullptr;
	}
	TInterfaceCache::iterator it = s_interfaceCache.find(resolved);
	if (it != s_interfaceCache.end())
		return it->second.empty() ? nullptr : &it->second;
	std::vector<SMeshInterface> &out = s_interfaceCache[resolved];

	SLoadedMax *lm = loadMaxFileCached(resolved);
	if (!lm || !lm->Scene)
	{
		fprintf(stderr, "WARNING: cannot load interface file '%s'\n", resolved.c_str());
		return nullptr;
	}
	CSceneClassContainer *ssc = lm->Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = nullptr;
	for (CStorageContainer::TStorageObjectConstIt nit = ssc->chunks().begin();
	     nit != ssc->chunks().end(); ++nit)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(nit->second);
		if (!node) continue;
		CSceneClass *base = baseObjectOf(*node, nullptr, nullptr);
		if (!base) continue;
		// The corpus interface files draw their border polygons as SplineShape/Line nodes
		// (+ Edit Mesh) — the reference's ConvertToType handles those transparently, and so
		// does evalNodeMesh via the spline-cap path (closed spline → capped mesh whose border
		// ring is exactly the interface polygon). Accept both superclasses.
		TSClassId scid = base->classDesc()->superClassId();
		if (scid != SCLASS_GEOMOBJECT && scid != 0x40 /* SHAPE */) continue;
		SEvalMesh mesh;
		if (!evalNodeMesh(*node, mesh, nullptr)) continue;
		if (mesh.Faces.empty()) continue;
		// object → world of the INTERFACE scene: objectTM = offsetTM * nodeTM
		Matrix3M nodeTM = getNodeTM(node, tmCache);
		MAXMATH::Point3M opos;
		MAXMATH::QuatM orot;
		MAXMATH::ScaleValueM oscale;
		readObjectOffset(node, opos, orot, oscale);
		Matrix3M objectTM = composePRS(opos, orot, oscale) * nodeTM;
		CMatrix toWorld;
		MAXSCENE::convertMatrix(toWorld, objectTM);

		std::vector<CVector> poly;
		CVector polyNormal;
		meshToOrderedPoly(mesh, toWorld, poly, polyNormal);
		if (poly.empty()) continue;

		SMeshInterface mi;
		uint numVerts = (uint)poly.size();
		mi.Verts.resize(numVerts);
		for (uint k = 0; k < numVerts; ++k)
			mi.Verts[k].Pos = poly[k];
		for (uint k = 0; k < numVerts; ++k)
		{
			CVector prevNorm = (mi.Verts[k].Pos - mi.Verts[(k + numVerts - 1) % numVerts].Pos) ^ polyNormal;
			CVector nextNorm = (mi.Verts[(k + 1) % numVerts].Pos - mi.Verts[k].Pos) ^ polyNormal;
			mi.Verts[k].Normal = (prevNorm + nextNorm).normed();
		}
		out.push_back(mi);
	}
	if (out.empty())
	{
		fprintf(stderr, "WARNING: no interface polygons in '%s'\n", resolved.c_str());
		return nullptr;
	}
	return &out;
}

// ---------------------------------------------------------------------------------------------
// MRM bookkeeping — ApplyMeshInterfacesForMRM (export_mesh_interface.cpp:199): store the
// interface polys in OBJECT space on the build, link every in-threshold vertex, and align each
// linked vertex through the CMeshMRMSkinned packed-vertex round-trip.

static void applyForMRM(const std::vector<SMeshInterface> &interfaces,
                        NL3D::CMesh::CMeshBuild &mbuild, const CMatrix &toWorldMat,
                        float threshold)
{
	CMatrix toObjectMat = toWorldMat;
	toObjectMat.invert();
	CMatrix toObjectMatNormal = toObjectMat;
	toObjectMatNormal.setPos(CVector::Null);
	toObjectMatNormal.invert();
	toObjectMatNormal.transpose();

	mbuild.Interfaces.resize(interfaces.size());
	for (uint m = 0; m < interfaces.size(); ++m)
	{
		mbuild.Interfaces[m].Vertices.resize(interfaces[m].Verts.size());
		for (uint k = 0; k < mbuild.Interfaces[m].Vertices.size(); ++k)
		{
			mbuild.Interfaces[m].Vertices[k].Pos = toObjectMat * interfaces[m].Verts[k].Pos;
			mbuild.Interfaces[m].Vertices[k].Normal = toObjectMatNormal * interfaces[m].Verts[k].Normal;
			mbuild.Interfaces[m].Vertices[k].Normal.normalize();
		}
	}

	mbuild.InterfaceLinks.resize(mbuild.Vertices.size());
	for (uint k = 0; k < mbuild.Vertices.size(); ++k)
	{
		mbuild.InterfaceLinks[k].InterfaceId = -1;
		for (uint m = 0; m < interfaces.size(); ++m)
		{
			uint snapTo;
			if (interfaces[m].canWeld(toWorldMat * mbuild.Vertices[k], threshold, snapTo))
			{
				mbuild.InterfaceLinks[k].InterfaceId = m;
				mbuild.InterfaceLinks[k].InterfaceVertexId = snapTo;
				mbuild.InterfaceVertexFlag.set(k);

				// Force pack / unpack to be aligned with CMeshMRMSkinned vertices — the
				// reference's exact round-trip, so interface vertices of separately-exported
				// meshes land on identical packed positions.
				NL3D::CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex vertex;
				vertex.setPos(mbuild.Vertices[k], NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE);
				vertex.getPos(mbuild.Vertices[k], NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE);
				break;
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Normal correction, variant 0 — ApplyMeshInterfacesUsingInterfaceNormals: welding corners
// SNAP position to the interface vertex and take its normal (both back in object space).

static void applyUsingInterfaceNormals(const std::vector<SMeshInterface> &interfaces,
                                       NL3D::CMesh::CMeshBuild &mbuild,
                                       const CMatrix &toWorldMat, float threshold)
{
	CMatrix invMat = toWorldMat.inverted();
	for (uint k = 0; k < mbuild.Faces.size(); ++k)
	{
		for (uint l = 0; l < 3; ++l)
		{
			for (uint m = 0; m < interfaces.size(); ++m)
			{
				CVector &vert = mbuild.Vertices[mbuild.Faces[k].Corner[l].Vertex];
				uint snapTo;
				if (interfaces[m].canWeld(toWorldMat * vert, threshold, snapTo))
				{
					vert = invMat * interfaces[m].Verts[snapTo].Pos;
					mbuild.Faces[k].Corner[l].Normal = invMat.mulVector(interfaces[m].Verts[snapTo].Normal);
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Normal correction, variant 1 (corpus-dominant) — ApplyMeshInterfacesUsingSceneNormals: for
// each welding corner, area-weighted normal of every scene face sharing a smoothing group and
// having any vertex within threshold. Faces come from the whole CURRENT scene, tree order.

struct SNodeFace
{
	CVector P[3];
	uint32 SmoothGroup;
	CVector getNormal() const { return ((P[1] - P[0]) ^ (P[2] - P[1])).normed(); }
	float getArea() const { return 0.5f * ((P[1] - P[0]) ^ (P[2] - P[0])).norm(); }
	void buildBBox(CAABBox &dest) const
	{
		CVector minV(P[0]), maxV(P[0]);
		minV.minof(minV, P[1]); minV.minof(minV, P[2]);
		maxV.maxof(maxV, P[1]); maxV.maxof(maxV, P[2]);
		dest.setMinMax(minV, maxV);
	}
};

// Tree-order collection of world-space faces intersecting the delimiter (the reference's
// AddNodeToQuadGrid semantics: node bbox test, per-face bbox test, recurse into children).
static void addNodeFaces(const CAABBox &delimiter, std::vector<SNodeFace> &dest, INode *node,
                         CSceneClassContainer *ssc, SNodeTMCache &tmCache)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(node);
	if (n)
	{
		CSceneClass *base = baseObjectOf(*n, nullptr, nullptr);
		// Evaluable classes only (EditableMesh/EditablePoly/parametric prims/splines): the
		// reference sweeps everything tri-convertible, which additionally includes Biped
		// Objects (0x9125, the Classic body-part boxes) — those have no headless mesh decode
		// (MGF Part J: derived from link length + cross-section), so scene sweeps that had
		// biped faces as weld candidates can differ in the area-weighted normal sum. Filter
		// silently rather than spam eval warnings for every bone box.
		bool evaluable = false;
		if (base)
		{
			NLMISC::CClassId cid = base->classDesc()->classId();
			TSClassId scid = base->classDesc()->superClassId();
			evaluable = cid == CLASSID_EDITABLE_MESH || cid == CLASSID_EDITABLE_POLY
				|| cid == PRIMMESH::CLASSID_BOX || cid == PRIMMESH::CLASSID_CYLINDER
				|| cid == PRIMMESH::CLASSID_SPHERE || cid == PRIMMESH::CLASSID_PLANE
				|| scid == 0x40 /* SHAPE */;
		}
		if (evaluable)
		{
			SEvalMesh mesh;
			if (evalNodeMesh(*n, mesh, nullptr) && !mesh.Faces.empty())
			{
				Matrix3M nodeTM = getNodeTM(n, tmCache);
				MAXMATH::Point3M opos;
				MAXMATH::QuatM orot;
				MAXMATH::ScaleValueM oscale;
				readObjectOffset(n, opos, orot, oscale);
				Matrix3M objectTM = composePRS(opos, orot, oscale) * nodeTM;
				CMatrix toWorld;
				MAXSCENE::convertMatrix(toWorld, objectTM);
				// node bbox test (reference buildMeshAABBox gate) folded into per-face tests:
				// per-face bbox intersect delimiter decides insertion either way, and skipping
				// the coarse node gate only costs time, not correctness.
				SNodeFace f;
				CAABBox faceBBox;
				for (uint k = 0; k < mesh.Faces.size(); ++k)
				{
					for (uint m = 0; m < 3; ++m)
						f.P[m] = worldVert(mesh, mesh.Faces[k].V[m], toWorld);
					f.buildBBox(faceBBox);
					if (delimiter.intersect(faceBBox))
					{
						f.SmoothGroup = mesh.Faces[k].SmGroup;
						dest.push_back(f);
					}
				}
			}
		}
	}
	std::vector<INode *> kids = PMAX_RIG::orderedChildrenOf(node, ssc);
	for (uint i = 0; i < kids.size(); ++i)
		addNodeFaces(delimiter, dest, kids[i], ssc, tmCache);
}

static void applyUsingSceneNormals(const std::vector<SMeshInterface> &interfaces,
                                   NL3D::CMesh::CMeshBuild &mbuild, const CMatrix &toWorldMat,
                                   float threshold, INode *sceneRoot,
                                   CSceneClassContainer *ssc, SNodeTMCache &tmCache)
{
	CMatrix toWorldMatInv = toWorldMat.inverted();
	std::vector<const SNodeFace *> candidateFaces;
	std::vector<SNodeFace> sceneFaces;
	for (uint k = 0; k < interfaces.size(); ++k)
	{
		CAABBox iBBox;
		interfaces[k].buildBBox(iBBox);
		// The reference extends the delimiter by adding ONLY the (max + threshold) point —
		// the min corner is NOT extended (export_mesh_interface.cpp:432 `iBBox.extend(
		// iBBox.getMax() + CVector(threshold,...))`). Replicated as-is.
		iBBox.extend(iBBox.getMax() + CVector(threshold, threshold, threshold));
		sceneFaces.clear();
		addNodeFaces(iBBox, sceneFaces, sceneRoot, ssc, tmCache);

		for (uint l = 0; l < mbuild.Faces.size(); ++l)
		{
			for (uint m = 0; m < 3; ++m)
			{
				candidateFaces.clear();
				CVector vert = toWorldMat * mbuild.Vertices[mbuild.Faces[l].Corner[m].Vertex];
				if (interfaces[k].canWeld(vert, threshold))
				{
					for (uint f = 0; f < sceneFaces.size(); ++f)
					{
						if ((sceneFaces[f].SmoothGroup & mbuild.Faces[l].SmoothGroup) == 0)
							continue;
						for (uint n2 = 0; n2 < 3; ++n2)
						{
							if ((sceneFaces[f].P[n2] - vert).norm() <= threshold)
							{
								candidateFaces.push_back(&sceneFaces[f]);
								break;
							}
						}
					}
					if (!candidateFaces.empty())
					{
						CVector dest = CVector::Null;
						for (uint c = 0; c < candidateFaces.size(); ++c)
							dest += candidateFaces[c]->getArea() * candidateFaces[c]->getNormal();
						dest.normalize();
						mbuild.Faces[l].Corner[m].Normal = toWorldMatInv.mulVector(dest);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------

bool useInterfaceMesh(INode &node)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string f = getScriptAppDataStr(n, NEL3D_APPDATA_INTERFACE_FILE, "");
	return !std::string(f.c_str()).empty();
}

void applyInterfaceToMeshBuild(INode &node, NL3D::CMesh::CMeshBuild &buildMesh,
                               const CMatrix &toWorldMat, SNodeTMCache &tmCache)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string interfaceFile = getScriptAppDataStr(n, NEL3D_APPDATA_INTERFACE_FILE, "");
	interfaceFile = interfaceFile.c_str(); // NUL strip (uninitialized appdata tails)
	if (interfaceFile.empty()) return;

	float threshold = getScriptAppDataFloat(n, NEL3D_APPDATA_INTERFACE_THRESHOLD, -1.f);
	if (threshold < 0.f)
	{
		fprintf(stderr, "WARNING: invalid interface threshold on '%s'\n", nodeName(node).c_str());
		return;
	}

	const std::vector<SMeshInterface> *interfaces = loadInterfaces(interfaceFile);
	if (!interfaces) return;

	buildMesh.InterfaceVertexFlag.resize((uint)buildMesh.Vertices.size());

	applyForMRM(*interfaces, buildMesh, toWorldMat, threshold);

	bool useSceneNodeNormals =
		getScriptAppDataInt(n, NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS, 0) != 0;
	if (!useSceneNodeNormals)
	{
		applyUsingInterfaceNormals(*interfaces, buildMesh, toWorldMat, threshold);
	}
	else
	{
		CSceneClassContainer *ssc = node.container();
		INode *root = ssc ? (INode *)ssc->scene()->rootNode() : nullptr;
		if (root)
			applyUsingSceneNormals(*interfaces, buildMesh, toWorldMat, threshold, root, ssc, tmCache);
	}
}

} /* namespace IFACEBUILD */

/* end of file */
