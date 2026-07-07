/**
 * \file mesh_eval.cpp
 * \brief See mesh_eval.h.
 *
 * Mesh chunk decode (GeomBuffers container 0x08FE on the geom object, corpus-established):
 *   0x0906 u32 mesh flags; 0x0908 f32; 0x0914 u32 count + count x Point3 vertices;
 *   0x0912 u32 count + count x { u32 v0, v1, v2; u32 smGroup; u32 faceFlags } — faceFlags is
 *   the Max Face::flags dword: edge-visibility low bits, matID in the HIGH WORD;
 *   0x0924/0x0928/0x092a u32s; 0x092c/0x092d/0x092e BitArray containers (selections);
 *   map channels, repeated per stored channel IN FILE ORDER:
 *     0x0959 u32 = channel index (1.. UVW, 0 = vertex color);
 *     0x2398 u32 (support flag, 1 in the corpus);
 *     0x2394 u32 count + count x Point3 map vertices;
 *     0x2396 u32 count + count x (u32 t0, t1, t2) map faces (parallel to mesh faces);
 *   0x0952/0x0953/0x0956/0x094c vertex-data channels (soft selection floats etc.) — not needed.
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

#include <nel/misc/types_nl.h>
#include "mesh_eval.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "../pipeline_max/builtin/geom_object.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;

namespace MESHEVAL {

// ---------------------------------------------------------------------------------------------
// GeomBuffers (Mesh) decode

static bool readCountPrefixed(CStorageRaw *raw, uint stride, const void **data, uint32 &count)
{
	if (!raw || raw->Value.size() < 4) return false;
	memcpy(&count, raw->Value.data(), 4);
	if (raw->Value.size() < 4 + (size_t)count * stride) return false;
	*data = raw->Value.data() + 4;
	return true;
}

static bool extractEditableMesh(CSceneClass *obj, SEvalMesh &out, const std::string &name)
{
	CGeomObject *geom = dynamic_cast<CGeomObject *>(obj);
	STORAGE::CGeomBuffers *gb = geom ? geom->geomBuffers() : NULL;
	if (!gb)
	{
		fprintf(stderr, "WARNING: mesh '%s' without geom buffers\n", name.c_str());
		return false;
	}

	// Vertices and faces — read the typed geom buffers (PMBS_GEOM_BUFFERS_PARSE); CVector and
	// CGeomTriIndexInfo share the byte layout of Point3M / SEvalFace, so a bulk copy reproduces
	// the exact bytes the raw path used (SEvalFace: V[3]=a,b,c, SmGroup=offset 12, Flags=offset 16).
	{
		const std::vector<NLMISC::CVector> *vv = gb->triVertices();
		if (!vv)
		{
			fprintf(stderr, "WARNING: mesh '%s' with missing vertex chunk\n", name.c_str());
			return false;
		}
		out.Verts.resize(vv->size());
		if (!vv->empty()) memcpy(&out.Verts[0], &(*vv)[0], vv->size() * 12);

		const std::vector<STORAGE::CGeomTriIndexInfo> *ff = gb->triFaces();
		if (!ff)
		{
			fprintf(stderr, "WARNING: mesh '%s' with missing face chunk\n", name.c_str());
			return false;
		}
		out.Faces.resize(ff->size());
		if (!ff->empty()) memcpy(&out.Faces[0], &(*ff)[0], ff->size() * 20);
	}

	// Map channels: iterate the container chunks in file order; 0x0959 announces the channel
	// index for the following 0x2394/0x2396 pair.
	{
		int currentChannel = -1;
		for (CStorageContainer::TStorageObjectConstIt it = gb->chunks().begin(); it != gb->chunks().end(); ++it)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (!raw) continue;
			if (it->first == 0x0959 && raw->Value.size() >= 4)
			{
				uint32 chan;
				memcpy(&chan, raw->Value.data(), 4);
				currentChannel = (int)chan;
			}
			else if (it->first == 0x2394 && currentChannel >= 0)
			{
				const void *data;
				uint32 n;
				if (readCountPrefixed(raw, 12, &data, n))
				{
					SMapChannel &mc = out.Maps[currentChannel];
					mc.Verts.resize(n);
					if (n) memcpy(&mc.Verts[0], data, (size_t)n * 12);
				}
			}
			else if (it->first == 0x2396 && currentChannel >= 0)
			{
				const void *data;
				uint32 n;
				if (readCountPrefixed(raw, 12, &data, n))
				{
					SMapChannel &mc = out.Maps[currentChannel];
					mc.Faces.resize(n);
					if (n) memcpy(&mc.Faces[0], data, (size_t)n * 12);
					if (n != out.Faces.size())
						fprintf(stderr, "WARNING: mesh '%s' channel %d face count %u != mesh %u\n",
						        name.c_str(), currentChannel, n, (uint)out.Faces.size());
				}
			}
		}
		// Drop channels that came through incomplete (no faces): they cannot be exported.
		for (std::map<int, SMapChannel>::iterator it = out.Maps.begin(); it != out.Maps.end();)
		{
			if (it->second.Faces.size() != out.Faces.size())
			{
				fprintf(stderr, "WARNING: mesh '%s' channel %d incomplete, dropped\n", name.c_str(), it->first);
				std::map<int, SMapChannel>::iterator dead = it++;
				out.Maps.erase(dead);
			}
			else
				++it;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Modifier application

// Edit Mesh modifier per-node data (mod-app 0x2500 -> 0x2512 -> 0x4000): 0x0100/0x0110 input
// vert/face counts, 0x0140 moved verts, 0x0170/0x0270 deleted vert/face BitArrays, 0x0210
// created verts. (Created faces 0x0410-family not yet decoded — warn when the counts imply some.)
struct SEditMeshEdits
{
	std::vector<std::pair<uint32, Point3M> > Moves;
	std::vector<Point3M> Created;
	std::vector<bool> DelVerts;
	std::vector<bool> DelFaces;
};

static bool readEditMeshBitArray(CStorageContainer *cont, std::vector<bool> &out)
{
	if (!cont) return false;
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != 0x2700) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 4) return false;
		uint32 n;
		memcpy(&n, raw->Value.data(), 4);
		if (raw->Value.size() < 4 + ((size_t)n + 7) / 8) return false;
		out.resize(n);
		for (uint32 i = 0; i < n; ++i)
			out[i] = (raw->Value[4 + i / 8] >> (i % 8)) & 1;
		return true;
	}
	return false;
}

static bool readEditMeshModApp(CStorageContainer *c2500, SEditMeshEdits &out)
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
						memcpy(&n, raw->Value.data(), 4);
						if (raw->Value.size() >= 4 + (size_t)n * 16)
						{
							for (uint32 i = 0; i < n; ++i)
							{
								uint32 idx;
								Point3M v;
								memcpy(&idx, raw->Value.data() + 4 + i * 16, 4);
								memcpy(&v, raw->Value.data() + 4 + i * 16 + 4, 12);
								out.Moves.push_back(std::make_pair(idx, v));
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
						memcpy(&n, raw->Value.data(), 4);
						if (raw->Value.size() >= 4 + (size_t)n * 12)
						{
							out.Created.resize(n);
							if (n) memcpy(&out.Created[0], raw->Value.data() + 4, (size_t)n * 12);
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

// Apply Edit Mesh edits: moves, then face deletes (mesh + map faces in parallel), then vertex
// deletes with reindexing (map verts are indexed independently and stay).
static void applyEditMeshEdits(const SEditMeshEdits &e, SEvalMesh &mesh, const std::string &name)
{
	for (uint i = 0; i < e.Moves.size(); ++i)
	{
		if (e.Moves[i].first < mesh.Verts.size())
		{
			mesh.Verts[e.Moves[i].first].x += e.Moves[i].second.x;
			mesh.Verts[e.Moves[i].first].y += e.Moves[i].second.y;
			mesh.Verts[e.Moves[i].first].z += e.Moves[i].second.z;
		}
	}
	if (!e.DelFaces.empty())
	{
		std::vector<SEvalFace> kept;
		std::map<int, std::vector<SMapFace> > keptMaps;
		for (uint i = 0; i < mesh.Faces.size(); ++i)
		{
			if (i < e.DelFaces.size() && e.DelFaces[i]) continue;
			kept.push_back(mesh.Faces[i]);
			for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
				if (i < mt->second.Faces.size())
					keptMaps[mt->first].push_back(mt->second.Faces[i]);
		}
		mesh.Faces.swap(kept);
		for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
			mt->second.Faces.swap(keptMaps[mt->first]);
	}
	if (!e.DelVerts.empty())
	{
		std::vector<uint32> remap(mesh.Verts.size());
		std::vector<Point3M> kept;
		for (uint i = 0; i < mesh.Verts.size(); ++i)
		{
			remap[i] = (uint32)kept.size();
			if (i >= e.DelVerts.size() || !e.DelVerts[i])
				kept.push_back(mesh.Verts[i]);
		}
		mesh.Verts.swap(kept);
		for (uint i = 0; i < mesh.Faces.size(); ++i)
		{
			mesh.Faces[i].V[0] = remap[mesh.Faces[i].V[0]];
			mesh.Faces[i].V[1] = remap[mesh.Faces[i].V[1]];
			mesh.Faces[i].V[2] = remap[mesh.Faces[i].V[2]];
		}
	}
	if (!e.Created.empty())
	{
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh creates %u vertices (created faces not decoded)\n",
		        name.c_str(), (uint)e.Created.size());
		for (uint i = 0; i < e.Created.size(); ++i)
			mesh.Verts.push_back(e.Created[i]);
	}
}

// XForm modifier: gizmo PRS (modifier reference 0) over the mod-context TM (mod-app 0x2510),
// applied to vertices: v' = v * ctxTM * gizmo * ctxTM^-1 (Max XForm semantics: the gizmo
// transform in the context frame).
static void applyXForm(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name)
{
	Matrix3M gizmo = Matrix3M::identity();
	Matrix3M ctx = Matrix3M::identity();
	CReferenceMaker *mrm = dynamic_cast<CReferenceMaker *>(mod);
	for (uint r = 0; mrm && r < mrm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(mrm->getReference(r));
		if (!ref) continue;
		if (ref->classDesc()->classId() == CLASSID_PRS_CTRL)
		{
			CReferenceMaker *prs = dynamic_cast<CReferenceMaker *>(ref);
			Point3M gp = posValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(0)));
			QuatM gr = rotValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(1)));
			ScaleValueM gs = scaleValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(2)));
			gizmo = composePRS(gp, gr, gs);
		}
	}
	if (app)
	{
		for (CStorageContainer::TStorageObjectConstIt it = app->chunks().begin(); it != app->chunks().end(); ++it)
		{
			if (it->first != 0x2510) continue;
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (raw && raw->Value.size() >= 48)
				memcpy(ctx.m, raw->Value.data(), 48);
		}
	}
	Matrix3M full = ctx * gizmo * inverseM3(ctx);
	for (uint i = 0; i < mesh.Verts.size(); ++i)
	{
		Point3M &v = mesh.Verts[i];
		float x = v.x * full.m[0][0] + v.y * full.m[1][0] + v.z * full.m[2][0] + full.m[3][0];
		float y = v.x * full.m[0][1] + v.y * full.m[1][1] + v.z * full.m[2][1] + full.m[3][1];
		float z = v.x * full.m[0][2] + v.y * full.m[1][2] + v.z * full.m[2][2] + full.m[3][2];
		v.x = x; v.y = y; v.z = z;
	}
	(void)name;
}

// ---------------------------------------------------------------------------------------------

bool evalNodeMesh(INode &node, SEvalMesh &out, std::vector<std::string> *warnings)
{
	std::string name = nodeName(node);
	std::vector<CSceneClass *> mods;
	std::vector<CStorageContainer *> modApps;
	CSceneClass *base = baseObjectOf(node, &mods, &modApps);
	if (!base) return false;

	NLMISC::CClassId cid = base->classDesc()->classId();
	if (cid == CLASSID_EDITABLE_MESH)
	{
		if (!extractEditableMesh(base, out, name))
			return false;
	}
	else
	{
		fprintf(stderr, "WARNING: mesh extraction for object class %s ('%s') not implemented\n",
		        cid.toString().c_str(), name.c_str());
		if (warnings) warnings->push_back("object:" + cid.toString());
		return false;
	}

	// Apply modifiers base-upward: mods was collected outermost wrapper first; within a wrapper,
	// reference order = stack order bottom-up already, so replay back-to-front.
	for (uint i = (uint)mods.size(); i > 0; --i)
	{
		CSceneClass *mod = mods[i - 1];
		CStorageContainer *app = (i - 1) < modApps.size() ? modApps[i - 1] : NULL;
		NLMISC::CClassId mcid = mod->classDesc()->classId();
		if (mcid == NLMISC::CClassId(0x00000050, 0x00000000)) // Edit Mesh
		{
			SEditMeshEdits edits;
			if (app && readEditMeshModApp(app, edits))
				applyEditMeshEdits(edits, out, name);
		}
		else if (mcid == NLMISC::CClassId(0x25215824, 0x00000000)) // XForm
		{
			applyXForm(mod, app, out, name);
		}
		else
		{
			fprintf(stderr, "WARNING: mesh '%s' has unhandled modifier %s; evaluated without it\n",
			        name.c_str(), mcid.toString().c_str());
			if (warnings) warnings->push_back("modifier:" + mcid.toString());
		}
	}
	return true;
}

} /* namespace MESHEVAL */

/* end of file */
