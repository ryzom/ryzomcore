/**
 * \file map_extender.cpp
 * \brief See map_extender.h.
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

#include "map_extender.h"

#include <cstring>

// ---------------------------------------------------------------------------------------------
// ClassDesc

class MapExtenderClassDesc : public ClassDesc
{
public:
	int IsPublic() { return TRUE; }
	void *Create(BOOL /* loading */ = FALSE) { return new MapExtenderMod(); }
	const TCHAR *ClassName() { return _T("Map Extender"); }
#if (MAX_VERSION_MAJOR >= 24)
	virtual const TCHAR *NonLocalizedClassName() { return _T("Map Extender"); }
#endif
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	Class_ID ClassID() { return MAP_EXTENDER_CLASS_ID; }
	const TCHAR *Category() { return _T("NeL Tools"); }
};

static MapExtenderClassDesc MapExtenderDesc;

ClassDesc *GetMapExtenderDesc()
{
	return &MapExtenderDesc;
}

// ---------------------------------------------------------------------------------------------
// MapExtenderData

MapExtenderData::MapExtenderData()
    : NumMapVerts(0), NumFaces(0), Channel(0)
{
}

LocalModData *MapExtenderData::Clone()
{
	MapExtenderData *d = new MapExtenderData();
	d->Chunks = Chunks;
	d->decodeFunctionalSet();
	return d;
}

static const SMapExtChunk *findChunk(const std::vector<SMapExtChunk> &chunks, USHORT id)
{
	for (size_t i = 0; i < chunks.size(); ++i)
		if (chunks[i].Id == id)
			return &chunks[i];
	return NULL;
}

bool MapExtenderData::decodeFunctionalSet()
{
	NumMapVerts = 0;
	NumFaces = 0;
	Channel = 0;
	MapVerts.clear();
	FaceCorners.clear();

	const SMapExtChunk *cNV = findChunk(Chunks, 0x03e8);
	const SMapExtChunk *cUV = findChunk(Chunks, 0x03e9);
	const SMapExtChunk *cNF = findChunk(Chunks, 0x03ea);
	const SMapExtChunk *cFC = findChunk(Chunks, 0x03eb);
	const SMapExtChunk *cCH = findChunk(Chunks, 0x03f3);
	if (!cNV || !cUV || !cNF || !cFC || !cCH)
		return false;
	if (cNV->Data.size() != 4 || cNF->Data.size() != 4 || cCH->Data.size() != 4)
		return false;

	DWORD nv, nf, ch;
	memcpy(&nv, &cNV->Data[0], 4);
	memcpy(&nf, &cNF->Data[0], 4);
	memcpy(&ch, &cCH->Data[0], 4);
	// Corpus channels are 1 or 2; reject anything outside Max's map range as corrupt.
	if (ch < 1 || ch >= MAX_MESHMAPS)
		return false;
	if (cUV->Data.size() != (size_t)nv * sizeof(Point3))
		return false;
	if (cFC->Data.size() != (size_t)nf * 3 * sizeof(DWORD))
		return false;

	NumMapVerts = nv;
	NumFaces = nf;
	Channel = ch;
	MapVerts.resize(nv);
	if (nv)
		memcpy(&MapVerts[0], &cUV->Data[0], (size_t)nv * sizeof(Point3));
	FaceCorners.resize((size_t)nf * 3);
	if (nf)
		memcpy(&FaceCorners[0], &cFC->Data[0], (size_t)nf * 3 * sizeof(DWORD));
	return true;
}

bool MapExtenderData::valid() const
{
	return NumFaces != 0 && !MapVerts.empty() && FaceCorners.size() == (size_t)NumFaces * 3;
}

// ---------------------------------------------------------------------------------------------
// MapExtenderMod

MapExtenderMod::MapExtenderMod()
{
}

RefTargetHandle MapExtenderMod::Clone(RemapDir &remap)
{
	MapExtenderMod *mod = new MapExtenderMod();
	BaseClone(this, mod, remap);
	return mod;
}

