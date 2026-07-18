/**
 * \file paint_core.cpp
 * \brief See paint_core.h (design doc §14-paint, P3b).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Port map (plugin_max/nel_patch_paint paint.cpp -> here):
//   DoPaint metaTile build        -> buildMeta / stitchEdge (typed SPatchMesh/SRPatchMesh data;
//                                    cross-zone links driven by the P3a weld's bind records)
//   getBindedEdge                 -> getBindedEdge
//   GetTile/SetTile               -> getTile/setTile (pristine carrier state + display mirror)
//   transformDesc/transformInvDesc-> same names (authored-space scene: per-zone Symmetry=false,
//                                    Rotate=0, so the transform collapses to identity; the
//                                    symVector consultation is kept for parity)
//   selectTile                    -> selectTile (group selection dropped, see header)
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

#include <nel/misc/plane.h>
#include <nel/3d/landscape.h>
#include <nel/3d/patch.h>
#include <nel/3d/tile_element.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

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

static const int ZP_MAX_UNDO = 64;
static const int ZP_DEPTH_SEARCH_MAX = 10; // plugin DEPTH_SEARCH_MAX (CalcRotPath)

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
}

CPaintCore::~CPaintCore()
{
	for (size_t i = 0; i < m_Carriers.size(); ++i)
		delete m_Carriers[i].Pristine;
}

// ---------------------------------------------------------------------------------------------
// Carrier resolution: the P2 write-target policy. Walk the node's derived chain top-first; the
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

// ---------------------------------------------------------------------------------------------
// init

bool CPaintCore::init(const std::vector<SPaintZoneInput> &zones, NL3D::CTileBank *bank,
                      float cellSize, float snap, bool lockBorders, std::string &err)
{
	m_Bank = bank;
	m_LockBorders = lockBorders;
	m_Zones.clear();
	m_Carriers.clear();

	std::map<const void *, uint> carrierIndex; // keyed by leaf ptr or rpo ptr (shared objects)
	for (size_t i = 0; i < zones.size(); ++i)
	{
		SZone z;
		z.In = zones[i];
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

		// symVector replication: the plugin's painting scene ran exportZone with
		// forceBuildZoneSymmetry, handing tile ops a CZoneSymmetrisation built over the
		// displayed patches with the ligo cellsize/snap. Only consulted under symmetry (the
		// authored-space scene displays none), but built the same way for parity. Needs the
		// bank; the bank-less flows (null-edit, dumps) never run tile ops.
		if (bank)
		{
			// The build must see the zone the way exportZone produced it: intra-zone binds only.
			// The P3a weld added session-only cross-zone bind records whose Next[] indexes live
			// in OTHER zones — getNeighborTile would follow them into foreign patch numbers.
			// Tiles referencing beyond the bank (stale sources authored against a bigger bank,
			// e.g. the Max 3 era zones) are blanked in this local copy: the symmetry state of
			// unresolvable tiles is meaningless and the xref would be out of range.
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

	buildMeta(err);
	return true;
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

// The cross-patch tile stitch (DoPaint lines ~3884-3923, symmetry dropped): link the tile run
// along edge e of (zi,p) to the reversed run along edge ee of (zj,pp).
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
						// Welded cross-zone border: the P3a weld left a one/one bind record with
						// a foreign zone id on the display patchinfo.
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

	// Frozen and locked flags (DoPaint lines ~3934-3977)
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
	// overrides the flags' displace bits). All OTHER flag bits — the case bits and the legacy
	// high bits many corpus records carry — are kept verbatim: the original loader keeps them
	// in memory and its save re-emits them, so a desc that flows GetTile -> setTile preserves
	// them exactly like the plugin (fresh descs start at zero, also like the plugin).
	desc.Num = t.Num;
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
	t.Num = desc.Num;
	t.Flags = desc.Flags;
	t.Noise = desc.getDisplace();
	for (int l = 0; l < 3; ++l)
	{
		t.Layer[l].Tile = (sint32)desc.Mat[l].Tile;
		t.Layer[l].Rotate = (sint32)(desc.Mat[l].Rotate & 3);
	}
	++m_StrokeSets;
}

// transformDesc port. The authored-space scene has Symmetry=false / Rotate=0 for every zone, so
// this is an intended identity; the full symVector-consulting logic is kept for parity with the
// plugin (and becomes live if a symmetric display space is ever assembled).
void CPaintCore::transformDesc(CTileDescP &desc, bool symmetry, uint rotate, uint zone, sint32 tileId) const
{
	rotate &= 3;
	if (!symmetry && rotate == 0) return; // fast identity (see note above)
	uint patch = (uint)(tileId / ZP_NUM_TILE_SEL);
	uint ttile = (uint)(tileId % ZP_NUM_TILE_SEL);
	uint v = ttile / ZP_MAX_TILE_IN_PATCH;
	uint u = ttile % ZP_MAX_TILE_IN_PATCH;
	uint symTile = orderS(zone, patch) * v + u;
	const NL3D::CZoneSymmetrisation &sym = m_Zones[zone].Sym;

	if (desc.getCase() != 0)
	{
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
			if (m_Bank->getTileSet(tileSet)->getOriented())
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
		uint tileRotation = desc.getLayer(l).Rotate;
		int tileSet, number;
		NL3D::CTileBank::TTileType type;
		m_Bank->getTileXRef((int)tile, tileSet, number, type);
		uint tileRotate = rotate;
		bool tileSymmetry = symmetry;
		bool goofy = false;
		if (symmetry)
		{
			if (m_Bank->getTileSet(tileSet)->getOriented())
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
	// Authored display space: Symmetry=false, Rotate=0
	transformDesc(desc, false, 0, zone, tileId);
}

// SetTile port: pristine write + display mirror into every zone sharing the carrier.
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

	// Authored write (identity transform in this scene)
	CTileDescP authored = maxDesc;
	transformInvDesc(authored, false, 0, zone, tileId);
	setTileDesc(zone, tileId, authored);

	// Display mirror: all zones sharing this carrier show the same tile record.
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
			std::vector<NL3D::CTileElement> *arr = changeTileArray(zi, (uint)patch);
			if (!arr) continue;
			int os = (int)orderS(zi, (uint)patch);
			NL3D::CTileElement &te = (*arr)[u + v * os];
			const CTileDescP &nelDesc = maxDesc; // display == authored here
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
		u.New = desc;
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

void CPaintCore::applyChanges()
{
	if (!m_Landscape || m_Changes.empty()) { m_Changes.clear(); return; }

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
		zone->changePatchTextureAndColor(ite->first.second, &ite->second, NULL);
		zone->refreshTesselationGeometry(ite->first.second);
	}
	m_Changes.clear();
}

// ---------------------------------------------------------------------------------------------
// selectTile port (group selection dropped: always the global 128/256 list of the set).

int CPaintCore::selectTile(uint tileSet, bool selectCycle, bool _256)
{
	if ((sint)tileSet >= m_Bank->getTileSetCount()) return -1;
	const NL3D::CTileSet *ts = m_Bank->getTileSet((sint)tileSet);
	uint32 index = selectCycle ? m_TileCycle++ : (uint32)rand();
	if (_256)
	{
		if (!ts->getNumTile256()) return -1;
		return ts->getTile256((sint)(index % (uint32)ts->getNumTile256()));
	}
	if (!ts->getNumTile128()) return -1;
	return ts->getTile128((sint)(index % (uint32)ts->getNumTile128()));
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
// line-for-line with the plugin (see paint.cpp ~1557-2361); only the data plumbing changed.

bool CPaintCore::propagateBorder(SPaintTile *tile, int curRotation, int curTileSet, std::set<SPaintTile *> &visited,
                                 std::vector<SUndoTile> &backupStack, bool recurseNoDiff)
{
	if (visited.find(tile) != visited.end()) return true;

	CTileDescP backup;
	getTileIdx((uint)tile->Zone, tile->TileId, backup);
	if (backup.isEmpty()) return true;

	visited.insert(tile);

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
			// Base layer: a full tile of the lowest set (random; the plugin group bias dropped)
			int nTile = selectTile((uint)ite->TileSet, false, false);
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

// ---------------------------------------------------------------------------------------------
// ClearATile / PutATile ports

bool CPaintCore::clearATile(SPaintTile *tile, bool _256, bool force128)
{
	CTileDescP backup;
	getTileIdx((uint)tile->Zone, tile->TileId, backup);
	if (backup.getCase() > 0 && !force128) _256 = true;
	if (_256)
	{
		if (tile->U & 1) tile = tile->Voisins[0];
		if (!tile) return false;
		if (tile->V & 1) tile = tile->Voisins[3];
		if (!tile) return false;
	}
	if (_256)
	{
		CTileDescP desc;
		desc.setTile(0, 0, 0, CTileIdx(), CTileIdx(), CTileIdx());
		int nRot;
		SPaintTile *neighbor[4] = { tile, tile->getRight256(0, nRot), tile->getBottom256(0, nRot), tile->getRightBottom256(0, nRot) };
		uint n;
		for (n = 0; n < 4; ++n)
		{
			if (!neighbor[n]) return false;
			if (isLocked(neighbor[n]) || neighbor[n]->Frozen) return false;
		}
		for (n = 0; n < 4; ++n)
		{
			CTileDescP descOrig;
			getTileIdx((uint)neighbor[n]->Zone, neighbor[n]->TileId, descOrig);
			desc.setDisplace(descOrig.getDisplace());
			setTile((uint)neighbor[n]->Zone, neighbor[n]->TileId, desc, NULL, true);
		}
	}
	else
	{
		if (isLocked(tile) || tile->Frozen) return false;
		CTileDescP desc;
		desc.setTile(0, 0, 0, CTileIdx(), CTileIdx(), CTileIdx());
		CTileDescP descOrig;
		getTileIdx((uint)tile->Zone, tile->TileId, descOrig);
		desc.setDisplace(descOrig.getDisplace());
		setTile((uint)tile->Zone, tile->TileId, desc, NULL, true);
	}
	return true;
}

bool CPaintCore::putATile(SPaintTile *pTile, int tileSet, int curRotation, bool selectCycle,
                          std::set<SPaintTile *> &visited, bool _256)
{
	if (_256)
	{
		if (pTile->U & 1) pTile = pTile->Voisins[0];
		if (!pTile) return false;
		if (pTile->V & 1) pTile = pTile->Voisins[3];
		if (!pTile) return false;
		if (!pTile->validFor256(0)) return false;
	}
	if (pTile->Frozen) return false;
	if (isLocked(pTile)) return false;

	CTileDescP backup;
	getTileIdx((uint)pTile->Zone, pTile->TileId, backup);

	std::vector<SUndoTile> backupStack;
	backupStack.reserve(300);

	if (_256)
	{
		int nRot;
		SPaintTile *other = pTile->getRight256(0, nRot);
		if (!other || isLocked(other) || other->Frozen) return false;
		other = pTile->getBottom256(0, nRot);
		if (!other || isLocked(other) || other->Frozen) return false;
		other = pTile->getRightBottom256(0, nRot);
		if (!other || isLocked(other) || other->Frozen) return false;
	}

	visited.insert(pTile);

	if (tileSet == -1)
		return clearATile(pTile, _256);

	int nTile = selectTile((uint)tileSet, selectCycle, _256);
	if (nTile == -1) return false;

	if (_256)
	{
		CTileDescP desc;
		desc.setTile(1, 1 + ((-curRotation) & 3), 0, CTileIdx(nTile, curRotation), CTileIdx(), CTileIdx());
		setTile((uint)pTile->Zone, pTile->TileId, desc, &backupStack, true);
		int nRot;
		SPaintTile *other = pTile->getRight256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation - 1) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		visited.insert(other);
		other = pTile->getBottom256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation + 1) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		visited.insert(other);
		other = pTile->getRightBottom256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation + 2) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		visited.insert(other);
	}
	else
	{
		CTileDescP desc;
		desc.setTile(1, 0, 0, CTileIdx(nTile, curRotation), CTileIdx(), CTileIdx());
		setTile((uint)pTile->Zone, pTile->TileId, desc, &backupStack, true);
	}

	bool bContinue = true;
	uint offset = (uint)rand();

	if (_256)
	{
		for (int n = 0; n < 4 && bContinue; ++n)
		{
			int nRot;
			SPaintTile *other;
			switch ((offset + n) & 0x3)
			{
			case 0:
				if (pTile->Voisins[3])
					if (!propagateBorder(pTile->Voisins[3], (pTile->Rotate[3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				if (pTile->Voisins[0])
					if (!propagateBorder(pTile->Voisins[0], (pTile->Rotate[0] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				break;
			case 1:
				other = pTile->getBottom256(0, nRot);
				if (other->Voisins[(0 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(0 - nRot) & 3], (other->Rotate[(0 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				if (other->Voisins[(1 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(1 - nRot) & 3], (other->Rotate[(1 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				break;
			case 2:
				other = pTile->getBottomRight256(0, nRot);
				if (other->Voisins[(1 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(1 - nRot) & 3], (other->Rotate[(1 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				if (other->Voisins[(2 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(2 - nRot) & 3], (other->Rotate[(2 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				break;
			case 3:
				other = pTile->getRight256(0, nRot);
				if (other->Voisins[(2 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(2 - nRot) & 3], (other->Rotate[(2 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				if (other->Voisins[(3 - nRot) & 3])
					if (!propagateBorder(other->Voisins[(3 - nRot) & 3], (other->Rotate[(3 - nRot) & 3] + curRotation) & 3, tileSet, visited, backupStack)) { bContinue = false; break; }
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			uint ii = (offset + i) & 0x3;
			if (pTile->Voisins[ii])
			{
				if (!propagateBorder(pTile->Voisins[ii], (pTile->Rotate[ii] + curRotation) & 3, tileSet, visited, backupStack))
				{
					bContinue = false;
					break;
				}
			}
		}
	}

	if (!bContinue)
	{
		// Revert everything, then try a transition tile at the picked position
		for (int back = (int)backupStack.size() - 1; back >= 0; --back)
			setTile(backupStack[back].Zone, backupStack[back].TileId, backupStack[back].Old, NULL, true);

		bool backup256 = backup.getCase() > 0;
		if (!_256 && !backup256)
		{
			CTileSetIdx tileSetCases[4][4];
			for (uint a = 0; a < 4; ++a)
			for (uint b = 0; b < 4; ++b)
			{
				tileSetCases[a][b].TileSet = -1;
				tileSetCases[a][b].Rotate = 0;
			}
			NL3D::CTileSet::TFlagBorder borderEdges[4][2];

			for (uint edge = 0; edge < 4; ++edge)
			{
				if (pTile->Voisins[edge])
				{
					CTileSetIdx pVoisinCorner[4];
					NL3D::CTileSet::TFlagBorder pBorder[4][3];
					CTileDescP pVoisinIndex;
					for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
					if (getBorderDesc(pTile->Voisins[edge], pVoisinCorner, pBorder, &pVoisinIndex))
					{
						int neigborEdge = (2 + edge + pTile->Rotate[edge]) & 3;
						tileSetCases[edge][edge] = pVoisinCorner[(neigborEdge + 1) & 3];
						tileSetCases[edge][edge].Rotate = (tileSetCases[edge][edge].Rotate - pTile->Rotate[edge]) & 3;
						tileSetCases[edge][(edge + 1) & 3] = pVoisinCorner[neigborEdge];
						tileSetCases[edge][(edge + 1) & 3].Rotate = (tileSetCases[edge][(edge + 1) & 3].Rotate - pTile->Rotate[edge]) & 3;
						for (uint subTile = 0; subTile < 2; ++subTile)
						{
							int slot = getLayer(pTile, (int)edge, pVoisinCorner[(neigborEdge + subTile) & 3].TileSet,
							                    (pVoisinCorner[(neigborEdge + subTile) & 3].Rotate - pTile->Rotate[edge]) & 3);
							if (slot < 0) return false;
							borderEdges[edge][1 - subTile] = NL3D::CTileSet::getInvertBorder(pBorder[neigborEdge][slot]);
						}
					}
				}
			}
			(void)borderEdges;

			CTileSetIdx finalCorner[4];
			for (uint corner = 0; corner < 4; ++corner)
			{
				finalCorner[corner].TileSet = -1;
				finalCorner[corner].Rotate = 0;
				for (uint layer = 0; layer < 4; ++layer)
				{
					if (finalCorner[corner].TileSet == -1
						|| tileSetCases[layer][corner].TileSet == -1
						|| tileSetCases[layer][corner] == finalCorner[corner])
					{
						if (tileSetCases[layer][corner].TileSet != -1)
							finalCorner[corner] = tileSetCases[layer][corner];
					}
					else return false;
				}
				if (finalCorner[corner].TileSet == -1)
				{
					finalCorner[corner].TileSet = tileSet;
					finalCorner[corner].Rotate = curRotation;
				}
			}

			std::vector<CTileSetIdx> setIndex;
			for (uint vv = 0; vv < 4; ++vv)
			{
				if (finalCorner[vv].TileSet == -1) return false;
				bool bFind = false;
				for (int w = 0; w < (int)setIndex.size(); ++w)
				{
					if (setIndex[w].TileSet == finalCorner[vv].TileSet)
					{
						CTileSetIdx complet = finalCorner[vv];
						complet.Rotate = (complet.Rotate + 2) & 3;
						if (setIndex[w].Rotate == complet.Rotate) return false;
						if (finalCorner[vv] == setIndex[w]) bFind = true;
					}
				}
				if (!bFind) setIndex.push_back(finalCorner[vv]);
			}
			std::sort(setIndex.begin(), setIndex.end());
			if (setIndex.size() > 3) return false;

			CTileIdx finalIndex[3];
			for (int l = 0; l < (int)setIndex.size(); ++l)
			{
				if (l == 0)
				{
					int nT = selectTile((uint)setIndex[l].TileSet, false, false);
					if (nT == -1) return false;
					finalIndex[l].Tile = (uint16)nT;
					finalIndex[l].Rotate = (uint8)(setIndex[l].Rotate & 3);
				}
				else
				{
					NL3D::CTileSet::TFlagBorder border[4];
					bool bFilled[4];
					int c;
					for (c = 0; c < 4; ++c)
						bFilled[c] = !(finalCorner[c] < setIndex[l]);
					for (uint e = 0; e < 4; ++e)
					{
						if (bFilled[e] && bFilled[(e + 1) & 3]) border[e] = NL3D::CTileSet::_1111;
						else if (!bFilled[e] && !bFilled[(e + 1) & 3]) border[e] = NL3D::CTileSet::_0000;
						else
						{
							bool found = false;
							if (pTile->Voisins[e])
							{
								CTileSetIdx pVoisinCorner[4];
								NL3D::CTileSet::TFlagBorder pBorder[4][3];
								CTileDescP pVoisinIndex;
								for (int k = 0; k < 4; ++k) { pVoisinCorner[k].TileSet = -1; pVoisinCorner[k].Rotate = 0; }
								if (getBorderDesc(pTile->Voisins[e], pVoisinCorner, pBorder, &pVoisinIndex))
								{
									int neigborEdge = (2 + e + pTile->Rotate[e]) & 3;
									int slot = getLayer(pTile, (int)e, setIndex[l].TileSet, setIndex[l].Rotate);
									if (slot != -1 && pBorder[neigborEdge][slot] != NL3D::CTileSet::dontcare)
									{
										border[e] = NL3D::CTileSet::getInvertBorder(pBorder[neigborEdge][slot]);
										found = true;
									}
								}
							}
							if (!found)
								border[e] = bFilled[e] ? NL3D::CTileSet::_1000 : NL3D::CTileSet::_0001;
						}
					}
					const NL3D::CTileSetTransition *tr = findTransition(setIndex[l].TileSet, setIndex[l].Rotate, border);
					if (!tr) return false;
					finalIndex[l].Rotate = (uint8)(setIndex[l].Rotate & 3);
					finalIndex[l].Tile = (uint16)tr->getTile();
				}
			}

			CTileDescP desc;
			getTileIdx((uint)pTile->Zone, pTile->TileId, desc);
			switch (setIndex.size())
			{
			case 1: desc.setTile(1, 0, desc.getDisplace(), finalIndex[0], CTileIdx(), CTileIdx()); break;
			case 2: desc.setTile(2, 0, desc.getDisplace(), finalIndex[0], finalIndex[1], CTileIdx()); break;
			case 3: desc.setTile(3, 0, desc.getDisplace(), finalIndex[0], finalIndex[1], finalIndex[2]); break;
			default: return false;
			}
			setTile((uint)pTile->Zone, pTile->TileId, desc, NULL, true);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CPaintCore::recursTile(SPaintTile *pTile, int tileSet, int recurs, std::set<SPaintTile *> &alreadyRecursed,
                            bool first, int rotation, bool _256)
{
	if (alreadyRecursed.find(pTile) == alreadyRecursed.end())
	{
		alreadyRecursed.insert(pTile);
		std::set<SPaintTile *> visited;
		putATile(pTile, tileSet, rotation, first, visited, _256);
	}
	if (recurs > 0)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (_256)
			{
				if (pTile->get2Voisin(i))
					recursTile(pTile->get2Voisin(i), tileSet, recurs - 2, alreadyRecursed, false,
					           (rotation + pTile->get2VoisinRotate(i)) & 3, true);
			}
			else
			{
				if (pTile->Voisins[i])
					recursTile(pTile->Voisins[i], tileSet, recurs - 1, alreadyRecursed, false,
					           (rotation + pTile->Rotate[i]) & 3, false);
			}
		}
	}
}

uint8 CPaintCore::calcRotPath(SPaintTile *from, SPaintTile *to, int depth, int rotate, int &dx, int &dy, int &cost)
{
	static const int x[4] = { -1, 0, 1, 0 };
	static const int y[4] = { 0, 1, 0, -1 };
	if (from == to)
	{
		cost = 0;
		dx = 0;
		dy = 0;
		return 0;
	}
	if (depth > 0)
	{
		uint8 ret = 0xff;
		cost = 1000000;
		int best = 0;
		for (int i = 0; i < 4; ++i)
		{
			if (from->Voisins[i])
			{
				int myDx, myDy, myCost;
				int myRet = calcRotPath(from->Voisins[i], to, depth - 1, (from->Rotate[i] + rotate) & 3, myDx, myDy, myCost);
				if (myRet != 0xff)
				{
					myDx += x[(i + rotate) & 3];
					myDy += y[(i + rotate) & 3];
					myCost++;
					if (myCost < cost)
					{
						cost = myCost;
						dx = myDx;
						dy = myDy;
						best = i;
						ret = (uint8)myRet;
					}
				}
			}
		}
		if (ret != 0xff)
			return (uint8)((from->Rotate[best] + ret) & 3);
	}
	return 0xff;
}

// ---------------------------------------------------------------------------------------------
// ops

bool CPaintCore::opTile(uint zone, uint patch, uint u, uint v, int tileSet, int rot, bool _256, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	// zone parameter is the zone ID; map to index
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (u >= orderS(zi, patch) || v >= orderT(zi, patch)) { err = "tile out of range"; return false; }
	if (tileSet >= m_Bank->getTileSetCount()) { err = "tile set out of range"; return false; }
	SPaintTile *t = metaAt(zi, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u));
	if (!t) { err = "tile not in grid"; return false; }
	m_StrokeSets = 0;
	std::set<SPaintTile *> visited;
	bool ok = putATile(t, tileSet, rot & 3, true, visited, _256);
	applyChanges();
	endStroke();
	if (!ok) err = "op failed (frozen/locked or unsolvable transition)";
	return ok;
}

bool CPaintCore::opClear(uint zone, uint patch, uint u, uint v, bool _256, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (u >= orderS(zi, patch) || v >= orderT(zi, patch)) { err = "tile out of range"; return false; }
	SPaintTile *t = metaAt(zi, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u));
	if (!t) { err = "tile not in grid"; return false; }
	m_StrokeSets = 0;
	bool ok = clearATile(t, _256);
	applyChanges();
	endStroke();
	if (!ok) err = "op failed (frozen/locked)";
	return ok;
}

bool CPaintCore::opTileStroke(uint zone, sint32 tileId, int tileSet, bool _256, bool first, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	SPaintTile *t = metaAt(zi, tileId);
	if (!t) { err = "tile not in grid"; return false; }

	// PutTile port: stroke rotation tracking
	if (first)
	{
		CTileDescP desc;
		getTileIdx(zi, tileId, desc);
		if (desc.isEmpty()) m_StrokeRotation = 0;
		else m_StrokeRotation = desc.getLayer(0).Rotate;
	}
	else if (m_StrokeOldTile >= 0)
	{
		SPaintTile *from = metaAt((uint)m_StrokeOldZone, m_StrokeOldTile);
		if (from)
		{
			int dx, dy, cost;
			uint8 deltaRot = calcRotPath(from, t, ZP_DEPTH_SEARCH_MAX, 0, dx, dy, cost);
			if (deltaRot != 0xff)
				m_StrokeRotation = (m_StrokeRotation + deltaRot) & 3;
			else
			{
				err = "stroke path lost";
				return false;
			}
		}
	}

	m_StrokeSets = 0;
	std::set<SPaintTile *> alreadyRecursed;
	recursTile(t, tileSet, 0 /* brush 0 for P3b */, alreadyRecursed, first, m_StrokeRotation, _256);
	applyChanges();
	m_StrokeOldTile = (sint32)tileId;
	m_StrokeOldZone = (sint32)zi;
	return true;
}

