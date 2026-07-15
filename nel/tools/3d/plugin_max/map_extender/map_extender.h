/**
 * \file map_extender.h
 * \brief Drop-in replacement for the legacy "Map Extender" modifier (mapext198m3.dlm,
 * Class_ID (0x2ec82081, 0x045a6271), OSM). The original third-party plugin is unavailable;
 * without it, 3ds Max loads a missing-modifier stand-in, stack evaluation loses the UV
 * channel the modifier carried, and collapsing/exporting produces garbage UVs (see
 * ryzomcore_wiki drafts/max_geometry_formats.md Part P and defective_max_files.md §4.4).
 *
 * The original stores NO parameters on the modifier itself (its object stream is an empty
 * 0x39bf → 0x0100 chunk pair corpus-wide); the computed UVW map is saved flat in the per-node
 * LocalModData. This replacement re-registers the Class_ID, loads that LocalModData verbatim
 * (typed functional set + raw-preserved unknowns so a re-save keeps every byte), and replays
 * the cached map onto the mesh's map channel in ModifyObject — the corpus-proven semantics of
 * the headless pipeline's MAPEXT::applyMapExtender (validated against the reference exports:
 * design doc §10z-quatorze/§10z-quinze). No authoring UI: a freshly-applied instance has no
 * data and is a no-op; the plugin exists so legacy scenes VIEW, COLLAPSE and EXPORT correctly.
 *
 * Per-node LocalModData chunk stream (corpus-decoded, Part P):
 *   0x03e8  uint32       vertex count of the cached map (map verts, seam duplicates included)
 *   0x03e9  Point3[n]    the cached UVW values
 *   0x03ea  uint32       face count the cache was computed against
 *   0x03eb  DWORD[n*3]   per-face map-vert corner indices (TVFace)
 *   0x03f3  uint32       target map channel (1 or 2 in the corpus)
 *   0x03ec/0x03ed/0x03ee/0x03ef/0x03f0/0x03f1/0x03f2/0x03f5/0x03f8/0x044c and the
 *   sub-containers 0x03f9/0x03fa: secondary channel snapshot, per-face flag/matID words,
 *   selection-ish state and a version stamp {3100, 198} — none of these are needed to apply
 *   the map (the reference exports validate against the functional set alone); they are
 *   raw-preserved through load/save so nothing is dropped on a re-save from Max.
 *
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

#ifndef PLUGIN_MAX_MAP_EXTENDER_H
#define PLUGIN_MAX_MAP_EXTENDER_H

#include <Max.h>
#include <maxversion.h>

#include <vector>

#define MAP_EXTENDER_CLASS_ID Class_ID(0x2ec82081, 0x045a6271)

// SDK signature drift across the Max versions the quick_start redist targets (9 .. 2023+),
// same conditions as nel_3dsmax_shared/string_common.h — inlined here so this plugin stays
// free of NeL dependencies.
#if (MAX_VERSION_MAJOR < 15)
#define MAPEXT_NOTIFY_REF_PARAMS Interval /* changeInt */, RefTargetHandle /* hTarget */, PartID & /* partID */, RefMessage /* message */
#else
#define MAPEXT_NOTIFY_REF_PARAMS const Interval & /* changeInt */, RefTargetHandle /* hTarget */, PartID & /* partID */, RefMessage /* message */, BOOL /* propagate */
#endif

extern HINSTANCE hInstance;

ClassDesc *GetMapExtenderDesc();

// One raw-preserved chunk of the original LocalModData stream. The two known sub-containers
// (0x03f9, 0x03fa) keep their children as a nested list so a re-save reproduces the container
// bit; every other id is an opaque leaf.
struct SMapExtChunk
{
	USHORT Id;
	bool Container;
	std::vector<BYTE> Data;             // leaf payload (empty for containers)
	std::vector<SMapExtChunk> Children; // container children (empty for leaves)
};

class MapExtenderData : public LocalModData
{
public:
	// The whole original chunk stream, in file order — save re-emits this verbatim.
	std::vector<SMapExtChunk> Chunks;

	// Typed views of the functional set (indexes into decoded copies, refreshed after load).
	// Empty/zero when the instance carries no data (freshly applied in Max).
	DWORD NumMapVerts;
	DWORD NumFaces;
	DWORD Channel;
	std::vector<Point3> MapVerts;   // 0x03e9
	std::vector<DWORD> FaceCorners; // 0x03eb, 3 per face

	MapExtenderData();
	LocalModData *Clone();

	// Re-derive the typed views from Chunks; false when the functional set is absent or
	// internally inconsistent (the instance then applies nothing).
	bool decodeFunctionalSet();
	bool valid() const;
};

class MapExtenderMod : public Modifier
{
public:
	MapExtenderMod();

	// Animatable
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return MAP_EXTENDER_CLASS_ID; }
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
#if (MAX_VERSION_MAJOR < 24)
	void GetClassName(TSTR &s) { s = _T("Map Extender"); }
#else
	void GetClassName(TSTR &s, bool /* localized */) const { s = _T("Map Extender"); }
#endif
#if (MAX_VERSION_MAJOR < 15)
	TCHAR *GetObjectName() { return _T("Map Extender"); }
#elif (MAX_VERSION_MAJOR < 24)
	const MCHAR *GetObjectName() { return _T("Map Extender"); }
#else
	const MCHAR *GetObjectName(bool /* localized */) const { return _T("Map Extender"); }
#endif
	int NumSubs() { return 0; }
	int NumRefs() { return 0; }
	RefTargetHandle GetReference(int /* i */) { return NULL; }
	void SetReference(int /* i */, RefTargetHandle /* rtarg */) {}
	RefResult NotifyRefChanged(MAPEXT_NOTIFY_REF_PARAMS)
	{
		return REF_SUCCEED;
	}
	CreateMouseCallBack *GetCreateMouseCallBack() { return NULL; }
	void BeginEditParams(IObjParam * /* ip */, ULONG /* flags */, Animatable * /* prev */) {}
	void EndEditParams(IObjParam * /* ip */, ULONG /* flags */, Animatable * /* next */) {}

	// ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// Modifier — the cached map is topology-addressed and time-invariant.
	ChannelMask ChannelsUsed() { return GEOM_CHANNEL | TOPO_CHANNEL | TEXMAP_CHANNEL; }
	ChannelMask ChannelsChanged() { return TEXMAP_CHANNEL; }
	Class_ID InputType() { return triObjectClassID; }
	Interval LocalValidity(TimeValue /* t */) { return FOREVER; }
	BOOL DependOnTopology(ModContext & /* mc */) { return TRUE; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

	// The original's modifier-level stream is a bare 0x39bf → empty 0x0100 pair; reproduce it.
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
};

#endif /* PLUGIN_MAX_MAP_EXTENDER_H */

/* end of file */
