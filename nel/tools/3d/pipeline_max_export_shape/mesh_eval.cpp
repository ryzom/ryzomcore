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
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
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

#include "../pipeline_max_export_common/edit_mesh_mod.h"
#include "../pipeline_max_export_common/old_param_block.h"
#include "../pipeline_max_export_common/parametric_mesh.h"

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
	memcpy(&count, nlVectorData(raw->Value), 4);
	if (raw->Value.size() < 4 + (size_t)count * stride) return false;
	*data = nlVectorData(raw->Value) + 4;
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
				memcpy(&chan, nlVectorData(raw->Value), 4);
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

// EditablePoly: the poly-buffer path. `<PolyObject base data>` (max_geometry_formats Part C) sits
// in the SAME GeomBuffers container the tri path reads, but under distinct ids: 0x0100 = poly
// vertices (CGeomPolyVertexInfo — position + a per-vertex uint32), 0x011a = poly faces
// (CGeomPolyFaceInfo — variable-size records with vertex-list, optional matID / smoothing group /
// triangulation cuts). CGeomObject::triangulatePolyFace already does the corpus-validated
// triangulation (dump tool exportObj drives it — same call used here). One SEvalFace is emitted
// per resulting triangle, materialId + smGroup filled from the parent poly face; per-face flags
// keep matID in the high 16 bits (MAX_FACE_MATID_SHIFT). Map channels are NOT read here yet —
// EditablePoly's MNMesh format stores per-poly map channels differently from the tri path's
// 0x2394/0x2396 pair (Part C notes the MNMesh sub-container is out of scope). Textured
// EditablePoly nodes will therefore diff on UVs until the poly map decode lands; untextured
// ones (collision hulls, primitive stand-ins) match. This is the same practical trade-off
// mesh_eval already makes for parametric primitives (see extractParametricPrimitive above).
static bool extractEditablePoly(CSceneClass *obj, SEvalMesh &out, const std::string &name)
{
	CGeomObject *geom = dynamic_cast<CGeomObject *>(obj);
	STORAGE::CGeomBuffers *gb = geom ? geom->geomBuffers() : NULL;
	if (!gb)
	{
		fprintf(stderr, "WARNING: poly '%s' without geom buffers\n", name.c_str());
		return false;
	}
	const std::vector<STORAGE::CGeomPolyVertexInfo> *pv = gb->polyVertices();
	const std::vector<STORAGE::CGeomPolyFaceInfo> *pf = gb->polyFaces();
	if (!pv || !pf)
	{
		fprintf(stderr, "WARNING: poly '%s' with missing poly buffers\n", name.c_str());
		return false;
	}
	out.Verts.resize(pv->size());
	for (uint i = 0; i < pv->size(); ++i)
	{
		const NLMISC::CVector &v = (*pv)[i].v;
		out.Verts[i].x = v.x;
		out.Verts[i].y = v.y;
		out.Verts[i].z = v.z;
	}
	std::vector<STORAGE::CGeomTriIndex> tris;
	for (uint i = 0; i < pf->size(); ++i)
	{
		const STORAGE::CGeomPolyFaceInfo &face = (*pf)[i];
		if (face.Vertices.size() < 3) continue;
		tris.clear();
		CGeomObject::triangulatePolyFace(tris, face);
		// EditablePoly Material is 0-based when the format stores it, else the packed-bitfield
		// path left it 0 (interpreted the same way as EditableMesh matID). Smoothing group is
		// the poly's bitmask; face-flags carry all-edges-visible (edges 0..2 set) since the poly
		// triangulation subdivides the polygon interior and Max's convention for these interior
		// edges is invisible — but since the export path reads the low 3 bits only to determine
		// edge visibility for shading, we leave them all visible here (matches the interior of a
		// triangulated poly as rendered).
		uint32 flags = ((uint32)(face.Material) & 0xFFFFu) << MAX_FACE_MATID_SHIFT;
		flags |= 0x7; // all edges visible
		for (uint t = 0; t < tris.size(); ++t)
		{
			SEvalFace ef;
			ef.V[0] = tris[t].a;
			ef.V[1] = tris[t].b;
			ef.V[2] = tris[t].c;
			ef.SmGroup = face.SmoothingGroups;
			ef.Flags = flags;
			out.Faces.push_back(ef);
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Modifier application

// Nel VertexTreePaint modifier (nel_vertex_tree_paint plugin, ClassId (0x40c7005e, 0x2a95082c),
// SuperClassId 0x810 OSModifier). Per-node payload (SDK LocalModData) lands under 0x2500 -> 0x2512
// as documented in vertex_tree_paint.cpp:569 (SaveLocalData). Sub-chunks:
//   0x0100 leaf (int32 version, currentVersion == 1)
//   0x0120 leaf (int32 numColors + numColors × COLORREF)
// COLORREF is Win32 DWORD 0x00BBGGRR (little-endian; low byte = R, then G, then B). Applied by
// writing map channel 0 (vertex color) of the current SEvalMesh: numColors matches the mesh's
// vertex count post-XForm/EditMesh; each COLORREF becomes a Point3M(r, g, b) in 0..1.
// Sourced from plugin_max/nel_vertex_tree_paint/vertex_tree_paint.cpp (VERSION_CHUNKID 0x100,
// COLORLIST_CHUNKID 0x120); mod-app framing corpus-verified against ge_mission_maduk.max.
static bool readVertexPaintColors(CStorageContainer *app, std::vector<uint32> &out)
{
	if (!app) return false;
	// Descend into 0x2512 (payload container), find 0x0120 leaf.
	const CStorageContainer::TStorageObjectContainer &c = app->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = c.begin(); it != c.end(); ++it)
	{
		if (it->first != 0x2512) continue;
		CStorageContainer *pl = dynamic_cast<CStorageContainer *>(it->second);
		if (!pl) return false;
		const CStorageContainer::TStorageObjectContainer &pc = pl->chunks();
		for (CStorageContainer::TStorageObjectConstIt jt = pc.begin(); jt != pc.end(); ++jt)
		{
			if (jt->first != 0x0120) continue;
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
			if (!raw || raw->Value.size() < 4) return false;
			uint32 numColors = 0;
			memcpy(&numColors, nlVectorData(raw->Value), 4);
			if (raw->Value.size() < 4 + (size_t)numColors * 4) return false;
			out.resize(numColors);
			memcpy(&out[0], nlVectorData(raw->Value) + 4, (size_t)numColors * 4);
			return true;
		}
		return false;
	}
	return false;
}

static void applyVertexPaint(const std::vector<uint32> &colors, SEvalMesh &mesh, const std::string &name)
{
	if (colors.empty()) return;
	// Nel VertexTreePaint's numColors is per SDK-mesh vertex; on a stack that just ran the base
	// EditableMesh + possible modifiers, current mesh.Verts.size() should equal numColors. If they
	// disagree we bail (mismatch means we don't understand this file's stack) rather than paint
	// wrong colors.
	if (colors.size() != mesh.Verts.size())
	{
		fprintf(stderr, "WARNING: VertexTreePaint on '%s': color count %u != vertex count %u; "
		                "skipping color application\n",
		        name.c_str(), (uint)colors.size(), (uint)mesh.Verts.size());
		return;
	}
	// Map channel 0 = vertex color. If a channel already exists (e.g. base EditableMesh had
	// authored colors), OVERWRITE — Nel VertexTreePaint is a REPLACING modifier, not additive
	// (per the reference plugin's Paint.cpp; the artist re-paints on top of any base colors).
	SMapChannel &ch = mesh.Maps[0];
	ch.Verts.resize(colors.size());
	for (uint i = 0; i < colors.size(); ++i)
	{
		// COLORREF = 0x00BBGGRR little-endian → (r, g, b) as (byte0, byte1, byte2). NeL Point3M
		// vertex color = (R, G, B) as floats 0..1.
		uint32 c = colors[i];
		float r = (c & 0xff) / 255.0f;
		float g = ((c >> 8) & 0xff) / 255.0f;
		float b = ((c >> 16) & 0xff) / 255.0f;
		Point3M p; p.x = r; p.y = g; p.z = b;
		ch.Verts[i] = p;
	}
	// Faces list mirrors the base tri faces: one per SEvalFace, T = V verbatim (per-vertex color,
	// no per-corner independence — this is how SDK vertex-color storage works and how the reference
	// buildMeshInterface pipes it into VBuild for map channel 0).
	ch.Faces.resize(mesh.Faces.size());
	for (uint i = 0; i < mesh.Faces.size(); ++i)
	{
		ch.Faces[i].T[0] = mesh.Faces[i].V[0];
		ch.Faces[i].T[1] = mesh.Faces[i].V[1];
		ch.Faces[i].T[2] = mesh.Faces[i].V[2];
	}
}

// Edit Mesh modifier per-node data (mod-app 0x2500 -> 0x2512 -> 0x4000). Decode + apply live in
// the shared pipeline_max_export_common/edit_mesh_mod library — pipeline_max_design.md §10x
// pinned the chunk map (0x0130 = created verts 16B srcTag+Point3, 0x0208 = created base faces
// 24B srcTag+v3+sm+ff, 0x0220 = face-attribute rewrites incl. matID and per-edge visibility) and
// the shared library carries the corrected decode; the earlier local reader here misread 0x0210
// (map-1 texture-vertex faces, 20B) as vertices at 12B, so any node whose Edit Mesh appended
// verts was silently garbled and any node whose matID was rewritten by 0x0220 shipped the raw
// base's matIDs. Shape adds two things over ig/cmb here: (a) per-face matID + edge-vis rewrites
// need to land in SEvalFace.Flags before deletes/appends (b) map channels must be reordered in
// parallel with the base face-delete pass (ig/cmb don't carry maps).

// Face factory for the shared apply: create an SEvalFace from EDITMESH::SFace. `vOffset` is
// ignored — created-face V[3] already reference the union input+created vert space per the
// shared apply order (see edit_mesh_mod.h). matID lives in the high 16 bits of SEvalFace::Flags
// (MAX_FACE_MATID_SHIFT); edge-vis in the low bits — same packing as MDELTA_CHUNK's 0x0220
// Values word (bits 0..2 = per-edge visibility, bit 3 = hidden, bits 5..20 = matID). Convert
// the packed FaceFlags word verbatim; the corpus base mesh reads the same field layout.
struct SEvalFaceFactory
{
	SEvalFace operator()(uint32 vOffset, const EDITMESH::SFace &sf) const
	{
		(void)vOffset;
		SEvalFace f;
		f.V[0] = sf.V[0];
		f.V[1] = sf.V[1];
		f.V[2] = sf.V[2];
		f.SmGroup = sf.SmGroup;
		f.Flags = sf.FaceFlags;
		return f;
	}
};

// Apply per-face attribute changes (0x0220): rewrite matID + edge-vis + hidden bits inside
// SEvalFace::Flags, gated per record by ApplyMask. Same convention as cmb's applyFaceAttribs;
// unified into shape's Flags word (matID at bits 16..31, edge-vis at bits 0..2, hidden at bit 4).
// FaceAttribs run BEFORE deletes/appends (they index the input face set).
static void applyFaceAttribs(const std::vector<EDITMESH::SFaceAttribChange> &attribs,
                             std::vector<SEvalFace> &faces)
{
	for (uint i = 0; i < attribs.size(); ++i)
	{
		const EDITMESH::SFaceAttribChange &a = attribs[i];
		if (a.Index >= faces.size()) continue;
		SEvalFace &f = faces[a.Index];
		if (a.applyMatId())
		{
			f.Flags &= 0x0000FFFFu;
			f.Flags |= (a.matId() & 0xFFFFu) << MAX_FACE_MATID_SHIFT;
		}
		for (int e = 0; e < 3; ++e)
		{
			if (!a.applyEdge(e)) continue;
			uint32 bit = 1u << e;
			if (a.edgeVis(e))
				f.Flags |= bit;
			else
				f.Flags &= ~bit;
		}
		if (a.applyHidden())
		{
			uint32 bit = 1u << 4;
			if (a.hidden()) f.Flags |= bit;
			else f.Flags &= ~bit;
		}
	}
}

// Apply the decoded Edit Mesh edits to shape's SEvalMesh, keeping the map channels coherent.
// The shared EDITMESH::applyEdits handles the base vertex/face lists (moves → face-deletes →
// append created verts → append created faces (0x0208) → vert-deletes with reindex) but doesn't
// know about map channels; we run the map-face delete pass here in parallel with the base face
// deletes, and each channel keeps its own vert list untouched.
static void applyEditMeshEdits(const EDITMESH::SEdits &e, SEvalMesh &mesh, const std::string &name)
{
	// (a) Face attribute rewrites on the INPUT face set — before deletes/appends.
	applyFaceAttribs(e.FaceAttribs, mesh.Faces);

	// (b) Reorder each map channel's face list in parallel with the base face-delete pass. The
	// shared applyEdits deletes base faces itself; do the same predicate here on the map faces
	// so they stay index-parallel with the base. Map verts (the channel's own vert list) are
	// index-independent and stay.
	std::map<int, std::vector<SMapFace> > keptMapFaces;
	if (!e.DelFaces.empty())
	{
		for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
		{
			std::vector<SMapFace> &kept = keptMapFaces[mt->first];
			kept.reserve(mesh.Faces.size());
			for (uint i = 0; i < mesh.Faces.size(); ++i)
			{
				if (i < e.DelFaces.size() && e.DelFaces[i]) continue;
				if (i < mt->second.Faces.size())
					kept.push_back(mt->second.Faces[i]);
			}
		}
	}

	// (c) Delegate to the shared apply: moves + face-deletes + created-verts + created-faces
	// (0x0208) + vert-delete-and-remap. SEvalMesh::Verts is std::vector<Point3M>; Point3M has
	// the same 12-byte layout as NLMISC::CVector, so a reinterpret-cast is safe here (same
	// pattern the mesh extractor uses for the typed geom buffers → SEvalFace bulk copies).
	std::vector<NLMISC::CVector> &vertsCV
		= *reinterpret_cast<std::vector<NLMISC::CVector> *>(&mesh.Verts);
	EDITMESH::applyEdits<SEvalFace, SEvalFaceFactory>(e, vertsCV, mesh.Faces,
	                                                  SEvalFaceFactory(), /* facesMode = */ 1);

	// (d) Commit the kept map-face lists and, for any created base faces just appended, extend
	// each map channel's face list with a placeholder (index 0 corner). The 0x0210 chunk carries
	// the map-1 texture-vertex faces for created base faces, but its vert indices reference the
	// map-1 texture-vertex space (not the base vert space) — applying it against the base mesh
	// caused edge-inconsistent topology in cmb (§10x); shape would additionally need per-channel
	// tex-vert creation to consume it. For now new faces get UV (0,0,0) — matches the corpus
	// observation that files with Edit-Mesh-created geometry typically carry unmapped materials
	// on the new region (portals/clusters).
	if (!e.DelFaces.empty())
	{
		for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
			mt->second.Faces.swap(keptMapFaces[mt->first]);
	}
	{
		SMapFace zero;
		zero.T[0] = zero.T[1] = zero.T[2] = 0;
		for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
			mt->second.Faces.resize(mesh.Faces.size(), zero);
	}
	if (!e.CreatedFacesA.empty())
	{
		// Warn only when created faces reference a channel that would care — silent when the
		// mesh has no map channels at all.
		if (!mesh.Maps.empty())
			fprintf(stderr, "WARNING: mesh '%s' Edit Mesh appends %u faces; map channels get "
			                "placeholder UVs (0x0210 tex-vert faces not applied)\n",
			        name.c_str(), (uint)e.CreatedFacesA.size());
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
				memcpy(ctx.m, nlVectorData(raw->Value), 48);
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

// Extract a parametric primitive's mesh into SEvalMesh: use the shared PRIMMESH library for
// GT-exact topology, assign all faces smoothing group 1 (Max's default single smoothing group
// for primitives) + material id 0, and leave map channels empty (mapping-coord UV generation
// per primitive type is unimplemented — Box gets 6-face box mapping, Cylinder cylindrical,
// Sphere spherical, Plane planar; formulas depend on the primitive's "generate mapping coords"
// checkbox and its specific dimensions, and the reference builds them via Max's own SDK. Skipped
// here — texture UVs for these prims default to (0,0,0) via buildMeshInterface's empty-channel
// path, which is only "wrong" for textured prims and correct for untextured/collision ones).
static bool extractParametricPrimitive(CSceneClass *base, SEvalMesh &out, const std::string &name)
{
	// Locate the primitive's ref-0 old-style ParamBlock (superclass 0x8).
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(base);
	CSceneClass *pblock = NULL;
	for (uint r = 0; rm && r < rm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
		if (ref && ref->classDesc()->superClassId() == SCLASS_PBLOCK)
		{
			pblock = ref;
			break;
		}
	}
	if (!pblock)
	{
		fprintf(stderr, "WARNING: parametric prim '%s' without ref-0 pblock\n", name.c_str());
		return false;
	}
	std::map<sint32, OLDPBLOCK::SParam> params;
	OLDPBLOCK::readOldParamBlock(pblock, params);

	std::vector<NLMISC::CVector> pverts;
	std::vector<PRIMMESH::SPrimTri> ptris;
	if (!PRIMMESH::buildParametricMesh(base->classDesc()->classId(), params, pverts, ptris))
		return false;

	out.Verts.resize(pverts.size());
	if (!pverts.empty()) memcpy(&out.Verts[0], &pverts[0], pverts.size() * 12);
	out.Faces.resize(ptris.size());
	for (uint i = 0; i < ptris.size(); ++i)
	{
		out.Faces[i].V[0] = ptris[i].V[0];
		out.Faces[i].V[1] = ptris[i].V[1];
		out.Faces[i].V[2] = ptris[i].V[2];
		out.Faces[i].SmGroup = 1; // Max's default single smoothing group for primitives
		out.Faces[i].Flags = 0;    // matID 0, all edges visible, not hidden
	}
	return true;
}

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
	else if (cid == CLASSID_EDITABLE_POLY)
	{
		if (!extractEditablePoly(base, out, name))
			return false;
	}
	else if (cid == PRIMMESH::CLASSID_BOX || cid == PRIMMESH::CLASSID_CYLINDER
	         || cid == PRIMMESH::CLASSID_SPHERE || cid == PRIMMESH::CLASSID_PLANE)
	{
		if (!extractParametricPrimitive(base, out, name))
			return false;
	}
	else
	{
		// Shape-superclass base objects (SplineShape 0x0a, Line 0x1040, Rectangle 0x1065) are
		// splines, not meshes — the reference plugin's buildShape produces no `.shape` for them
		// unless routed to `buildRemanence` (USE_REMANENCE appdata gated in the caller); a plain
		// SplineShape without that appdata gets a fall-through NULL. Report them as a distinct
		// warn class so the harness bucket doesn't hide them under "mesh-eval" (which is meant for
		// genuine GeomObject-class decode gaps).
		TSClassId scid = base->classDesc()->superClassId();
		if (scid == SCLASS_SHAPE)
		{
			fprintf(stderr, "WARNING: shape-class base object %s ('%s') — spline extraction not "
			                "implemented (SplineShape/Line/Rectangle would need CSegRemanence or a "
			                "dedicated spline decode; §10z-bis open item)\n",
			        cid.toString().c_str(), name.c_str());
			if (warnings) warnings->push_back("shape-class:" + cid.toString());
		}
		else
		{
			fprintf(stderr, "WARNING: mesh extraction for object class %s ('%s') not implemented\n",
			        cid.toString().c_str(), name.c_str());
			if (warnings) warnings->push_back("object:" + cid.toString());
		}
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
			EDITMESH::SEdits edits;
			if (app && EDITMESH::readModApp(app, edits))
				applyEditMeshEdits(edits, out, name);
		}
		else if (mcid == NLMISC::CClassId(0x25215824, 0x00000000)) // XForm
		{
			applyXForm(mod, app, out, name);
		}
		else if (mcid == NLMISC::CClassId(0x40c7005e, 0x2a95082c)) // Nel VertexTreePaint
		{
			std::vector<uint32> colors;
			if (readVertexPaintColors(app, colors))
				applyVertexPaint(colors, out, name);
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