void CPaintCore::endStroke()
{
	if (m_CurStroke.empty()) return;
	m_UndoStack.push_back(m_CurStroke);
	m_CurStroke.clear();
	m_RedoStack.clear();
	while ((int)m_UndoStack.size() > ZP_MAX_UNDO)
		m_UndoStack.pop_front();
}

void CPaintCore::applyUndoList(const std::vector<SUndoTile> &list, bool useOld)
{
	if (useOld)
	{
		for (int i = (int)list.size() - 1; i >= 0; --i)
			setTile(list[i].Zone, list[i].TileId, list[i].Old, NULL, false, true);
	}
	else
	{
		for (size_t i = 0; i < list.size(); ++i)
			setTile(list[i].Zone, list[i].TileId, list[i].New, NULL, false, true);
	}
	applyChanges();
}

bool CPaintCore::opUndo()
{
	if (m_UndoStack.empty()) return false;
	std::vector<SUndoTile> list = m_UndoStack.back();
	m_UndoStack.pop_back();
	applyUndoList(list, true);
	m_RedoStack.push_back(list);
	return true;
}

bool CPaintCore::opRedo()
{
	if (m_RedoStack.empty()) return false;
	std::vector<SUndoTile> list = m_RedoStack.back();
	m_RedoStack.pop_back();
	applyUndoList(list, false);
	m_UndoStack.push_back(list);
	return true;
}

