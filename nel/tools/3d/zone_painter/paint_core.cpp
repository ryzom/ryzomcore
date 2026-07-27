/**
 * \file paint_core.cpp
 * \brief See paint_core.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 */
// Port map (plugin_max/nel_patch_paint paint.cpp -> here):
//   DoPaint metaTile build        -> buildMeta / stitchEdge (typed SPatchMesh/SRPatchMesh data;
//                                    cross-zone links driven by welded bind records)
//   getBindedEdge                 -> getBindedEdge
//   GetTile/SetTile               -> getTile/setTile (pristine carrier state + display mirror)
//   transformDesc/transformInvDesc-> same names (plugin SetTile/GetTile: per-zone Symmetry/
//                                    Rotate from EPM_Mesh; GetTile = transformDesc(sym,4-rot),
//                                    SetTile Max write = transformInvDesc(sym,4-rot); landscape
//                                    mirror remaps through each shared zone's transform)
//   selectTile                    -> selectTile (with bank group bias)
//   GetBorderDesc/FindTransition/
//   getLayer/PropagateBorder      -> same names (patch-subobject clips and TileTrick dropped)
//   ClearATile/PutATile/RecursTile/
//   PutTile/CalcRotPath           -> clearATile/putATile/recursTile/opTileStroke/calcRotPath
//   CNelPatchChanger              -> changeTileArray/applyChanges
//   CTileUndo                     -> the stroke stacks (bounded LIFO, redo kept)
//   HitATile/CheckTri/intersect   -> pickTile/checkTri (display CBezierPatch::eval quads)

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
#include "paint_core.h"

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/plane.h>
#include <nel/3d/driver.h>
#include <nel/3d/landscape.h>
#include <nel/3d/patch.h>
#include <nel/3d/tile_color.h>
#include <nel/3d/tile_element.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;

namespace ZPPAINT {

// Bind type enum (typeBind; same values as the on-disk SRpoVertexBind.Type)
enum { ZP_BIND_25 = 0, ZP_BIND_75 = 1, ZP_BIND_50 = 2, ZP_BIND_SINGLE = 3 };

// Scene class ids of the paint-bearing modifiers (same constants as patch_eval.h; redefined
// here to keep this TU independent of the header-only eval unit).
static const NLMISC::CClassId ZP_CLASSID_NEL_EDIT_PATCH(0x4dd14a3c, 0x4ac23c0c);
static const NLMISC::CClassId ZP_CLASSID_NEL_PATCH_PAINT(0x0c49560f, 0x3c3d68e7);

static bool g_WarnedInvalidTileSet = false;
static void warnInvalidTileSet()
{
	if (!g_WarnedInvalidTileSet)
	{
		fprintf(stderr, "WARNING: invalid tile set data encountered during paint (reported once)\n");
		g_WarnedInvalidTileSet = true;
	}
}

CPaintCore::CPaintCore()
{
	m_Bank = NULL;
	m_Landscape = NULL;
	m_LockBorders = false;
	m_TileCycle = 0;
	m_StrokeRotation = 0;
	m_StrokeOldTile = -1;
	m_StrokeOldZone = -1;
	m_StrokeSets = 0;
	m_BrushSize = 0;
	m_TileGroup = 0;
	m_BrushMaskLoaded = false;
	m_BrushMaskMode = false;
	m_StoredIncludeMeshes = -1;
	m_StoredPreloadTiles = -1;
	m_PropChangedCb = NULL;
	m_HaveLastEdit = false;
	m_LastEditRadius = 0.f;
}

CPaintCore::~CPaintCore()
{
	for (size_t i = 0; i < m_Carriers.size(); ++i)
		delete m_Carriers[i].Pristine;
}

// ---------------------------------------------------------------------------------------------
// Carrier resolution: write-target policy. Walk the node's derived chain top-first; the
// TOPMOST modifier slot whose 0x2512->0x1000 per-node local data carries an RFINALPATCH 0x4001
// leaf is the carrier; without any such slot the base RPO 0x08FD is.

static CStorageContainer *zpContainerChild(const CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
		if (it->first == id) return dynamic_cast<CStorageContainer *>(it->second);
	return NULL;
}

static CStorageRaw *zpRawChild(const CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
		if (it->first == id) return dynamic_cast<CStorageRaw *>(it->second);
	return NULL;
}

static bool zpResolveCarrier(CNodeImpl *node, CRklPatchObject *&rpo, CStorageRaw *&snapLeaf, std::string &err)
{
	rpo = NULL;
	snapLeaf = NULL;
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	if (!obj) { err = "node without object"; return false; }
	int guard = 8;
	while (obj && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		// Modifier slots are stored top-first; the first 0x4001-carrying slot found in this
		// order is the topmost paint-bearing state.
		for (uint m = 0; m < derived->modifierCount() && !snapLeaf; ++m)
		{
			CSceneClass *mod = derived->modifier(m);
			if (!mod) continue;
			NLMISC::CClassId mcid = mod->classDesc()->classId();
			if (mcid != ZP_CLASSID_NEL_EDIT_PATCH && mcid != ZP_CLASSID_NEL_PATCH_PAINT) continue;
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(derived->localModData(m));
			if (!data) continue;
			CStorageContainer *wrap = zpContainerChild(data, 0x1000);
			if (!wrap) continue;
			snapLeaf = zpRawChild(wrap, 0x4001);
		}
		obj = derived->baseObject();
		if (snapLeaf) break;
	}
	// Base RPO (needed for the setRPatch fallback even when a snapshot carries the state)
	CSceneClass *base = dynamic_cast<CSceneClass *>(node->getReference(1));
	guard = 8;
	while (base && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(base);
		if (!derived) break;
		base = derived->baseObject();
	}
	rpo = dynamic_cast<CRklPatchObject *>(base);
	if (!rpo) { err = "object is not an RklPatch"; return false; }
	return true;
}

// Stored painter UI flags: RPO_INCLUDE_MESHES 0x4003 / RPO_PRELOAD_TILES 0x4010 in the
// modifier per-node local data (siblings of the RFINALPATCH 0x4001 in the 0x1000 wrapper).
// BOTH patch plugins persist 0x4003 (PaintPatchData AND EditPatchData save the same
// includeMeshes UI state); 0x4010 is painter-only. ON DISK the payload is a single C++ bool
// byte (the chunks ride the 64-bit header extension in the corpus); tolerate any size >= 1
// and read byte 0. First modifier (top-first) carrying the chunk wins.
static void zpHarvestPainterFlags(CNodeImpl *node, sint &includeMeshes, sint &preloadTiles)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	int guard = 8;
	while (obj && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		for (uint m = 0; m < derived->modifierCount(); ++m)
		{
			CSceneClass *mod = derived->modifier(m);
			if (!mod) continue;
			NLMISC::CClassId mcid = mod->classDesc()->classId();
			if (mcid != ZP_CLASSID_NEL_PATCH_PAINT && mcid != ZP_CLASSID_NEL_EDIT_PATCH) continue;
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(derived->localModData(m));
			if (!data) continue;
			CStorageContainer *wrap = zpContainerChild(data, 0x1000);
			if (!wrap) continue;
			CStorageRaw *im = zpRawChild(wrap, 0x4003);
			if (im && !im->Value.empty() && includeMeshes < 0)
				includeMeshes = im->Value[0] ? 1 : 0;
			CStorageRaw *pt = zpRawChild(wrap, 0x4010);
			if (pt && !pt->Value.empty() && preloadTiles < 0)
				preloadTiles = pt->Value[0] ? 1 : 0;
		}
		obj = derived->baseObject();
	}
}

// ---------------------------------------------------------------------------------------------
// init

