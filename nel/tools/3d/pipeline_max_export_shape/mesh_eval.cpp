/**
 * \file mesh_eval.cpp
 * \brief See mesh_eval.h.
 *
 * Mesh chunk decode (GeomBuffers container 0x08FE on the geom object, corpus-established):
 *   0x0906 u32 mesh flags; 0x0908 f32; 0x0914 u32 count + count x Point3 vertices;
 *   0x0912 u32 count + count x { u32 v0, v1, v2; u32 smGroup; u32 faceFlags } — faceFlags is
 *   the Max Face::flags dword: edge-visibility low bits, matID in the HIGH WORD;
 *   0x0924/0x0928/0x092a u32s; 0x092c/0x092d/0x092e BitArray containers (selections);
 *   map channels, repeated per stored channel IN FILE ORDER (typed in the library since
 *   §10j-neuf — CGeomBuffers::mapChannels()):
 *     0x0959 u32 = channel index (1.. UVW, 0 = vertex color);
 *     0x2398 u32 (support flag, 1 in the corpus);
 *     0x2394 u32 count + count x Point3 map vertices;
 *     0x2396 u32 count + count x (u32 t0, t1, t2) map faces (parallel to mesh faces);
 *   0x0952/0x0953/0x0956/0x0958/0x094c vertex-data channels (soft selection floats etc.) — not
 *   needed, stay raw.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * \author Grok 4.5
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

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <set>

#include <nel/misc/common.h>

#include "../pipeline_max/builtin/geom_object.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_transform.h"
#include "../pipeline_max/builtin/param_block.h"
#include "../pipeline_max/update1/editable_mesh.h"

#include "../pipeline_max_export_common/edit_mesh_mod.h"
#include "../pipeline_max_export_common/map_extender_mod.h"
#include "../pipeline_max_export_common/old_param_block.h"
#include "../pipeline_max_export_common/parametric_mesh.h"
#include "../pipeline_max_export_common/spline_mesh.h"
#include "../pipeline_max_export_common/spline_shape.h"
#include "../pipeline_max_export_common/uvw_map_mod.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;

namespace MESHEVAL {

// ---------------------------------------------------------------------------------------------
// GeomBuffers (Mesh) decode

// A bit-array chunk: a container holding one 0x2700 leaf, uint32 bit count then the bits, 32 per dword,
// least significant first.
static bool readBitArray(const CStorageContainer *parent, uint16 id, std::vector<bool> &bits)
{
	const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(parent->findStorageObject(id));
	const CStorageRaw *raw = c ? dynamic_cast<const CStorageRaw *>(c->findStorageObject(0x2700)) : nullptr;
	if (!raw || raw->Value.size() < 4) return false;
	uint32 n;
	memcpy(&n, &raw->Value[0], 4);
	if (raw->Value.size() < 4 + (((size_t)n + 31) / 32) * 4) return false;
	bits.resize(n);
	for (uint32 i = 0; i < n; ++i)
		bits[i] = ((raw->Value[4 + i / 8] >> (i % 8)) & 1) != 0;
	return true;
}

// The selection the mesh was saved with: its level (0x092a: 1 object, 2 vertex, 4 face, 8 edge) and the
// selections of every level (0x092c vertices, 0x092d faces, 0x092e edges, 3 per face), which are stored at
// every level and only scope the modifiers at their own level.
static void readMeshSelection(CSceneClass *obj, const STORAGE::CGeomBuffers *gb, SEvalMesh &out)
{
	const CStorageRaw *lvl = dynamic_cast<const CStorageRaw *>(gb->findStorageObject(0x092a));
	out.SelLevel = 1;
	if (lvl && lvl->Value.size() >= 4)
		memcpy(&out.SelLevel, &lvl->Value[0], 4);
	static const uint16 s_Ids[] = { 0x092c, 0x092d, 0x092e };
	std::vector<bool> *const sels[] = { &out.SelVerts, &out.SelFaces, &out.SelEdges };
	for (uint i = 0; i < 3; ++i)
		if (gb->findStorageObject(s_Ids[i]) && !readBitArray(gb, s_Ids[i], *sels[i]))
			out.SelBitsUnknown = true;
	PIPELINE::MAX::UPDATE1::CEditableMesh *em = dynamic_cast<PIPELINE::MAX::UPDATE1::CEditableMesh *>(obj);
	out.SelSoft = em && em->softSelection();
}

// The vertices the current selection covers, one flag per vertex: the selected vertices, or the vertices of
// the selected faces or edges. False when the level is unknown or its selection does not match the mesh.
static bool selectedVertices(const SEvalMesh &mesh, std::vector<bool> &verts)
{
	const uint nv = (uint)mesh.Verts.size();
	const uint nf = (uint)mesh.Faces.size();
	if (mesh.SelLevel == 2 && mesh.SelVerts.size() == nv)
	{
		verts = mesh.SelVerts;
		return true;
	}
	if (mesh.SelLevel == 4 && mesh.SelFaces.size() == nf)
	{
		verts.assign(nv, false);
		for (uint f = 0; f < nf; ++f)
			if (mesh.SelFaces[f])
				for (uint c = 0; c < 3; ++c)
					if (mesh.Faces[f].V[c] < nv) verts[mesh.Faces[f].V[c]] = true;
		return true;
	}
	if (mesh.SelLevel == 8 && mesh.SelEdges.size() == nf * 3)
	{
		verts.assign(nv, false);
		for (uint e = 0; e < nf * 3; ++e)
		{
			if (!mesh.SelEdges[e]) continue;
			const SEvalFace &face = mesh.Faces[e / 3];
			uint a = face.V[e % 3], b = face.V[(e % 3 + 1) % 3];
			if (a < nv) verts[a] = true;
			if (b < nv) verts[b] = true;
		}
		return true;
	}
	return false;
}

// A selection bit array of an Edit Mesh delta: absent means nothing selected (count bits, all clear); false
// when the chunk is there but not readable.
static bool readDeltaSelection(const CStorageContainer *delta, uint16 id, size_t count, std::vector<bool> &bits)
{
	if (!delta->findStorageObject(id))
	{
		bits.assign(count, false);
		return true;
	}
	return readBitArray(delta, id, bits);
}

// The selection an Edit Mesh hands up the stack. Its level is the modifier's 0x2800 (int16: 0 object,
// 1 vertex, 2 edge, 3 face, 4 polygon, 5 element) and its soft selection the modifier's 0x2820 (on at a
// sub-object level only). The per-node data (mod-app 0x2512) has flags 0x2740, whose bit 2 says that the
// delta 0x4000 holds data for this object; then the delta stores the selections over the output mesh as bit
// arrays, 0x0400 vertices, 0x0410 edges (3 per face) and 0x0420 faces, each absent when nothing is selected.
// Without that bit, the Edit Mesh was never edited on this object and passes the selections below through.
// Validated on zo_cn_mairie.max (zo_mairie_globes, both cases).
static void applyEditMeshSelection(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh)
{
	const CStorageRaw *lvl = findRawChunk(mod, 0x2800);
	sint16 level = -1;
	if (lvl && lvl->Value.size() >= 2)
		memcpy(&level, &lvl->Value[0], 2);
	static const uint32 s_Levels[] = { 1, 2, 8, 4, 4, 4 };
	if (level < 0 || level > 5)
	{
		mesh.SelLevelUnknown = mesh.SelBitsUnknown = true;
		return;
	}
	mesh.SelLevel = s_Levels[level];
	mesh.SelLevelUnknown = false;
	const CStorageRaw *ar = findRawChunk(mod, 0x2820);
	uint32 soft = 0;
	if (ar && ar->Value.size() >= 4)
		memcpy(&soft, &ar->Value[0], 4);
	const bool ownSoft = soft != 0 && level != 0;
	const CStorageContainer *data = app ? dynamic_cast<const CStorageContainer *>(
		CDerivedObject::modAppLocalModData(app)) : nullptr;
	const CStorageRaw *flags = data ? dynamic_cast<const CStorageRaw *>(data->findStorageObject(0x2740)) : nullptr;
	if (!flags || flags->Value.size() < 4)
	{
		mesh.SelBitsUnknown = true;
		mesh.SelSoft = mesh.SelSoft || ownSoft;
		return;
	}
	uint32 f;
	memcpy(&f, &flags->Value[0], 4);
	if (!(f & 4))
	{
		mesh.SelSoft = mesh.SelSoft || ownSoft; // not edited here: the input selections pass through
		return;
	}
	mesh.SelSoft = ownSoft;
	const CStorageContainer *delta = dynamic_cast<const CStorageContainer *>(data->findStorageObject(0x4000));
	mesh.SelBitsUnknown = !delta
		|| !readDeltaSelection(delta, 0x0400, mesh.Verts.size(), mesh.SelVerts)
		|| !readDeltaSelection(delta, 0x0410, mesh.Faces.size() * 3, mesh.SelEdges)
		|| !readDeltaSelection(delta, 0x0420, mesh.Faces.size(), mesh.SelFaces);
}

static bool extractEditableMesh(CSceneClass *obj, SEvalMesh &out, const std::string &name)
{
	CGeomObject *geom = dynamic_cast<CGeomObject *>(obj);
	STORAGE::CGeomBuffers *gb = geom ? geom->geomBuffers() : nullptr;
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
	readMeshSelection(obj, gb, out);

	// Map channels: the typed CGeomBuffers map-channel view (chunk groups in file order; a group
	// whose 0x0959 announce is absent keeps Channel -1 and is dropped, matching the historical
	// raw read). CVector/CGeomTriIndex share the byte layout of Point3M/SMapFace, so the bulk
	// copies reproduce the exact bytes the raw path used.
	{
		std::vector<STORAGE::CGeomBuffers::CMapChannelView> channels;
		gb->mapChannels(channels);
		for (uint c = 0; c < channels.size(); ++c)
		{
			const STORAGE::CGeomBuffers::CMapChannelView &ch = channels[c];
			if (ch.Channel < 0) continue;
			if (ch.Verts)
			{
				SMapChannel &mc = out.Maps[ch.Channel];
				mc.Verts.resize(ch.Verts->size());
				if (!ch.Verts->empty()) memcpy(&mc.Verts[0], &(*ch.Verts)[0], ch.Verts->size() * 12);
			}
			if (ch.Faces)
			{
				SMapChannel &mc = out.Maps[ch.Channel];
				mc.Faces.resize(ch.Faces->size());
				if (!ch.Faces->empty()) memcpy(&mc.Faces[0], &(*ch.Faces)[0], ch.Faces->size() * 12);
				if (ch.Faces->size() != out.Faces.size())
					fprintf(stderr, "WARNING: mesh '%s' channel %d face count %u != mesh %u\n",
					        name.c_str(), (int)ch.Channel, (uint)ch.Faces->size(), (uint)out.Faces.size());
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
	STORAGE::CGeomBuffers *gb = geom ? geom->geomBuffers() : nullptr;
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
	// Descend into 0x2512 (the LocalModData payload container), find the 0x0120 leaf.
	CStorageContainer *pl = dynamic_cast<CStorageContainer *>(CDerivedObject::modAppLocalModData(app));
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
// the shared pipeline_max_export_common/edit_mesh_mod library (0x0130 = created verts 16B
// srcTag+float[3], 0x0208 = created base faces 24B srcTag+v3+sm+ff, 0x0210 = face-vertex remap,
// 0x0220 = face-attribute rewrites incl. matID and per-edge visibility). Shape adds over ig/cmb:
// (a) per-face matID + edge-vis rewrites and the 0x0230 smoothing-group changes land in SEvalFace
// before deletes/appends, (b) the map channel edits (0x0320 groups: map vert sets and appends, the
// map face remap, the map faces of the created faces) and the map faces follow the base face
// deletes (ig/cmb don't carry maps).

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
// know about map channels; the channel edits run here on the input map faces, the map-face delete
// pass runs in parallel with the base face deletes, and the created faces get their map faces from
// the channel's group. A vert delete does not touch the map verts.
static void applyEditMeshEdits(const EDITMESH::SEdits &e, SEvalMesh &mesh, const std::string &name)
{
	const uint inFaces = (uint)mesh.Faces.size();
	uint outOfRange = 0;
	uint dropped = 0; // map channel records on input map verts or faces the channel does not have
	if (!e.IrregularIds.empty())
	{
		std::string ids;
		for (uint i = 0; i < e.IrregularIds.size(); ++i)
			ids += NLMISC::toString("%s0x%04x", i ? ", " : "", (uint)e.IrregularIds[i]);
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh data has unreadable chunks %s; their records are not applied\n",
		        name.c_str(), ids.c_str());
	}
	// The mesh the delta was recorded on against the one evaluated here: when they differ, the edits land
	// on other vertices and faces than intended (an unevaluated modifier below, or the asset changed below
	// the Edit Mesh after it was edited).
	// An Edit Mesh never edited on this object records 0 and 0 and has nothing to apply.
	const bool recorded = e.HasInputCounts && (e.InputVertCount || e.InputFaceCount);
	const bool geometryMatches = !recorded || (e.InputVertCount == mesh.Verts.size() && e.InputFaceCount == inFaces);
	if (!geometryMatches)
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh was recorded on %u vertices and %u faces, the evaluated mesh has "
		                "%u and %u\n",
		        name.c_str(), e.InputVertCount, e.InputFaceCount, (uint)mesh.Verts.size(), inFaces);

	// (a) Face attribute rewrites and smoothing-group changes on the INPUT face set, before
	// deletes/appends.
	applyFaceAttribs(e.FaceAttribs, mesh.Faces);
	for (uint i = 0; i < e.FaceSmooth.size(); ++i)
	{
		const EDITMESH::SFaceSmoothChange &s = e.FaceSmooth[i];
		if (s.Index >= inFaces) { ++outOfRange; continue; }
		SEvalFace &f = mesh.Faces[s.Index];
		f.SmGroup = (f.SmGroup & ~s.Mask) | (s.Values & s.Mask);
	}

	// (a2) Map channel edits on the input map verts and faces. The group applies to the channel as it
	// reaches this modifier even when that has another map vert count than the one recorded (the
	// original modifier works that way): the created map verts follow the actual input map verts, so a
	// recorded index at or past the recorded count moves with them, and a record on an input map vert or
	// face the channel does not have is dropped. A channel the delta carries but the mesh does not have
	// starts in its default form, with the mesh's topology: white for vertex colour, the vertex
	// positions for the others.
	struct SCarried
	{
		const EDITMESH::SMapEdits *M;
		uint32 InVNum; // the actual input map vert count
		// a recorded map vert index in the output of this group, or -1 when there is none
		sint64 adjust(uint32 i) const
		{
			if (i >= M->VertCount) return (sint64)i - M->VertCount + InVNum;
			return i < InVNum ? (sint64)i : -1;
		}
	};
	std::map<int, SCarried> carried; // channels whose created faces have map faces
	std::set<int> seenChannels;
	for (uint g = 0; g < e.Maps.size(); ++g)
	{
		const EDITMESH::SMapEdits &m = e.Maps[g];
		if (m.VertSets.empty() && m.CreatedVerts.empty() && m.CreatedFaces.empty() && m.FaceRemap.empty()
		    && e.CreatedFacesA.empty())
			continue; // a group without records changes nothing
		if (!m.ChannelKnown)
			continue; // reported with the unreadable chunks above
		if (!seenChannels.insert(m.Channel).second)
		{
			fprintf(stderr, "WARNING: mesh '%s' Edit Mesh has a second group for map channel %d; it is not applied\n",
			        name.c_str(), (int)m.Channel);
			continue;
		}
		if (m.VertCount == 0xFFFFFFFF || m.FaceCount == 0xFFFFFFFF)
		{
			fprintf(stderr, "WARNING: mesh '%s' Edit Mesh map channel %d without its input counts; the channel edits "
			                "are not applied\n", name.c_str(), (int)m.Channel);
			continue;
		}
		if (m.CreatedFaces.size() != e.CreatedFacesA.size())
		{
			fprintf(stderr, "WARNING: mesh '%s' Edit Mesh map channel %d has %u created map faces for %u created "
			                "faces; the channel edits are not applied\n",
			        name.c_str(), (int)m.Channel, (uint)m.CreatedFaces.size(), (uint)e.CreatedFacesA.size());
			continue;
		}
		std::map<int, SMapChannel>::iterator mt = mesh.Maps.find(m.Channel);
		if (mt == mesh.Maps.end())
		{
			SMapChannel &ch = mesh.Maps[m.Channel];
			ch.Verts.resize(mesh.Verts.size());
			for (uint v = 0; v < mesh.Verts.size(); ++v)
			{
				if (m.Channel == 0) { ch.Verts[v].x = ch.Verts[v].y = ch.Verts[v].z = 1.f; }
				else ch.Verts[v] = mesh.Verts[v];
			}
			ch.Faces.resize(inFaces);
			for (uint f = 0; f < inFaces; ++f)
				for (uint c = 0; c < 3; ++c)
					ch.Faces[f].T[c] = mesh.Faces[f].V[c];
			mt = mesh.Maps.find(m.Channel);
		}
		SMapChannel &ch = mt->second;
		if (ch.Faces.size() != inFaces)
		{
			fprintf(stderr, "WARNING: mesh '%s' map channel %d has %u map faces on %u faces; the Edit Mesh channel "
			                "edits are not applied\n",
			        name.c_str(), (int)m.Channel, (uint)ch.Faces.size(), inFaces);
			continue;
		}
		SCarried cr;
		cr.M = &m;
		cr.InVNum = (uint32)ch.Verts.size();
		if (geometryMatches && cr.InVNum != m.VertCount)
			fprintf(stderr, "WARNING: mesh '%s' Edit Mesh map channel %d was recorded on %u map verts, the incoming "
			                "channel has %u; applied with the created map verts after those (as the modifier does)\n",
			        name.c_str(), (int)m.Channel, m.VertCount, cr.InVNum);
		for (uint i = 0; i < m.CreatedVerts.size(); ++i)
		{
			MAXMATH::Point3M p;
			p.x = m.CreatedVerts[i].x;
			p.y = m.CreatedVerts[i].y;
			p.z = m.CreatedVerts[i].z;
			ch.Verts.push_back(p);
		}
		for (uint i = 0; i < m.VertSets.size(); ++i)
		{
			const sint64 t = cr.adjust(m.VertSets[i].first);
			if (t < 0) { ++dropped; continue; }
			if ((size_t)t >= ch.Verts.size()) { ++outOfRange; continue; }
			MAXMATH::Point3M &p = ch.Verts[(size_t)t];
			p.x = m.VertSets[i].second.x;
			p.y = m.VertSets[i].second.y;
			p.z = m.VertSets[i].second.z;
		}
		for (uint i = 0; i < m.FaceRemap.size(); ++i)
		{
			const EDITMESH::SFaceVertRemap &r = m.FaceRemap[i];
			if (r.Index >= ch.Faces.size()) { ++dropped; continue; }
			for (int c = 0; c < 3; ++c)
			{
				if (!r.applyCorner(c)) continue;
				const sint64 t = cr.adjust(r.V[c]);
				if (t < 0) { ++dropped; continue; }
				if ((size_t)t >= ch.Verts.size()) { ++outOfRange; continue; }
				ch.Faces[r.Index].T[c] = (uint32)t;
			}
		}
		carried[m.Channel] = cr;
	}

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

	// (d) Commit the kept map-face lists, then give the created base faces just appended their map
	// faces from the channel's group. A channel the delta does not carry (or whose group was not
	// applied above) has no map faces for them: those get a placeholder (map vert 0) and a warning.
	if (!e.DelFaces.empty())
	{
		for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
			mt->second.Faces.swap(keptMapFaces[mt->first]);
	}
	std::string placeholders;
	for (std::map<int, SMapChannel>::iterator mt = mesh.Maps.begin(); mt != mesh.Maps.end(); ++mt)
	{
		SMapChannel &ch = mt->second;
		std::map<int, SCarried>::const_iterator ct = carried.find(mt->first);
		if (ct != carried.end() && ch.Faces.size() + ct->second.M->CreatedFaces.size() == mesh.Faces.size())
		{
			const std::vector<EDITMESH::SMapFaceIdx> &created = ct->second.M->CreatedFaces;
			for (uint i = 0; i < created.size(); ++i)
			{
				SMapFace f;
				for (int c = 0; c < 3; ++c)
				{
					const sint64 t = ct->second.adjust(created[i].T[c]);
					if (t < 0) { ++dropped; f.T[c] = 0; }
					else if ((size_t)t >= ch.Verts.size()) { ++outOfRange; f.T[c] = 0; }
					else f.T[c] = (uint32)t;
				}
				ch.Faces.push_back(f);
			}
			continue;
		}
		if (ch.Faces.size() < mesh.Faces.size() && !e.CreatedFacesA.empty())
			placeholders += NLMISC::toString("%s%d", placeholders.empty() ? "" : ", ", mt->first);
		SMapFace zero;
		zero.T[0] = zero.T[1] = zero.T[2] = 0;
		ch.Faces.resize(mesh.Faces.size(), zero);
	}
	if (!placeholders.empty())
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh appends %u faces without map faces on channel %s; "
		                "they get map vert 0 there\n",
		        name.c_str(), (uint)e.CreatedFacesA.size(), placeholders.c_str());
	if (outOfRange)
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh has %u smoothing, map vert or map face records outside "
		                "the evaluated mesh; they are not applied\n",
		        name.c_str(), outOfRange);
	if (dropped && geometryMatches) // else the geometry warning above covers it
		fprintf(stderr, "WARNING: mesh '%s' Edit Mesh has %u map channel records on map verts or faces the incoming "
		                "channel does not have (recorded on another input); dropped, as the modifier does\n",
		        name.c_str(), dropped);
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
		if (CControlPRS *prs = dynamic_cast<CControlPRS *>(mrm->getReference(r)))
		{
			Point3M gp = posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController()));
			QuatM gr = rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController()));
			ScaleValueM gs = scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController()));
			gizmo = composePRS(gp, gr, gs);
		}
	}
	CDerivedObject::modAppContextTM(app, &ctx.m[0][0]);
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
// Gizmo deformers (Bend, Twist, Taper): reference 0 is the gizmo (a PRS controller), reference 1 the center (a
// position controller), reference 2 the parameters, a ParamBlock2 or, in older files, an old ParamBlock
// with the same parameter indices. The deformer maps a point into its space (Tm), deforms it there along
// Z, and maps it back (InvTm); it moves the vertices of the current selection only. Validated against
// reference exports, positions equal to float precision: zo_hom_cheveux_lead.max (two Bends and a Twist
// on a vertex selection), neutrecreature.max (a Twist with a moved center, on three meshes) and
// zo_cn_mairie.max (zo_mairie_globes: a Bend, a Twist and three Tapers on Edit Mesh selections).

// The constant parameters of reference 2, by index. False when any parameter is animated
// (controller-backed), even one the settings leave unused, or the block is missing: the caller then
// leaves the modifier out.
static bool readGizmoModParams(CSceneClass *mod, std::map<sint32, float> &out, const std::string &name)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CSceneClass *pb = rm && rm->nbReferences() > 2 ? dynamic_cast<CSceneClass *>(rm->getReference(2)) : nullptr;
	if (!pb)
		return false;
	if (pb->classDesc()->superClassId() == 0x82)
	{
		SPB2Block blk;
		if (!readPB2Block(pb, blk))
			return false;
		for (std::map<uint16, SPB2Param>::const_iterator it = blk.Params.begin(); it != blk.Params.end(); ++it)
		{
			const SPB2Param &p = it->second;
			if (p.IsTab)
				continue;
			if (!p.HasConstant)
			{
				fprintf(stderr, "WARNING: mesh '%s' modifier parameter %u is animated, not supported\n",
				        name.c_str(), (uint)p.Id);
				return false;
			}
			out[(sint32)p.Id] = (p.Type == PB2_INT || p.Type == PB2_BOOL) ? (float)p.I : p.F[0];
		}
		return true;
	}
	if (pb->classDesc()->superClassId() == SCLASS_PBLOCK)
	{
		typedef PIPELINE::MAX::BUILTIN::CParamBlock CParamBlock;
		CParamBlock *opb = dynamic_cast<CParamBlock *>(pb);
		if (!opb)
			return false;
		const std::vector<CParamBlock::SParam> &params = opb->params();
		for (std::vector<CParamBlock::SParam>::const_iterator it = params.begin(); it != params.end(); ++it)
		{
			if (it->Index < 0)
				continue;
			if (!it->HasConstant)
			{
				fprintf(stderr, "WARNING: mesh '%s' modifier parameter %i is animated, not supported\n",
				        name.c_str(), (int)it->Index);
				return false;
			}
			const bool isInt = it->Kind == CParamBlock::KindInt || it->Kind == CParamBlock::KindBool;
			out[it->Index] = isInt ? (float)it->I : it->F[0];
		}
		return true;
	}
	return false;
}

static bool hasParams(const std::map<sint32, float> &p, sint32 count)
{
	for (sint32 i = 0; i < count; ++i)
		if (p.find(i) == p.end())
			return false;
	return true;
}

// Post-multiply by a rotation about X, Y or Z (row vectors, v' = v * M).
static Matrix3M rotateM(const Matrix3M &m, int axis, double angle)
{
	return rotateAxisM3(m, axis, angle);
}

static Matrix3M translateM(const Matrix3M &m, float x, float y, float z)
{
	Matrix3M r = m;
	r.m[3][0] += x; r.m[3][1] += y; r.m[3][2] += z;
	return r;
}

// The deformer space: object space -> mod context (the 0x2510 TM) -> gizmo -> center,
// Tm = context * Inverse(Translate(center) * gizmo), then the deformer's axis matrix, which turns its
// axis onto Z (Tm = Tm * axis, InvTm = Inverse(axis) * Inverse(Tm)).
// The order of a moved gizmo against the center is not validated yet: the reference files have the gizmo
// at its default.
struct SGizmoModSpace
{
	Matrix3M Tm, InvTm;
	float Box[6]; // the mod-context box (0x2511), min then max
};

// False when the box is missing or empty (min above max, as stored for a modifier applied to an empty
// selection): the deformers take their extent from it and would throw the vertices far away.
static bool gizmoModSpace(CSceneClass *mod, CStorageContainer *app, const Matrix3M &axis, SGizmoModSpace &out)
{
	Matrix3M gizmo = Matrix3M::identity();
	Point3M center = { 0.f, 0.f, 0.f };
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	if (rm && rm->nbReferences() > 0)
	{
		if (CControlPRS *prs = dynamic_cast<CControlPRS *>(rm->getReference(0)))
		{
			gizmo = composePRS(posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController())),
			                   rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController())),
			                   scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController())));
		}
	}
	if (rm && rm->nbReferences() > 1)
	{
		if (CSceneClass *pc = dynamic_cast<CSceneClass *>(rm->getReference(1)))
			center = posValueAt0(pc);
	}
	Matrix3M ctx = Matrix3M::identity();
	CDerivedObject::modAppContextTM(app, &ctx.m[0][0]);
	Matrix3M centered = translateM(Matrix3M::identity(), center.x, center.y, center.z) * gizmo;
	Matrix3M tm = ctx * inverseM3(centered);
	out.Tm = tm * axis;
	out.InvTm = inverseM3(axis) * inverseM3(tm);
	if (!CDerivedObject::modAppContextBox(app, out.Box))
		return false;
	for (int i = 0; i < 3; ++i)
		if (out.Box[3 + i] < out.Box[i])
			return false;
	return true;
}

// The axis matrix: the bend or twist axis turned onto Z (0 = X: -pi/2 about Y, 1 = Y: pi/2 about X).
static Matrix3M gizmoModAxis(int axis)
{
	if (axis == 0) return rotateM(Matrix3M::identity(), 1, -NLMISC::Pi * 0.5);
	if (axis == 1) return rotateM(Matrix3M::identity(), 0, NLMISC::Pi * 0.5);
	return Matrix3M::identity();
}

// The vertices a modifier acting on the current selection moves: true for the whole mesh (object level,
// or a selection that is not known, with a warning), else false with \a sel holding one flag per vertex.
static bool modifierSelection(const SEvalMesh &mesh, std::vector<bool> &sel, const char *what,
                              const std::string &name, std::vector<std::string> *warnings)
{
	bool all = mesh.SelLevel == 1;
	if (mesh.SelLevelUnknown || (!all && (mesh.SelBitsUnknown || !selectedVertices(mesh, sel))))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s applied to the whole mesh: the selection it sees is not known\n",
		        name.c_str(), what);
		if (warnings) warnings->push_back(std::string("selection:") + what);
		return true;
	}
	if (!all && mesh.SelSoft)
	{
		fprintf(stderr, "WARNING: mesh '%s' %s applied to the selected vertices only: the soft selection falloff "
		        "around them is not evaluated\n", name.c_str(), what);
		if (warnings) warnings->push_back(std::string("softselection:") + what);
	}
	return all;
}

// Move the selected vertices (all when the selection is the whole mesh) through \a deformer.
template <class TDeformer>
static void deformSelected(SEvalMesh &mesh, const TDeformer &deformer, const char *what, const std::string &name,
                           std::vector<std::string> *warnings)
{
	std::vector<bool> sel;
	const bool all = modifierSelection(mesh, sel, what, name, warnings);
	for (uint i = 0; i < mesh.Verts.size(); ++i)
		if (all || sel[i])
			mesh.Verts[i] = deformer.map(mesh.Verts[i]);
}

struct SBendDeformer
{
	Matrix3M Tm, InvTm, TmPastUpper, TmPastLower;
	double Radius, Lower, Upper;
	bool Limited;

	Point3M map(Point3M p) const
	{
		if (Radius == 0 && !Limited) return p;
		p = transformPoint(p, Tm);
		if (Limited)
		{
			if (p.z < Lower) return transformPoint(transformPoint(p, TmPastLower), InvTm);
			if (p.z > Upper) return transformPoint(transformPoint(p, TmPastUpper), InvTm);
		}
		if (Radius == 0) return transformPoint(p, InvTm);
		double x = p.x, yr = p.z / Radius;
		double c = cos(NLMISC::Pi - yr), s = sin(NLMISC::Pi - yr);
		p.x = (float)(Radius * c + Radius - x * c);
		p.z = (float)(Radius * s - x * s);
		return transformPoint(p, InvTm);
	}
};

static void applyBend(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                      std::vector<std::string> *warnings)
{
	// 0 angle (degrees), 1 direction (degrees), 2 axis (0 X, 1 Y, 2 Z), 3 limits on, 4 lower limit, 5 upper limit
	std::map<sint32, float> p;
	if (!readGizmoModParams(mod, p, name) || !hasParams(p, 3))
	{
		fprintf(stderr, "WARNING: mesh '%s' Bend parameters not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:bend");
		return;
	}
	const float angle = p[0], dir = p[1];
	const int naxis = (int)p[2];
	SBendDeformer d;
	d.Limited = p.count(3) && p[3] != 0.f;
	d.Lower = p.count(4) ? p[4] : 0.f;
	d.Upper = p.count(5) ? p[5] : 0.f;
	Matrix3M axis = rotateM(gizmoModAxis(naxis), 2, dir * NLMISC::Pi / 180.0);
	SGizmoModSpace space;
	if (!gizmoModSpace(mod, app, axis, space))
	{
		fprintf(stderr, "WARNING: mesh '%s' Bend without a usable mod-context box; evaluated without it\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:bend");
		return;
	}
	d.Tm = space.Tm;
	d.InvTm = space.InvTm;
	// the bend radius: the arc length (the box extent along the bend axis, or the limit range) over the angle
	const int ax = naxis < 0 || naxis > 2 ? 2 : naxis;
	const double len = d.Limited ? d.Upper - d.Lower : (double)space.Box[3 + ax] - (double)space.Box[ax];
	const double rad = angle * NLMISC::Pi / 180.0;
	d.Radius = fabs(rad) < 0.000001 ? 0.0 : len / rad;
	if (d.Limited)
	{
		// beyond the limits the part continues straight, turned and moved like the limit end; the limit
		// ends are mapped with the limits off
		d.Limited = false;
		const double range = d.Upper - d.Lower;
		const double upperShare = range == 0.0 ? 1.0 : d.Upper / range;
		const double lowerShare = range == 0.0 ? 1.0 : d.Lower / range;
		for (int k = 0; k < 2; ++k)
		{
			const float z = (float)(k == 0 ? d.Upper : d.Lower);
			Matrix3M t = translateM(Matrix3M::identity(), 0.f, 0.f, -z);
			t = rotateM(t, 1, angle * (k == 0 ? upperShare : lowerShare) * NLMISC::Pi / 180.0);
			t = translateM(t, 0.f, 0.f, z);
			Point3M pt = { 0.f, 0.f, z };
			Point3M mapped = transformPoint(d.map(transformPoint(pt, d.InvTm)), d.Tm);
			t = translateM(t, mapped.x - pt.x, mapped.y - pt.y, mapped.z - pt.z);
			if (k == 0)
				d.TmPastUpper = t;
			else
				d.TmPastLower = t;
		}
		d.Limited = true;
	}
	deformSelected(mesh, d, "Bend", name, warnings);
}

struct STwistDeformer
{
	Matrix3M Tm, InvTm;
	float Angle, AnglePerUnit, Height, BiasExponent, Lower, Upper;
	bool Biased, Limited;

	Point3M map(Point3M p) const
	{
		if (Angle == 0.f) return p;
		p = transformPoint(p, Tm);
		float x = p.x, y = p.y, z = p.z;
		if (Limited) z = z < Lower ? Lower : (z > Upper ? Upper : z);
		float a;
		if (Biased)
		{
			float u = z / Height;
			a = Angle * (float)pow(fabs(u), BiasExponent);
			if (u < 0.f) a = -a;
		}
		else
		{
			a = z * AnglePerUnit;
		}
		float c = (float)cos(a), s = (float)sin(a);
		p.x = c * x + s * y;
		p.y = -s * x + c * y;
		return transformPoint(p, InvTm);
	}
};

static void applyTwist(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                       std::vector<std::string> *warnings)
{
	// 0 angle (degrees), 1 bias (-100..100), 2 axis (0 X, 1 Y, 2 Z), 3 limits on, 4 lower limit, 5 upper limit
	std::map<sint32, float> p;
	if (!readGizmoModParams(mod, p, name) || !hasParams(p, 3))
	{
		fprintf(stderr, "WARNING: mesh '%s' Twist parameters not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:twist");
		return;
	}
	const float angle = p[0], bias = p[1];
	const int naxis = (int)p[2];
	STwistDeformer d;
	d.Limited = p.count(3) && p[3] != 0.f;
	d.Lower = p.count(4) ? p[4] : 0.f;
	d.Upper = p.count(5) ? p[5] : 0.f;
	d.Biased = bias != 0.f;
	if (d.Biased)
	{
		float b = 1.f - (bias + 100.f) / 200.f;
		if (b < 0.00001f) b = 0.00001f;
		if (b > 0.99999f) b = 0.99999f;
		d.BiasExponent = (float)(log(b) / log(0.5));
	}
	else
	{
		d.BiasExponent = 1.f;
	}
	SGizmoModSpace space;
	if (!gizmoModSpace(mod, app, gizmoModAxis(naxis), space))
	{
		fprintf(stderr, "WARNING: mesh '%s' Twist without a usable mod-context box; evaluated without it\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:twist");
		return;
	}
	d.Tm = space.Tm;
	d.InvTm = space.InvTm;
	// the twist spreads over the box extent along the twist axis
	const int ax = naxis < 0 || naxis > 2 ? 2 : naxis;
	d.Height = space.Box[3 + ax] - space.Box[ax];
	const float rad = (float)(angle * NLMISC::Pi / 180.0);
	d.Angle = d.Height == 0.f ? 0.f : rad;
	d.AnglePerUnit = d.Height == 0.f ? 0.f : rad / d.Height;
	deformSelected(mesh, d, "Twist", name, warnings);
}

struct STaperDeformer
{
	Matrix3M Tm, InvTm;
	float Length, Amount, Curve, Lower, Upper;
	bool Limited, Symmetric, ScaleX, ScaleY;

	Point3M map(Point3M p) const
	{
		if (Length == 0.f) return p;
		p = transformPoint(p, Tm);
		// the position along the axis as a share of the length; beyond the limits, the scale of the limit
		float z;
		if (Limited && p.z < Lower) z = Lower / Length;
		else if (Limited && p.z > Upper) z = Upper / Length;
		else z = p.z / Length;
		if (Symmetric && z < 0.f) z = -z;
		const float f = 1.f + z * Amount + Curve * z * (1.f - z);
		if (ScaleX) p.x *= f;
		if (ScaleY) p.y *= f;
		return transformPoint(p, InvTm);
	}
};

static void applyTaper(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                       std::vector<std::string> *warnings)
{
	// 0 amount, 1 curve, 2 primary axis (0 X, 1 Y, 2 Z), 3 effect (0 X, 1 Y, 2 both), 4 symmetry, 5 limits on,
	// 6 lower limit, 7 upper limit; the effect axes are those of the deformer space, where the primary axis is Z
	std::map<sint32, float> p;
	if (!readGizmoModParams(mod, p, name) || !hasParams(p, 8))
	{
		fprintf(stderr, "WARNING: mesh '%s' Taper parameters not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:taper");
		return;
	}
	const int naxis = (int)p[2];
	const int effect = (int)p[3];
	STaperDeformer d;
	d.Amount = p[0];
	d.Curve = p[1];
	d.ScaleX = effect != 1;
	d.ScaleY = effect != 0;
	d.Symmetric = p[4] != 0.f;
	d.Limited = p[5] != 0.f;
	d.Lower = p[6];
	d.Upper = p[7];
	SGizmoModSpace space;
	if (!gizmoModSpace(mod, app, gizmoModAxis(naxis), space))
	{
		fprintf(stderr, "WARNING: mesh '%s' Taper without a usable mod-context box; evaluated without it\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:taper");
		return;
	}
	d.Tm = space.Tm;
	d.InvTm = space.InvTm;
	// the taper spreads over the box extent along the primary axis, also with the limits on
	const int ax = naxis < 0 || naxis > 2 ? 2 : naxis;
	d.Length = space.Box[3 + ax] - space.Box[ax];
	deformSelected(mesh, d, "Taper", name, warnings);
}

struct SStretchDeformer
{
	Matrix3M Tm, InvTm;
	float Low, High, From, To, XY, ZS;
	bool Limited;

	Point3M map(Point3M p) const
	{
		p = transformPoint(p, Tm);
		float z = p.z;
		if (Limited) z = z > To ? To : (z < From ? From : z);
		const float n = (z - Low) / (High - Low);
		const float a = 4.f * (1.f - XY), b = -4.f * (1.f - XY);
		const float f = (a * n + b) * n + 1.f;
		p.x *= f;
		p.y *= f;
		if (!Limited) p.z *= ZS;
		else if (p.z < From) p.z += (ZS - 1.f) * From;
		else if (p.z > To) p.z += (ZS - 1.f) * To;
		else p.z *= ZS;
		return transformPoint(p, InvTm);
	}
};

// Stretch: the same references as Bend (reference 2 an old ParamBlock: 0 amount, 1 amplify, 2 axis, 3 limits
// on, 4 lower and 5 upper limit, values at tick 0). Along the axis, over the box extent (or the limits), the
// height scales by amount + 1 when stretching (1 / (1 - amount) when squashing) and the cross section by a
// parabola through 1 at both ends and 1 / (amplifier * amount + 1) in the middle (amplifier * -amount + 1
// when squashing; amplifier = amplify + 1, or 1 / (1 - amplify) when negative). Corpus: medusea.max, where the
// amount is 0 at tick 0, so the formula is not validated by a reference.
static void applyStretch(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                         std::vector<std::string> *warnings)
{
	typedef PIPELINE::MAX::BUILTIN::CParamBlock CParamBlock;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CParamBlock *pb = rm && rm->nbReferences() > 2 ? dynamic_cast<CParamBlock *>(rm->getReference(2)) : nullptr;
	float amount = 0.f, amplify = 0.f, from = 0.f, to = 0.f;
	sint32 axis = 2, limited = 0;
	if (!pb || !pb->getFloatAt0(0, amount) || !pb->getFloatAt0(1, amplify) || !pb->getInt(2, axis))
	{
		fprintf(stderr, "WARNING: mesh '%s' Stretch parameters not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:stretch");
		return;
	}
	pb->getInt(3, limited);
	if (limited && (!pb->getFloatAt0(4, from) || !pb->getFloatAt0(5, to)))
	{
		fprintf(stderr, "WARNING: mesh '%s' Stretch limits not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:stretch");
		return;
	}
	if (amount == 0.f || (limited && to == from))
		return; // no change, not even the round trip
	SGizmoModSpace space;
	if (!gizmoModSpace(mod, app, gizmoModAxis(axis), space))
	{
		fprintf(stderr, "WARNING: mesh '%s' Stretch without a usable mod-context box; evaluated without it\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:stretch");
		return;
	}
	const int ax = axis < 0 || axis > 2 ? 2 : axis;
	SStretchDeformer d;
	d.Tm = space.Tm;
	d.InvTm = space.InvTm;
	d.Limited = limited != 0;
	d.From = from;
	d.To = to;
	d.Low = d.Limited ? from : space.Box[ax];
	d.High = d.Limited ? to : space.Box[3 + ax];
	if (d.High == d.Low)
		return;
	const float amplifier = amplify >= 0.f ? amplify + 1.f : 1.f / (1.f - amplify);
	if (amount < 0.f)
	{
		d.XY = amplifier * -amount + 1.f;
		d.ZS = -1.f / (amount - 1.f);
	}
	else
	{
		d.XY = 1.f / (amplifier * amount + 1.f);
		d.ZS = amount + 1.f;
	}
	deformSelected(mesh, d, "Stretch", name, warnings);
}

// ---------------------------------------------------------------------------------------------

// A modifier switched off in its stack: the base container 0x39bf of the modifier holds an empty 0x0110 leaf
// next to the empty 0x0100 every modifier has. The stack evaluation then passes it by. Corpus (all 2850
// shape-batch files): seven modifiers carry it, five Morphers on the face meshes of haircut scenes, a
// VertexTreePaint and an Edit Mesh. In fy_hof_cheveux_artistic01.max and ma_hom_cheveux_style01.max the
// reference exports match only with the Morpher left out (the face feeds the haircut's border normals),
// while the Morphers without it are applied (fy_hom_cheveux_artistic03.max); the other five give the same
// result either way. The other states of a modifier (off in the viewport or the renderer only) are not seen;
// any other leaf in 0x39bf, or a 0x0110 with a value, warns.
static bool modifierSwitchedOff(CSceneClass *mod, const std::string &name)
{
	const CStorageContainer *base = dynamic_cast<const CStorageContainer *>(findChunk(mod, 0x39bf));
	if (!base)
		return false;
	bool off = false;
	const CStorageContainer::TStorageObjectContainer &chunks = base->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
		const bool empty = raw && raw->Value.empty();
		if (it->first == 0x0110)
			off = true;
		if ((it->first != 0x0100 && it->first != 0x0110) || !empty)
			fprintf(stderr, "WARNING: mesh '%s' modifier %s has an unknown state chunk 0x%04x in its base data\n",
			        name.c_str(), mod->classDesc()->classId().toString().c_str(), (uint)it->first);
	}
	return off;
}

// ---------------------------------------------------------------------------------------------
// Morpher: blends the incoming mesh towards stored target shapes, one per channel, by the channel
// percentages at tick 0. Modelling uses it for variations, so it matters even without animation.
// The channels are the modifier's containers 0x0200 + i (0x0191 the channel count in newer files):
// 0x0101 (0x0100 in older files) the point count, 0x0140 the target points (three floats each), 0x0110
// the channel's vertex selection (bit array), 0x0130 its flags (int32 active, modded, use limits, use
// selection; float lower and upper limit; int32 active override), 0x0180 the number of progressive
// targets. The channel percentage is a one-float ParamBlock at reference 1 + i (channels from 100 up:
// 1 + i % 100 + i / 100 * 3000), the target node at reference 101 + i; reference 0 holds the global
// options (0 use limits, 1 lower, 2 upper limit, 3 use the channel selections, 5 auto-load the targets).
// A channel moves a point by (target - incoming) / 100 * percentage, the percentage cut to whole limits when
// they are on, and the moves of all channels add up. A channel whose point count differs from the mesh is
// left out, as the modifier does. At a sub-object level only the selected vertices move. The target shapes
// are the saved ones: with auto-load on, the modifier would re-read them from target nodes still in the
// scene, which warns.

static bool readRawUInt32(const CStorageContainer *c, uint16 id, uint32 &out)
{
	const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(c->findStorageObject(id));
	if (!raw || raw->Value.size() < 4) return false;
	memcpy(&out, &raw->Value[0], 4);
	return true;
}

static void applyMorpher(CSceneClass *mod, SEvalMesh &mesh, const std::string &name,
                         std::vector<std::string> *warnings)
{
	typedef PIPELINE::MAX::BUILTIN::CParamBlock CParamBlock;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	if (!rm)
	{
		fprintf(stderr, "WARNING: mesh '%s' Morpher without references; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:morpher");
		return;
	}
	bool globalLimits = false, globalSel = false, autoLoad = false;
	float globalLower = 0.f, globalUpper = 100.f;
	CParamBlock *g = rm->nbReferences() > 0 ? dynamic_cast<CParamBlock *>(rm->getReference(0)) : nullptr;
	sint32 v;
	if (g && g->getInt(0, v) && g->getFloatAt0(1, globalLower) && g->getFloatAt0(2, globalUpper))
	{
		globalLimits = v != 0;
		if (g->getInt(3, v)) globalSel = v != 0;
		if (g->getInt(5, v)) autoLoad = v != 0;
	}
	else
	{
		fprintf(stderr, "WARNING: mesh '%s' Morpher global options not readable; percentages not limited\n",
		        name.c_str());
		if (warnings) warnings->push_back("morpher:options");
	}
	uint32 channelCount = 0;
	const CStorageRaw *countChunk = findRawChunk(mod, 0x0191);
	const bool hasChannelCount = countChunk && countChunk->Value.size() >= 4;
	if (hasChannelCount)
		memcpy(&channelCount, &countChunk->Value[0], 4);

	const uint nv = (uint)mesh.Verts.size();
	const std::vector<MAXMATH::Point3M> &base = mesh.Verts; // only written after the channels
	std::vector<MAXMATH::Point3M> moved = base;
	std::vector<bool> touched(nv, false);
	uint used = 0;
	bool autoLoadWarned = false;
	const CStorageContainer::TStorageObjectContainer &orphans = mod->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first < 0x0200 || it->first >= 0x2000) // not a channel (0x39bf is the base modifier data)
			continue;
		const CStorageContainer *ch = dynamic_cast<const CStorageContainer *>(it->second);
		if (!ch)
			continue;
		const uint index = it->first - 0x0200;
		if (hasChannelCount && index >= channelCount)
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u beyond the channel count %u; left out\n",
			        name.c_str(), index, channelCount);
			if (warnings) warnings->push_back("morpher:channel");
			continue;
		}
		const uint refOffset = index % 100 + index / 100 * 3000;
		const CStorageRaw *params = dynamic_cast<const CStorageRaw *>(ch->findStorageObject(0x0130));
		if (!params || params->Value.size() < 28)
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u without its flags; left out\n", name.c_str(), index);
			if (warnings) warnings->push_back("morpher:channel");
			continue;
		}
		sint32 active, useLimit, useSel, activeOverride;
		float lower, upper;
		memcpy(&active, &params->Value[0], 4);
		memcpy(&useLimit, &params->Value[8], 4);
		memcpy(&useSel, &params->Value[12], 4);
		memcpy(&lower, &params->Value[16], 4);
		memcpy(&upper, &params->Value[20], 4);
		memcpy(&activeOverride, &params->Value[24], 4);
		if (!active || !activeOverride)
			continue;
		uint32 count = 0;
		if (!readRawUInt32(ch, 0x0101, count) && !readRawUInt32(ch, 0x0100, count))
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u without its point count; left out\n",
			        name.c_str(), index);
			if (warnings) warnings->push_back("morpher:channel");
			continue;
		}
		if (count != nv)
			continue;
		if (autoLoad && !autoLoadWarned && 101 + refOffset < rm->nbReferences()
		    && dynamic_cast<INode *>(rm->getReference(101 + refOffset)))
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher loads its targets from their nodes; the saved targets are "
			        "used\n", name.c_str());
			if (warnings) warnings->push_back("morpher:autoload");
			autoLoadWarned = true;
		}
		uint32 progressive = 0;
		if (readRawUInt32(ch, 0x0180, progressive) && progressive != 0)
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u has progressive targets, not supported; "
			        "channel left out\n", name.c_str(), index);
			if (warnings) warnings->push_back("morpher:progressive");
			continue;
		}
		const CStorageRaw *points = dynamic_cast<const CStorageRaw *>(ch->findStorageObject(0x0140));
		if (!points || points->Value.size() < (size_t)count * 12)
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u without its target points; channel left out\n",
			        name.c_str(), index);
			if (warnings) warnings->push_back("morpher:points");
			continue;
		}
		float percent = 0.f;
		CParamBlock *cb = 1 + refOffset < rm->nbReferences()
			? dynamic_cast<CParamBlock *>(rm->getReference(1 + refOffset)) : nullptr;
		if (!cb || !cb->getFloatAt0(0, percent))
		{
			fprintf(stderr, "WARNING: mesh '%s' Morpher channel %u percentage not readable; channel left out\n",
			        name.c_str(), index);
			if (warnings) warnings->push_back("morpher:percent");
			continue;
		}
		if (useLimit || globalLimits)
		{
			const int upperLimit = globalLimits ? (int)globalUpper : (int)upper;
			const int lowerLimit = globalLimits ? (int)globalLower : (int)lower;
			if (percent > upperLimit) percent = (float)upperLimit;
			if (percent < lowerLimit) percent = (float)lowerLimit;
		}
		std::vector<bool> chSel;
		const bool bySel = useSel || globalSel;
		if (bySel && (!readBitArray(ch, 0x0110, chSel) || chSel.size() < nv))
			chSel.resize(nv, false);
		if (getenv("PMB_STACK_DUMP"))
			fprintf(stderr, "PMB_STACK_DUMP '%s' Morpher channel %u at %g%%\n", name.c_str(), index, percent);
		for (uint p = 0; p < nv; ++p)
		{
			if (bySel && !chSel[p])
				continue;
			float target[3];
			memcpy(target, &points->Value[p * 12], 12);
			// the delta per percent in float, as the modifier rebuilds it from the incoming points
			const float dx = (target[0] - base[p].x) / 100.0f;
			const float dy = (target[1] - base[p].y) / 100.0f;
			const float dz = (target[2] - base[p].z) / 100.0f;
			moved[p].x += dx * percent;
			moved[p].y += dy * percent;
			moved[p].z += dz * percent;
			touched[p] = true;
		}
		++used;
	}
	if (!used)
		return;

	std::vector<bool> sel;
	const bool all = modifierSelection(mesh, sel, "Morpher", name, warnings);
	for (uint p = 0; p < nv; ++p)
		if (touched[p] && (all || sel[p]))
			mesh.Verts[p] = moved[p];
}

// Edit Poly modifier: records editing operations per node and replays them on the poly mesh it gets.
// Without a recorded operation (no 0x0230 or 0x0260 in its per-node data 0x2512) and no operation in
// progress (parameter 12 of the ParamBlock2 at reference 0 is zero), it passes the mesh through and hands up
// its own selection: the level in the modifier's chunk 0x0100 (int32: 0 object, 1 vertex, 2 edge, 3 border,
// 4 face, 5 element) and the vertex selection in 0x0200 of the per-node data (one bit per vertex). Its edge
// and face selections (0x0210, 0x0220) are per polygon edge and polygon, which the triangle mesh does not
// keep, so a selection at those levels is not known. Parameter 4 on keeps the selections from below;
// parameter 5 is the soft selection. Recorded operations are not replayed: the modifier is then left out
// with a warning. Corpus: the jungle water shapes of zonematerial-jungle-5.max and
// zonematerial-jungle-5_waterfall.max, no operation recorded.
static void applyEditPoly(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                          std::vector<std::string> *warnings)
{
	const NLMISC::CClassId mcid = mod->classDesc()->classId();
	const CStorageContainer *data = app ? dynamic_cast<const CStorageContainer *>(
		CDerivedObject::modAppLocalModData(app)) : nullptr;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CSceneClass *pb = rm && rm->nbReferences() > 0 ? dynamic_cast<CSceneClass *>(rm->getReference(0)) : nullptr;
	SPB2Block blk;
	std::map<uint16, SPB2Param>::const_iterator op, useStack, soft;
	const bool readable = pb && readPB2Block(pb, blk);
	if (readable)
	{
		op = blk.Params.find(12);
		useStack = blk.Params.find(4);
		soft = blk.Params.find(5);
	}
	const bool edits = !data || data->findStorageObject(0x0230) || data->findStorageObject(0x0260)
		|| !readable || op == blk.Params.end() || !op->second.HasConstant || op->second.I != 0;
	const CStorageRaw *lvl = findRawChunk(mod, 0x0100);
	sint32 level = -1;
	if (lvl && lvl->Value.size() >= 4)
		memcpy(&level, &lvl->Value[0], 4);
	if (edits || level < 0 || level > 5)
	{
		fprintf(stderr, "WARNING: mesh '%s' Edit Poly with recorded operations (or unreadable), not supported; "
		        "evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		mesh.SelLevelUnknown = mesh.SelBitsUnknown = true;
		return;
	}
	static const uint32 s_Levels[] = { 1, 2, 8, 8, 4, 4 };
	mesh.SelLevel = s_Levels[level];
	mesh.SelLevelUnknown = false;
	if (useStack == blk.Params.end() || !useStack->second.HasConstant || soft == blk.Params.end()
	    || !soft->second.HasConstant)
	{
		fprintf(stderr, "WARNING: mesh '%s' Edit Poly selection options not readable; taken as off\n", name.c_str());
		if (warnings) warnings->push_back("selection:editpoly");
	}
	for (uint16 hid = 0x0224; hid <= 0x0228; hid += 4)
	{
		std::vector<bool> hidden;
		if (readBitArray(data, hid, hidden) && std::find(hidden.begin(), hidden.end(), true) != hidden.end())
		{
			fprintf(stderr, "WARNING: mesh '%s' Edit Poly hides vertices or faces, not supported; they stay visible\n",
			        name.c_str());
			if (warnings) warnings->push_back("modifier:editpoly-hide");
			break;
		}
	}
	const bool keepBelow = useStack != blk.Params.end() && useStack->second.HasConstant && useStack->second.I != 0;
	const bool softOn = soft != blk.Params.end() && soft->second.HasConstant && soft->second.I != 0;
	if (keepBelow)
	{
		mesh.SelSoft = mesh.SelSoft || (softOn && level != 0);
		return;
	}
	mesh.SelSoft = softOn && level != 0;
	std::vector<bool> verts;
	mesh.SelBitsUnknown = !readBitArray(data, 0x0200, verts) || verts.size() != mesh.Verts.size();
	if (!mesh.SelBitsUnknown)
		mesh.SelVerts = verts;
	mesh.SelFaces.clear(); // per polygon: an edge, face or element selection is not known
	mesh.SelEdges.clear();
}

// Mesh Select: hands up a selection. The level is a uint32 in the modifier's 0x0100 (0 object, 1 vertex,
// 2 edge, 3 face, 4 polygon, 5 element; edge and face swapped when the version 0x0230 is below 3); the
// per-node data 0x2512 holds 0x0200 the vertex, 0x0210 the face and 0x0220 the edge selection (3 per face),
// each absent when empty and fitted to the mesh. A mod-app without per-node data passes the selections below
// through. Soft selection is parameter 0 of the ParamBlock2 at reference 0. Corpus: ge_hom_armor02.max
// (GE_HOM_armor02_hand, polygon level, nothing selected).
static void applyMeshSelect(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                            std::vector<std::string> *warnings)
{
	const CStorageRaw *lvl = findRawChunk(mod, 0x0100);
	const CStorageRaw *ver = findRawChunk(mod, 0x0230);
	uint32 level = 0xffffffff, version = 0;
	if (lvl && lvl->Value.size() >= 4) memcpy(&level, &lvl->Value[0], 4);
	if (ver && ver->Value.size() >= 4) memcpy(&version, &ver->Value[0], 4);
	if (level > 5)
	{
		fprintf(stderr, "WARNING: mesh '%s' Mesh Select level not readable; the selection is not known\n", name.c_str());
		if (warnings) warnings->push_back("selection:meshselect");
		mesh.SelLevelUnknown = mesh.SelBitsUnknown = true;
		return;
	}
	if (version < 3 && (level == 2 || level == 3))
		level = 5 - level;
	static const uint32 s_Levels[] = { 1, 2, 8, 4, 4, 4 };
	mesh.SelLevel = s_Levels[level];
	mesh.SelLevelUnknown = false;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CSceneClass *pb = rm && rm->nbReferences() > 0 ? dynamic_cast<CSceneClass *>(rm->getReference(0)) : nullptr;
	SPB2Block blk;
	float soft = 0.f;
	std::map<uint16, SPB2Param>::const_iterator softParam;
	if (pb && readPB2Block(pb, blk) && (softParam = blk.Params.find(0)) != blk.Params.end()
	    && softParam->second.HasConstant)
	{
		soft = (float)softParam->second.I;
	}
	else
	{
		fprintf(stderr, "WARNING: mesh '%s' Mesh Select soft selection option not readable; taken as off\n",
		        name.c_str());
		if (warnings) warnings->push_back("selection:meshselect");
	}
	mesh.SelSoft = soft != 0.f && level != 0;
	const CStorageContainer *data = app ? dynamic_cast<const CStorageContainer *>(
		CDerivedObject::modAppLocalModData(app)) : nullptr;
	if (!data)
		return; // the selections below pass through
	static const uint16 s_Ids[] = { 0x0200, 0x0210, 0x0220 };
	std::vector<bool> *const sels[] = { &mesh.SelVerts, &mesh.SelFaces, &mesh.SelEdges };
	const size_t sizes[] = { mesh.Verts.size(), mesh.Faces.size(), mesh.Faces.size() * 3 };
	mesh.SelBitsUnknown = false;
	for (uint i = 0; i < 3; ++i)
	{
		sels[i]->clear();
		if (data->findStorageObject(s_Ids[i]) && !readBitArray(data, s_Ids[i], *sels[i]))
			mesh.SelBitsUnknown = true;
		sels[i]->resize(sizes[i], false);
	}
}

// OldVertexPaint (the legacy vertex paint, not the NeL VertexTreePaint): the per-node data 0x2512 holds
// colour lists, 0x0120 for map channel 0 (0x0124 illumination and 0x0128 alpha, not seen): int32 count,
// int32 allocated, then allocated records of 16 bytes {uint32 colour 0x00BBGGRR, float weight, int32 face,
// int32 corner}, the record index being the vertex index. A missing channel starts as a white map with the
// mesh's topology. Each painted vertex (colour not 0xFFFFFFFF) gets one new map vertex, weight * colour +
// (1 - weight) * the incoming colour at the record's face and corner, and every corner of the vertex points to
// it. Validated on fy_cn_smokehouse.max (entreporte): all 86 reference vertex colours.
static void applyOldVertexPaint(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                                std::vector<std::string> *warnings)
{
	const NLMISC::CClassId mcid = mod->classDesc()->classId();
	const CStorageContainer *data = app ? dynamic_cast<const CStorageContainer *>(
		CDerivedObject::modAppLocalModData(app)) : nullptr;
	if (!data)
		return; // nothing painted on this node
	if (!mesh.SelLevelUnknown && mesh.SelLevel == 8)
		return; // at edge level the modifier does nothing
	if (data->findStorageObject(0x0124) || data->findStorageObject(0x0128))
	{
		fprintf(stderr, "WARNING: mesh '%s' OldVertexPaint illumination or alpha list not supported; left out\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
	}
	const CStorageRaw *list = dynamic_cast<const CStorageRaw *>(data->findStorageObject(0x0120));
	if (!list)
		return;
	const bool objectLevel = !mesh.SelLevelUnknown && mesh.SelLevel == 1;
	const bool vertexLevel = !mesh.SelLevelUnknown && mesh.SelLevel == 2 && !mesh.SelBitsUnknown
		&& mesh.SelVerts.size() == mesh.Verts.size();
	if (!objectLevel && !vertexLevel)
	{
		fprintf(stderr, "WARNING: mesh '%s' OldVertexPaint on a face selection or an unknown selection not supported; "
		        "evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	sint32 count = 0, allocated = 0;
	if (list->Value.size() >= 8)
	{
		memcpy(&count, &list->Value[0], 4);
		memcpy(&allocated, &list->Value[4], 4);
	}
	if (list->Value.size() < 8 || count < 0 || allocated < count || list->Value.size() != 8 + (size_t)allocated * 16)
	{
		fprintf(stderr, "WARNING: mesh '%s' OldVertexPaint colour list layout not supported; evaluated without it\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	const uint nv = (uint)mesh.Verts.size(), nf = (uint)mesh.Faces.size();
	if ((uint)count != nv)
	{
		fprintf(stderr, "WARNING: mesh '%s' OldVertexPaint has %i colours for %u vertices\n", name.c_str(), (int)count, nv);
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
	}
	const bool existed = mesh.mapSupport(0);
	if (existed && mesh.Maps[0].Faces.size() != nf)
	{
		fprintf(stderr, "WARNING: mesh '%s' map channel 0 does not match the faces; OldVertexPaint left out\n",
		        name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	SMapChannel &ch = mesh.Maps[0];
	if (!existed)
	{
		Point3M white = { 1.f, 1.f, 1.f };
		ch.Verts.assign(nv, white);
		ch.Faces.resize(nf);
		for (uint f = 0; f < nf; ++f)
			for (uint c = 0; c < 3; ++c)
				ch.Faces[f].T[c] = mesh.Faces[f].V[c];
	}
	std::vector<std::vector<uint32> > corners(nv); // face * 3 + corner per vertex
	for (uint f = 0; f < nf; ++f)
		for (uint c = 0; c < 3; ++c)
			if (mesh.Faces[f].V[c] < nv) corners[mesh.Faces[f].V[c]].push_back(f * 3 + c);
	const bool bySel = vertexLevel;
	for (uint v = 0; v < (uint)count && v < nv; ++v)
	{
		uint32 colour;
		float weight;
		sint32 face, corner;
		memcpy(&colour, &list->Value[8 + (size_t)v * 16], 4);
		memcpy(&weight, &list->Value[8 + (size_t)v * 16 + 4], 4);
		memcpy(&face, &list->Value[8 + (size_t)v * 16 + 8], 4);
		memcpy(&corner, &list->Value[8 + (size_t)v * 16 + 12], 4);
		if (colour == 0xffffffff || (bySel && !mesh.SelVerts[v]))
			continue;
		Point3M inc = { 1.f, 1.f, 1.f };
		if (existed && face >= 0 && (uint)face < nf && corner >= 0 && corner < 3
		    && ch.Faces[face].T[corner] < ch.Verts.size())
			inc = ch.Verts[ch.Faces[face].T[corner]];
		const float r = (float)(colour & 0xff) / 255.f;
		const float g = (float)((colour >> 8) & 0xff) / 255.f;
		const float b = (float)((colour >> 16) & 0xff) / 255.f;
		Point3M c;
		c.x = weight * r + (1.f - weight) * inc.x;
		c.y = weight * g + (1.f - weight) * inc.y;
		c.z = weight * b + (1.f - weight) * inc.z;
		const uint32 k = (uint32)ch.Verts.size();
		ch.Verts.push_back(c);
		for (uint i = 0; i < corners[v].size(); ++i)
			ch.Faces[corners[v][i] / 3].T[corners[v][i] % 3] = k;
	}
}

// Mirror: reference 0 is the gizmo, reference 1 an old ParamBlock (0 axis: X, Y, Z, XY, YZ, ZX; 1 copy;
// 2 offset). It works on a face set (the faces of the selection, OR-ed into the stored face selection at
// vertex and edge level, every face at object level). With copy on, the faces of the set are appended again,
// with their vertices in first-use order (walking the faces and their corners) and their map vertices the
// same way, and the clones become the set. Single-axis mirrors swap corners 0 and 1 of the faces of the set
// (in every map channel too, and edge visibility 1 and 2). The vertices of the set then go through
// M = Inverse(gizmo * Inverse(context)) with the mirrored columns negated and the offset added, and back
// through gizmo * Inverse(context). Validated on tr_hall_conseil.max (TR_conseil_fond_marin, the clone
// order) and tr_hall_vitrine_reunion.max (TR_hall_reu_vitrine_decors, the flip).
static void applyMirror(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, const std::string &name,
                        std::vector<std::string> *warnings)
{
	typedef PIPELINE::MAX::BUILTIN::CParamBlock CParamBlock;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CParamBlock *pb = rm && rm->nbReferences() > 1 ? dynamic_cast<CParamBlock *>(rm->getReference(1)) : nullptr;
	sint32 axis = -1, copy = 0;
	float offset = 0.f;
	if (!pb || !pb->getInt(0, axis) || !pb->getInt(1, copy) || !pb->getFloatAt0(2, offset) || axis < 0 || axis > 5)
	{
		fprintf(stderr, "WARNING: mesh '%s' Mirror parameters not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:mirror");
		return;
	}
	Matrix3M gizmo = Matrix3M::identity();
	if (CControlPRS *prs = rm->nbReferences() > 0 ? dynamic_cast<CControlPRS *>(rm->getReference(0)) : nullptr)
	{
		gizmo = composePRS(posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController())),
		                   rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController())),
		                   scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController())));
	}
	Matrix3M ctx = Matrix3M::identity();
	CDerivedObject::modAppContextTM(app, &ctx.m[0][0]);
	const Matrix3M itm = gizmo * inverseM3(ctx);
	Matrix3M m = inverseM3(itm);
	const int axes[2] = { axis % 3, axis < 3 ? -1 : (axis + 1) % 3 };
	for (int a = 0; a < 2; ++a)
	{
		if (axes[a] < 0) continue;
		for (int r = 0; r < 4; ++r)
			m.m[r][axes[a]] = -m.m[r][axes[a]];
		m.m[3][axes[a]] += offset;
	}

	// the face set
	uint nf = (uint)mesh.Faces.size();
	const uint nv0 = (uint)mesh.Verts.size();
	std::vector<bool> faces;
	bool known = !mesh.SelLevelUnknown;
	if (known && mesh.SelLevel == 1)
	{
		faces.assign(nf, true);
	}
	else if (known && !mesh.SelBitsUnknown && mesh.SelFaces.size() == nf)
	{
		faces = mesh.SelFaces;
		if (mesh.SelLevel == 2 && mesh.SelVerts.size() == nv0)
		{
			for (uint f = 0; f < nf; ++f)
				for (uint c = 0; c < 3; ++c)
					if (mesh.Faces[f].V[c] < nv0 && mesh.SelVerts[mesh.Faces[f].V[c]]) faces[f] = true;
		}
		else if (mesh.SelLevel == 8 && mesh.SelEdges.size() == nf * 3)
		{
			for (uint f = 0; f < nf; ++f)
				for (uint c = 0; c < 3; ++c)
					if (mesh.SelEdges[f * 3 + c]) faces[f] = true;
		}
		else if (mesh.SelLevel != 4)
		{
			known = false;
		}
	}
	else
	{
		known = false;
	}
	if (!known)
	{
		fprintf(stderr, "WARNING: mesh '%s' Mirror applied to the whole mesh: the selection it sees is not known\n",
		        name.c_str());
		if (warnings) warnings->push_back("selection:Mirror");
		faces.assign(nf, true);
	}
	else if (!copy && mesh.SelSoft && mesh.SelLevel != 1)
	{
		fprintf(stderr, "WARNING: mesh '%s' Mirror applied to the selected vertices only: the soft selection falloff "
		        "around them is not evaluated\n", name.c_str());
		if (warnings) warnings->push_back("softselection:Mirror");
	}

	if (copy)
	{
		std::vector<uint> set;
		for (uint f = 0; f < nf; ++f)
			if (faces[f]) set.push_back(f);
		std::vector<sint32> clone(nv0, -1);
		for (uint i = 0; i < set.size(); ++i)
		{
			SEvalFace face = mesh.Faces[set[i]];
			for (uint c = 0; c < 3; ++c)
			{
				const uint32 v = face.V[c];
				if (v >= nv0) continue;
				if (clone[v] < 0)
				{
					clone[v] = (sint32)mesh.Verts.size();
					mesh.Verts.push_back(mesh.Verts[v]);
				}
				face.V[c] = (uint32)clone[v];
			}
			mesh.Faces.push_back(face);
		}
		for (std::map<int, SMapChannel>::iterator it = mesh.Maps.begin(); it != mesh.Maps.end(); ++it)
		{
			SMapChannel &ch = it->second;
			if (ch.Faces.size() != nf)
			{
				fprintf(stderr, "WARNING: mesh '%s' map channel %i does not match the faces; Mirror does not copy it\n",
				        name.c_str(), it->first);
				if (warnings) warnings->push_back("modifier:mirror");
				continue;
			}
			const uint nt = (uint)ch.Verts.size();
			std::vector<sint32> mclone(nt, -1);
			for (uint i = 0; i < set.size(); ++i)
			{
				SMapFace mf = ch.Faces[set[i]];
				for (uint c = 0; c < 3; ++c)
				{
					const uint32 t = mf.T[c];
					if (t >= nt) continue;
					if (mclone[t] < 0)
					{
						mclone[t] = (sint32)ch.Verts.size();
						ch.Verts.push_back(ch.Verts[t]);
					}
					mf.T[c] = (uint32)mclone[t];
				}
				ch.Faces.push_back(mf);
			}
		}
		faces.assign(nf, false);
		faces.resize(mesh.Faces.size(), true);
		nf = (uint)mesh.Faces.size();
		mesh.SelSoft = false;
	}

	if (axis < 3)
	{
		for (uint f = 0; f < nf; ++f)
		{
			if (!faces[f]) continue;
			SEvalFace &face = mesh.Faces[f];
			std::swap(face.V[0], face.V[1]);
			const uint32 e1 = face.Flags & 2, e2 = face.Flags & 4;
			face.Flags = (face.Flags & ~6u) | (e1 << 1) | (e2 >> 1);
			if (mesh.SelEdges.size() == (size_t)nf * 3)
			{
				const bool b = mesh.SelEdges[f * 3 + 1];
				mesh.SelEdges[f * 3 + 1] = mesh.SelEdges[f * 3 + 2];
				mesh.SelEdges[f * 3 + 2] = b;
			}
			for (std::map<int, SMapChannel>::iterator it = mesh.Maps.begin(); it != mesh.Maps.end(); ++it)
				if (it->second.Faces.size() == nf)
					std::swap(it->second.Faces[f].T[0], it->second.Faces[f].T[1]);
		}
	}

	const uint nv = (uint)mesh.Verts.size();
	std::vector<bool> moving(nv, false);
	for (uint f = 0; f < nf; ++f)
		if (faces[f])
			for (uint c = 0; c < 3; ++c)
				if (mesh.Faces[f].V[c] < nv) moving[mesh.Faces[f].V[c]] = true;
	for (uint v = 0; v < nv; ++v)
		if (moving[v])
			mesh.Verts[v] = transformPoint(transformPoint(mesh.Verts[v], m), itm);

	mesh.SelFaces = faces;
	if (copy)
	{
		mesh.SelVerts.resize(nv, false);
		mesh.SelEdges.resize(nf * 3, false);
		if (mesh.SelLevel == 2 || mesh.SelLevel == 8)
			mesh.SelBitsUnknown = true; // what the copied vertices and edges hold is not known
	}
}

// Unwrap UVW: the texture vertices and faces are stored on the modifier (the corpus has only this older
// layout; the per-node data holds a face selection, not used): 0x0100 the vertex count N, 0x0270 N records
// of 20 bytes (float x, y, z, float soft selection weight, flags byte, 3 padding bytes), 0x0230 the face
// count F, 0x0310 F records (int32 count, count texture vertex indices, face index, material id, flags,
// count geometry vertex indices, and 6 * count more int32 when flags has 0x20), 0x0260 the channel (0 map
// channel 1, 1 channel 0, k from 2 up channel k), 0x0710 apply to the whole object. The data is used only when
// F equals the incoming face count and every face is a triangle; at face level only the selected faces take
// it (the texture vertices appended), otherwise every face. An instanced Unwrap gives its data to the first
// node in storage order whose stack holds it; every other node keeps its incoming map (fungusa.max).
// Validated against the reference exports (PapillonA01/B01, GE_Acc_CattleStick_A, Tr_Wea_Petit_Bouclier,
// Tr_Wea_Pistolet_sousmarin, gun01, CricketA, FungusA01..06).
static void applyUnwrapUvw(CSceneClass *mod, INode &node, SEvalMesh &mesh, const std::string &name,
                           std::vector<std::string> *warnings)
{
	const NLMISC::CClassId mcid = mod->classDesc()->classId();
	const CStorageRaw *nRaw = findRawChunk(mod, 0x0100);
	const CStorageRaw *vRaw = findRawChunk(mod, 0x0270);
	const CStorageRaw *fCount = findRawChunk(mod, 0x0230);
	const CStorageRaw *fRaw = findRawChunk(mod, 0x0310);
	sint32 nv = -1, nfStored = -1;
	if (nRaw && nRaw->Value.size() >= 4) memcpy(&nv, &nRaw->Value[0], 4);
	if (fCount && fCount->Value.size() >= 4) memcpy(&nfStored, &fCount->Value[0], 4);
	if (nv < 0 || nfStored < 0 || !vRaw || vRaw->Value.size() != (size_t)nv * 20 || !fRaw)
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW data layout not supported; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	// the faces (a record takes at least 24 bytes)
	if ((size_t)nfStored > fRaw->Value.size() / 24)
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW face count %i does not fit its records; evaluated without it\n",
		        name.c_str(), (int)nfStored);
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	std::vector<uint32> tv((size_t)nfStored * 3);
	bool triangles = true;
	size_t at = 0;
	const std::vector<uint8> &fr = fRaw->Value;
	for (sint32 f = 0; f < nfStored; ++f)
	{
		sint32 count;
		if (at + 4 > fr.size()) { at = (size_t)-1; break; }
		memcpy(&count, &fr[at], 4);
		at += 4;
		if (count < 1 || count > 64 || at + ((size_t)count * 2 + 3) * 4 > fr.size()) { at = (size_t)-1; break; }
		uint32 flags;
		memcpy(&flags, &fr[at + ((size_t)count + 2) * 4], 4);
		if (count == 3)
			memcpy(&tv[(size_t)f * 3], &fr[at], 12);
		else
			triangles = false;
		at += ((size_t)count * 2 + 3) * 4;
		if (flags & 0x20)
			at += (size_t)count * 6 * 4;
	}
	if (at != fr.size())
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW face records not readable; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	for (size_t i = 0; i < tv.size(); ++i)
	{
		if (tv[i] >= (uint32)nv)
		{
			fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW face uses texture vertex %u of %i; evaluated without it\n",
			        name.c_str(), tv[i], (int)nv);
			if (warnings) warnings->push_back("modifier:" + mcid.toString());
			return;
		}
	}
	// which node owns the data: the first in storage order whose derived objects hold the modifier (walked
	// directly, without resolving references to other files)
	CSceneClass *nodeClass = dynamic_cast<CSceneClass *>(&node);
	if (!nodeClass || nodeClass->container() != mod->container())
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW comes from another file; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	INode *owner = nullptr;
	const CStorageContainer::TStorageObjectContainer &objs = mod->container()->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = objs.begin(); it != objs.end() && !owner; ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (!n || n->nbReferences() < 2) continue;
		for (CDerivedObject *d = dynamic_cast<CDerivedObject *>(n->getReference(1)); d && !owner;
		     d = dynamic_cast<CDerivedObject *>(d->baseObject()))
		{
			for (uint i = 0; i < d->modifierCount(); ++i)
				if (d->modifier(i) == mod) owner = n;
		}
	}
	if (!owner)
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW owner not found; evaluated without it\n", name.c_str());
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	if (owner != &node)
		return; // another node of the instanced modifier owns the data: the incoming map passes through
	const uint nf = (uint)mesh.Faces.size();
	if (!triangles || (uint)nfStored != nf)
	{
		// the modifier then passes its incoming map through
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW stores %i faces%s for %u in the mesh; its data is not used\n",
		        name.c_str(), (int)nfStored, triangles ? "" : " (not all triangles)", nf);
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	const CStorageRaw *chRaw = findRawChunk(mod, 0x0260);
	const CStorageRaw *wholeRaw = findRawChunk(mod, 0x0710);
	sint32 sel = 0, whole = 0;
	if (chRaw && chRaw->Value.size() >= 4) memcpy(&sel, &chRaw->Value[0], 4);
	if (wholeRaw && wholeRaw->Value.size() >= 4) memcpy(&whole, &wholeRaw->Value[0], 4);
	const int channel = sel == 0 ? 1 : (sel == 1 ? 0 : sel);
	bool faceLevel = !whole && !mesh.SelLevelUnknown && mesh.SelLevel == 4;
	if (!whole && (mesh.SelLevelUnknown || (faceLevel && (mesh.SelBitsUnknown || mesh.SelFaces.size() != nf))))
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW applied to every face: the selection it sees is not known\n",
		        name.c_str());
		if (warnings) warnings->push_back("selection:unwrap");
		faceLevel = false;
	}
	std::map<int, SMapChannel>::iterator existing = mesh.Maps.find(channel);
	if (faceLevel && (existing == mesh.Maps.end() || existing->second.Faces.size() != nf))
	{
		fprintf(stderr, "WARNING: mesh '%s' Unwrap UVW on a face selection without an incoming channel %i not "
		        "supported; evaluated without it\n", name.c_str(), channel);
		if (warnings) warnings->push_back("modifier:" + mcid.toString());
		return;
	}
	SMapChannel &ch = mesh.Maps[channel];
	const uint32 offset = faceLevel ? (uint32)ch.Verts.size() : 0;
	if (!faceLevel)
	{
		ch.Verts.clear();
		ch.Faces.assign(nf, SMapFace());
	}
	for (sint32 i = 0; i < nv; ++i)
	{
		Point3M p;
		memcpy(&p, &vRaw->Value[(size_t)i * 20], 12);
		ch.Verts.push_back(p);
	}
	for (uint f = 0; f < nf; ++f)
	{
		if (faceLevel && !mesh.SelFaces[f]) continue;
		for (uint c = 0; c < 3; ++c)
			ch.Faces[f].T[c] = offset + tv[(size_t)f * 3 + c];
	}
}

// The object TM of a node at tick 0: its object offset, then its node TM.
static Matrix3M objectTMAt0(CNodeImpl *n, SNodeTMCache &cache)
{
	if (!n) return Matrix3M::identity();
	Point3M opos;
	QuatM orot;
	ScaleValueM oscale;
	readObjectOffset(n, opos, orot, oscale);
	return composePRS(opos, orot, oscale) * getNodeTM(n, cache);
}

// ---------------------------------------------------------------------------------------------
// FFD lattices. FFD(box), FFD(Cyl) and the FFD space warps an FFD Binding names interpolate their control
// points with a tension and continuity cubic over the lattice padded by one mirrored layer (at the default
// 25/25 this is Catmull-Rom); FFD 2x2x2, 3x3x3 and 4x4x4 weigh their points as a Bezier volume. Both map a
// point into the unit space of their source box first: L = context * Inverse(gizmo), the box minimum taken
// off the translation, every column scaled by 1 / box size; the deformed point goes back through Inverse(L).
// The lattice is stored on the modifier object: 0x2000 the layout version (1), 0x0300 the point counts along
// x, y and z (FFD(box), FFD(Cyl)), 0x0200 the control points in the unit space, 0x0500 (or 0x0400 for the
// fixed-size FFDs) offsets taken off them. Reference 0 is the lattice gizmo, reference 1 the ParamBlock2
// (2 deform all vertices, 3 falloff, 4 tension, 5 continuity; space warps: 6 and 7 the size), references
// 3 and up the controllers of animated control points. Validated against the reference exports, positions
// within the float tolerance: pr_fx_lombrique.max and sfx_glueweb.max (FFD(box)), the jungle waterfalls
// (three stacked FFD(box) under a scaled context), bugballa.max (FFD 3x3x3) and medusea.max (FFD Binding to
// an FFD(Cyl) space warp). The corpus has no moved gizmo, falloff, all-vertices mode or animated point.

struct SFFDLattice
{
	int Dim[3];
	std::vector<Point3M> Points; // control points minus their offsets, in the stored order
	bool AllVertices;
	float Falloff, Tension, Continuity;
	bool Cylinder;
	Matrix3M L, InvL;
};

static Point3M ffdSub(const Point3M &a, const Point3M &b) { Point3M r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static Point3M ffdAdd(const Point3M &a, const Point3M &b) { Point3M r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static Point3M ffdScale(const Point3M &a, float s) { Point3M r = { a.x * s, a.y * s, a.z * s }; return r; }
static float ffdDot(const Point3M &a, const Point3M &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Point3M ffdCross(const Point3M &a, const Point3M &b)
{
	Point3M r = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	return r;
}
// zero for a zero vector: exactly collinear lattice rows give one
static Point3M ffdNormalize(const Point3M &a)
{
	const float l = sqrtf(ffdDot(a, a));
	if (l == 0.f) { Point3M z = { 0.f, 0.f, 0.f }; return z; }
	return ffdScale(a, 1.f / l);
}

// The constant value of a ParamBlock2 parameter by id; false when it is missing or animated.
static bool ffdParam(const SPB2Block &blk, uint16 id, float &out)
{
	std::map<uint16, SPB2Param>::const_iterator it = blk.Params.find(id);
	if (it == blk.Params.end() || !it->second.HasConstant)
		return false;
	const SPB2Param &p = it->second;
	out = (p.Type == PB2_INT || p.Type == PB2_BOOL) ? (float)p.I : p.F[0];
	return true;
}

// The source box of the lattice. Modifiers: the mod-context box made thick (empty: the unit cube around
// the origin; an axis thinner than 0.001 gets 0.001). Fixed-size FFDs and space warps: only a zero size
// becomes 0.001.
static void ffdBox(float box[6], bool thick)
{
	if (thick && (box[3] < box[0] || box[4] < box[1] || box[5] < box[2]))
	{
		for (int i = 0; i < 3; ++i) { box[i] = -0.5f; box[3 + i] = 0.5f; }
	}
	for (int i = 0; i < 3; ++i)
	{
		const float size = box[3 + i] - box[i];
		if (thick ? fabsf(size) < 0.001f : size == 0.f)
			box[3 + i] = box[i] + 0.001f;
	}
}

static void ffdSpace(SFFDLattice &lat, const Matrix3M &ctx, const Matrix3M &gizmo, const float box[6])
{
	Matrix3M l = ctx * inverseM3(gizmo);
	l.m[3][0] -= box[0];
	l.m[3][1] -= box[1];
	l.m[3][2] -= box[2];
	for (int c = 0; c < 3; ++c)
	{
		const float inv = 1.f / (box[3 + c] - box[c]);
		for (int r = 0; r < 4; ++r)
			l.m[r][c] *= inv;
	}
	lat.L = l;
	lat.InvL = inverseM3(l);
}

// Read the lattice of an FFD(box), FFD(Cyl) or FFD space warp object (dimension chunk 0x0300, offsets
// 0x0500), or of a fixed-size FFD of n points per axis (offsets 0x0400). False with a warning when the layout
// is not the known one or a control point or parameter is animated.
static bool readFFDLattice(CSceneClass *obj, int fixedSize, SFFDLattice &lat, const char *what, const std::string &name)
{
	const CStorageRaw *ver = findRawChunk(obj, 0x2000);
	sint32 version = -1;
	if (ver && ver->Value.size() >= 4) memcpy(&version, &ver->Value[0], 4);
	if (version != 1)
	{
		fprintf(stderr, "WARNING: mesh '%s' %s lattice layout version %i not supported\n", name.c_str(), what, (int)version);
		return false;
	}
	if (fixedSize)
	{
		lat.Dim[0] = lat.Dim[1] = lat.Dim[2] = fixedSize;
	}
	else
	{
		const CStorageRaw *dim = findRawChunk(obj, 0x0300);
		if (!dim || dim->Value.size() < 12)
		{
			fprintf(stderr, "WARNING: mesh '%s' %s without lattice dimensions\n", name.c_str(), what);
			return false;
		}
		memcpy(lat.Dim, &dim->Value[0], 12);
	}
	if (lat.Dim[0] < 2 || lat.Dim[1] < 2 || lat.Dim[2] < 2 || lat.Dim[0] > 64 || lat.Dim[1] > 64 || lat.Dim[2] > 64)
	{
		fprintf(stderr, "WARNING: mesh '%s' %s lattice %ix%ix%i not supported\n", name.c_str(), what,
		        lat.Dim[0], lat.Dim[1], lat.Dim[2]);
		return false;
	}
	const size_t count = (size_t)lat.Dim[0] * lat.Dim[1] * lat.Dim[2];
	const CStorageRaw *pts = findRawChunk(obj, 0x0200);
	const CStorageRaw *off = findRawChunk(obj, fixedSize ? 0x0400 : 0x0500);
	if (!pts || pts->Value.size() != count * 12 || (off && off->Value.size() != count * 12))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s control points or offsets do not match the lattice\n", name.c_str(), what);
		return false;
	}
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	for (uint r = 3; rm && r < rm->nbReferences(); ++r)
	{
		if (rm->getReference(r))
		{
			fprintf(stderr, "WARNING: mesh '%s' %s has animated control points, not supported\n", name.c_str(), what);
			return false;
		}
	}
	lat.Points.resize(count);
	for (size_t i = 0; i < count; ++i)
	{
		float p[3], o[3] = { 0.f, 0.f, 0.f };
		memcpy(p, &pts->Value[i * 12], 12);
		if (off && off->Value.size() == count * 12)
			memcpy(o, &off->Value[i * 12], 12);
		lat.Points[i].x = p[0] - o[0];
		lat.Points[i].y = p[1] - o[1];
		lat.Points[i].z = p[2] - o[2];
	}
	CSceneClass *pb = rm && rm->nbReferences() > 1 ? dynamic_cast<CSceneClass *>(rm->getReference(1)) : nullptr;
	SPB2Block blk;
	float allVertices = 0.f;
	lat.Falloff = 0.f;
	lat.Tension = lat.Continuity = 25.f;
	if (!pb || pb->classDesc()->superClassId() != 0x82 || !readPB2Block(pb, blk) || !ffdParam(blk, 2, allVertices))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s parameters not readable\n", name.c_str(), what);
		return false;
	}
	if (!fixedSize && (!ffdParam(blk, 3, lat.Falloff) || !ffdParam(blk, 4, lat.Tension) || !ffdParam(blk, 5, lat.Continuity)))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s falloff, tension or continuity animated, not supported\n", name.c_str(), what);
		return false;
	}
	lat.AllVertices = allVertices != 0.f;
	lat.Cylinder = false;
	return true;
}

// The lattice padded by one layer on each side (around the cylinder, x wraps instead): beyond an end the
// last segment continues mirrored across the line of the segment before it (copied when the axis has only
// two points), one axis after the other, x first.
struct SFFDPadded
{
	const SFFDLattice *Lat;
	std::vector<Point3M> Cache;
	std::vector<bool> Done;

	void init(const SFFDLattice &lat)
	{
		Lat = &lat;
		const size_t n = (size_t)(lat.Dim[0] + 2) * (lat.Dim[1] + 2) * (lat.Dim[2] + 2);
		Cache.resize(n);
		Done.assign(n, false);
	}

	static Point3M mirror(const Point3M &a, const Point3M &v)
	{
		const Point3M na = ffdNormalize(a);
		const Point3M n = ffdCross(ffdNormalize(ffdCross(na, v)), na);
		return ffdSub(ffdScale(na, ffdDot(v, na)), ffdScale(n, ffdDot(v, n)));
	}

	Point3M at(int i, int j, int k)
	{
		const int *d = Lat->Dim;
		if (Lat->Cylinder)
			i = (i % d[0] + d[0]) % d[0];
		const size_t key = ((size_t)(k + 1) * (d[1] + 2) + (j + 1)) * (d[0] + 2) + (i + 1);
		if (Done[key])
			return Cache[key];
		Point3M r;
		const int idx[3] = { i, j, k };
		bool inside = true;
		for (int a = 0; a < 3; ++a)
			if (idx[a] < 0 || idx[a] >= d[a]) inside = false;
		if (inside)
		{
			r = Lat->Points[((size_t)k * d[1] + j) * d[0] + i];
		}
		else
		{
			int c[3];
			for (int a = 0; a < 3; ++a)
				c[a] = idx[a] < 0 ? 0 : (idx[a] >= d[a] ? d[a] - 1 : idx[a]);
			r = at(c[0], c[1], c[2]);
			for (int a = 0; a < 3; ++a)
			{
				if (idx[a] >= 0 && idx[a] < d[a])
					continue;
				const int step = idx[a] < 0 ? 1 : -1; // towards the inside
				const int c0 = idx[a] < 0 ? 0 : d[a] - 1;
				int p0[3] = { idx[0], idx[1], idx[2] }, p1[3] = { idx[0], idx[1], idx[2] }, p2[3] = { idx[0], idx[1], idx[2] };
				p0[a] = c0;
				p1[a] = c0 + step;
				p2[a] = c0 + 2 * step;
				const Point3M x0 = at(p0[0], p0[1], p0[2]);
				const Point3M x1 = at(p1[0], p1[1], p1[2]);
				Point3M e;
				if (d[a] > 2)
				{
					const Point3M x2 = at(p2[0], p2[1], p2[2]);
					e = mirror(ffdSub(x0, x1), ffdSub(x1, x2));
				}
				else
				{
					e = ffdSub(x0, x1);
				}
				r = ffdAdd(r, e);
			}
		}
		Cache[key] = r;
		Done[key] = true;
		return r;
	}
};

struct SFFDBoxDeformer
{
	SFFDLattice *Lat;
	SFFDPadded *Padded;
	float M1, M2;

	void weights(float s, float w[4]) const
	{
		const float s2 = s * s, s3 = s2 * s;
		const float h0 = 1.f - 3.f * s2 + 2.f * s3, h1 = 3.f * s2 - 2.f * s3, h2 = s - 2.f * s2 + s3, h3 = s3 - s2;
		w[0] = -h2 * M2;
		w[1] = h0 + h2 * (M2 - M1) - h3 * M1;
		w[2] = h1 + h2 * M1 + h3 * (M1 - M2);
		w[3] = h3 * M2;
	}

	static Point3M blend(const Point3M k[4], const float w[4])
	{
		Point3M r;
		r.x = k[0].x * w[0] + k[1].x * w[1] + k[2].x * w[2] + k[3].x * w[3];
		r.y = k[0].y * w[0] + k[1].y * w[1] + k[2].y * w[2] + k[3].y * w[3];
		r.z = k[0].z * w[0] + k[1].z * w[1] + k[2].z * w[2] + k[3].z * w[3];
		return r;
	}

	Point3M map(Point3M p) const
	{
		const SFFDLattice &lat = *Lat;
		Point3M pp = transformPoint(p, lat.L);
		if (lat.Cylinder)
		{
			const float x = pp.x - 0.5f, y = pp.y - 0.5f;
			const float l = sqrtf(x * x + y * y);
			float angle = 0.f;
			if (l != 0.f)
			{
				angle = atan2f(y, x);
				if (angle < 0.f) angle += 2.f * (float)NLMISC::Pi;
				angle /= 2.f * (float)NLMISC::Pi;
			}
			pp.x = angle;
			pp.y = l / 0.7071067812f;
		}
		const float c[3] = { pp.x, pp.y, pp.z };
		const int first = lat.Cylinder ? 1 : 0;
		float dist = 0.f;
		if (!lat.AllVertices)
		{
			for (int a = first; a < 3; ++a)
				if (c[a] < -0.001f || c[a] > 1.001f) return p;
		}
		else if (lat.Falloff > 0.f)
		{
			for (int a = first; a < 3; ++a)
			{
				const float over = c[a] < 0.f ? -c[a] : (c[a] > 1.f ? c[a] - 1.f : 0.f);
				if (over > dist) dist = over;
			}
			if (dist > lat.Falloff) return p;
		}
		int cell[3];
		float local[3];
		for (int a = 0; a < 3; ++a)
		{
			if (a == 0 && lat.Cylinder)
			{
				const float x = c[0] * lat.Dim[0];
				cell[0] = (int)x;
				local[0] = x - cell[0];
				continue;
			}
			const float x = c[a] * (lat.Dim[a] - 1);
			int i = (int)x;
			if (i < 0) i = 0;
			if (i > lat.Dim[a] - 2) i = lat.Dim[a] - 2;
			cell[a] = i;
			local[a] = x - i;
		}
		float wu[4], wv[4], ww[4];
		weights(local[0], wu);
		weights(local[1], wv);
		weights(local[2], ww);
		Point3M u[4];
		for (int a = 0; a < 4; ++a)
		{
			Point3M t[4];
			for (int b = 0; b < 4; ++b)
			{
				Point3M k[4];
				for (int cc = 0; cc < 4; ++cc)
					k[cc] = Padded->at(cell[0] - 1 + a, cell[1] - 1 + b, cell[2] - 1 + cc);
				t[b] = blend(k, ww);
			}
			u[a] = blend(t, wv);
		}
		Point3M r = transformPoint(blend(u, wu), lat.InvL);
		if (dist > 0.f)
		{
			float s = dist / lat.Falloff;
			s = s * s * (3.f - 2.f * s);
			r = ffdAdd(ffdScale(p, s), ffdScale(r, 1.f - s));
		}
		return r;
	}
};

static void ffdBoxDeformer(SFFDLattice &lat, SFFDPadded &padded, SFFDBoxDeformer &d)
{
	padded.init(lat);
	d.Lat = &lat;
	d.Padded = &padded;
	const float t = (lat.Tension - 25.f) / 25.f, c = (lat.Continuity - 25.f) / 25.f;
	const float a = (1.f - t) / 2.f;
	d.M1 = a * (1.f - c);
	d.M2 = a * (1.f + c);
}

struct SFFDBezierDeformer
{
	SFFDLattice *Lat;

	static void bernstein(int n, float s, float b[4])
	{
		const float t = 1.f - s;
		if (n == 2) { b[0] = t; b[1] = s; }
		else if (n == 3) { b[0] = t * t; b[1] = 2.f * s * t; b[2] = s * s; }
		else { b[0] = t * t * t; b[1] = 3.f * s * t * t; b[2] = 3.f * s * s * t; b[3] = s * s * s; }
	}

	Point3M map(Point3M p) const
	{
		const SFFDLattice &lat = *Lat;
		const Point3M pp = transformPoint(p, lat.L);
		if (!lat.AllVertices && (pp.x < -0.001f || pp.x > 1.001f || pp.y < -0.001f || pp.y > 1.001f
		                         || pp.z < -0.001f || pp.z > 1.001f))
			return p;
		const int n = lat.Dim[0];
		float bx[4], by[4], bz[4];
		bernstein(n, pp.x, bx);
		bernstein(n, pp.y, by);
		bernstein(n, pp.z, bz);
		Point3M q = { 0.f, 0.f, 0.f };
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				for (int k = 0; k < n; ++k)
					q = ffdAdd(q, ffdScale(lat.Points[((size_t)i * n + j) * n + k], bx[i] * by[j] * bz[k]));
		return transformPoint(q, lat.InvL);
	}
};

// FFD(box) and FFD(Cyl) modifiers (cylinder: true), and the fixed-size FFDs (fixedSize 2, 3 or 4).
static void applyFFD(CSceneClass *mod, CStorageContainer *app, SEvalMesh &mesh, int fixedSize, bool cylinder,
                     const char *what, const std::string &name, std::vector<std::string> *warnings)
{
	SFFDLattice lat;
	if (!readFFDLattice(mod, fixedSize, lat, what, name))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s evaluated without it\n", name.c_str(), what);
		if (warnings) warnings->push_back(std::string("modifier:") + what);
		return;
	}
	lat.Cylinder = cylinder;
	Matrix3M gizmo = Matrix3M::identity();
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	if (CControlPRS *prs = rm && rm->nbReferences() > 0 ? dynamic_cast<CControlPRS *>(rm->getReference(0)) : nullptr)
	{
		gizmo = composePRS(posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController())),
		                   rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController())),
		                   scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController())));
	}
	Matrix3M ctx = Matrix3M::identity();
	CDerivedObject::modAppContextTM(app, &ctx.m[0][0]);
	float box[6];
	if (!CDerivedObject::modAppContextBox(app, box))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s without its mod-context box; evaluated without it\n", name.c_str(), what);
		if (warnings) warnings->push_back(std::string("modifier:") + what);
		return;
	}
	if (fixedSize && (box[3] < box[0] || box[4] < box[1] || box[5] < box[2]))
	{
		// applied to an empty selection: the fixed-size lattices take the box as it is and would collapse the mesh
		fprintf(stderr, "WARNING: mesh '%s' %s with an empty mod-context box; evaluated without it\n", name.c_str(), what);
		if (warnings) warnings->push_back(std::string("modifier:") + what);
		return;
	}
	ffdBox(box, !fixedSize);
	ffdSpace(lat, ctx, gizmo, box);
	if (fixedSize)
	{
		SFFDBezierDeformer d;
		d.Lat = &lat;
		deformSelected(mesh, d, what, name, warnings);
	}
	else
	{
		SFFDPadded padded;
		SFFDBoxDeformer d;
		ffdBoxDeformer(lat, padded, d);
		deformSelected(mesh, d, what, name, warnings);
	}
}

// Moves points into world space (through the object TM of the node), deforms them, and back.
template <class TDeformer>
struct SWorldSpaceDeformer
{
	const TDeformer *Deformer;
	Matrix3M ToWorld, FromWorld;
	Point3M map(Point3M p) const { return transformPoint(Deformer->map(transformPoint(p, ToWorld)), FromWorld); }
};

// FFD Binding: the world-space modifier a space-warp FFD puts on each bound node. Reference 1 is the node of
// the space warp; its object holds the lattice, its object TM at tick 0 places it. The box comes from the
// space warp's size: FFD(Cyl) (-r, -r, -h/2)..(r, r, h/2) with r parameter 7 and h parameter 6, FFD(Box)
// width 7, length 6, height 8 (not in the corpus).
static void applyFFDBinding(CSceneClass *mod, INode &node, SEvalMesh &mesh, const std::string &name,
                            std::vector<std::string> *warnings)
{
	static const char *what = "FFD Binding";
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mod);
	CNodeImpl *warpNode = rm && rm->nbReferences() > 1 ? dynamic_cast<CNodeImpl *>(rm->getReference(1)) : nullptr;
	CSceneClass *warp = warpNode ? dynamic_cast<CSceneClass *>(warpNode->getReference(1)) : nullptr;
	const NLMISC::CClassId wcid = warp ? warp->classDesc()->classId() : NLMISC::CClassId::Null;
	const bool cyl = wcid == NLMISC::CClassId(0xfa4700be, 0xbbe85051);
	const bool boxWarp = wcid == NLMISC::CClassId(0x67ea40b3, 0xfe7a30c4);
	SFFDLattice lat;
	if (!cyl && !boxWarp)
	{
		fprintf(stderr, "WARNING: mesh '%s' %s does not name an FFD space warp node; evaluated without it\n",
		        name.c_str(), what);
		if (warnings) warnings->push_back("modifier:ffdbinding");
		return;
	}
	if (!readFFDLattice(warp, 0, lat, what, name))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s evaluated without it\n", name.c_str(), what);
		if (warnings) warnings->push_back("modifier:ffdbinding");
		return;
	}
	lat.Cylinder = cyl;
	CReferenceMaker *wrm = dynamic_cast<CReferenceMaker *>(warp);
	CSceneClass *pb = wrm && wrm->nbReferences() > 1 ? dynamic_cast<CSceneClass *>(wrm->getReference(1)) : nullptr;
	SPB2Block blk;
	float p6 = 0.f, p7 = 0.f, p8 = 0.f;
	if (!pb || !readPB2Block(pb, blk) || !ffdParam(blk, 6, p6) || !ffdParam(blk, 7, p7) || (boxWarp && !ffdParam(blk, 8, p8)))
	{
		fprintf(stderr, "WARNING: mesh '%s' %s space warp size not readable; evaluated without it\n", name.c_str(), what);
		if (warnings) warnings->push_back("modifier:ffdbinding");
		return;
	}
	float box[6];
	if (cyl)
	{
		box[0] = -p7; box[1] = -p7; box[2] = -p6 / 2.f;
		box[3] = p7; box[4] = p7; box[5] = p6 / 2.f;
	}
	else
	{
		box[0] = -p7 / 2.f; box[1] = -p6 / 2.f; box[2] = -p8 / 2.f;
		box[3] = p7 / 2.f; box[4] = p6 / 2.f; box[5] = p8 / 2.f;
	}
	for (int a = 0; a < 3; ++a)
		if (box[3 + a] < box[a]) std::swap(box[a], box[3 + a]);
	ffdBox(box, false);
	SNodeTMCache cache;
	Matrix3M warpTM = objectTMAt0(warpNode, cache);
	ffdSpace(lat, inverseM3(warpTM), Matrix3M::identity(), box);
	SFFDPadded padded;
	SFFDBoxDeformer d;
	ffdBoxDeformer(lat, padded, d);
	SWorldSpaceDeformer<SFFDBoxDeformer> w;
	w.Deformer = &d;
	w.ToWorld = objectTMAt0(dynamic_cast<CNodeImpl *>(&node), cache);
	w.FromWorld = inverseM3(w.ToWorld);
	deformSelected(mesh, w, what, name, warnings);
}

// ---------------------------------------------------------------------------------------------

// Extract a parametric primitive's mesh into SEvalMesh: use the shared PRIMMESH library for
// GT-exact topology + per-face matId/smGroup (§10z-cinq). Map channel 1 is filled for Box and
// Plane "generate mapping coords" UVs (Cylinder/Sphere seam handling still open — §10z-cinq
// follow-up); when UVs aren't produced, buildMeshInterface's empty-channel path supplies
// (0,0,0), correct for untextured/collision prims.
static bool extractParametricPrimitive(CSceneClass *base, SEvalMesh &out, const std::string &name)
{
	// Locate the primitive's ref-0 old-style ParamBlock (superclass 0x8).
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(base);
	CSceneClass *pblock = nullptr;
	for (uint r = 0; rm && r < rm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
		if (ref && ref->classDesc()->superClassId() == SCLASS_PBLOCK)
		{
			pblock = ref;
			break;
		}
	}
	std::map<sint32, OLDPBLOCK::SParam> params;
	if (pblock)
	{
		OLDPBLOCK::readOldParamBlock(pblock, params);
	}
	else
	{
		// Max 4+ primitive classes (Plane, and any Box/Cylinder/Sphere re-created in a later Max)
		// store a ParamBlock2 (superclass 0x82) instead of the old-style pblock. The PB2 param ids
		// for these prims coincide with the old pblock indices the generator uses (Plane: 0=length,
		// 1=width, 2/3=segments — SDK PBlock2 param ids; verified on fy_cn_smokehouse.max's
		// 'fy_smoke_water' Plane against its reference CWaterShape polygon extent). Translate the
		// inline constants into the same index-keyed param map.
		for (uint r = 0; rm && r < rm->nbReferences(); ++r)
		{
			CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
			if (!ref || ref->classDesc()->superClassId() != 0x82) continue;
			SPB2Block blk;
			if (!readPB2Block(ref, blk)) continue;
			for (std::map<uint16, SPB2Param>::const_iterator it = blk.Params.begin();
			     it != blk.Params.end(); ++it)
			{
				const SPB2Param &p = it->second;
				if (!p.HasConstant || p.IsTab) continue;
				OLDPBLOCK::SParam &dst = params[(sint32)p.Id];
				dst.IsPoint3 = false;
				if (p.Type == PB2_FLOAT || p.Type == PB2_WORLD || p.Type == PB2_ANGLE
					|| p.Type == PB2_PCNT_FRAC)
				{
					dst.IsInt = false;
					dst.V[0] = p.F[0];
					dst.V[1] = dst.V[2] = 0.f;
					dst.I = (sint32)p.F[0];
				}
				else if (p.Type == PB2_INT || p.Type == PB2_BOOL)
				{
					dst.IsInt = true;
					dst.I = p.I;
					dst.V[0] = (float)p.I;
					dst.V[1] = dst.V[2] = 0.f;
				}
				else
				{
					params.erase((sint32)p.Id);
					continue;
				}
				if (getenv("PMB_PRIM_DUMP"))
					fprintf(stderr, "PMB_PRIM_DUMP '%s' pb2 block %u param %u type 0x%x f=%g i=%d\n",
					        name.c_str(), blk.BlockId, p.Id, p.Type, dst.V[0], dst.I);
			}
		}
		if (params.empty())
		{
			fprintf(stderr, "WARNING: parametric prim '%s' without ref-0 pblock\n", name.c_str());
			return false;
		}
	}

	std::vector<NLMISC::CVector> pverts;
	std::vector<PRIMMESH::SPrimTri> ptris;
	// UV generation for Box + Plane (Cylinder/Sphere seam handling still open — §10z-cinq).
	// Per-corner UVs (3 × nFaces); the mesh-build path dedups the final VB on (pos,n,uv).
	std::vector<NLMISC::CVector> puvs;
	std::vector<uint32> puvFaces;
	if (!PRIMMESH::buildParametricMesh(base->classDesc()->classId(), params, pverts, ptris, &puvs, &puvFaces))
		return false;

	out.Verts.resize(pverts.size());
	if (!pverts.empty()) memcpy(&out.Verts[0], &pverts[0], pverts.size() * 12);
	out.Faces.resize(ptris.size());
	// PRIMMESH::buildParametricMesh assigns Max's own per-face matId + smoothing group per prim
	// class (Box: 6 distinct per-side matIds + smoothing bits; Cylinder: bottom cap/sides/top cap;
	// Sphere/Plane: uniform). Corpus-validated GT: ~/shape_export_dataset/manifest.txt. Before
	// this, all faces got matId 0 + sg 1, which collapsed a Box + MultiMtl (box01.shape corpus
	// class, DIFF "materials: 1 vs 6") into a single rdrpass and produced the wrong smoothing.
	for (uint i = 0; i < ptris.size(); ++i)
	{
		out.Faces[i].V[0] = ptris[i].V[0];
		out.Faces[i].V[1] = ptris[i].V[1];
		out.Faces[i].V[2] = ptris[i].V[2];
		out.Faces[i].SmGroup = ptris[i].SmGroup;
		out.Faces[i].Flags = (ptris[i].MatId & 0xFFFFu) << MAX_FACE_MATID_SHIFT;
		out.Faces[i].Flags |= 0x7; // all edges visible
	}
	// Map channel 1: the primitive's own shared map verts when the generator numbers them (Box, hemisphere), else
	// per-corner UVs: face t's three corners map to indices t*3+{0,1,2} into the UV vert list.
	if (!puvs.empty() && puvFaces.size() == ptris.size() * 3)
	{
		SMapChannel &mc = out.Maps[1];
		mc.Verts.resize(puvs.size());
		memcpy(&mc.Verts[0], &puvs[0], puvs.size() * 12);
		mc.Faces.resize(ptris.size());
		for (uint i = 0; i < ptris.size(); ++i)
			for (uint c = 0; c < 3; ++c)
				mc.Faces[i].T[c] = puvFaces[i * 3 + c];
	}
	else if (!puvs.empty() && puvs.size() == ptris.size() * 3)
	{
		SMapChannel &mc = out.Maps[1];
		mc.Verts.resize(puvs.size());
		if (!puvs.empty()) memcpy(&mc.Verts[0], &puvs[0], puvs.size() * 12);
		mc.Faces.resize(ptris.size());
		for (uint i = 0; i < ptris.size(); ++i)
		{
			mc.Faces[i].T[0] = i * 3 + 0;
			mc.Faces[i].T[1] = i * 3 + 1;
			mc.Faces[i].T[2] = i * 3 + 2;
		}
	}
	return true;
}

// Spline (Shape-superclass) → mesh, replicating Max's SplineShape→TriObject conversion that the
// reference exporter triggers via ConvertToType: open splines yield an EMPTY mesh (0 verts,
// 0 faces — the corpus references literally store empty CMesh shapes for them), closed splines
// yield a capped mesh. Decode via the shared SPLINESHAPE library; cap replication is developed
// against the 7 closed-spline corpus references (see PMB_SPLINE_DUMP).
static bool extractSplineMesh(CSceneClass *base, SEvalMesh &out, const std::string &name)
{
	SPLINESHAPE::SShape shape;
	if (!SPLINESHAPE::decodeShapeObject(base, shape))
	{
		// Parametric shape classes store no BezierShape chunks — generate. Rectangle (0x1065):
		// knots from pblock length/width/fillet per the SDK sample source (SimpleSpline carries
		// TWO old pblocks: the interpolation block {steps int, optimize bool, adaptive bool} and
		// the shape block {length, width, fillet floats} — identified by content).
		if (base->classDesc()->classId() == SPLINESHAPE::CLASSID_RECTANGLE)
		{
			CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(base);
			float length = 0.f, width = 0.f, fillet = 0.f;
			bool haveParams = false;
			for (uint r = 0; rm && r < rm->nbReferences(); ++r)
			{
				CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
				if (!ref || ref->classDesc()->superClassId() != SCLASS_PBLOCK) continue;
				std::map<sint32, OLDPBLOCK::SParam> params;
				OLDPBLOCK::readOldParamBlock(ref, params);
				if (getenv("PMB_SPLINE_DUMP"))
				{
					fprintf(stderr, "PMB_SPLINE_DUMP '%s' rect pblock ref[%u]:", name.c_str(), r);
					for (std::map<sint32, OLDPBLOCK::SParam>::iterator it = params.begin();
					     it != params.end(); ++it)
						fprintf(stderr, " [%d]=%s%g", (int)it->first,
						        it->second.IsInt ? "i" : "f",
						        it->second.IsInt ? (double)it->second.I : (double)it->second.V[0]);
					fprintf(stderr, "\n");
				}
				// The interpolation block's params are ints/bools; the shape block's are floats.
				if (params.size() >= 3 && !params[0].IsInt && !params[1].IsInt)
				{
					length = OLDPBLOCK::paramFloat(params, 0);
					width = OLDPBLOCK::paramFloat(params, 1);
					fillet = OLDPBLOCK::paramFloat(params, 2);
					haveParams = true;
				}
				else if (params.size() >= 1 && params[0].IsInt)
				{
					shape.Steps = params[0].I; // IPB_STEPS
					shape.HaveSteps = true;
				}
			}
			if (haveParams)
			{
				SPLINESHAPE::SSpline sp;
				SPLINESHAPE::buildRectangleKnots(length, width, fillet, sp);
				shape.Curves.push_back(sp);
			}
		}
		if (shape.Curves.empty())
		{
			fprintf(stderr, "WARNING: shape-class '%s' — no spline data decoded\n", name.c_str());
			return false;
		}
	}
	if (getenv("PMB_SPLINE_DUMP"))
	{
		// Full chunk tree of the shape object (orphaned + chunks) — locating the real closed flag
		// and any tessellation params.
		struct SDump
		{
			static void tree(const CStorageContainer::TStorageObjectContainer &ch, uint depth)
			{
				std::string ind(depth * 2, ' ');
				for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
				{
					CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
					if (sub && !raw)
					{
						fprintf(stderr, "%s0x%04x container\n", ind.c_str(), it->first);
						tree(sub->chunks(), depth + 1);
					}
					else if (raw)
					{
						fprintf(stderr, "%s0x%04x leaf size=%u", ind.c_str(), it->first, (uint)raw->Value.size());
						size_t n = std::min(raw->Value.size(), (size_t)32);
						fprintf(stderr, " [");
						for (size_t k = 0; k < n; ++k)
						{
							if (k && k % 4 == 0) fprintf(stderr, " ");
							fprintf(stderr, "%02x", raw->Value[k]);
						}
						fprintf(stderr, "]\n");
					}
				}
			}
		};
		fprintf(stderr, "PMB_SPLINE_DUMP node='%s' object chunk tree (orphaned):\n", name.c_str());
		SDump::tree(base->orphanedChunks(), 1);
		fprintf(stderr, "PMB_SPLINE_DUMP node='%s' object chunk tree (chunks):\n", name.c_str());
		SDump::tree(base->chunks(), 1);
		fprintf(stderr, "PMB_SPLINE_DUMP node='%s' curves=%u\n", name.c_str(), (uint)shape.Curves.size());
		for (uint c = 0; c < shape.Curves.size(); ++c)
		{
			const SPLINESHAPE::SSpline &sp = shape.Curves[c];
			fprintf(stderr, "  curve %u closed=%d knots=%u\n", c, (int)sp.Closed, (uint)sp.Knots.size());
			for (uint k = 0; k < sp.Knots.size(); ++k)
				fprintf(stderr, "    k%u kt=%d lt=%d du=%g p0=(%.9g %.9g %.9g) p1=(%.9g %.9g %.9g) p2=(%.9g %.9g %.9g) fl=0x%x\n",
				        k, sp.Knots[k].KType, sp.Knots[k].LType, sp.Knots[k].Du,
				        sp.Knots[k].Knot.x, sp.Knots[k].Knot.y, sp.Knots[k].Knot.z,
				        sp.Knots[k].InVec.x, sp.Knots[k].InVec.y, sp.Knots[k].InVec.z,
				        sp.Knots[k].OutVec.x, sp.Knots[k].OutVec.y, sp.Knots[k].OutVec.z,
				        sp.Knots[k].Flags);
		}
	}
	uint nClosed = 0;
	for (uint c = 0; c < shape.Curves.size(); ++c)
		if (shape.Curves[c].Closed && shape.Curves[c].Knots.size() >= 3)
			++nClosed;
	if (nClosed > 1)
		fprintf(stderr, "WARNING: shape-class '%s' has %u closed curves; capping the first only "
		                "(hole capping has no corpus instance)\n", name.c_str(), nClosed);

	SPLINEMESH::SSplineMesh sm;
	SPLINEMESH::buildSplineMesh(shape, shape.HaveSteps ? shape.Steps : 0,
	                            /*optimize*/ true, /*adaptive*/ false, sm);
	out.Verts.resize(sm.Verts.size());
	if (!sm.Verts.empty()) memcpy(&out.Verts[0], &sm.Verts[0], sm.Verts.size() * 12);
	out.Faces.resize(sm.Faces.size());
	for (uint i = 0; i < sm.Faces.size(); ++i)
	{
		SEvalFace &f = out.Faces[i];
		f.V[0] = sm.Faces[i].V[0];
		f.V[1] = sm.Faces[i].V[1];
		f.V[2] = sm.Faces[i].V[2];
		f.SmGroup = 1;   // cap faces share one smoothing group (reference normals are smooth)
		f.Flags = 0x7;   // matID 0, all edges visible
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
		out.SelLevelUnknown = out.SelBitsUnknown = true; // the poly mesh selections are not decoded
	}
	else if (cid == PRIMMESH::CLASSID_BOX || cid == PRIMMESH::CLASSID_CYLINDER
	         || cid == PRIMMESH::CLASSID_SPHERE || cid == PRIMMESH::CLASSID_PLANE)
	{
		if (!extractParametricPrimitive(base, out, name))
			return false;
	}
	else
	{
		// Shape-superclass base objects (SplineShape 0x0a, Line 0x1040, Rectangle 0x1065): the
		// reference plugin CONVERTS them to TriObject (os.obj->ConvertToType), which produces
		// Max's spline mesh — a capped planar-ish mesh for closed splines, an EMPTY mesh (0 verts,
		// 0 faces) for open ones. Corpus GT: the 9 characters/sfx spline references (rectangle01
		// 4v/2f, shape01 18v/16f, shape04 9v/7f, …, shape02/03 + tr_wea_hache2m_trail_00 empty).
		TSClassId scid = base->classDesc()->superClassId();
		if (scid == SCLASS_SHAPE)
		{
			if (!extractSplineMesh(base, out, name))
			{
				if (warnings) warnings->push_back("shape-class:" + cid.toString());
				return false;
			}
		}
		else
		{
			fprintf(stderr, "WARNING: mesh extraction for object class %s ('%s') not implemented\n",
			        cid.toString().c_str(), name.c_str());
			if (warnings) warnings->push_back("object:" + cid.toString());
			return false;
		}
	}

	// Apply modifiers base-upward: mods was collected outermost wrapper first; within a wrapper,
	// reference order = stack order bottom-up already, so replay back-to-front.
	if (getenv("PMB_STACK_DUMP"))
		fprintf(stderr, "PMB_STACK_DUMP '%s' base %s verts=%u faces=%u mods=%u\n", name.c_str(),
		        cid.toString().c_str(), (uint)out.Verts.size(), (uint)out.Faces.size(), (uint)mods.size());
	for (uint i = (uint)mods.size(); i > 0; --i)
	{
		CSceneClass *mod = mods[i - 1];
		CStorageContainer *app = (i - 1) < modApps.size() ? modApps[i - 1] : nullptr;
		NLMISC::CClassId mcid = mod->classDesc()->classId();
		if (modifierSwitchedOff(mod, name))
		{
			if (getenv("PMB_STACK_DUMP"))
				fprintf(stderr, "PMB_STACK_DUMP '%s' mod %s switched off, skipped\n", name.c_str(),
				        mcid.toString().c_str());
			continue;
		}
		if (getenv("PMB_STACK_DUMP"))
		{
			std::string maps;
			for (std::map<int, SMapChannel>::const_iterator mt = out.Maps.begin(); mt != out.Maps.end(); ++mt)
				maps += NLMISC::toString(" ch%d=%u/%u", mt->first, (uint)mt->second.Verts.size(),
				                         (uint)mt->second.Faces.size());
			fprintf(stderr, "PMB_STACK_DUMP '%s' before %s '%s': verts=%u faces=%u%s\n", name.c_str(),
			        mcid.toString().c_str(), mod->className().c_str(), (uint)out.Verts.size(),
			        (uint)out.Faces.size(), maps.c_str());
		}
		if (mcid == NLMISC::CClassId(0x00000050, 0x00000000)) // Edit Mesh
		{
			EDITMESH::SEdits edits;
			if (app && EDITMESH::readModApp(app, edits))
				applyEditMeshEdits(edits, out, name);
			applyEditMeshSelection(mod, app, out);
		}
		else if (mcid == NLMISC::CClassId(0x25215824, 0x00000000)) // XForm
		{
			applyXForm(mod, app, out, name);
		}
		else if (mcid == NLMISC::CClassId(0x00000010, 0x00000000)
		         && mod->classDesc()->superClassId() == 0x810) // Bend
		{
			applyBend(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x00000090, 0x00000000)
		         && mod->classDesc()->superClassId() == 0x810) // Twist
		{
			applyTwist(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x00000020, 0x00000000)
		         && mod->classDesc()->superClassId() == 0x810) // Taper
		{
			applyTaper(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x17bb6854, 0xa5cba2a3)) // Morpher
		{
			applyMorpher(mod, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x8ab36cc5, 0x82d7fe74) && mod->classDesc()->superClassId() == 0x810)
		{
			applyFFD(mod, app, out, 0, false, "FFD(box)", name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x98f37a63, 0x3ffe9bca) && mod->classDesc()->superClassId() == 0x810)
		{
			applyFFD(mod, app, out, 0, true, "FFD(Cyl)", name, warnings);
		}
		else if (mcid.a() == 0x21325596 && (mcid.b() == 0x2cd10bd0 || mcid.b() == 0x2cd10bd9 || mcid.b() == 0x2cd10bd8)
		         && mod->classDesc()->superClassId() == 0x810)
		{
			const int size = mcid.b() == 0x2cd10bd0 ? 2 : (mcid.b() == 0x2cd10bd9 ? 3 : 4);
			applyFFD(mod, app, out, size, false, size == 2 ? "FFD 2x2x2" : (size == 3 ? "FFD 3x3x3" : "FFD 4x4x4"),
			         name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0xd6636ea2, 0x9aa42bf3) && mod->classDesc()->superClassId() == 0x820)
		{
			bool skinned = false;
			for (uint k = 0; k < mods.size(); ++k)
			{
				const NLMISC::CClassId kc = mods[k]->classDesc()->classId();
				if ((kc == NLMISC::CClassId(0x00000100, 0x00000000) && mods[k]->classDesc()->superClassId() == 0x810)
				    || kc == NLMISC::CClassId(0x0095c723, 0x00015666))
					skinned = true;
			}
			if (skinned)
			{
				// a skinned shape is exported in world space; the space warp's round trip is not known there
				fprintf(stderr, "WARNING: mesh '%s' FFD Binding on a skinned mesh not supported; evaluated without it\n",
				        name.c_str());
				if (warnings) warnings->push_back("modifier:ffdbinding");
			}
			else
			{
				applyFFDBinding(mod, node, out, name, warnings);
			}
		}
		else if (mcid == NLMISC::CClassId(0x79aa6e1d, 0x71a075b7) && mod->classDesc()->superClassId() == 0x810)
		{
			applyEditPoly(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x000100b0, 0x00bc2d21) && mod->classDesc()->superClassId() == 0x810)
		{
			applyStretch(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x8aad7d94, 0x80aae03e) && mod->classDesc()->superClassId() == 0x810)
		{
			// Copy Map: the reference exports of all seven corpus instances behave as if it were absent. On
			// their lightmapped faces the lightmap UVs are a uniform scale of the channel 2 below it, not of
			// channel 1 (Tr_mairie_lanterne_interne_village_a, FY_Acc_StatueFem, FY_CN_caravanserail_B), most
			// likely because its plugin was missing where they were made. It is passed through to match them;
			// its parameters (old ParamBlock 1, 2, 0, 0, 0 everywhere) are not decoded, and other values warn.
			typedef PIPELINE::MAX::BUILTIN::CParamBlock CParamBlock;
			CReferenceMaker *crm = dynamic_cast<CReferenceMaker *>(mod);
			CParamBlock *cpb = crm && crm->nbReferences() > 0 ? dynamic_cast<CParamBlock *>(crm->getReference(0)) : nullptr;
			static const sint32 s_Corpus[5] = { 1, 2, 0, 0, 0 };
			bool asCorpus = cpb != nullptr;
			for (sint32 k = 0; k < 5 && asCorpus; ++k)
			{
				sint32 v;
				asCorpus = cpb->getInt(k, v) && v == s_Corpus[k];
			}
			if (!asCorpus)
			{
				fprintf(stderr, "WARNING: mesh '%s' Copy Map with other settings than the corpus has; passed through\n",
				        name.c_str());
				if (warnings) warnings->push_back("modifier:copymap");
			}
		}
		else if (mcid == NLMISC::CClassId(0x02df2e3a, 0x72ba4e1f) && mod->classDesc()->superClassId() == 0x810)
		{
			applyUnwrapUvw(mod, node, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0xef92aa7c, 0x511bbe75) && mod->classDesc()->superClassId() == 0x810)
		{
			applyMirror(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x73d8ff93, 0x00000000) && mod->classDesc()->superClassId() == 0x810)
		{
			applyMeshSelect(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x663b6fab, 0x73f47385) && mod->classDesc()->superClassId() == 0x810)
		{
			applyOldVertexPaint(mod, app, out, name, warnings);
		}
		else if (mcid == NLMISC::CClassId(0x40c7005e, 0x2a95082c)) // Nel VertexTreePaint
		{
			std::vector<uint32> colors;
			if (readVertexPaintColors(app, colors))
				applyVertexPaint(colors, out, name);
		}
		else if ((mcid == NLMISC::CClassId(0x00000100, 0x00000000)
		          && mod->classDesc()->superClassId() == 0x810) // Physique (geometry-neutral)
		         || mcid == NLMISC::CClassId(0x0095c723, 0x00015666)) // Skin (geometry-neutral)
		{
			// Skinning weights are applied after mesh eval (PHYSIQUESKIN); the modifier itself
			// does not rewrite geometry at export time (the reference disables it first).
		}
		else if (UVWMAP::isUvwMapModifier(mod))
		{
			// UVW Map (pipeline_max_export_common/uvw_map_mod): planar, cylindrical, spherical, box and face;
			// ball and the newer types warn. At face level only the selected faces are mapped.
			const uint typeMask = (1u << UVWMAP::MAP_PLANAR) | (1u << UVWMAP::MAP_CYLINDRICAL)
				| (1u << UVWMAP::MAP_SPHERICAL) | (1u << UVWMAP::MAP_BOX) | (1u << UVWMAP::MAP_FACE);
			const uint nf = (uint)out.Faces.size();
			std::vector<sint32> faceVerts(nf * 3);
			std::vector<uint32> faceFlags(nf);
			for (uint f = 0; f < nf; ++f)
			{
				for (uint c2 = 0; c2 < 3; ++c2)
					faceVerts[f * 3 + c2] = (sint32)out.Faces[f].V[c2];
				faceFlags[f] = out.Faces[f].Flags;
			}
			std::map<int, UVWMAP::SMeshView::SChannel> chans;
			for (std::map<int, SMapChannel>::const_iterator it = out.Maps.begin(); it != out.Maps.end(); ++it)
			{
				if (it->second.Faces.size() != nf)
					continue;
				UVWMAP::SMeshView::SChannel &sc = chans[it->first];
				sc.UVs.resize(it->second.Verts.size());
				for (uint k = 0; k < it->second.Verts.size(); ++k)
					sc.UVs[k] = NLMISC::CVector(it->second.Verts[k].x, it->second.Verts[k].y, it->second.Verts[k].z);
				sc.FaceUVs.resize(nf * 3);
				for (uint f = 0; f < nf; ++f)
					for (uint c2 = 0; c2 < 3; ++c2)
						sc.FaceUVs[f * 3 + c2] = (sint32)it->second.Faces[f].T[c2];
			}
			const bool faceLevel = !out.SelLevelUnknown && out.SelLevel == 4;
			const bool faceSelKnown = faceLevel && !out.SelBitsUnknown && out.SelFaces.size() == nf;
			if (out.SelLevelUnknown || (faceLevel && !faceSelKnown))
			{
				fprintf(stderr, "WARNING: mesh '%s' UVW Map applied to every face: the selection it sees is not known\n",
				        name.c_str());
				if (warnings) warnings->push_back("selection:uvwmap");
			}
			UVWMAP::SMeshView mv;
			mv.Verts = reinterpret_cast<std::vector<MAXMATH::Point3M> *>(&out.Verts);
			mv.FaceVerts = &faceVerts;
			mv.FaceFlags = &faceFlags;
			mv.FaceSel = faceSelKnown ? &out.SelFaces : nullptr;
			mv.Maps = &chans;
			int mapType = -1, channel = -1;
			uint unmapped = 0;
			if (UVWMAP::applyUvwMap(mod, app, mv, typeMask, &mapType, &channel, &unmapped))
			{
				if (unmapped)
				{
					fprintf(stderr, "WARNING: mesh '%s' UVW Map face mapping: %u faces without exactly one hidden edge "
					        "get (0, 0)\n", name.c_str(), unmapped);
					if (warnings) warnings->push_back("modifier:uvwmap-face");
				}
				const UVWMAP::SMeshView::SChannel &sc = chans[channel];
				SMapChannel &mc = out.Maps[channel];
				mc.Verts.resize(sc.UVs.size());
				for (uint k = 0; k < sc.UVs.size(); ++k)
				{
					mc.Verts[k].x = sc.UVs[k].x;
					mc.Verts[k].y = sc.UVs[k].y;
					mc.Verts[k].z = sc.UVs[k].z;
				}
				mc.Faces.resize(nf);
				for (uint f = 0; f < nf; ++f)
					for (uint c2 = 0; c2 < 3; ++c2)
						mc.Faces[f].T[c2] = (uint32)sc.FaceUVs[f * 3 + c2];
			}
			else
			{
				fprintf(stderr, "WARNING: mesh '%s' has unhandled modifier %s; evaluated without it (UVW Map type %d "
				        "not supported, or an animated parameter)\n", name.c_str(), mcid.toString().c_str(), mapType);
				if (warnings) warnings->push_back("modifier:" + mcid.toString());
			}
		}
		else if (MAPEXT::isMapExtenderModifier(mod))
		{
			// Map Extender (mapext198m3.dlm): plugin object holds no settings (empty 0x39bf);
			// the computed UVW map is stored flat in the LocalModData cache
			// (0x2500 → 0x2512 → 0x03e8/0x03e9/0x03ea/0x03eb, channel in 0x03f3). Apply as a
			// replacing map channel. Reference NeL exports of these nodes have garbage UVs
			// (§9 / §10i) — our recovery is strictly better; harness still buckets MAPEXT for
			// UV-sensitive comparison against that garbage reference.
			int channel = 1;
			std::vector<NLMISC::CVector> uvs;
			std::vector<uint32> faceUVs;
			std::string err;
			if (MAPEXT::applyMapExtender(mod, app, (uint)out.Faces.size(), channel, uvs, faceUVs, &err))
			{
				SMapChannel &mc = out.Maps[channel];
				mc.Verts.resize(uvs.size());
				if (!uvs.empty())
					memcpy(&mc.Verts[0], &uvs[0], uvs.size() * 12);
				mc.Faces.resize(out.Faces.size());
				for (uint f = 0; f < out.Faces.size(); ++f)
					for (uint c2 = 0; c2 < 3; ++c2)
						mc.Faces[f].T[c2] = faceUVs[f * 3 + c2];
			}
			else
			{
				fprintf(stderr, "WARNING: mesh '%s' Map Extender cache not applied (%s)\n",
				        name.c_str(), err.c_str());
				if (warnings) warnings->push_back("modifier:" + mcid.toString());
			}
		}
		else
		{
			const ISceneClassDesc *desc = mod->classDesc();
			fprintf(stderr, "WARNING: mesh '%s' has unhandled modifier %s '%s' [%s]; evaluated without it\n",
			        name.c_str(), mcid.toString().c_str(), ucstring(desc->displayName()).toUtf8().c_str(),
			        ucstring(desc->dllPluginDesc()->internalName()).toUtf8().c_str());
			if (warnings) warnings->push_back("modifier:" + mcid.toString());
			out.SelLevelUnknown = out.SelBitsUnknown = true; // it may be a selection modifier
		}
		if (getenv("PMB_STACK_DUMP"))
			fprintf(stderr, "PMB_STACK_DUMP '%s' after mod %s verts=%u faces=%u\n", name.c_str(),
			        mcid.toString().c_str(), (uint)out.Verts.size(), (uint)out.Faces.size());
	}
	return true;
}

} /* namespace MESHEVAL */

/* end of file */