// ---------------------------------------------------------------------------------------------
// Pick (HitATile/CheckTri port, display bezier quads, closest hit wins)

static bool zpCheckTri(const NLMISC::CVector &v0, const NLMISC::CVector &v1, const NLMISC::CVector &v2,
                       const NLMISC::CVector &pos, const NLMISC::CVector &dirIn, NLMISC::CVector &hit)
{
	NLMISC::CVector dir = dirIn.normed();
	NLMISC::CVector center = (v0 + v1 + v2) / 3.f;
	NLMISC::CPlane plane;
	plane.make(v0, v1, v2);
	NLMISC::CVector normal = plane.getNormal();
	if ((plane * pos) < 0.f) return false;
	if ((dir * (center - pos)) < 0.f) return false;
	hit = plane.intersect(pos, pos + dir);
	bool positive = ((v0 - hit) ^ (v1 - hit)) * normal > 0.f;
	if ((((v1 - hit) ^ (v2 - hit)) * normal > 0.f) != positive) return false;
	return (((v2 - hit) ^ (v0 - hit)) * normal > 0.f) == positive;
}

sint32 CPaintCore::tileCorners(uint zone, sint32 tileId, NLMISC::CVector corners[4]) const
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return -1;
	int patch = tileId / ZP_NUM_TILE_SEL;
	int ttile = tileId % ZP_NUM_TILE_SEL;
	int v = ttile / ZP_MAX_TILE_IN_PATCH;
	int u = ttile % ZP_MAX_TILE_IN_PATCH;
	if ((size_t)patch >= m_Zones[zi].In.Patches->size()) return -1;
	const NL3D::CBezierPatch &bp = (*m_Zones[zi].In.Patches)[patch].Patch;
	float nU = (float)orderS(zi, (uint)patch);
	float nV = (float)orderT(zi, (uint)patch);
	corners[0] = bp.eval((float)u / nU, (float)v / nV);
	corners[1] = bp.eval((float)u / nU, (float)(v + 1) / nV);
	corners[2] = bp.eval((float)(u + 1) / nU, (float)(v + 1) / nV);
	corners[3] = bp.eval((float)(u + 1) / nU, (float)v / nV);
	return 0;
}