bool CPaintCore::init(const std::vector<SPaintZoneInput> &zones, NL3D::CTileBank *bank,
                      float cellSize, float snap, bool lockBorders, std::string &err)
{
	// Re-init safe (working-set rebuild): free prior pristine copies + clear undo.
	// Undo history is intentionally discarded on any working-set change.
	for (size_t i = 0; i < m_Carriers.size(); ++i)
		delete m_Carriers[i].Pristine;
	m_Carriers.clear();
	m_Zones.clear();
	m_CurStroke.clear();
	m_UndoStack.clear();
	m_RedoStack.clear();
	m_Changes.clear();
	m_ColorChanges.clear();
	m_Landscape = NULL; // caller re-attaches after landscape reassembly
	m_Bank = bank;
	m_LockBorders = lockBorders;
	m_StrokeRotation = 0;
	m_StrokeOldTile = -1;
	m_StrokeOldZone = -1;
	m_StrokeSets = 0;
	// Group lists are rebuilt below with push_back; clear or every re-init doubles them
	// (and the group-cycle modulo drifts across working-set rebuilds).
	m_GroupTile128.clear();
	m_GroupTile256.clear();
	// Stored painter flags are harvested first-win per working set, not per process.
	m_StoredIncludeMeshes = -1;
	m_StoredPreloadTiles = -1;
	// The marker names a world point derived from zone ids that are about to be reassigned.
	m_HaveLastEdit = false;
	m_LastEditRadius = 0.f;

	std::map<const void *, uint> carrierIndex; // keyed by leaf ptr or rpo ptr (shared objects)
	for (size_t i = 0; i < zones.size(); ++i)
	{
		SZone z;
		z.In = zones[i];
		zpHarvestPainterFlags(zones[i].Node, m_StoredIncludeMeshes, m_StoredPreloadTiles);
		CRklPatchObject *rpo;
		CStorageRaw *leaf;
		if (!zpResolveCarrier(zones[i].Node, rpo, leaf, err))
		{
			// Degrade like the plugin: the zone stays displayed but is not paintable/writable.
			fprintf(stderr, "WARNING: zone %s: %s (not paintable)\n", zones[i].Name.c_str(), err.c_str());
			err.clear();
			continue;
		}
		const void *key = leaf ? (const void *)leaf : (const void *)rpo;
		std::map<const void *, uint>::iterator it = carrierIndex.find(key);
		if (it == carrierIndex.end())
		{
			SCarrier c;
			c.Rpo = rpo;
			c.SnapLeaf = leaf;
			c.Pristine = new SRPatchMesh();
			// PRISTINE DECODE: from the carrier's raw bytes, never from the evaluated state.
			bool decoded;
			if (leaf)
			{
				c.OriginalBytes = leaf->Value;
				decoded = decodeRPatchMesh(nlVectorData(leaf->Value), leaf->Value.size(), *c.Pristine, err);
			}
			else
			{
				const CStorageRaw *raw = rpo->rpoChunk();
				if (raw) { c.OriginalBytes = raw->Value; decoded = decodeRpoChunk(nlVectorData(raw->Value), raw->Value.size(), *c.Pristine, err); }
				else { decoded = false; err = "no 0x08FD chunk"; }
			}
			if (!decoded)
			{
				delete c.Pristine;
				fprintf(stderr, "WARNING: zone %s: carrier decode: %s (not paintable)\n", zones[i].Name.c_str(), err.c_str());
				err.clear();
				continue;
			}
			it = carrierIndex.insert(std::make_pair(key, (uint)m_Carriers.size())).first;
			m_Carriers.push_back(c);
		}

		// The pristine tile grid must match the evaluated one the display was built from (the
		// snapshots replicate the final state; a mismatch would break the display mirror).
		{
			const SRPatchMesh *pr = m_Carriers[it->second].Pristine;
			bool gridOk = pr->Patches.size() == zones[i].EvalRp->Patches.size();
			for (size_t p = 0; gridOk && p < pr->Patches.size(); ++p)
			{
				if (pr->Patches[p].NbTilesU != zones[i].EvalRp->Patches[p].NbTilesU
					|| pr->Patches[p].NbTilesV != zones[i].EvalRp->Patches[p].NbTilesV)
					gridOk = false;
			}
			if (!gridOk)
			{
				fprintf(stderr, "WARNING: zone %s: carrier/evaluated tile grid mismatch (not paintable)\n", zones[i].Name.c_str());
				continue;
			}
		}
		z.Carrier = it->second;
		m_Carriers[z.Carrier].Zones.push_back((uint)m_Zones.size());
		if (!zones[i].Frozen) m_Carriers[z.Carrier].AnyUnfrozen = true;

		// Zone symmetry state for transformDesc under Symmetry/Rotate. The plugin's painting
		// scene ran exportZone with forceBuildZoneSymmetry over the displayed patches with
		// ligo cellsize/snap. Only consulted under symmetry, but built the same way for parity.
		// Needs the bank; bank-less flows (null-edit, dumps) never run tile ops.
		if (bank)
		{
			// The build must see the zone the way exportZone produced it: intra-zone binds only.
			// Session weld may add cross-zone bind records whose Next[] indexes live in OTHER
			// zones; getNeighborTile would follow them into foreign patch numbers.
			// Tiles referencing beyond the bank (stale sources authored against a bigger bank)
			// are blanked in this local copy: the symmetry state of unresolvable tiles is
			// meaningless and the xref would be out of range.
			std::vector<NL3D::CPatchInfo> symPatches = *zones[i].Patches;
			for (size_t p = 0; p < symPatches.size(); ++p)
			{
				for (uint e = 0; e < 4; ++e)
					if (symPatches[p].BindEdges[e].NPatchs != 0 && symPatches[p].BindEdges[e].ZoneId != zones[i].ZoneId)
						symPatches[p].BindEdges[e] = NL3D::CPatchInfo::CBindInfo();
				for (size_t t = 0; t < symPatches[p].Tiles.size(); ++t)
				{
					NL3D::CTileElement &te = symPatches[p].Tiles[t];
					for (uint l = 0; l < 3; ++l)
					{
						if (te.Tile[l] != 0xffff && (int)te.Tile[l] >= bank->getTileCount())
						{
							te.Tile[0] = te.Tile[1] = te.Tile[2] = 0xffff;
							te.setTile256Info(false, 0);
							break;
						}
					}
				}
			}
			NL3D::CZoneSymmetrisation::CError symError;
			if (!z.Sym.build(symPatches, cellSize, snap, *bank, symError, NLMISC::CMatrix::Identity))
			{
				fprintf(stderr, "WARNING: zone %s: zone symmetry state build failed (non-fatal in authored space)\n",
				        zones[i].Name.c_str());
			}
		}

		m_Zones.push_back(z);
	}

	// Per-tileset group tile lists (paint_ui CBankCont port): set-local indices of tiles whose
	// bank record carries the group flag and a diffuse file name.
	if (bank)
	{
		m_GroupTile128.resize(bank->getTileSetCount());
		m_GroupTile256.resize(bank->getTileSetCount());
		for (sint ts = 0; ts < bank->getTileSetCount(); ++ts)
		{
			const NL3D::CTileSet *set = bank->getTileSet(ts);
			m_GroupTile128[ts].resize(NL3D_CTILE_NUM_GROUP);
			m_GroupTile256[ts].resize(NL3D_CTILE_NUM_GROUP);
			for (uint group = 0; group < NL3D_CTILE_NUM_GROUP; ++group)
			{
				sint tile;
				for (tile = 0; tile < set->getNumTile128(); ++tile)
				{
					const NL3D::CTile *pt = bank->getTile(set->getTile128(tile));
					if ((pt->getGroupFlags() & (1 << group))
						&& !pt->getRelativeFileName(NL3D::CTile::diffuse).empty())
						m_GroupTile128[ts][group].push_back((uint)tile);
				}
				for (tile = 0; tile < set->getNumTile256(); ++tile)
				{
					const NL3D::CTile *pt = bank->getTile(set->getTile256(tile));
					if ((pt->getGroupFlags() & (1 << group))
						&& !pt->getRelativeFileName(NL3D::CTile::diffuse).empty())
						m_GroupTile256[ts][group].push_back((uint)tile);
				}
			}
		}
	}

	buildMeta(err);
	// Snapshot export props so dirty tracks appdata edits outside the carriers.
	for (size_t i = 0; i < m_Zones.size(); ++i)
		readPropSnap(m_Zones[i].In.Node, m_Zones[i].PropSnap);
	return true;
}

// ---------------------------------------------------------------------------------------------
// Export-prop appdata snapshot / dirty

void CPaintCore::readPropSnap(CNodeImpl *node, SPropSnap &out)
{
	out = SPropSnap();
	if (!node)
		return;
	std::string s;
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_ZONE_ROTATE, s))
	{
		out.HasRotate = true;
		out.Rotate = s;
	}
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_ZONE_SYMMETRY, s))
	{
		out.HasSymmetry = true;
		out.Symmetry = s;
	}
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE, s))
	{
		out.HasPassable = true;
		out.Passable = s;
	}
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, s))
	{
		out.HasUseBB = true;
		out.UseBB = s;
	}
}

bool CPaintCore::propsDirty(uint zoneIdx) const
{
	if (zoneIdx >= m_Zones.size())
		return false;
	const SZone &z = m_Zones[zoneIdx];
	if (z.In.Frozen)
		return false;
	SPropSnap cur;
	readPropSnap(z.In.Node, cur);
	const SPropSnap &s = z.PropSnap;
	if (cur.HasRotate != s.HasRotate || (cur.HasRotate && cur.Rotate != s.Rotate))
		return true;
	if (cur.HasSymmetry != s.HasSymmetry || (cur.HasSymmetry && cur.Symmetry != s.Symmetry))
		return true;
	if (cur.HasPassable != s.HasPassable || (cur.HasPassable && cur.Passable != s.Passable))
		return true;
	if (cur.HasUseBB != s.HasUseBB || (cur.HasUseBB && cur.UseBB != s.UseBB))
		return true;
	return false;
}

void CPaintCore::snapZoneProps(uint zoneId)
{
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId != zoneId)
			continue;
		readPropSnap(m_Zones[i].In.Node, m_Zones[i].PropSnap);
		return;
	}
}

// ---------------------------------------------------------------------------------------------
// metaTile graph (DoPaint port)

uint CPaintCore::orderS(uint zone, uint patch) const
{
	return 1u << m_Zones[zone].In.EvalRp->Patches[patch].NbTilesU;
}

uint CPaintCore::orderT(uint zone, uint patch) const
{
	return 1u << m_Zones[zone].In.EvalRp->Patches[patch].NbTilesV;
}

SPaintTile *CPaintCore::metaAt(uint zone, sint32 tileId)
{
	if (zone >= m_Zones.size()) return NULL;
	if (tileId < 0 || (size_t)tileId >= m_Zones[zone].Meta.size()) return NULL;
	SPaintTile *t = &m_Zones[zone].Meta[tileId];
	return t->TileId >= 0 ? t : NULL;
}

void CPaintCore::collectDuals(SPaintTile *tile, std::vector<SPaintTile *> &out) const
{
	out.clear();
	if (!tile || tile->TileId < 0) return;
	const uint zi = (uint)tile->Zone;
	if (zi >= m_Zones.size()) return;
	const uint carrier = m_Zones[zi].Carrier;
	const std::vector<uint> &shared = m_Carriers[carrier].Zones;
	if (shared.size() <= 1) return;
	for (size_t s = 0; s < shared.size(); ++s)
	{
		const uint zj = shared[s];
		if (zj == zi) continue;
		if (tile->TileId < 0 || (size_t)tile->TileId >= m_Zones[zj].Meta.size()) continue;
		SPaintTile *d = const_cast<SPaintTile *>(&m_Zones[zj].Meta[tile->TileId]);
		if (d->TileId >= 0) out.push_back(d);
	}
}