void MapExtenderMod::ModifyObject(TimeValue /* t */, ModContext &mc, ObjectState *os, INode * /* node */)
{
	MapExtenderData *d = (MapExtenderData *)mc.localData;
	// A freshly-applied instance has no cache — nothing to apply (this replacement has no
	// authoring path; it exists to evaluate legacy data).
	if (!d || !d->valid())
		return;
	if (!os->obj->IsSubClassOf(triObjectClassID))
		return;
	Mesh &mesh = ((TriObject *)os->obj)->GetMesh();

	// The cache is addressed by face index against the mesh at THIS stack position; a count
	// mismatch means the topology below the modifier changed after the cache was written —
	// refuse rather than mis-index (same rule as the headless pipeline; no genuinely-stale
	// cache exists in the corpus, see Part P).
	if ((DWORD)mesh.getNumFaces() != d->NumFaces)
		return;

	int ch = (int)d->Channel;
	mesh.setMapSupport(ch, TRUE);
	mesh.setNumMapVerts(ch, (int)d->NumMapVerts);
	UVVert *mv = mesh.mapVerts(ch);
	if (mv && d->NumMapVerts)
		memcpy(mv, &d->MapVerts[0], (size_t)d->NumMapVerts * sizeof(Point3));
	TVFace *mf = mesh.mapFaces(ch);
	if (mf)
	{
		for (DWORD i = 0; i < d->NumFaces; ++i)
			mf[i].setTVerts(d->FaceCorners[i * 3 + 0], d->FaceCorners[i * 3 + 1],
			                d->FaceCorners[i * 3 + 2]);
	}

	os->obj->UpdateValidity(TEXMAP_CHAN_NUM, FOREVER);
}

// ---------------------------------------------------------------------------------------------
// I/O — modifier level: the original's object stream is exactly one 0x39bf container holding
// one empty 0x0100 chunk, corpus-wide (159/159 instances) — which is precisely what the Max
// SDK's own Modifier::Save writes (verified against a corpus nel_vertex_tree_paint object,
// whose stream shows the same 0x39bf pair followed by that plugin's own version chunk). So the
// original's Save was literally `Modifier::Save(isave)` with no chunks of its own; match it.

IOResult MapExtenderMod::Save(ISave *isave)
{
	return Modifier::Save(isave);
}

IOResult MapExtenderMod::Load(ILoad *iload)
{
	Modifier::Load(iload);
	// No chunks of our own; skim anything left so a future variant doesn't abort the load.
	while (iload->OpenChunk() == IO_OK)
		iload->CloseChunk();
	return IO_OK;
}

// ---------------------------------------------------------------------------------------------
// I/O — per-node LocalModData: raw-preserving chunk-tree load/save. The two known
// sub-containers (0x03f9, 0x03fa) recurse; every other id is an opaque leaf.

static bool isKnownContainer(USHORT id)
{
	return id == 0x03f9 || id == 0x03fa;
}

static IOResult loadChunkTree(ILoad *iload, std::vector<SMapExtChunk> &out)
{
	while (iload->OpenChunk() == IO_OK)
	{
		SMapExtChunk c;
		c.Id = iload->CurChunkID();
		c.Container = isKnownContainer(c.Id);
		if (c.Container)
		{
			IOResult res = loadChunkTree(iload, c.Children);
			if (res != IO_OK)
				return res;
		}
		else
		{
			ULONG len = (ULONG)iload->CurChunkLength();
			c.Data.resize(len);
			if (len)
			{
				ULONG nread = 0;
				IOResult res = iload->Read(&c.Data[0], len, &nread);
				if (res != IO_OK || nread != len)
					return IO_ERROR;
			}
		}
		out.push_back(c);
		iload->CloseChunk();
	}
	return IO_OK;
}

static IOResult saveChunkTree(ISave *isave, const std::vector<SMapExtChunk> &chunks)
{
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		const SMapExtChunk &c = chunks[i];
		isave->BeginChunk(c.Id);
		if (c.Container)
		{
			IOResult res = saveChunkTree(isave, c.Children);
			if (res != IO_OK)
				return res;
		}
		else if (!c.Data.empty())
		{
			ULONG nw = 0;
			IOResult res = isave->Write(&c.Data[0], (ULONG)c.Data.size(), &nw);
			if (res != IO_OK || nw != c.Data.size())
				return IO_ERROR;
		}
		isave->EndChunk();
	}
	return IO_OK;
}

IOResult MapExtenderMod::LoadLocalData(ILoad *iload, LocalModData **pld)
{
	MapExtenderData *d = new MapExtenderData();
	IOResult res = loadChunkTree(iload, d->Chunks);
	if (res != IO_OK)
	{
		delete d;
		return res;
	}
	d->decodeFunctionalSet();
	*pld = d;
	return IO_OK;
}

IOResult MapExtenderMod::SaveLocalData(ISave *isave, LocalModData *ld)
{
	MapExtenderData *d = (MapExtenderData *)ld;
	if (!d)
		return IO_OK;
	return saveChunkTree(isave, d->Chunks);
}

/* end of file */