bool CPaintCore::pickTile(const NLMISC::CVector &pos, const NLMISC::CVector &dir, uint &zone, sint32 &tileId,
                          NLMISC::CVector &hitOut)
{
	float bestDist = 1e30f;
	bool found = false;
	for (size_t zi = 0; zi < m_Zones.size(); ++zi)
	{
		const SZone &z = m_Zones[zi];
		for (size_t p = 0; p < z.In.EvalRp->Patches.size(); ++p)
		{
			uint nU = orderS((uint)zi, (uint)p);
			uint nV = orderT((uint)zi, (uint)p);
			const NL3D::CBezierPatch &bp = (*z.In.Patches)[p].Patch;
			for (uint u = 0; u < nU; ++u)
			for (uint v = 0; v < nV; ++v)
			{
				NLMISC::CVector c[4];
				c[0] = bp.eval((float)u / nU, (float)v / nV);
				c[1] = bp.eval((float)u / nU, (float)(v + 1) / nV);
				c[2] = bp.eval((float)(u + 1) / nU, (float)(v + 1) / nV);
				c[3] = bp.eval((float)(u + 1) / nU, (float)v / nV);
				NLMISC::CVector hit;
				if (zpCheckTri(c[0], c[1], c[3], pos, dir, hit) || zpCheckTri(c[1], c[2], c[3], pos, dir, hit))
				{
					float d = (hit - pos).norm();
					if (d < bestDist)
					{
						bestDist = d;
						zone = z.In.ZoneId;
						tileId = (sint32)(p * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u);
						hitOut = hit;
						found = true;
					}
				}
			}
		}
	}
	return found;
}