void CPaintCore::markVisitedWithDuals(SPaintTile *tile, std::set<SPaintTile *> &visited) const
{
	if (!tile) return;
	visited.insert(tile);
	std::vector<SPaintTile *> duals;
	collectDuals(tile, duals);
	for (size_t i = 0; i < duals.size(); ++i)
		visited.insert(duals[i]);
}

bool CPaintCore::tileSeamsLegal(SPaintTile *tile) const
{
	if (!tile || tile->TileId < 0) return true;
	CTileDescP desc;
	getTileIdx((uint)tile->Zone, tile->TileId, desc);
	if (desc.isEmpty()) return true;
	CTileSetIdx corner[4];
	NL3D::CTileSet::TFlagBorder border[4][3];
	CTileDescP idx;
	for (int c = 0; c < 4; ++c) { corner[c].TileSet = -1; corner[c].Rotate = 0; }
	if (!const_cast<CPaintCore *>(this)->getBorderDesc(tile, corner, border, &idx)) return true;
	for (uint e = 0; e < 4; ++e)
	{
		SPaintTile *nb = tile->Voisins[e];
		if (!nb) continue;
		CTileDescP ndesc;
		getTileIdx((uint)nb->Zone, nb->TileId, ndesc);
		if (ndesc.isEmpty()) continue;
		CTileSetIdx ncorner[4];
		NL3D::CTileSet::TFlagBorder nborder[4][3];
		CTileDescP nidx;
		for (int c = 0; c < 4; ++c) { ncorner[c].TileSet = -1; ncorner[c].Rotate = 0; }
		if (!const_cast<CPaintCore *>(this)->getBorderDesc(nb, ncorner, nborder, &nidx)) continue;
		int edge = (2 + (int)e + tile->Rotate[e]) & 3;
		CTileSetIdx a1 = ncorner[(edge + 1) & 3];
		CTileSetIdx a2 = ncorner[edge];
		if (corner[e].TileSet != a1.TileSet || corner[(e + 1) & 3].TileSet != a2.TileSet)
			return false;
	}
	return true;
}

bool CPaintCore::writtenSeamsLegal(const std::vector<SUndoTile> &backupStack) const
{
	// Dedup (zoneIdx, tileId); backup may list the same slot more than once.
	std::set<std::pair<uint, sint32> > seen;
	for (size_t i = 0; i < backupStack.size(); ++i)
	{
		const SUndoTile &b = backupStack[i];
		if (!seen.insert(std::make_pair(b.Zone, b.TileId)).second) continue;
		SPaintTile *t = const_cast<CPaintCore *>(this)->metaAt(b.Zone, b.TileId);
		if (t && !tileSeamsLegal(t)) return false;
	}
	return true;
}

// getBindedEdge port: which edge of nPatch the bound vertex nVertInPatch's bind seam lies on.
int CPaintCore::getBindedEdge(uint zone, int nPatch, int nVertInPatch) const
{
	const SPatchMesh &pm = *m_Zones[zone].In.Pm;
	const SRPatchMesh &rp = *m_Zones[zone].In.EvalRp;
	sint32 nVertInMesh = pm.Patches[nPatch].V[nVertInPatch];
	const SRpoVertexBind &b = rp.Verts[nVertInMesh];
	if (!b.Binded) return -1;
	sint32 nVertexBefore = pm.Patches[nPatch].V[(nVertInPatch - 1) & 3];
	sint32 nVertexAfter = pm.Patches[nPatch].V[(nVertInPatch + 1) & 3];
	switch (b.Type)
	{
	case ZP_BIND_SINGLE:
		if (pm.Patches[b.Patch].V[b.Edge] == pm.Patches[nPatch].V[(nVertInPatch + 1) & 3])
			return nVertInPatch;
		return (nVertInPatch - 1) & 3;
	case ZP_BIND_25:
	{
		const SRpoVertexBind &before = rp.Verts[nVertexBefore];
		if (before.Binded && before.Patch == b.Patch && before.Edge == b.Edge && before.Type == ZP_BIND_50)
			return (nVertInPatch - 1) & 3;
		return nVertInPatch;
	}
	case ZP_BIND_50:
	{
		const SRpoVertexBind &before = rp.Verts[nVertexBefore];
		if (before.Binded && before.Patch == b.Patch && before.Edge == b.Edge && before.Type == ZP_BIND_75)
			return (nVertInPatch - 1) & 3;
		(void)nVertexAfter;
		return nVertInPatch;
	}
	case ZP_BIND_75:
	{
		const SRpoVertexBind &after = rp.Verts[nVertexAfter];
		if (after.Binded && after.Patch == b.Patch && after.Edge == b.Edge && after.Type == ZP_BIND_50)
			return nVertInPatch;
		return (nVertInPatch - 1) & 3;
	}
	}
	return -1;
}

// getOffset port: first tile of an edge walk (no symmetry in the authored-space scene).
static int zpGetOffset(int edge, int nU, int nV)
{
	switch (edge & 3)
	{
	case 0: return 0;
	case 1: return (nV - 1) * ZP_MAX_TILE_IN_PATCH;
	case 2: return (nV - 1) * ZP_MAX_TILE_IN_PATCH + nU - 1;
	case 3: return nU - 1;
	}
	return 0;
}

// The cross-patch tile stitch: link the tile run along edge e of (zi,p) to the reversed run
// along edge ee of (zj,pp). Edge rot is purely topological from bind edge indices (plugin
// formula). Per-zone display Rotate is handled by getTileIdx/setTile transformDesc, not
// folded into metaTile.Rotate (folding regressed R!=0 primary-side seam solves).
void CPaintCore::stitchEdge(uint zi, uint p, uint e, uint zj, uint pp, uint ee, int dividEdge, int offsetEdge)
{
	static const int delta[4] = { ZP_MAX_TILE_IN_PATCH, 1, -ZP_MAX_TILE_IN_PATCH, -1 };
	int nU = (int)orderS(zi, p), nV = (int)orderT(zi, p);
	int nUOther = (int)orderS(zj, pp), nVOther = (int)orderT(zj, pp);
	int rot = (2 - (int)e + (int)ee) & 3;
	int nTile = (e & 1) ? nU : nV;
	int nTile2 = (ee & 1) ? nUOther : nVOther;
	if (nTile != (nTile2 >> dividEdge)) return; // incompatible tile counts across the seam
	int offset = zpGetOffset((int)e, nU, nV);
	int offsetOther = zpGetOffset((int)ee, nUOther, nVOther);
	SPaintTile *t1 = &m_Zones[zi].Meta[p * ZP_NUM_TILE_SEL + offset];
	SPaintTile *t2 = &m_Zones[zj].Meta[pp * ZP_NUM_TILE_SEL + offsetOther];
	t2 += (nTile2 - 1) * delta[ee];
	t2 -= (delta[ee] * nTile2 * offsetEdge) >> 2;
	for (int end = 0; end < nTile; ++end)
	{
		t1->Voisins[e] = t2;
		t1->Rotate[e] = (uint8)rot;
		t2->Voisins[ee] = t1;
		t2->Rotate[ee] = (uint8)((-rot) & 3);
		t1 += delta[e];
		t2 -= delta[ee];
	}
}

void CPaintCore::buildMeta(std::string &err)
{
	// Per-tile grid entries + intra-patch links
	for (size_t zi = 0; zi < m_Zones.size(); ++zi)
	{
		SZone &z = m_Zones[zi];
		size_t numPatches = z.In.EvalRp->Patches.size();
		z.Meta.clear();
		z.Meta.resize(numPatches * ZP_NUM_TILE_SEL);
		for (size_t p = 0; p < numPatches; ++p)
		{
			int nU = (int)orderS((uint)zi, (uint)p);
			int nV = (int)orderT((uint)zi, (uint)p);
			for (int u = 0; u < nU; ++u)
			for (int v = 0; v < nV; ++v)
			{
				SPaintTile *t = &z.Meta[p * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u];
				t->Zone = (sint16)zi;
				t->Patch = (sint32)p;
				t->TileId = (sint32)(p * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u);
				t->U = (uint8)u;
				t->V = (uint8)v;
				t->Voisins[3] = (v == 0) ? NULL : &z.Meta[p * ZP_NUM_TILE_SEL + (v - 1) * ZP_MAX_TILE_IN_PATCH + u];
				t->Voisins[1] = (v == nV - 1) ? NULL : &z.Meta[p * ZP_NUM_TILE_SEL + (v + 1) * ZP_MAX_TILE_IN_PATCH + u];
				t->Voisins[0] = (u == 0) ? NULL : &z.Meta[p * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u - 1];
				t->Voisins[2] = (u == nU - 1) ? NULL : &z.Meta[p * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u + 1];
				// World center/radius over the display bezier quad (color brush range tests,
				// same scheme as the plugin's DoPaint tile map)
				const NL3D::CBezierPatch &bp = (*z.In.Patches)[p].Patch;
				NLMISC::CVector c0 = bp.eval((float)u / nU, (float)v / nV);
				NLMISC::CVector c1 = bp.eval((float)(u + 1) / nU, (float)v / nV);
				NLMISC::CVector c2 = bp.eval((float)(u + 1) / nU, (float)(v + 1) / nV);
				NLMISC::CVector c3 = bp.eval((float)u / nU, (float)(v + 1) / nV);
				t->Center = (c0 + c1 + c2 + c3) / 4.f;
				t->Radius = std::max(std::max((c0 - t->Center).norm(), (c1 - t->Center).norm()),
				                     std::max((c2 - t->Center).norm(), (c3 - t->Center).norm()));
			}
		}
	}

	// Cross-patch links: intra-mesh shared edges, intra-mesh binds, welded cross-zone borders
	for (size_t zi = 0; zi < m_Zones.size(); ++zi)
	{
		SZone &z = m_Zones[zi];
		const SPatchMesh &pm = *z.In.Pm;
		const SRPatchMesh &rp = *z.In.EvalRp;
		for (size_t p = 0; p < pm.Patches.size(); ++p)
		{
			for (uint e = 0; e < 4; ++e)
			{
				sint32 edgeIdx = pm.Patches[p].Edge[e];
				if (edgeIdx < 0 || (size_t)edgeIdx >= pm.Edges.size()) continue;
				const SPmEdge &edge = pm.Edges[edgeIdx];
				sint32 otherPatch = -1;
				if (edge.Patches.size() > 0)
				{
					if (edge.Patches[0] == (sint32)p)
					{
						if (edge.Patches.size() > 1) otherPatch = edge.Patches[1];
					}
					else otherPatch = edge.Patches[0];
				}
				if (otherPatch != -1)
				{
					// Shared interior edge of the same mesh
					int ee = -1;
					for (int k = 0; k < 4; ++k)
						if (pm.Patches[otherPatch].Edge[k] == edgeIdx) { ee = k; break; }
					if (ee >= 0)
						stitchEdge((uint)zi, (uint)p, e, (uint)zi, (uint)otherPatch, (uint)ee, 0, 0);
				}
				else
				{
					// Intra-mesh bind seam?
					int vertBinded = -1;
					if (rp.Verts[pm.Patches[p].V[e]].Binded) vertBinded = (int)e;
					if (rp.Verts[pm.Patches[p].V[(e + 1) & 3]].Binded) vertBinded = (e + 1) & 3;
					if (vertBinded != -1 && getBindedEdge((uint)zi, (int)p, vertBinded) == (int)e)
					{
						const SRpoVertexBind &b = rp.Verts[pm.Patches[p].V[vertBinded]];
						int offsetEdge = 0, dividEdge = 0;
						switch (b.Type)
						{
						case ZP_BIND_25: dividEdge = 2; offsetEdge = (vertBinded == (int)e) ? 3 : 2; break;
						case ZP_BIND_75: dividEdge = 2; offsetEdge = (vertBinded == (int)e) ? 1 : 0; break;
						case ZP_BIND_50: dividEdge = 2; offsetEdge = (vertBinded == (int)e) ? 2 : 1; break;
						case ZP_BIND_SINGLE: dividEdge = 1; offsetEdge = (vertBinded == (int)e) ? 2 : 0; break;
						}
						stitchEdge((uint)zi, (uint)p, e, (uint)zi, (uint)b.Patch, (uint)b.Edge, dividEdge, offsetEdge);
					}
					else
					{
						// Welded cross-zone border: one/one bind record with a foreign zone id
						// on the display patchinfo.
						const NL3D::CPatchInfo::CBindInfo &bi = (*z.In.Patches)[p].BindEdges[e];
						if (bi.NPatchs == 1 && bi.ZoneId != z.In.ZoneId)
						{
							// zone id -> zones vector index
							for (size_t zj = 0; zj < m_Zones.size(); ++zj)
							{
								if (m_Zones[zj].In.ZoneId == bi.ZoneId)
								{
									stitchEdge((uint)zi, (uint)p, e, (uint)zj, bi.Next[0], bi.Edge[0], 0, 0);
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	// Frozen and locked flags
	for (size_t zi = 0; zi < m_Zones.size(); ++zi)
	{
		SZone &z = m_Zones[zi];
		for (size_t k = 0; k < z.Meta.size(); ++k)
		{
			SPaintTile *t = &z.Meta[k];
			if (t->TileId < 0) continue;
			t->Frozen = z.In.Frozen;
			t->Locked = 0;
			for (uint n = 0; n < 4; ++n)
			{
				SPaintTile *neighbor = t->Voisins[n];
				if (!neighbor) { t->Locked |= (uint8)(1 << n); continue; }
				if (m_Zones[neighbor->Zone].In.Frozen) t->Locked |= (uint8)(1 << n);
			}
		}
	}
	(void)err;
}

// ---------------------------------------------------------------------------------------------
// Pristine tile access

const SRPatchMesh *CPaintCore::pristineOf(uint zone) const { return m_Carriers[m_Zones[zone].Carrier].Pristine; }
SRPatchMesh *CPaintCore::pristineOf(uint zone) { return m_Carriers[m_Zones[zone].Carrier].Pristine; }

void CPaintCore::getTileRaw(uint zone, sint32 tileId, CTileDescP &desc) const
{
	int patch = tileId / ZP_NUM_TILE_SEL;
	int tile = tileId % ZP_NUM_TILE_SEL;
	int v = tile / ZP_MAX_TILE_IN_PATCH;
	int u = tile % ZP_MAX_TILE_IN_PATCH;
	const SRpoPatch &up = pristineOf(zone)->Patches[patch];
	const SRpoTile &t = up.Tiles[u + v * (1 << up.NbTilesU)];
	// Noise is the authoritative displace on read (the original loader's setDisplace(Noise)
	// overrides the flags' displace bits). All OTHER flag bits (case bits and legacy high bits
	// many corpus records carry) are kept verbatim: the original loader keeps them in memory
	// and its save re-emits them, so a desc that flows GetTile -> setTile preserves them
	// exactly like the plugin (fresh descs start at zero, also like the plugin).
	// Clamp Num to the 3-layer desc capacity: corrupt/stale records can carry Num > 3
	// and every layer walk indexes Mat[l] without a further guard.
	desc.Num = (uint16)(t.Num > 3 ? 3 : t.Num);
	desc.Flags = (uint16)((t.Flags & ~0x78) | ((t.Noise & 0xf) << 3));
	for (int l = 0; l < 3; ++l)
	{
		desc.Mat[l].Tile = (uint16)t.Layer[l].Tile;
		desc.Mat[l].Rotate = (uint8)(t.Layer[l].Rotate & 3);
	}
}

void CPaintCore::setTileDesc(uint zone, sint32 tileId, const CTileDescP &desc)
{
	int patch = tileId / ZP_NUM_TILE_SEL;
	int tile = tileId % ZP_NUM_TILE_SEL;
	int v = tile / ZP_MAX_TILE_IN_PATCH;
	int u = tile % ZP_MAX_TILE_IN_PATCH;
	SRpoPatch &up = pristineOf(zone)->Patches[patch];
	SRpoTile &t = up.Tiles[u + v * (1 << up.NbTilesU)];
	// PRISTINE MUTATION: tile-record fields only; Reserved/OldA/OldB retention stays untouched.
	// Flags written wholesale (case + displace + preserved high bits, see getTileRaw).
	// Layers are written for USED slots only: unused on-disk layers carry -1/0x7fffffff-style
	// markers (rpo_data.h) that the 16-bit desc round trip cannot represent. Writing them
	// back zero-extended would drift bytes the edit never meant to touch.
	t.Num = (uint16)(desc.Num > 3 ? 3 : desc.Num);
	t.Flags = desc.Flags;
	t.Noise = desc.getDisplace();
	for (int l = 0; l < (int)t.Num && l < 3; ++l)
	{
		t.Layer[l].Tile = (sint32)desc.Mat[l].Tile;
		t.Layer[l].Rotate = (sint32)(desc.Mat[l].Rotate & 3);
	}
	++m_StrokeSets;
}

PIPELINE::MAX::NELPATCH::SRpoTile *CPaintCore::pristineTileRecord(uint zone, sint32 tileId)
{
	int patch = tileId / ZP_NUM_TILE_SEL;
	int tile = tileId % ZP_NUM_TILE_SEL;
	int v = tile / ZP_MAX_TILE_IN_PATCH;
	int u = tile % ZP_MAX_TILE_IN_PATCH;
	SRpoPatch &up = pristineOf(zone)->Patches[patch];
	return &up.Tiles[u + v * (1 << up.NbTilesU)];
}

void CPaintCore::captureRawTile(uint zone, sint32 tileId, uint16 &num, uint16 &flags,
                                uint8 &noise, sint32 tile[3], sint32 rot[3])
{
	const SRpoTile *t = pristineTileRecord(zone, tileId);
	num = t->Num;
	flags = t->Flags;
	noise = t->Noise;
	for (int l = 0; l < 3; ++l)
	{
		tile[l] = t->Layer[l].Tile;
		rot[l] = t->Layer[l].Rotate;
	}
}

void CPaintCore::restoreRawTile(const SUndoTile &u, bool useOld)
{
	// Verbatim pristine-record restore AFTER the desc-based replay updated the live display:
	// undo/redo lands on the exact on-disk bytes even where the desc round trip is lossy.
	SRpoTile *t = pristineTileRecord(u.Zone, u.TileId);
	t->Num = useOld ? u.OldRawNum : u.NewRawNum;
	t->Flags = useOld ? u.OldRawFlags : u.NewRawFlags;
	t->Noise = useOld ? u.OldRawNoise : u.NewRawNoise;
	for (int l = 0; l < 3; ++l)
	{
		t->Layer[l].Tile = useOld ? u.OldRawTile[l] : u.NewRawTile[l];
		t->Layer[l].Rotate = useOld ? u.OldRawRot[l] : u.NewRawRot[l];
	}
}

// transformDesc port (plugin paint.cpp). Live under per-zone Symmetry/Rotate (display
// instances / land placement Flip/Rot). goofy states come from the per-zone CZoneSymmetrisation.
void CPaintCore::transformDesc(CTileDescP &desc, bool symmetry, uint rotate, uint zone, sint32 tileId) const
{
	rotate &= 3;
	if (!symmetry && rotate == 0) return; // fast identity
	uint patch = (uint)(tileId / ZP_NUM_TILE_SEL);
	uint ttile = (uint)(tileId % ZP_NUM_TILE_SEL);
	uint v = ttile / ZP_MAX_TILE_IN_PATCH;
	uint u = ttile % ZP_MAX_TILE_IN_PATCH;
	uint symTile = orderS(zone, patch) * v + u;
	const NL3D::CZoneSymmetrisation &sym = m_Zones[zone].Sym;

	if (desc.getCase() != 0
	    && (uint)desc.getLayer(0).Tile < (uint)m_Bank->getTileCount())
	{
		// Stale sources can carry tiles beyond the bank, and a bank tile can xref no set
		// (tileSet -1, NeL warns on the same case in zone.cpp). Guard both before indexing.
		uint8 case256 = (uint8)(desc.getCase() - 1);
		uint tileRotate = rotate;
		bool tileSymmetry = symmetry;
		int tileSet, number;
		NL3D::CTileBank::TTileType type;
		m_Bank->getTileXRef(desc.getLayer(0).Tile, tileSet, number, type);
		NL3D::CPatchInfo::getTileSymmetryRotate(*m_Bank, desc.getLayer(0).Tile, tileSymmetry, tileRotate);
		bool goofy = false;
		uint tileRotation = desc.getLayer(0).Rotate;
		if (symmetry)
		{
			if (tileSet >= 0 && tileSet < m_Bank->getTileSetCount()
			    && m_Bank->getTileSet(tileSet)->getOriented())
			{
				if (sym.getOrientedTileBorderState(patch, symTile) != NL3D::CZoneSymmetrisation::Nothing)
					goofy = sym.getOrientedTileBorderState(patch, symTile) == NL3D::CZoneSymmetrisation::Goofy;
				else
					goofy = sym.getTileState(patch, symTile, 0) == NL3D::CZoneSymmetrisation::Goofy;
			}
			else
			{
				if (sym.getTileBorderState(patch, symTile) != NL3D::CZoneSymmetrisation::Nothing)
					goofy = sym.getTileBorderState(patch, symTile) == NL3D::CZoneSymmetrisation::Goofy;
				else
					goofy = sym.getTileState(patch, symTile, 0) == NL3D::CZoneSymmetrisation::Goofy;
			}
		}
		NL3D::CPatchInfo::transform256Case(*m_Bank, case256, tileRotation, tileSymmetry, tileRotate, goofy);
		desc.setCase(case256 + 1);
	}

	for (int l = 0; l < desc.getNumLayer(); ++l)
	{
		uint tile = desc.getLayer(l).Tile;
		if (tile >= (uint)m_Bank->getTileCount())
			continue; // stale out-of-bank tile: leave the layer untransformed (NeL skips too)
		uint tileRotation = desc.getLayer(l).Rotate;
		int tileSet, number;
		NL3D::CTileBank::TTileType type;
		m_Bank->getTileXRef((int)tile, tileSet, number, type);
		uint tileRotate = rotate;
		bool tileSymmetry = symmetry;
		bool goofy = false;
		if (symmetry)
		{
			if (tileSet >= 0 && tileSet < m_Bank->getTileSetCount()
			    && m_Bank->getTileSet(tileSet)->getOriented())
			{
				if (sym.getOrientedTileBorderState(patch, symTile) != NL3D::CZoneSymmetrisation::Nothing)
					goofy = sym.getOrientedTileBorderState(patch, symTile) == NL3D::CZoneSymmetrisation::Goofy;
				else
					goofy = sym.getTileState(patch, symTile, l) == NL3D::CZoneSymmetrisation::Goofy;
			}
			else
			{
				if (sym.getTileBorderState(patch, symTile) != NL3D::CZoneSymmetrisation::Nothing)
					goofy = sym.getTileBorderState(patch, symTile) == NL3D::CZoneSymmetrisation::Goofy;
				else
					goofy = sym.getTileState(patch, symTile, l) == NL3D::CZoneSymmetrisation::Goofy;
			}
		}
		if (NL3D::CPatchInfo::getTileSymmetryRotate(*m_Bank, tile, tileSymmetry, tileRotate))
		{
			if (NL3D::CPatchInfo::transformTile(*m_Bank, tile, tileRotation, tileSymmetry, tileRotate, goofy))
			{
				desc.Mat[l].Tile = (uint16)tile;
				desc.Mat[l].Rotate = (uint8)(tileRotation & 3);
			}
		}
	}
}

void CPaintCore::transformInvDesc(CTileDescP &desc, bool symmetry, uint rotate, uint zone, sint32 tileId) const
{
	transformDesc(desc, false, (4 - rotate) & 3, zone, tileId);
	transformDesc(desc, symmetry, 0, zone, tileId);
}

void CPaintCore::getTileIdx(uint zone, sint32 tileId, CTileDescP &desc) const
{
	getTileRaw(zone, tileId, desc);
	// Plugin GetTile: authored → display via transformDesc(sym, 4-rot)
	const SPaintZoneInput &in = m_Zones[zone].In;
	transformDesc(desc, in.Symmetry, (4 - in.Rotate) & 3, zone, tileId);
}

// SetTile port: pristine write + display mirror into every zone sharing the carrier.
// Plugin-faithful transform assembly (paint.cpp SetTile with EPM_Mesh::Rotate/Symmetry).
void CPaintCore::setTile(uint zone, sint32 tileId, const CTileDescP &desc,
                         std::vector<SUndoTile> *backupStack, bool undo, bool updateDisplace)
{
	CTileDescP oldDesc;
	getTileIdx(zone, tileId, oldDesc);

	CTileDescP maxDesc = desc;
	if (backupStack)
	{
		SUndoTile b;
		b.Zone = zone;
		b.TileId = tileId;
		b.Old = oldDesc;
		backupStack->push_back(b);
	}
	if (!updateDisplace)
		maxDesc.setDisplace(oldDesc.getDisplace());

	// Raw pristine snapshot BEFORE the write (byte-exact undo, see SUndoTile::*Raw*)
	uint16 oldRawNum = 0, oldRawFlags = 0;
	uint8 oldRawNoise = 0;
	sint32 oldRawTile[3], oldRawRot[3];
	if (undo)
		captureRawTile(zone, tileId, oldRawNum, oldRawFlags, oldRawNoise, oldRawTile, oldRawRot);

	// Display → authored (plugin: transformInvDesc(sym, 4-rot))
	const SPaintZoneInput &srcIn = m_Zones[zone].In;
	CTileDescP authored = maxDesc;
	transformInvDesc(authored, srcIn.Symmetry, (4 - srcIn.Rotate) & 3, zone, tileId);
	setTileDesc(zone, tileId, authored);

	// Intermediate like plugin copyDesc: undo only source symmetry (rotation left for remap)
	CTileDescP copyDesc = maxDesc;
	transformInvDesc(copyDesc, srcIn.Symmetry, 0, zone, tileId);

	// Display mirror: each shared zone gets transformDesc through ITS transform
	if (m_Landscape)
	{
		const std::vector<uint> &shared = m_Carriers[m_Zones[zone].Carrier].Zones;
		for (size_t s = 0; s < shared.size(); ++s)
		{
			uint zi = shared[s];
			int patch = tileId / ZP_NUM_TILE_SEL;
			int ttile = tileId % ZP_NUM_TILE_SEL;
			int v = ttile / ZP_MAX_TILE_IN_PATCH;
			int u = ttile % ZP_MAX_TILE_IN_PATCH;
			int os = (int)orderS(zi, (uint)patch);
			// Plugin: under symmetry, landscape U is flipped
			if (m_Zones[zi].In.Symmetry)
				u = os - u - 1;
			std::vector<NL3D::CTileElement> *arr = changeTileArray(zi, (uint)patch);
			if (!arr) continue;
			// Plugin: transformInvDesc(src.sym, 4-src.rot) then transformDesc(i.sym, 4-i.rot)
			CTileDescP nelDesc = copyDesc;
			transformInvDesc(nelDesc, srcIn.Symmetry, (4 - srcIn.Rotate) & 3, zone, tileId);
			transformDesc(nelDesc, m_Zones[zi].In.Symmetry, (4 - m_Zones[zi].In.Rotate) & 3, zi, tileId);
			NL3D::CTileElement &te = (*arr)[u + v * os];
			for (int l = 0; l < 3; ++l)
			{
				if (l >= nelDesc.getNumLayer())
				{
					te.Tile[l] = 0xffff;
				}
				else
				{
					te.Tile[l] = nelDesc.getLayer(l).Tile;
					te.setTileOrient(l, (uint8)(nelDesc.getLayer(l).Rotate & 3));
				}
			}
			if (te.Tile[0] == 0xffff)
				te.setTile256Info(false, 0);
			else if (nelDesc.getCase() == 0)
				te.setTile256Info(false, 0);
			else
				te.setTile256Info(true, nelDesc.getCase() - 1);
			te.setTileSubNoise(nelDesc.getDisplace());
		}
	}

	if (undo)
	{
		SUndoTile u;
		u.Zone = zone;
		u.TileId = tileId;
		u.Old = oldDesc;
		// Record what was APPLIED, not what was requested: with updateDisplace off the
		// displace byte was forced back to oldDesc's value above, and redo replays New
		// with updateDisplace on. Recording the raw caller desc would zero a preserved
		// displace on undo+redo (state after redo must equal state after the op).
		u.New = maxDesc;
		u.HaveRaw = true;
		u.OldRawNum = oldRawNum;
		u.OldRawFlags = oldRawFlags;
		u.OldRawNoise = oldRawNoise;
		for (int l = 0; l < 3; ++l) { u.OldRawTile[l] = oldRawTile[l]; u.OldRawRot[l] = oldRawRot[l]; }
		captureRawTile(zone, tileId, u.NewRawNum, u.NewRawFlags, u.NewRawNoise,
		               u.NewRawTile, u.NewRawRot);
		m_CurStroke.push_back(u);
	}
}

// ---------------------------------------------------------------------------------------------
// Display change batching (CNelPatchChanger port): per (zoneId, patch) a copy of the live
// zone's tile array; applyChanges pushes them back and refreshes tesselation (plus the direct
// bind neighbors, like the plugin, so cross-border geometry noise stays in sync).

std::vector<NL3D::CTileElement> *CPaintCore::changeTileArray(uint zone, uint patch)
{
	if (!m_Landscape) return NULL;
	std::pair<int, int> key((int)m_Zones[zone].In.ZoneId, (int)patch);
	TChangeMap::iterator it = m_Changes.find(key);
	if (it == m_Changes.end())
	{
		NL3D::CZone *lz = m_Landscape->getZone((sint)m_Zones[zone].In.ZoneId);
		if (!lz) return NULL;
		it = m_Changes.insert(std::make_pair(key, lz->getPatchTexture((sint)patch))).first;
	}
	return &it->second;
}

std::vector<NL3D::CTileColor> *CPaintCore::changeColorArray(uint zone, uint patch)
{
	if (!m_Landscape) return NULL;
	std::pair<int, int> key((int)m_Zones[zone].In.ZoneId, (int)patch);
	TColorChangeMap::iterator it = m_ColorChanges.find(key);
	if (it == m_ColorChanges.end())
	{
		NL3D::CZone *lz = m_Landscape->getZone((sint)m_Zones[zone].In.ZoneId);
		if (!lz) return NULL;
		it = m_ColorChanges.insert(std::make_pair(key, lz->getPatchColor((sint)patch))).first;
	}
	return &it->second;
}

void CPaintCore::applyChanges()
{
	// Color-only batches ride the same apply (changePatchTextureAndColor takes either or both)
	if (!m_Landscape || (m_Changes.empty() && m_ColorChanges.empty()))
	{
		m_Changes.clear();
		m_ColorChanges.clear();
		return;
	}
	// Fold color-only patches into the walk below by inserting their keys with NULL tiles
	for (TColorChangeMap::iterator ic = m_ColorChanges.begin(); ic != m_ColorChanges.end(); ++ic)
	{
		if (m_Changes.find(ic->first) == m_Changes.end())
		{
			NL3D::CZone *zone = m_Landscape->getZone(ic->first.first);
			if (zone)
				zone->changePatchTextureAndColor(ic->first.second, NULL, &ic->second);
		}
	}

	// Neighbor tesselation refresh set
	std::set<std::pair<uint, uint> > setNewPatch;
	for (TChangeMap::iterator ite = m_Changes.begin(); ite != m_Changes.end(); ++ite)
	{
		const NL3D::CZone *zone = m_Landscape->getZone(ite->first.first);
		if (!zone) continue;
		const NL3D::CPatch *patch = zone->getPatch(ite->first.second);
		if (!patch) continue;
		for (uint edge = 0; edge < 4; ++edge)
		{
			NL3D::CPatch::CBindInfo neighborEdge;
			patch->getBindNeighbor(edge, neighborEdge);
			if (neighborEdge.Zone)
			{
				for (uint i = 0; i < (uint)neighborEdge.NPatchs; ++i)
					setNewPatch.insert(std::pair<uint, uint>(neighborEdge.Zone->getZoneId(), neighborEdge.Next[i]->getPatchId()));
			}
		}
	}
	for (std::set<std::pair<uint, uint> >::iterator iteNew = setNewPatch.begin(); iteNew != setNewPatch.end(); ++iteNew)
	{
		if (m_Changes.find(std::pair<int, int>((int)iteNew->first, (int)iteNew->second)) == m_Changes.end())
		{
			NL3D::CZone *zone = m_Landscape->getZone((sint)iteNew->first);
			if (zone) zone->refreshTesselationGeometry((sint)iteNew->second);
		}
	}

	for (TChangeMap::iterator ite = m_Changes.begin(); ite != m_Changes.end(); ++ite)
	{
		NL3D::CZone *zone = m_Landscape->getZone(ite->first.first);
		if (!zone) continue;
		TColorChangeMap::iterator ic = m_ColorChanges.find(ite->first);
		zone->changePatchTextureAndColor(ite->first.second, &ite->second,
		                                 ic != m_ColorChanges.end() ? &ic->second : NULL);
		zone->refreshTesselationGeometry(ite->first.second);
	}
	m_Changes.clear();
	m_ColorChanges.clear();
}

// ---------------------------------------------------------------------------------------------
// selectTile port (with the plugin's group selection: group 0 = the set's global list,
// 1..12 = the bank group's set-local tile list).

int CPaintCore::selectTile(uint tileSet, bool selectCycle, bool _256, uint group)
{
	// Negative sets other than the -1 clear sentinel (script input) must not index the bank.
	if ((sint)tileSet < 0 || (sint)tileSet >= m_Bank->getTileSetCount()) return -1;
	const NL3D::CTileSet *ts = m_Bank->getTileSet((sint)tileSet);
	uint32 index = selectCycle ? m_TileCycle++ : (uint32)rand();
	if (_256)
	{
		if (group == 0)
		{
			if (!ts->getNumTile256()) return -1;
			return ts->getTile256((sint)(index % (uint32)ts->getNumTile256()));
		}
		const std::vector<uint> &groupArray = m_GroupTile256[tileSet][group - 1];
		if (groupArray.empty()) return -1;
		return ts->getTile256((sint)groupArray[index % (uint32)groupArray.size()]);
	}
	if (group == 0)
	{
		if (!ts->getNumTile128()) return -1;
		return ts->getTile128((sint)(index % (uint32)ts->getNumTile128()));
	}
	const std::vector<uint> &groupArray = m_GroupTile128[tileSet][group - 1];
	if (groupArray.empty()) return -1;
	return ts->getTile128((sint)groupArray[index % (uint32)groupArray.size()]);
}

bool CPaintCore::isLocked(SPaintTile *tile, uint8 mask) const
{
	if (m_LockBorders) return (tile->Locked & mask) != 0;
	return false;
}

// ---------------------------------------------------------------------------------------------
// GetBorderDesc port: corner tile sets + per-layer edge flags of a filled tile.

bool CPaintCore::getBorderDesc(SPaintTile *tile, CTileSetIdx corner[4], NL3D::CTileSet::TFlagBorder border[4][3],
                               CTileDescP *index)
{
	CTileDescP backup;
	getTileIdx((uint)tile->Zone, tile->TileId, backup);
	if (backup.isEmpty()) return false;

	int nLayer = backup.getNumLayer();
	CTileIdx pIndexx[3];
	for (int nL = 0; nL < nLayer; ++nL)
	{
		CTileIdx idx = backup.getLayer(nL);
		pIndexx[nL] = idx;
		int tileSet, number;
		NL3D::CTileBank::TTileType type;
		if ((int)idx.Tile >= m_Bank->getTileCount()) { warnInvalidTileSet(); return false; }
		m_Bank->getTileXRef(idx.Tile, tileSet, number, type);
		for (int i = 0; i < 4; ++i)
		{
			static const NL3D::CTileSet::TBorder toBorder[4] = { NL3D::CTileSet::left, NL3D::CTileSet::bottom, NL3D::CTileSet::right, NL3D::CTileSet::top };
			NL3D::CTileSet::TFlagBorder b = NL3D::CTileSet::_1111;
			if (type == NL3D::CTileBank::transition)
				b = NL3D::CTileSet::getOrientedBorder(toBorder[i], NL3D::CTileSet::getEdgeType((NL3D::CTileSet::TTransition)number, toBorder[i]));
			if (nL == 0 && b != NL3D::CTileSet::_1111) { warnInvalidTileSet(); return false; }
			switch (b)
			{
			case NL3D::CTileSet::_1111:
			case NL3D::CTileSet::_1110:
			case NL3D::CTileSet::_1000:
				corner[(i + backup.getLayer(nL).Rotate) & 3].TileSet = tileSet;
				corner[(i + backup.getLayer(nL).Rotate) & 3].Rotate = idx.Rotate;
				break;
			default:
				break;
			}
			border[(i + backup.getLayer(nL).Rotate) & 3][nL] = b;
		}
	}
	index->setTile(nLayer, backup.getCase(), backup.getDisplace(), pIndexx[0], pIndexx[1], pIndexx[2]);
	return true;
}

const NL3D::CTileSetTransition *CPaintCore::findTransition(int tileSet, int rotate, const NL3D::CTileSet::TFlagBorder *border) const
{
	NL3D::CTileSet::TFlagBorder conv[4];
	for (int i = 0; i < 4; ++i)
	{
		static const NL3D::CTileSet::TBorder toBorder[4] = { NL3D::CTileSet::left, NL3D::CTileSet::bottom, NL3D::CTileSet::right, NL3D::CTileSet::top };
		conv[i] = NL3D::CTileSet::getOrientedBorder(toBorder[i], border[(i + rotate) & 3]);
	}
	NL3D::CTileSet::TTransition t = NL3D::CTileSet::getTransitionTile(conv[3], conv[1], conv[0], conv[2]);
	if (t == NL3D::CTileSet::notfound) return NULL;
	return m_Bank->getTileSet(tileSet)->getTransition(t);
}

int CPaintCore::getLayer(SPaintTile *tile, int border, int tileSet, int rotate)
{
	int nLayer = -1;
	CTileDescP desc;
	getTileIdx((uint)tile->Voisins[border]->Zone, tile->Voisins[border]->TileId, desc);
	for (int o = 0; o < desc.getNumLayer(); ++o)
	{
		CTileIdx index = desc.getLayer(o);
		index.Rotate = (uint8)((index.Rotate - tile->Rotate[border]) & 3);
		NL3D::CTileBank::TTileType type;
		int ts, number;
		if ((int)index.Tile < m_Bank->getTileCount())
		{
			m_Bank->getTileXRef(index.Tile, ts, number, type);
			if (ts == tileSet && (int)index.Rotate == rotate) nLayer = o;
		}
	}
	return nLayer;
}

// ---------------------------------------------------------------------------------------------
// PropagateBorder port: the transition constraint solver. Structure and corner arithmetic kept
// line-for-line with the plugin; only the data plumbing changed.

bool CPaintCore::propagateBorder(SPaintTile *tile, int curRotation, int curTileSet, std::set<SPaintTile *> &visited,
                                 std::vector<SUndoTile> &backupStack, bool recurseNoDiff)
{
	if (visited.find(tile) != visited.end()) return true;

	CTileDescP backup;
	getTileIdx((uint)tile->Zone, tile->TileId, backup);
	if (backup.isEmpty()) return true;

	// Mark dual meta tiles (shared carrier, same tileId) visited so the solver does not
	// re-enter the same pristine slot through the other zone's graph mid-propagation. Dual
	// neighbor links are still walked at the end of this function.
	markVisitedWithDuals(tile, visited);

	bool _256 = backup.getCase() > 0;

	bool bModified[4] = { false, false, false, false };
	bool bTouched[4] = { false, false, false, false };
	bool bSameEdge[4] = { true, true, true, true };
	bool bVisited[4] = { false, false, false, false };
	int extraOrdinary[4] = { 0, 0, 0, 0 };
	int extraOrdinarySmallEdge[4] = { 0, 0, 0, 0 };
	CTileSetIdx nCorner[4];
	int i;
	for (i = 0; i < 4; ++i) { nCorner[i].TileSet = -1; nCorner[i].Rotate = 0; }
	NL3D::CTileSet::TFlagBorder nBorder[4][3];
	CTileDescP pIndex;
	if (!getBorderDesc(tile, nCorner, nBorder, &pIndex)) { warnInvalidTileSet(); return false; }
	bool bDiff = false;

	int v;
	for (v = 0; v < 4; ++v)
	{
		if (tile->Voisins[v])
		{
			CTileSetIdx pVoisinCorner[4];
			NL3D::CTileSet::TFlagBorder pBorder[4][3];
			CTileDescP pVoisinIndex;
			for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
			bool bFill = getBorderDesc(tile->Voisins[v], pVoisinCorner, pBorder, &pVoisinIndex);
			if (bFill)
			{
				int edge = (2 + v + tile->Rotate[v]) & 3;
				if (visited.find(tile->Voisins[v]) != visited.end())
				{
					bVisited[v] = true;
					pVoisinCorner[(edge + 1) & 3].Rotate = (pVoisinCorner[(edge + 1) & 3].Rotate - tile->Rotate[v]) & 3;
					pVoisinCorner[edge].Rotate = (pVoisinCorner[edge].Rotate - tile->Rotate[v]) & 3;
					if (bTouched[v])
					{
						if (nCorner[v] != pVoisinCorner[(edge + 1) & 3])
						{
							int delta = (pVoisinCorner[(edge + 1) & 3].Rotate - nCorner[v].Rotate) & 3;
							if (delta != 2)
							{
								if (nCorner[v] < pVoisinCorner[(edge + 1) & 3])
								{
									nCorner[v] = pVoisinCorner[(edge + 1) & 3];
									extraOrdinarySmallEdge[v] = (v - 1) & 3;
								}
								else extraOrdinarySmallEdge[v] = v;
							}
							else return false;
							extraOrdinary[v]++;
							bDiff = true;
						}
					}
					else
					{
						if (nCorner[v] != pVoisinCorner[(edge + 1) & 3])
						{
							nCorner[v] = pVoisinCorner[(edge + 1) & 3];
							bDiff = true;
							bModified[v] = true;
						}
						bTouched[v] = true;
					}
					int nNextCorner = (v + 1) & 3;
					if (bTouched[nNextCorner])
					{
						if (nCorner[nNextCorner] != pVoisinCorner[edge])
						{
							int delta = (pVoisinCorner[edge].Rotate - nCorner[nNextCorner].Rotate) & 3;
							if (delta != 2)
							{
								if (nCorner[nNextCorner] < pVoisinCorner[edge])
								{
									nCorner[nNextCorner] = pVoisinCorner[edge];
									extraOrdinarySmallEdge[nNextCorner] = nNextCorner;
								}
								else extraOrdinarySmallEdge[nNextCorner] = v;
							}
							else return false;
							extraOrdinary[nNextCorner]++;
							bDiff = true;
						}
					}
					else
					{
						if (nCorner[nNextCorner] != pVoisinCorner[edge])
						{
							nCorner[nNextCorner] = pVoisinCorner[edge];
							bDiff = true;
							bModified[nNextCorner] = true;
						}
						bTouched[nNextCorner] = true;
					}
				}
			}
		}
	}

	bool _isLocked = isLocked(tile);

	// Force visit of the sibling tiles of the same 256 (256 mode)
	if (_256 && !tile->Frozen)
	{
		int nCase = backup.getCase() - 1;
		int nRotate = backup.getLayer(0).Rotate;
		SPaintTile *other = tile->Voisins[(1 + nCase + nRotate) & 3];
		if (other)
		{
			int rot = tile->Rotate[(1 + nCase + nRotate) & 3];
			CTileDescP desc1;
			getTileIdx((uint)other->Zone, other->TileId, desc1);
			if (!desc1.isEmpty())
			{
				if (desc1.getCase() != (1 + ((nCase + 1) & 3))
					|| desc1.getLayer(0).Tile != backup.getLayer(0).Tile
					|| (int)desc1.getLayer(0).Rotate != ((backup.getLayer(0).Rotate - rot) & 3))
					bDiff = true;
			}
			else bDiff = true;
			if (isLocked(other)) _isLocked = true;
		}
		other = tile->Voisins[(2 + nCase + nRotate) & 3];
		if (other)
		{
			int rot = tile->Rotate[(2 + nCase + nRotate) & 3];
			CTileDescP desc1;
			getTileIdx((uint)other->Zone, other->TileId, desc1);
			if (!desc1.isEmpty())
			{
				if (desc1.getCase() != (1 + ((nCase + 3) & 3))
					|| desc1.getLayer(0).Tile != backup.getLayer(0).Tile
					|| (int)desc1.getLayer(0).Rotate != ((backup.getLayer(0).Rotate - rot) & 3))
					bDiff = true;
			}
			else bDiff = true;
			if (isLocked(other)) _isLocked = true;
		}
	}

	// Invalid corner config: same tile set only on a diagonal
	for (i = 0; i < 2; ++i)
	{
		if ((nCorner[i] == nCorner[(i + 2) & 3])
			&& (nCorner[(i + 1) & 3] != nCorner[(i + 2) & 3])
			&& (nCorner[(i + 3) & 3] != nCorner[(i + 2) & 3]))
			return false;
	}

	// Same edge flags
	for (v = 0; v < 4; ++v)
	{
		if (tile->Voisins[v] == NULL) bSameEdge[v] = false;
		else
		{
			CTileDescP desc;
			getTileIdx((uint)tile->Voisins[v]->Zone, tile->Voisins[v]->TileId, desc);
			if (bModified[v] || bModified[(v + 1) & 3] || desc.isEmpty()) bSameEdge[v] = false;
		}
	}

	if (!bDiff && pIndex.getNumLayer() == 1) return true;

	if (tile->Frozen || _isLocked) return false;

	// Corner tile set list (rotation-aware, +2 conflicts rejected)
	std::vector<CTileSetIdx> setIndex;
	for (v = 0; v < 4; ++v)
	{
		if (nCorner[v].TileSet == -1) return false; // degenerate corner data
		bool bFind = false;
		for (int vv = 0; vv < (int)setIndex.size(); ++vv)
		{
			if (setIndex[vv].TileSet == nCorner[v].TileSet)
			{
				CTileSetIdx complet = nCorner[v];
				complet.Rotate = (complet.Rotate + 2) & 3;
				if (setIndex[vv].Rotate == complet.Rotate) return false;
				if (nCorner[v] == setIndex[vv]) bFind = true;
			}
		}
		if (!bFind) setIndex.push_back(nCorner[v]);
	}
	std::sort(setIndex.begin(), setIndex.end());
	if (setIndex.size() > 3) return false;

	std::vector<CTileSetIdx>::iterator ite = setIndex.begin();
	CTileIdx finalIndex[3];

	for (int l = 0; l < (int)setIndex.size(); ++l)
	{
		if (l == 0)
		{
			// Base layer: a full tile of the lowest set. When the tile being rewritten was a
			// single-layer tile of the same set, prefer a tile from that tile's bank groups
			// (the plugin's group bias, keeps painted group patterns coherent).
			int nTile = -1;
			if (backup.getNumLayer() == 1 && (int)backup.getLayer(0).Tile < m_Bank->getTileCount())
			{
				int tileSet, number;
				NL3D::CTileBank::TTileType type;
				m_Bank->getTileXRef(backup.getLayer(0).Tile, tileSet, number, type);
				if (tileSet == ite->TileSet)
				{
					uint flags = m_Bank->getTile(backup.getLayer(0).Tile)->getGroupFlags();
					for (int f = 0; f < NL3D_CTILE_NUM_GROUP; ++f)
					{
						if (flags & (1u << f))
						{
							nTile = selectTile((uint)ite->TileSet, false, false, (uint)(f + 1));
							if (nTile != -1) break;
						}
					}
				}
			}
			if (nTile == -1)
				nTile = selectTile((uint)ite->TileSet, false, false, 0);
			if (nTile == -1) return false;
			finalIndex[0].Tile = (uint16)nTile;
			finalIndex[0].Rotate = (uint8)(ite->Rotate & 3);
		}
		else
		{
			NL3D::CTileSet::TFlagBorder border[4];
			bool bFilled[4];
			int c;
			for (c = 0; c < 4; ++c)
				bFilled[c] = !(nCorner[c] < *ite);
			for (c = 0; c < 4; ++c)
			{
				if (bFilled[c])
				{
					if (bFilled[(c + 1) & 3]) border[c] = NL3D::CTileSet::_1111;
					else
					{
						if (extraOrdinary[c] && extraOrdinarySmallEdge[c] == c)
							border[c] = NL3D::CTileSet::_1000;
						else if (setIndex.size() == 3)
						{
							NL3D::CTileSet::TFlagBorder wanted, invWanted;
							if (*ite < nCorner[c]) { wanted = NL3D::CTileSet::_1000; invWanted = NL3D::CTileSet::_1110; }
							else { wanted = NL3D::CTileSet::_1110; invWanted = NL3D::CTileSet::_1000; }
							border[c] = wanted;
							if (tile->Voisins[c])
							{
								CTileSetIdx pVoisinCorner[4];
								NL3D::CTileSet::TFlagBorder pBorder[4][3];
								CTileDescP pVoisinIndex;
								for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
								if (getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex))
								{
									int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
									if (nLayer != -1)
									{
										int edge = (2 + c + tile->Rotate[c]) & 3;
										if (pBorder[edge][nLayer] == NL3D::CTileSet::getInvertBorder(invWanted))
										{
											pBorder[edge][nLayer] = NL3D::CTileSet::getInvertBorder(wanted);
											NL3D::CTileSet::TFlagBorder newBorder[4];
											for (int nB = 0; nB < 4; ++nB) newBorder[nB] = pBorder[nB][nLayer];
											const NL3D::CTileSetTransition *tr = findTransition(ite->TileSet, ite->Rotate + tile->Rotate[c], newBorder);
											if (!tr) { warnInvalidTileSet(); return false; }
											pVoisinIndex.Mat[nLayer].Tile = (uint16)tr->getTile();
											if (tile->Voisins[c]->Frozen) return false;
											if (isLocked(tile->Voisins[c])) return false;
											setTile((uint)tile->Voisins[c]->Zone, tile->Voisins[c]->TileId, pVoisinIndex, &backupStack, true);
										}
									}
								}
							}
						}
						else
						{
							if (tile->Voisins[c] && visited.find(tile->Voisins[c]) != visited.end())
							{
								CTileSetIdx pVoisinCorner[4];
								NL3D::CTileSet::TFlagBorder pBorder[4][3];
								CTileDescP pVoisinIndex;
								for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
								bool bOk = getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex);
								if (!bOk) return false;
								int edge = (2 + c + tile->Rotate[c]) & 3;
								int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
								if (nLayer == -1) return false;
								border[c] = NL3D::CTileSet::getInvertBorder(pBorder[edge][nLayer]);
							}
							else
							{
								bool bComputed = false;
								if ((bVisited[c] || !recurseNoDiff) && bSameEdge[c])
								{
									bSameEdge[c] = false;
									CTileSetIdx pVoisinCorner[4];
									NL3D::CTileSet::TFlagBorder pBorder[4][3];
									CTileDescP pVoisinIndex;
									for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
									bool bOk = getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex);
									if (bOk)
									{
										int edge = (2 + c + tile->Rotate[c]) & 3;
										int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
										if (nLayer != -1
											&& (pBorder[edge][nLayer] == NL3D::CTileSet::_0111 || pBorder[edge][nLayer] == NL3D::CTileSet::_0001))
										{
											border[c] = NL3D::CTileSet::getInvertBorder(pBorder[edge][nLayer]);
											bSameEdge[c] = true;
											bComputed = true;
										}
									}
								}
								if (!bComputed)
								{
									bSameEdge[c] = false;
									border[c] = (rand() & 1) ? NL3D::CTileSet::_1000 : NL3D::CTileSet::_1110;
								}
							}
						}
					}
				}
				else
				{
					int nNextCorner = (c + 1) & 3;
					if (!bFilled[nNextCorner]) border[c] = NL3D::CTileSet::_0000;
					else
					{
						if (extraOrdinary[nNextCorner] && extraOrdinarySmallEdge[nNextCorner] == c)
							border[c] = NL3D::CTileSet::_0001;
						else if (setIndex.size() == 3)
						{
							NL3D::CTileSet::TFlagBorder wanted, invWanted;
							if (*ite < nCorner[c]) { wanted = NL3D::CTileSet::_0001; invWanted = NL3D::CTileSet::_0111; }
							else { wanted = NL3D::CTileSet::_0111; invWanted = NL3D::CTileSet::_0001; }
							border[c] = wanted;
							if (tile->Voisins[c])
							{
								CTileSetIdx pVoisinCorner[4];
								NL3D::CTileSet::TFlagBorder pBorder[4][3];
								CTileDescP pVoisinIndex;
								for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
								if (getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex))
								{
									int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
									if (nLayer != -1)
									{
										int edge = (2 + c + tile->Rotate[c]) & 3;
										if (pBorder[edge][nLayer] == NL3D::CTileSet::getInvertBorder(invWanted))
										{
											pBorder[edge][nLayer] = NL3D::CTileSet::getInvertBorder(wanted);
											NL3D::CTileSet::TFlagBorder newBorder[4];
											for (int nB = 0; nB < 4; ++nB) newBorder[nB] = pBorder[nB][nLayer];
											const NL3D::CTileSetTransition *tr = findTransition(ite->TileSet, ite->Rotate + tile->Rotate[c], newBorder);
											if (!tr) { warnInvalidTileSet(); return false; }
											pVoisinIndex.Mat[nLayer].Tile = (uint16)tr->getTile();
											if (tile->Voisins[c]->Frozen) return false;
											if (isLocked(tile->Voisins[c])) return false;
											setTile((uint)tile->Voisins[c]->Zone, tile->Voisins[c]->TileId, pVoisinIndex, &backupStack, true);
										}
									}
								}
							}
						}
						else
						{
							if (tile->Voisins[c] && visited.find(tile->Voisins[c]) != visited.end())
							{
								CTileSetIdx pVoisinCorner[4];
								NL3D::CTileSet::TFlagBorder pBorder[4][3];
								CTileDescP pVoisinIndex;
								for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
								bool bOk = getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex);
								if (!bOk) return false;
								int edge = (2 + c + tile->Rotate[c]) & 3;
								int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
								if (nLayer == -1) return false;
								border[c] = NL3D::CTileSet::getInvertBorder(pBorder[edge][nLayer]);
							}
							else
							{
								bool bComputed = false;
								if ((bVisited[c] || !recurseNoDiff) && bSameEdge[c])
								{
									bSameEdge[c] = false;
									CTileSetIdx pVoisinCorner[4];
									NL3D::CTileSet::TFlagBorder pBorder[4][3];
									CTileDescP pVoisinIndex;
									for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
									bool bOk = getBorderDesc(tile->Voisins[c], pVoisinCorner, pBorder, &pVoisinIndex);
									if (bOk)
									{
										int edge = (2 + c + tile->Rotate[c]) & 3;
										int nLayer = getLayer(tile, c, ite->TileSet, ite->Rotate);
										if (nLayer != -1
											&& (pBorder[edge][nLayer] == NL3D::CTileSet::_1110 || pBorder[edge][nLayer] == NL3D::CTileSet::_1000))
										{
											border[c] = NL3D::CTileSet::getInvertBorder(pBorder[edge][nLayer]);
											bSameEdge[c] = true;
											bComputed = true;
										}
									}
								}
								if (!bComputed)
								{
									bSameEdge[c] = false;
									border[c] = (rand() & 1) ? NL3D::CTileSet::_0001 : NL3D::CTileSet::_0111;
								}
							}
						}
					}
				}
			}

			const NL3D::CTileSetTransition *tr = findTransition(ite->TileSet, ite->Rotate, border);
			if (!tr) { warnInvalidTileSet(); return false; }
			finalIndex[l].Rotate = (uint8)(ite->Rotate & 3);
			finalIndex[l].Tile = (uint16)tr->getTile();
		}
		++ite;
	}

	// Write the solved descriptor
	CTileDescP desc;
	getTileIdx((uint)tile->Zone, tile->TileId, desc);
	switch (setIndex.size())
	{
	case 1: desc.setTile(1, 0, desc.getDisplace(), finalIndex[0], CTileIdx(), CTileIdx()); break;
	case 2: desc.setTile(2, 0, desc.getDisplace(), finalIndex[0], finalIndex[1], CTileIdx()); break;
	case 3: desc.setTile(3, 0, desc.getDisplace(), finalIndex[0], finalIndex[1], finalIndex[2]); break;
	default: return false;
	}
	setTile((uint)tile->Zone, tile->TileId, desc, &backupStack, true);

	if (_256)
	{
		int nCase = backup.getCase() - 1;
		int nRotate = backup.getLayer(0).Rotate;
		bSameEdge[(1 + nCase + nRotate) & 3] = false;
		bSameEdge[(2 + nCase + nRotate) & 3] = false;
	}

	if (!bDiff && !recurseNoDiff) return true;

	for (v = 0; v < 4; ++v)
	{
		if (tile->Voisins[v] && visited.find(tile->Voisins[v]) == visited.end())
		{
			if (bModified[v] || bModified[(v + 1) & 3] || !bSameEdge[v])
				if (!propagateBorder(tile->Voisins[v], (tile->Rotate[v] + curRotation) & 3, curTileSet, visited, backupStack, false))
					return false;
		}
	}
	return true;
}

} /* namespace ZPPAINT */

/* end of file */