// ---------------------------------------------------------------------------------------------
// write-back + dumps

bool CPaintCore::writeBack(std::string &err)
{
	for (size_t c = 0; c < m_Carriers.size(); ++c)
	{
		SCarrier &car = m_Carriers[c];
		// Frozen-only carriers are reference display: never rewritten (their tiles cannot have
		// been edited either; ops enforce the per-tile frozen flag).
		if (!car.AnyUnfrozen) continue;
		if (car.SnapLeaf)
		{
			encodeRPatchMesh(*car.Pristine, car.SnapLeaf->Value);
		}
		else
		{
			if (!car.Rpo->setRPatch(*car.Pristine))
			{
				err = "setRPatch failed";
				return false;
			}
		}
	}
	return true;
}

void CPaintCore::dumpRpo(FILE *out) const
{
	for (size_t c = 0; c < m_Carriers.size(); ++c)
	{
		const SCarrier &car = m_Carriers[c];
		fprintf(out, "carrier %u kind=%s zones=", (uint)c, car.SnapLeaf ? "snapshot" : "rpo");
		for (size_t s = 0; s < car.Zones.size(); ++s)
			fprintf(out, "%s%u", s ? "," : "", m_Zones[car.Zones[s]].In.ZoneId);
		fprintf(out, " unfrozen=%d\n", car.AnyUnfrozen ? 1 : 0);
		const SRPatchMesh &rp = *car.Pristine;
		for (size_t p = 0; p < rp.Patches.size(); ++p)
		{
			const SRpoPatch &up = rp.Patches[p];
			int os = 1 << up.NbTilesU, ot = 1 << up.NbTilesV;
			fprintf(out, " patch %u order %dx%d\n", (uint)p, os, ot);
			for (int v = 0; v < ot; ++v)
			for (int u = 0; u < os; ++u)
			{
				const SRpoTile &t = up.Tiles[u + v * os];
				fprintf(out, "  tile %d %d: num=%u flags=0x%04x noise=%u l0=(%d,r%d) l1=(%d,r%d) l2=(%d,r%d)\n",
				        u, v, t.Num, t.Flags, t.Noise,
				        t.Layer[0].Tile, t.Layer[0].Rotate,
				        t.Layer[1].Tile, t.Layer[1].Rotate,
				        t.Layer[2].Tile, t.Layer[2].Rotate);
			}
		}
	}
}

void CPaintCore::dumpBankXRef(FILE *out) const
{
	static const char *typeNames[4] = { "128", "256", "transition", "undefined" };
	for (int t = 0; t < m_Bank->getTileCount(); ++t)
	{
		int tileSet, number;
		NL3D::CTileBank::TTileType type;
		m_Bank->getTileXRef(t, tileSet, number, type);
		fprintf(out, "tile %d set %d number %d type %s\n", t, tileSet, number,
		        typeNames[(type >= 0 && type < 4) ? type : 3]);
	}
}

bool CPaintCore::dumpCarrierBlob(uint zone, std::vector<uint8> &out) const
{
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId == zone)
		{
			out = m_Carriers[m_Zones[i].Carrier].OriginalBytes;
			return true;
		}
	}
	return false;
}

void CPaintCore::getTile(uint zone, sint32 tileId, CTileDescP &desc) const
{
	// public overload: zone is a zone ID
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId == zone)
		{
			getTileRaw((uint)i, tileId, desc);
			transformDesc(desc, false, 0, (uint)i, tileId);
			return;
		}
	}
	desc.setEmpty();
}

uint CPaintCore::tileSetCount() const
{
	return (uint)m_Bank->getTileSetCount();
}

std::string CPaintCore::tileSetName(int tileSet) const
{
	if (!m_Bank || tileSet < 0 || tileSet >= m_Bank->getTileSetCount()) return "<none>";
	return m_Bank->getTileSet(tileSet)->getName();
}

int CPaintCore::tileSetOfTile(uint tile) const
{
	if (!m_Bank || (int)tile >= m_Bank->getTileCount()) return -1;
	int tileSet, number;
	NL3D::CTileBank::TTileType type;
	m_Bank->getTileXRef((int)tile, tileSet, number, type);
	return tileSet;
}

bool CPaintCore::zoneFrozen(uint zone) const
{
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) return m_Zones[i].In.Frozen;
	return true;
}

} /* namespace ZPPAINT */

/* end of file */
