/**
 * \file paint_ops.cpp
 * \brief CPaintCore paint algorithms and op-layer. See paint_core.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 *
 * ClearATile/PutATile transition solver, tile/color/displace op-layer methods, brush mask,
 * undo/redo/prop, checkSeams, preloadTiles, pickTile, writeBack, dirty tracking, dumps,
 * and read-only accessors.
 */

/*
 * Copyright (C) 2026 by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE. If not, see
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

	// Seed + duals share one pristine slot; mark all so dual graphs cannot re-seed it.
	markVisitedWithDuals(pTile, visited);

	if (tileSet == -1)
		return clearATile(pTile, _256);

	int nTile = selectTile((uint)tileSet, selectCycle, _256);
	if (nTile == -1) return false;

	// Stroke mark for abort: every write below uses undo=true, so m_CurStroke grows for
	// this put only. On refuse (propagate fail / residual illegal seams / transition
	// fallback abort) restore pristine bytes via HaveRaw and truncate the stroke so no
	// phantom undo entries remain and unused-layer bytes stay exact.
	const size_t strokeMark = m_CurStroke.size();

	if (_256)
	{
		CTileDescP desc;
		desc.setTile(1, 1 + ((-curRotation) & 3), 0, CTileIdx(nTile, curRotation), CTileIdx(), CTileIdx());
		setTile((uint)pTile->Zone, pTile->TileId, desc, &backupStack, true);
		int nRot;
		SPaintTile *other = pTile->getRight256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation - 1) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		markVisitedWithDuals(other, visited);
		other = pTile->getBottom256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation + 1) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		markVisitedWithDuals(other, visited);
		other = pTile->getRightBottom256(0, nRot);
		desc.setTile(1, 1 + ((-curRotation + 2) & 3), 0, CTileIdx(nTile, (curRotation - nRot) & 3), CTileIdx(), CTileIdx());
		setTile((uint)other->Zone, other->TileId, desc, &backupStack, true);
		markVisitedWithDuals(other, visited);
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
	// Dual meta tiles (same carrier+tileId on another zone) own complementary voisin graphs
	// under self-instance welds. After the seed zone's neighbors, also propagate from each
	// dual's unvisited neighbors. Interior duals are already marked visited via
	// markVisitedWithDuals so this is a no-op away from the weld; near-seam paints gain the
	// primary (or instance) side of the shared slot.
	if (bContinue)
	{
		std::vector<SPaintTile *> duals;
		collectDuals(pTile, duals);
		for (size_t di = 0; di < duals.size() && bContinue; ++di)
		{
			SPaintTile *d = duals[di];
			for (int i = 0; i < 4; ++i)
			{
				uint ii = (offset + i) & 0x3;
				if (d->Voisins[ii] && visited.find(d->Voisins[ii]) == visited.end())
				{
					if (!propagateBorder(d->Voisins[ii], (d->Rotate[ii] + curRotation) & 3, tileSet, visited, backupStack))
					{
						bContinue = false;
						break;
					}
				}
			}
		}
	}

	if (!bContinue)
	{
		// Revert the attempted put (byte-exact, no stroke pollution), then try a
		// transition tile at the picked position. Early returns below in the prep
		// leave the stroke clean because the abort already ran.
		abortStrokeTo(strokeMark);
		backupStack.clear();

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
					// Plugin: explicit group 0 here; the fallback base ignores the group bias.
					int nT = selectTile((uint)setIndex[l].TileSet, false, false, 0);
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
			// Residual gate: transition-at-pick must also leave legal seams
			if (!tileSeamsLegal(pTile))
			{
				abortStrokeTo(strokeMark);
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		// Residual gate: full put claimed success; refuse if any WRITTEN tile has an
		// illegal seam (hard UV/rot cases the dual-graph walk still cannot close; primary
		// and instance both hit them). Revert the stroke so no illegal state persists.
		// Check backupStack only (not duals merely marked visited) so mirrored duals with
		// transformDesc display-space noise do not false-positive the gate.
		if (!writtenSeamsLegal(backupStack))
		{
			fprintf(stderr, "putATile: refuse write that would leave illegal seams (zone %u tileId %d)\n",
			        (uint)pTile->Zone, (int)pTile->TileId);
			abortStrokeTo(strokeMark);
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
	// -1 is the clear sentinel; anything else negative would index the bank out of bounds
	if (tileSet < -1 || tileSet >= m_Bank->getTileSetCount()) { err = "tile set out of range"; return false; }
	SPaintTile *t = metaAt(zi, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u));
	if (!t) { err = "tile not in grid"; return false; }
	m_StrokeSets = 0;
	noteEditTile(zi, t->TileId);
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
	noteEditTile(zi, t->TileId);
	bool ok = clearATile(t, _256);
	applyChanges();
	endStroke();
	if (!ok) err = "op failed (frozen/locked)";
	return ok;
}

bool CPaintCore::opTileStroke(uint zone, sint32 tileId, int tileSet, bool _256, bool first, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	// -1 is the clear sentinel; anything else negative would index the bank out of bounds
	if (tileSet < -1 || tileSet >= m_Bank->getTileSetCount()) { err = "tile set out of range"; return false; }
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
	noteEditTile(zi, tileId);
	std::set<SPaintTile *> alreadyRecursed;
	// Brush size -> recursion depth, the plugin's PutTile call: brushValue[brushSize] with the
	// same depth whether 128 or 256 (RecursTile steps -2 per 256 hop).
	recursTile(t, tileSet, ZP_BRUSH_VALUE[m_BrushSize], alreadyRecursed, first, m_StrokeRotation, _256);
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

void CPaintCore::abortStrokeTo(size_t mark)
{
	// Reverse of the tile entries pushed after `mark`: desc restore + raw pristine restore,
	// NO undo push (would re-pollute the stroke) and no endStroke. applyChanges is the
	// caller's job; putATile abort paths leave the batch for the outer op apply.
	if (m_CurStroke.size() <= mark)
		return;
	for (int i = (int)m_CurStroke.size() - 1; i >= (int)mark; --i)
	{
		if (m_CurStroke[i].Kind != 0)
			continue;
		setTile(m_CurStroke[i].Zone, m_CurStroke[i].TileId, m_CurStroke[i].Old, NULL, false, true);
		if (m_CurStroke[i].HaveRaw)
			restoreRawTile(m_CurStroke[i], true);
	}
	m_CurStroke.resize(mark);
}

static void zpApplyPropRaw(CNodeImpl *node, uint32 appDataId, bool has, const std::string &value)
{
	if (!node)
		return;
	STORAGE::CAppData *ad = node->appData();
	if (!ad)
		return;
	if (has)
		ad->setScriptString(appDataId, value);
	else
		ad->erase(STORAGE::CAppData::ScriptClassId, STORAGE::CAppData::ScriptSuperClassId, appDataId);
}

void CPaintCore::applyUndoList(const std::vector<SUndoTile> &list, bool useOld)
{
	// Bind/edge-flag restores fire ONE state-changed callback per zone AFTER the whole list
	// replays: the callback re-derives BindEdges and rebuilds the landscape zone, and doing
	// that per record would rebuild once per vertex of a released group.
	std::set<uint> rpChangedZones;
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i].Kind == 4 || list[i].Kind == 5)
			rpChangedZones.insert(list[i].Zone);
	if (useOld)
	{
		for (int i = (int)list.size() - 1; i >= 0; --i)
		{
			if (list[i].Kind == 5)
				applyEdgeFlagUndo(list[i], true);
			else if (list[i].Kind == 4)
				applyBindUndo(list[i], true);
			else if (list[i].Kind == 3)
				applyGeomUndo(list[i], true);
			else if (list[i].Kind == 2)
			{
				// Prop: restore old raw presence/value
				uint zi = (uint)-1;
				for (size_t z = 0; z < m_Zones.size(); ++z)
					if (m_Zones[z].In.ZoneId == list[i].Zone) { zi = (uint)z; break; }
				if (zi != (uint)-1)
				{
					zpApplyPropRaw(m_Zones[zi].In.Node, list[i].AppDataId, list[i].OldHas, list[i].OldValue);
					if (m_PropChangedCb)
						m_PropChangedCb(list[i].Zone, list[i].AppDataId);
				}
			}
			else if (list[i].Kind == 1)
				setColorRaw(list[i].Zone, (uint)list[i].Patch, list[i].S, list[i].T, list[i].OldColor, false);
			else
			{
				setTile(list[i].Zone, list[i].TileId, list[i].Old, NULL, false, true);
				if (list[i].HaveRaw)
					restoreRawTile(list[i], true);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < list.size(); ++i)
		{
			if (list[i].Kind == 5)
				applyEdgeFlagUndo(list[i], false);
			else if (list[i].Kind == 4)
				applyBindUndo(list[i], false);
			else if (list[i].Kind == 3)
				applyGeomUndo(list[i], false);
			else if (list[i].Kind == 2)
			{
				uint zi = (uint)-1;
				for (size_t z = 0; z < m_Zones.size(); ++z)
					if (m_Zones[z].In.ZoneId == list[i].Zone) { zi = (uint)z; break; }
				if (zi != (uint)-1)
				{
					zpApplyPropRaw(m_Zones[zi].In.Node, list[i].AppDataId, list[i].NewHas, list[i].NewValue);
					if (m_PropChangedCb)
						m_PropChangedCb(list[i].Zone, list[i].AppDataId);
				}
			}
			else if (list[i].Kind == 1)
				setColorRaw(list[i].Zone, (uint)list[i].Patch, list[i].S, list[i].T, list[i].NewColor, false);
			else
			{
				setTile(list[i].Zone, list[i].TileId, list[i].New, NULL, false, true);
				if (list[i].HaveRaw)
					restoreRawTile(list[i], false);
			}
		}
	}
	applyChanges();
	if (m_RpStateChangedCb)
		for (std::set<uint>::const_iterator it = rpChangedZones.begin();
		     it != rpChangedZones.end(); ++it)
			m_RpStateChangedCb(*it);
}

bool CPaintCore::opProp(uint zoneId, uint32 appDataId, bool newHas, const std::string &newValue,
                        std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1)
	{
		err = "unknown zone";
		return false;
	}
	SZone &z = m_Zones[zi];
	if (z.In.Frozen)
	{
		err = "read-only";
		return false;
	}
	CNodeImpl *node = z.In.Node;
	if (!node)
	{
		err = "no node";
		return false;
	}
	// Capture old raw
	SUndoTile rec;
	rec.Kind = 2;
	rec.Zone = zoneId;
	rec.AppDataId = appDataId;
	std::string oldS;
	rec.OldHas = APPDATA::getScriptAppData(node, appDataId, oldS);
	if (rec.OldHas)
		rec.OldValue = oldS;
	rec.NewHas = newHas;
	rec.NewValue = newValue;
	// No-op if identical
	if (rec.OldHas == rec.NewHas && (!rec.NewHas || rec.OldValue == rec.NewValue))
		return true;
	zpApplyPropRaw(node, appDataId, newHas, newValue);
	// Commit (not discard) any in-flight paint stroke first: a Lua script can issue a
	// prop op between stroke segments, and clearing would drop those undo records while
	// their pristine mutations stay applied.
	endStroke();
	m_CurStroke.push_back(rec);
	endStroke();
	if (m_PropChangedCb)
		m_PropChangedCb(zoneId, appDataId);
	return true;
}

uint CPaintCore::opMovePatchElems(uint zoneId, const std::vector<SGeomElemRef> &elems,
                                  const std::vector<NLMISC::CVector> &objDeltas, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone"; return 0; }
	SZone &z = m_Zones[zi];
	if (z.In.Frozen) { err = "read-only"; return 0; }
	if (!z.In.Node) { err = "no node"; return 0; }

	if (objDeltas.size() != elems.size()) { err = "delta count mismatch"; return 0; }
	std::vector<SUndoTile> recs;
	recs.reserve(elems.size());
	for (size_t i = 0; i < elems.size(); ++i)
	{
		const float objDelta[3] = { objDeltas[i].x, objDeltas[i].y, objDeltas[i].z };
		SGeomWriteTarget t;
		std::string e;
		if (!resolveGeomWriteTarget(z.In.Node, elems[i].Idx, elems[i].Elem, t, e))
		{
			if (err.empty()) err = e;
			continue;
		}
		SUndoTile rec;
		rec.Kind = 3;
		rec.Zone = zoneId;
		rec.VertIdx = elems[i].Idx;
		rec.ElemKind = (uint8)elems[i].Elem;
		if (!geomTargetGet(t, rec.OldPos)) { if (err.empty()) err = "target read failed"; continue; }
		for (int k = 0; k < 3; ++k)
			rec.NewPos[k] = rec.OldPos[k] + objDelta[k];
		if (!geomTargetSet(t, rec.NewPos)) { if (err.empty()) err = "target write failed"; continue; }
		recs.push_back(rec);
		if (m_GeomChangedCb)
			m_GeomChangedCb(zoneId, rec.VertIdx, (int)elems[i].Elem, objDelta);
	}
	if (recs.empty())
		return 0;

	// One stroke for the whole selection: an artist who dragged twelve vertices expects one
	// Ctrl+Z, not twelve. Commit any in-flight paint stroke first for the same reason opProp
	// does - a scripted move between stroke segments must not drop their records.
	endStroke();
	for (size_t i = 0; i < recs.size(); ++i)
		m_CurStroke.push_back(recs[i]);
	endStroke();
	markGeomDirty(zoneId);
	return (uint)recs.size();
}

/** Undo/redo of a Kind 3 record: re-resolve the target and put back the stored triple. */
void CPaintCore::applyGeomUndo(const SUndoTile &rec, bool useOld)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == rec.Zone) { zi = (uint)i; break; }
	if (zi == (uint)-1 || !m_Zones[zi].In.Node)
		return;
	SGeomWriteTarget t;
	std::string e;
	if (!resolveGeomWriteTarget(m_Zones[zi].In.Node, rec.VertIdx, (EGeomElem)rec.ElemKind, t, e))
		return;
	const float *pos = useOld ? rec.OldPos : rec.NewPos;
	const float *from = useOld ? rec.NewPos : rec.OldPos;
	if (!geomTargetSet(t, pos))
		return;
	markGeomDirty(rec.Zone);
	if (m_GeomChangedCb)
	{
		// The difference of the two stored values, which is the object-space delta whatever
		// the target holds - undo of a mapper delta and undo of a PatchMesh position are the
		// same shift to the display.
		const float d[3] = { pos[0] - from[0], pos[1] - from[1], pos[2] - from[2] };
		m_GeomChangedCb(rec.Zone, rec.VertIdx, (int)rec.ElemKind, d);
	}
}

// SRpoVertexBind <-> the SUndoTile flat form, in field order. The whole record travels
// through undo so a released bind's caches come back verbatim - that is what makes
// unbind -> undo -> save byte-identical to the baseline.
static void zpFlattenBind(const SRpoVertexBind &b, uint32 out[11])
{
	out[0] = b.Binded; out[1] = b.Type; out[2] = b.Edge; out[3] = b.Patch;
	out[4] = b.Before; out[5] = b.Before2; out[6] = b.After; out[7] = b.After2;
	out[8] = b.T; out[9] = b.Type2; out[10] = b.PrimVert;
}

static void zpUnflattenBind(const uint32 in[11], SRpoVertexBind &b)
{
	b.Binded = (uint8)in[0]; b.Type = in[1]; b.Edge = in[2]; b.Patch = in[3];
	b.Before = in[4]; b.Before2 = in[5]; b.After = in[6]; b.After2 = in[7];
	b.T = in[8]; b.Type2 = in[9]; b.PrimVert = in[10];
}

uint CPaintCore::opEditBinds(uint zoneId, const std::vector<SBindEdit> &binds,
                             const std::vector<SGeomElemRef> &snapElems,
                             const std::vector<NLMISC::CVector> &snapDeltas, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone"; return 0; }
	SZone &z = m_Zones[zi];
	if (z.In.Frozen) { err = "read-only"; return 0; }
	if (snapDeltas.size() != snapElems.size()) { err = "snap count mismatch"; return 0; }
	SRPatchMesh *rp = pristineOf(zi);
	if (!rp) { err = "no carrier"; return 0; }

	std::vector<SUndoTile> recs;
	recs.reserve(binds.size());
	for (size_t i = 0; i < binds.size(); ++i)
	{
		const SBindEdit &e = binds[i];
		if (e.Vert >= rp->Verts.size())
		{
			if (err.empty()) err = "bind vertex out of range";
			continue;
		}
		SRpoVertexBind &b = rp->Verts[e.Vert];
		SUndoTile rec;
		rec.Kind = 4;
		rec.Zone = zoneId;
		rec.VertIdx = e.Vert;
		zpFlattenBind(b, rec.OldBind);
		if (e.Binded)
		{
			// Legacy BindingVertex: full target written, caches invalidated for the loader
			// (and our own eval refresh) to rebuild.
			b.Binded = 1;
			b.Type = e.Type;
			b.Type2 = e.Type;
			b.Edge = e.Edge;
			b.Patch = e.Patch;
			b.PrimVert = e.PrimVert;
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		}
		else
		{
			// Legacy UnBindingVertex: flag and caches only; the dead target fields stay.
			b.Binded = 0;
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		}
		zpFlattenBind(b, rec.NewBind);
		bool same = true;
		for (int k = 0; k < 11 && same; ++k)
			same = rec.OldBind[k] == rec.NewBind[k];
		if (same)
			continue;
		recs.push_back(rec);
	}

	// Geometry snap (new binds land their vertices on the bindWhere points) - same records
	// the move op writes, deltas already object-space.
	std::vector<SUndoTile> geomRecs;
	geomRecs.reserve(snapElems.size());
	for (size_t i = 0; i < snapElems.size(); ++i)
	{
		SGeomWriteTarget t;
		std::string e;
		if (!resolveGeomWriteTarget(z.In.Node, snapElems[i].Idx, snapElems[i].Elem, t, e))
		{
			if (err.empty()) err = e;
			continue;
		}
		SUndoTile rec;
		rec.Kind = 3;
		rec.Zone = zoneId;
		rec.VertIdx = snapElems[i].Idx;
		rec.ElemKind = (uint8)snapElems[i].Elem;
		if (!geomTargetGet(t, rec.OldPos)) { if (err.empty()) err = "target read failed"; continue; }
		for (int k = 0; k < 3; ++k)
			rec.NewPos[k] = rec.OldPos[k] + (k == 0 ? snapDeltas[i].x : (k == 1 ? snapDeltas[i].y : snapDeltas[i].z));
		if (!geomTargetSet(t, rec.NewPos)) { if (err.empty()) err = "target write failed"; continue; }
		geomRecs.push_back(rec);
		const float d[3] = { snapDeltas[i].x, snapDeltas[i].y, snapDeltas[i].z };
		if (m_GeomChangedCb)
			m_GeomChangedCb(zoneId, rec.VertIdx, (int)snapElems[i].Elem, d);
	}

	if (recs.empty() && geomRecs.empty())
		return 0;
	// One stroke for the whole edit, bind records and snap together - an unbind or a bind is
	// one action however many vertices the group held.
	endStroke();
	for (size_t i = 0; i < recs.size(); ++i)
		m_CurStroke.push_back(recs[i]);
	for (size_t i = 0; i < geomRecs.size(); ++i)
		m_CurStroke.push_back(geomRecs[i]);
	endStroke();
	if (!geomRecs.empty())
		markGeomDirty(zoneId);
	if (!recs.empty() && m_RpStateChangedCb)
		m_RpStateChangedCb(zoneId);
	return (uint)recs.size();
}

uint CPaintCore::opSetEdgeFlags(uint zoneId, const std::vector<SEdgeFlagEdit> &writes, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone"; return 0; }
	if (m_Zones[zi].In.Frozen) { err = "read-only"; return 0; }
	SRPatchMesh *rp = pristineOf(zi);
	if (!rp) { err = "no carrier"; return 0; }

	std::vector<SUndoTile> recs;
	recs.reserve(writes.size());
	for (size_t i = 0; i < writes.size(); ++i)
	{
		const SEdgeFlagEdit &w = writes[i];
		if (w.Patch >= rp->Patches.size() || w.EdgeSlot >= 4)
		{
			if (err.empty()) err = "edge flag target out of range";
			continue;
		}
		uint32 &flags = rp->Patches[w.Patch].EdgeFlags[w.EdgeSlot];
		if (flags == w.NewFlags)
			continue;
		SUndoTile rec;
		rec.Kind = 5;
		rec.Zone = zoneId;
		rec.Patch = (sint32)w.Patch;
		rec.S = (sint32)w.EdgeSlot;
		rec.OldColor = flags;
		rec.NewColor = w.NewFlags;
		flags = w.NewFlags;
		recs.push_back(rec);
	}
	if (recs.empty())
		return 0;
	endStroke();
	for (size_t i = 0; i < recs.size(); ++i)
		m_CurStroke.push_back(recs[i]);
	endStroke();
	if (m_RpStateChangedCb)
		m_RpStateChangedCb(zoneId);
	return (uint)recs.size();
}

bool CPaintCore::getVertBind(uint zoneId, uint16 vert, bool &binded, uint32 &type, uint32 &edge,
                             uint32 &patch, uint32 &primVert) const
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1) return false;
	const SRPatchMesh *rp = pristineOf(zi);
	if (!rp || vert >= rp->Verts.size()) return false;
	const SRpoVertexBind &b = rp->Verts[vert];
	binded = b.Binded != 0;
	type = b.Type;
	edge = b.Edge;
	patch = b.Patch;
	primVert = b.PrimVert;
	return true;
}

bool CPaintCore::getPatchEdgeFlags(uint zoneId, uint16 patch, uint32 outFlags[4]) const
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zoneId) { zi = (uint)i; break; }
	if (zi == (uint)-1) return false;
	const SRPatchMesh *rp = pristineOf(zi);
	if (!rp || patch >= rp->Patches.size()) return false;
	for (int e = 0; e < 4; ++e)
		outFlags[e] = rp->Patches[patch].EdgeFlags[e];
	return true;
}

/** Undo/redo of a Kind 4 record: put back the stored bind record verbatim. */
void CPaintCore::applyBindUndo(const SUndoTile &rec, bool useOld)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == rec.Zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return;
	SRPatchMesh *rp = pristineOf(zi);
	if (!rp || rec.VertIdx >= rp->Verts.size()) return;
	zpUnflattenBind(useOld ? rec.OldBind : rec.NewBind, rp->Verts[rec.VertIdx]);
}

/** Undo/redo of a Kind 5 record: put back the stored flag word. */
void CPaintCore::applyEdgeFlagUndo(const SUndoTile &rec, bool useOld)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == rec.Zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return;
	SRPatchMesh *rp = pristineOf(zi);
	if (!rp || rec.Patch < 0 || (size_t)rec.Patch >= rp->Patches.size()
	    || rec.S < 0 || rec.S >= 4)
		return;
	rp->Patches[rec.Patch].EdgeFlags[rec.S] = useOld ? rec.OldColor : rec.NewColor;
}

bool CPaintCore::opUndo()
{
	// Commit (not discard) any in-flight stroke first: undoing beneath an open
	// stroke corrupts the undo pairing. The stroke's pre-stroke "old" snapshots
	// would commit later over the undone state, and the popped entry is lost when
	// that commit clears the redo stack. The mouse path commits before calling
	// here; scripted undo reaches this directly.
	endStroke();
	if (m_UndoStack.empty()) return false;
	std::vector<SUndoTile> list = m_UndoStack.back();
	m_UndoStack.pop_back();
	applyUndoList(list, true);
	m_RedoStack.push_back(list);
	return true;
}

bool CPaintCore::opRedo()
{
	// Same commit-first rule as opUndo. Committing a live stroke clears the redo
	// stack, so redo-under-open-stroke correctly reports "redo stack empty":
	// new paint invalidates redo, matching the interactive path.
	endStroke();
	if (m_RedoStack.empty()) return false;
	std::vector<SUndoTile> list = m_RedoStack.back();
	m_RedoStack.pop_back();
	applyUndoList(list, false);
	m_UndoStack.push_back(list);
	return true;
}

// ---------------------------------------------------------------------------------------------
// Vertex colors (CPaintColor port). Colors live in the pristine SRpoPatch.Colors as raw
// 0xAARRGGBB (the on-disk uint32; the high byte is the alpha the original get/setVertexColor
// round-trips). PRISTINE DISCIPLINE: only these values ever mutate; the display mirror writes
// the 565 conversion into the live zone color arrays.

uint32 CPaintCore::getColorRaw(uint zoneIdx, uint patch, sint32 s, sint32 t) const
{
	const SRpoPatch &up = pristineOf(zoneIdx)->Patches[patch];
	int os = (1 << up.NbTilesU) + 1;
	return up.Colors[s + t * os];
}

void CPaintCore::setColorRaw(uint zoneIdx, uint patch, sint32 s, sint32 t, uint32 color, bool undo)
{
	SRpoPatch &up = pristineOf(zoneIdx)->Patches[patch];
	int os = (1 << up.NbTilesU) + 1;
	uint32 old = up.Colors[s + t * os];
	up.Colors[s + t * os] = color;
	++m_StrokeSets;

	// Display mirror: every zone sharing the carrier; S flips under Symmetry (plugin paint_vcolor)
	if (m_Landscape)
	{
		const std::vector<uint> &shared = m_Carriers[m_Zones[zoneIdx].Carrier].Zones;
		for (size_t i = 0; i < shared.size(); ++i)
		{
			uint zi = shared[i];
			std::vector<NL3D::CTileColor> *arr = changeColorArray(zi, patch);
			if (!arr) continue;
			sint32 ds = s;
			if (m_Zones[zi].In.Symmetry)
				ds = os - s - 1;
			NLMISC::CRGBA rgba((uint8)((color >> 16) & 0xff), (uint8)((color >> 8) & 0xff), (uint8)(color & 0xff));
			(*arr)[ds + t * os].Color565 = rgba.get565();
		}
	}

	if (undo)
	{
		SUndoTile u;
		u.Kind = 1;
		u.Zone = zoneIdx;
		u.Patch = (sint32)patch;
		u.S = s;
		u.T = t;
		u.OldColor = old;
		u.NewColor = color;
		m_CurStroke.push_back(u);
	}
}

// getVertexInNeighbor port: grid vertex vertexId of tile (0=(u,v), 1=(u,v+1), 2=(u+1,v+1),
// 3=(u+1,v)) mapped through neighbor edge n into the neighbor patch's grid slot.
static bool zpVertexInNeighbor(SPaintTile *tile, int vertexId, int neighbor,
                               SColorSlot &out, SPaintTile *&outTile, int &outVertexId)
{
	if (!tile->Voisins[neighbor]) return false;
	int neighborVertexId = (((vertexId == neighbor) ? vertexId - 1 : vertexId + 1) + tile->Rotate[neighbor]) & 3;
	SPaintTile *nt = tile->Voisins[neighbor];
	out.ZoneIdx = (uint)nt->Zone;
	out.Patch = nt->Patch;
	out.S = nt->U + (((neighborVertexId == 2) || (neighborVertexId == 3)) ? 1 : 0);
	out.T = nt->V + (((neighborVertexId == 1) || (neighborVertexId == 2)) ? 1 : 0);
	outTile = nt;
	outVertexId = neighborVertexId;
	return true;
}

// Transitive co-location closure of a grid vertex: BFS over (tile, vertexId) pairs across the
// two edges adjacent to the vertex, using the stitched metaTile graph (intra-mesh edges, binds
// and the welded cross-zone borders all included). Every slot in the closure denotes the same
// world vertex; painting writes them all with the identical value (the continuity rule).
void CPaintCore::vertexClosure(uint zoneIdx, SPaintTile *tile, int vertexId, std::vector<SColorSlot> &out)
{
	out.clear();
	std::set<SColorSlot> seen;
	std::vector<std::pair<SPaintTile *, int> > queue;
	std::set<std::pair<SPaintTile *, int> > visited;
	queue.push_back(std::make_pair(tile, vertexId));
	while (!queue.empty())
	{
		SPaintTile *t = queue.back().first;
		int vid = queue.back().second;
		queue.pop_back();
		if (!visited.insert(std::make_pair(t, vid)).second) continue;
		SColorSlot slot;
		slot.ZoneIdx = (uint)t->Zone;
		slot.Patch = t->Patch;
		slot.S = t->U + ((vid == 2 || vid == 3) ? 1 : 0);
		slot.T = t->V + ((vid == 1 || vid == 2) ? 1 : 0);
		if (seen.insert(slot).second) out.push_back(slot);
		// The two edges adjacent to this vertex are vid and (vid-1)&3 (plugin diagram)
		for (int k = 0; k < 2; ++k)
		{
			int n = k ? ((vid - 1) & 3) : vid;
			SColorSlot nslot;
			SPaintTile *nt;
			int nvid;
			if (zpVertexInNeighbor(t, vid, n, nslot, nt, nvid))
				queue.push_back(std::make_pair(nt, nvid));
		}
		// Sibling tiles of the SAME patch sharing this vertex map to the same slot; enqueue
		// them so their outward edges are explored too (corner closure around the vertex).
		int du = (vid == 2 || vid == 3) ? 0 : -1; // grid tiles adjacent to the vertex
		int dv = (vid == 1 || vid == 2) ? 0 : -1;
		for (int su = 0; su <= 1; ++su)
		for (int sv = 0; sv <= 1; ++sv)
		{
			int uu = (int)t->U + du + su;
			int vv = (int)t->V + dv + sv;
			if (uu < 0 || vv < 0) continue;
			if ((uint)uu >= orderS((uint)t->Zone, (uint)t->Patch) || (uint)vv >= orderT((uint)t->Zone, (uint)t->Patch)) continue;
			SPaintTile *st = &m_Zones[t->Zone].Meta[t->Patch * ZP_NUM_TILE_SEL + vv * ZP_MAX_TILE_IN_PATCH + uu];
			if (st->TileId < 0) continue;
			// vertexId of the shared vertex within st
			int svid;
			int ds = slot.S - st->U, dt = slot.T - st->V;
			if (ds == 0 && dt == 0) svid = 0;
			else if (ds == 0 && dt == 1) svid = 1;
			else if (ds == 1 && dt == 1) svid = 2;
			else svid = 3;
			queue.push_back(std::make_pair(st, svid));
		}
	}
	(void)zoneIdx;
}

bool CPaintCore::colorVertexBorderLocked(uint zoneIdx, uint patch, sint32 s, sint32 t) const
{
	if (!m_LockBorders)
		return false;
	const sint32 os = (sint32)orderS(zoneIdx, patch), ot = (sint32)orderT(zoneIdx, patch);
	if (s < 0 || s > os || t < 0 || t > ot)
		return true; // out of range: refuse defensively
	// Owner tile + the two sides adjacent to this corner (paint_vcolor.cpp:150-215):
	// (s,t) interior -> tile (s,t) sides left(0)/top(3); right edge -> (s-1,t) right(2)/top(3);
	// bottom edge -> (s,t-1) left(0)/bottom(1); corner -> (s-1,t-1) right(2)/bottom(1).
	sint32 tu = s, tv = t;
	int sideA = 0, sideB = 3;
	if (s == os && t == ot) { tu = s - 1; tv = t - 1; sideA = 2; sideB = 1; }
	else if (s == os) { tu = s - 1; sideA = 2; sideB = 3; }
	else if (t == ot) { tv = t - 1; sideA = 0; sideB = 1; }
	const SPaintTile &tile = m_Zones[zoneIdx].Meta[patch * ZP_NUM_TILE_SEL
	                                              + tv * ZP_MAX_TILE_IN_PATCH + tu];
	if (tile.TileId < 0)
		return true; // dead meta cell: refuse (no side information)
	const bool aLocked = (tile.Voisins[sideA] == NULL) || tile.Voisins[sideA]->Frozen;
	const bool bLocked = (tile.Voisins[sideB] == NULL) || tile.Voisins[sideB]->Frozen;
	return aLocked || bLocked;
}

bool CPaintCore::setVertexColorShared(const std::vector<SColorSlot> &slots, NLMISC::CRGBA color, uint blend)
{
	// Frozen zones' carriers are never rewritten: their slots drop out of the write set (the
	// plugin painted the live zone's boundary freely against frozen neighbor zones too); the
	// continuity guarantee holds across every WRITABLE slot of the closure.
	std::vector<SColorSlot> writable;
	for (size_t i = 0; i < slots.size(); ++i)
		if (!m_Zones[slots[i].ZoneIdx].In.Frozen) writable.push_back(slots[i]);
	if (writable.empty()) return false;
	// Blend ONCE against the primary slot's old color, then write the identical result to
	// every co-located slot: shared border vertices get the same color on both sides even
	// when the sides' stored colors had drifted.
	uint32 oldRaw = getColorRaw(writable[0].ZoneIdx, (uint)writable[0].Patch, writable[0].S, writable[0].T);
	NLMISC::CRGBA old((uint8)((oldRaw >> 16) & 0xff), (uint8)((oldRaw >> 8) & 0xff), (uint8)(oldRaw & 0xff), (uint8)(oldRaw >> 24));
	NLMISC::CRGBA blended;
	blended.blendFromui(old, color, blend);
	uint32 raw = ((uint32)blended.A << 24) | ((uint32)blended.R << 16) | ((uint32)blended.G << 8) | blended.B;
	for (size_t i = 0; i < writable.size(); ++i)
		setColorRaw(writable[i].ZoneIdx, (uint)writable[i].Patch, writable[i].S, writable[i].T, raw, true);
	return true;
}

// Nearest grid tile of a zone to a world point (explicit-hit brush seeding).
bool CPaintCore::nearestTile(uint zone, const NLMISC::CVector &pos, sint32 &tileId)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return false;
	float best = 1e30f;
	tileId = -1;
	SZone &z = m_Zones[zi];
	for (size_t k = 0; k < z.Meta.size(); ++k)
	{
		if (z.Meta[k].TileId < 0) continue;
		float d = (z.Meta[k].Center - pos).norm();
		if (d < best)
		{
			best = d;
			tileId = z.Meta[k].TileId;
		}
	}
	return tileId >= 0;
}

bool CPaintCore::opColorVertex(uint zone, uint patch, sint32 s, sint32 t, NLMISC::CRGBA color, uint blend, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	sint32 os = (sint32)orderS(zi, patch), ot = (sint32)orderT(zi, patch);
	if (s < 0 || s > os || t < 0 || t > ot) { err = "vertex out of range"; return false; }
	// Find a tile adjacent to the vertex + its vertexId
	sint32 tu = std::min(std::max(s - 1, (sint32)0), os - 1);
	sint32 tv = std::min(std::max(t - 1, (sint32)0), ot - 1);
	// prefer the tile whose top-left is the vertex when possible
	if (s < os && t < ot) { tu = s; tv = t; }
	SPaintTile *tile = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + tv * ZP_MAX_TILE_IN_PATCH + tu];
	if (tile->TileId < 0) { err = "tile not in grid"; return false; }
	if (colorVertexBorderLocked(zi, patch, s, t)) { err = "locked border"; return false; }
	int ds = (int)(s - tile->U), dt = (int)(t - tile->V);
	int vid = (ds == 0 && dt == 0) ? 0 : (ds == 0 && dt == 1) ? 1 : (ds == 1 && dt == 1) ? 2 : 3;
	std::vector<SColorSlot> slots;
	vertexClosure(zi, tile, vid, slots);
	m_StrokeSets = 0;
	noteEditTile(zi, tile->TileId);
	bool ok = setVertexColorShared(slots, color, blend > 256 ? 256 : blend);
	applyChanges();
	endStroke();
	if (!ok) err = "vertex frozen";
	return ok;
}

// Color-brush bitmap mask (CPaintColor::loadBrush port): any .tga; grayscale files load as
// luminance (loadGrayscaleAsAlpha(false)), converted to RGBA. The sampling path reads RGB and
// averages, exactly like the plugin. Loading turns the mask mode on (the plugin's
// SelectColorBrush flow called setBrushMode(true) right after loadBrush).
bool CPaintCore::loadBrushMask(const std::string &fileName, std::string &err)
{
	std::string path = fileName;
	if (!NLMISC::CFile::fileExists(path))
	{
		std::string looked = NLMISC::CPath::lookup(fileName, false, false);
		if (!looked.empty()) path = looked;
	}
	try
	{
		NLMISC::CIFile inputFile;
		if (!inputFile.open(path))
		{
			err = "cannot open brush mask " + fileName;
			return false;
		}
		NLMISC::CBitmap bitmap;
		bitmap.loadGrayscaleAsAlpha(false);
		if (!bitmap.load(inputFile))
		{
			err = "cannot read brush mask " + path;
			return false;
		}
		if (!bitmap.convertToType(NLMISC::CBitmap::RGBA))
		{
			err = "cannot convert brush mask " + path;
			return false;
		}
		m_BrushMask = bitmap;
	}
	catch (const NLMISC::Exception &e)
	{
		err = std::string("brush mask: ") + e.what();
		return false;
	}
	m_BrushMaskLoaded = m_BrushMask.getWidth() != 0 && m_BrushMask.getHeight() != 0;
	m_BrushMaskMode = m_BrushMaskLoaded;
	m_BrushMaskName = m_BrushMaskLoaded ? NLMISC::CFile::getFilename(path) : std::string();
	if (!m_BrushMaskLoaded) { err = "empty brush mask " + path; return false; }
	return true;
}

void CPaintCore::clearBrushMask()
{
	m_BrushMask.reset();
	m_BrushMaskLoaded = false;
	m_BrushMaskMode = false;
	m_BrushMaskName.clear();
}

bool CPaintCore::setBrushMaskMode(bool on)
{
	// Plugin setBrushMode: only on when a valid bitmap is loaded
	m_BrushMaskMode = on && m_BrushMaskLoaded;
	return m_BrushMaskMode;
}

// The color brush (CPaintColor::paint/paintATile/paintAVertex port): walk the metaTile graph
// from the seed within the radius; each candidate grid vertex is blended ONCE (distance/
// hardness/opacity falloff against its world position on the display bezier surface) and
// written through its whole closure. Active mask: per-vertex blend modulated by the mask
// bitmap projected on the brush plane (see the header doc; paintAVertex port).
bool CPaintCore::opColorBrush(uint zone, sint32 seedTileId, const NLMISC::CVector &hit, float radius,
                              NLMISC::CRGBA color, uint hardness, uint opacity, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	SPaintTile *seed = metaAt(zi, seedTileId);
	if (!seed) { err = "tile not in grid"; return false; }
	if (radius <= 0.f) { err = "bad radius"; return false; }
	float hard = (float)(hardness > 255 ? 255 : hardness) / 255.f;
	float opa = (float)(opacity > 255 ? 255 : opacity) / 255.f;
	// The brush's own hit point and radius describe the edit better than any single tile.
	noteEditAt(hit, radius);

	// Brush-plane base vectors (CPaintColor::paint port). The plugin's topVector is the hit
	// tile quad's normal ((p1-p0)^(p2-p0) over corners (u,v),(u,v+1),(u+1,v+1)); the seed tile
	// plays that role here (deterministic from the display bezier, headless-stable).
	bool maskOn = m_BrushMaskMode && m_BrushMaskLoaded;
	NLMISC::CVector paintBaseX, paintBaseY;
	if (maskOn)
	{
		const NL3D::CBezierPatch &sbp = (*m_Zones[zi].In.Patches)[seed->Patch].Patch;
		float snU = (float)orderS(zi, (uint)seed->Patch);
		float snV = (float)orderT(zi, (uint)seed->Patch);
		NLMISC::CVector p0 = sbp.eval((float)seed->U / snU, (float)seed->V / snV);
		NLMISC::CVector p1 = sbp.eval((float)seed->U / snU, (float)(seed->V + 1) / snV);
		NLMISC::CVector p2 = sbp.eval((float)(seed->U + 1) / snU, (float)(seed->V + 1) / snV);
		NLMISC::CVector topVector = ((p1 - p0) ^ (p2 - p0)).normed();
		if (fabs(topVector * NLMISC::CVector::K) > fabs(topVector * NLMISC::CVector::J))
		{
			paintBaseX = NLMISC::CVector::J ^ topVector;
			paintBaseX.normalize();
			paintBaseY = topVector ^ paintBaseX;
			paintBaseY.normalize();
		}
		else
		{
			paintBaseX = topVector ^ NLMISC::CVector::K;
			paintBaseX.normalize();
			paintBaseY = topVector ^ paintBaseX;
			paintBaseY.normalize();
		}
	}

	m_StrokeSets = 0;

	// BFS tiles in range
	std::set<SPaintTile *> visited;
	std::vector<SPaintTile *> queue;
	queue.push_back(seed);
	std::set<SColorSlot> vertexDone; // canonical (first) slot of each painted closure
	uint painted = 0;
	while (!queue.empty())
	{
		SPaintTile *t = queue.back();
		queue.pop_back();
		if (!visited.insert(t).second) continue;
		if ((t->Center - hit).norm() > radius + t->Radius) continue;
		// Candidate vertices: the plugin's per-tile scheme (top-left always, border extras);
		// visiting all four corners is equivalent under the closure dedup.
		for (int vid = 0; vid < 4; ++vid)
		{
			// lockBorders: refuse open/frozen-border vertices like the plugin's paintAVertex
			// (checked in the visited tile's own zone; owner-tile mapping in the helper).
			{
				const sint32 vs = (sint32)t->U + ((vid == 2 || vid == 3) ? 1 : 0);
				const sint32 vt = (sint32)t->V + ((vid == 1 || vid == 2) ? 1 : 0);
				if (colorVertexBorderLocked((uint)t->Zone, (uint)t->Patch, vs, vt))
					continue;
			}
			std::vector<SColorSlot> slots;
			vertexClosure(zi, t, vid, slots);
			if (slots.empty()) continue;
			// dedup by the closure's canonical (minimal) slot
			SColorSlot canon = slots[0];
			for (size_t i = 1; i < slots.size(); ++i)
				if (slots[i] < canon) canon = slots[i];
			if (!vertexDone.insert(canon).second) continue;
			// World position of the vertex on the display surface
			const SZone &vz = m_Zones[canon.ZoneIdx];
			const NL3D::CBezierPatch &bp = (*vz.In.Patches)[canon.Patch].Patch;
			float os = (float)orderS(canon.ZoneIdx, (uint)canon.Patch);
			float ot = (float)orderT(canon.ZoneIdx, (uint)canon.Patch);
			NLMISC::CVector pos = bp.eval((float)canon.S / os, (float)canon.T / ot);
			float dist = (pos - hit).norm();
			if (dist > radius) continue;
			// Blend with distance (paintAVertex): 256*opa*((1-hard)*blendDist + hard)
			float blendDist = (radius - dist) / radius;
			float finalFactor = 256.f * opa * ((1.f - hard) * blendDist + hard);
			uint blend = (uint)std::max(std::min(finalFactor, 256.f), 0.f);
			// Mask modulation (paintAVertex "Use a brush ?" branch, exact integer arithmetic):
			// project the vertex delta on the brush plane, sample the bitmap bilinearly,
			// scale the blend by the sampled luminance mean. All-white mask: blend*255/255.
			if (maskOn)
			{
				NLMISC::CVector deltaPos = pos - hit;
				float bitmapX = (1.f + (paintBaseX * deltaPos) / radius) / 2.f;
				float bitmapY = (1.f + (paintBaseY * deltaPos) / radius) / 2.f;
				NLMISC::CRGBAF colorF = m_BrushMask.getColor(bitmapX, bitmapY);
				colorF *= 255.f;
				NLMISC::CRGBA maskColor;
				maskColor.R = (uint8)colorF.R;
				maskColor.G = (uint8)colorF.G;
				maskColor.B = (uint8)colorF.B;
				maskColor.A = (maskColor.R + maskColor.G + maskColor.B) / 3;
				blend = blend * maskColor.A / 255;
			}
			if (setVertexColorShared(slots, color, blend)) ++painted;
		}
		for (int n = 0; n < 4; ++n)
			if (t->Voisins[n]) queue.push_back(t->Voisins[n]);
	}
	applyChanges();
	if (!painted) { err = "no vertex in range (or frozen)"; return false; }
	return true;
}

// ---------------------------------------------------------------------------------------------
// Region fills (CFillPatch ports)

bool CPaintCore::isLockedEx(SPaintTile *tile)
{
	if (!m_LockBorders) return false;
	CTileDescP backup;
	getTileIdx((uint)tile->Zone, tile->TileId, backup);
	if (backup.getCase() > 0)
	{
		if (tile->U & 1) tile = tile->Voisins[0];
		if (!tile) return true;
		if (tile->V & 1) tile = tile->Voisins[3];
		if (!tile) return true;
		int nRot;
		SPaintTile *r = tile->getRight256(0, nRot);
		SPaintTile *b = tile->getBottom256(0, nRot);
		SPaintTile *rb = tile->getRightBottom256(0, nRot);
		return tile->Locked != 0 || !r || r->Locked != 0 || !b || b->Locked != 0 || !rb || rb->Locked != 0;
	}
	return tile->Locked != 0;
}

bool CPaintCore::isLocked256(SPaintTile *tile)
{
	if (!m_LockBorders) return false;
	if (tile->U & 1) tile = tile->Voisins[0];
	if (!tile) return true;
	if (tile->V & 1) tile = tile->Voisins[3];
	if (!tile) return true;
	if (tile->Locked) return true;
	if (!tile->Voisins[2] || tile->Voisins[2]->Locked) return true;
	if (!tile->Voisins[1] || tile->Voisins[1]->Locked) return true;
	if (!tile->Voisins[2]->Voisins[1] || tile->Voisins[2]->Voisins[1]->Locked) return true;
	return false;
}

// CFillPatch::fillTile port: fill every tile of the patch with a random tile of the set;
// borders whose outside neighbor is non-empty and not (single layer, same set, matching
// rotation) are CLEARED instead (the plugin's rule), which by construction leaves only legal
// seams (same-set, empty or cleared).
bool CPaintCore::fillTileImpl(uint zi, uint patch, int tileSet, int rot, bool _256)
{
	CTileDescP descFill;
	uint numU = orderS(zi, patch), numV = orderT(zi, patch);
	if (m_Zones[zi].In.Frozen) return false;
	for (uint v = 0; v < numV; v += (1u << (_256 ? 1 : 0)))
	for (uint u = 0; u < numU; u += (1u << (_256 ? 1 : 0)))
	{
		int nTile = 0;
		if (tileSet != -1)
		{
			nTile = selectTile((uint)tileSet, false, _256, m_TileGroup);
			if (nTile == -1) return false;
		}
		uint span = _256 ? 1 : 0;
		bool locked = false, nearLocked = false;
		uint uu, vv;
		// A 256 block on an odd-order patch overlaps dead default cells (TileId -1).
		// The plugin's fixed 16x16 UI_PATCH::Tile array made writes to those slack
		// entries harmless no-ops; the port's Tiles are sized OrderS x OrderT, so
		// dead cells must be skipped per-cell everywhere below (never indexed).
		for (vv = 0; vv <= span && !locked; ++vv)
		for (uu = 0; uu <= span; ++uu)
		{
			SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + (vv + v) * ZP_MAX_TILE_IN_PATCH + uu + u];
			if (t->TileId < 0) continue;
			if (_256 ? isLocked256(t) : isLockedEx(t)) { locked = true; break; }
			for (uint n = 0; n < 4; ++n)
				if (t->Voisins[n] && (_256 ? isLocked256(t->Voisins[n]) : isLockedEx(t->Voisins[n])))
					nearLocked = true;
		}
		if (locked) continue;
		if (nearLocked)
		{
			for (vv = 0; vv <= span; ++vv)
			for (uu = 0; uu <= span; ++uu)
			{
				SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + (vv + v) * ZP_MAX_TILE_IN_PATCH + uu + u];
				if (t->TileId < 0) continue;
				clearATile(t, _256, !_256);
			}
			continue;
		}
		// Compatibility of the outside borders
		bool compatible = true;
		if (tileSet != -1)
		{
			for (vv = 0; vv <= span && compatible; ++vv)
			for (uu = 0; uu <= span && compatible; ++uu)
			{
				SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + (vv + v) * ZP_MAX_TILE_IN_PATCH + uu + u];
				if (t->TileId < 0) continue;
				for (uint n = 0; n < 4; ++n)
				{
					SPaintTile *nb = t->Voisins[n];
					if (nb && ((uint)nb->Zone != zi || nb->Patch != (sint32)patch))
					{
						CTileDescP descNei;
						getTileIdx((uint)nb->Zone, nb->TileId, descNei);
						if (descNei.getNumLayer() == 0) continue;
						if (descNei.getNumLayer() == 1
							&& (int)descNei.getLayer(0).Rotate == ((t->Rotate[n] + rot) & 3)
							&& (int)descNei.getLayer(0).Tile < m_Bank->getTileCount())
						{
							int neiTileSet, number;
							NL3D::CTileBank::TTileType type;
							m_Bank->getTileXRef(descNei.getLayer(0).Tile, neiTileSet, number, type);
							if (tileSet == neiTileSet) continue;
						}
						compatible = false;
						break;
					}
				}
			}
		}
		if (compatible)
		{
			for (vv = 0; vv <= span; ++vv)
			for (uu = 0; uu <= span; ++uu)
			{
				SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + (vv + v) * ZP_MAX_TILE_IN_PATCH + uu + u];
				if (t->TileId < 0) continue;
				if (tileSet != -1)
				{
					if (_256)
					{
						switch (((uu & 1) << 1) | (vv & 1))
						{
						case 0: descFill.setTile(1, ((0 - rot) & 3) + 1, 0, CTileIdx(nTile, rot), CTileIdx(), CTileIdx()); break;
						case 1: descFill.setTile(1, ((1 - rot) & 3) + 1, 0, CTileIdx(nTile, rot), CTileIdx(), CTileIdx()); break;
						case 2: descFill.setTile(1, ((3 - rot) & 3) + 1, 0, CTileIdx(nTile, rot), CTileIdx(), CTileIdx()); break;
						case 3: descFill.setTile(1, ((2 - rot) & 3) + 1, 0, CTileIdx(nTile, rot), CTileIdx(), CTileIdx()); break;
						}
					}
					else
						descFill.setTile(1, 0, 0, CTileIdx(nTile, rot), CTileIdx(), CTileIdx());
				}
				else
					descFill.setTile(0, 0, 0, CTileIdx(), CTileIdx(), CTileIdx());
				CTileDescP descOrig;
				getTileIdx(zi, t->TileId, descOrig);
				descFill.setDisplace(descOrig.getDisplace());
				setTile(zi, t->TileId, descFill, NULL, true);
			}
		}
		else
		{
			for (vv = 0; vv <= span; ++vv)
			for (uu = 0; uu <= span; ++uu)
			{
				SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + (vv + v) * ZP_MAX_TILE_IN_PATCH + uu + u];
				if (t->TileId < 0) continue;
				clearATile(t, _256, !_256);
			}
		}
	}
	return true;
}

bool CPaintCore::opFillTile(uint zone, uint patch, int tileSet, int rot, bool _256, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	// -1 is the clear sentinel; anything else negative would index the bank out of bounds
	if (tileSet < -1 || tileSet >= m_Bank->getTileSetCount()) { err = "tile set out of range"; return false; }
	m_StrokeSets = 0;
	noteEditPatch(zi, patch);
	bool ok = fillTileImpl(zi, patch, tileSet, rot & 3, _256);
	applyChanges();
	endStroke();
	if (!ok) err = "fill failed (frozen zone or empty tile set)";
	return ok;
}

// CFillPatch::fillColor port. Deviation from the plugin (which wrote only the filled patch's
// grid): border vertices write through their closures so the neighbor patches' shared
// vertices stay continuous.
bool CPaintCore::opFillColor(uint zone, uint patch, NLMISC::CRGBA color, uint blend, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (m_Zones[zi].In.Frozen) { err = "zone frozen"; return false; }
	sint32 numU = (sint32)orderS(zi, patch) + 1;
	sint32 numV = (sint32)orderT(zi, patch) + 1;
	m_StrokeSets = 0;
	noteEditPatch(zi, patch);
	if (blend > 256) blend = 256;
	for (sint32 t = 0; t < numV; ++t)
	for (sint32 s = 0; s < numU; ++s)
	{
		// closure via the adjacent tile
		sint32 tu = std::min(std::max(s - 1, (sint32)0), numU - 2);
		sint32 tv = std::min(std::max(t - 1, (sint32)0), numV - 2);
		if (s < numU - 1 && t < numV - 1) { tu = s; tv = t; }
		SPaintTile *tile = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + tv * ZP_MAX_TILE_IN_PATCH + tu];
		if (tile->TileId < 0) continue;
		int ds = (int)(s - tile->U), dt = (int)(t - tile->V);
		int vid = (ds == 0 && dt == 0) ? 0 : (ds == 0 && dt == 1) ? 1 : (ds == 1 && dt == 1) ? 2 : 3;
		std::vector<SColorSlot> slots;
		vertexClosure(zi, tile, vid, slots);
		setVertexColorShared(slots, color, blend);
	}
	applyChanges();
	endStroke();
	return true;
}

// ---------------------------------------------------------------------------------------------
// Displace painting (PutADisplacetile / fillDisplace ports; explicit index instead of the
// plugin's DisplaceTile UI state). The displace bits and the v9 Noise byte stay in sync;
// the live mirror shows the new noise through the tesselation refresh.

void CPaintCore::displaceOne(SPaintTile *tile, uint displace)
{
	CTileDescP desc;
	getTileIdx((uint)tile->Zone, tile->TileId, desc);
	int t0 = (int)desc.getLayer(0).Tile;
	if (desc.isEmpty() || t0 < 0 || t0 >= m_Bank->getTileCount()) return; // plugin: valid layer 0 only
	desc.setDisplace((uint8)(displace & 0xf));
	setTile((uint)tile->Zone, tile->TileId, desc, NULL, true, true);
}

// RecursTile displace-mode port (the plugin's PutDisplace path): one PutADisplacetile per tile
// not already recursed, then spread depth-first on the 128 grid (PutDisplace always passes
// _256=false). Frozen tiles are skipped: the plugin wrote through SetTile freely, but frozen
// carriers are immutable reference display in this tool.
void CPaintCore::recursDisplace(SPaintTile *pTile, uint displace, int recurs, std::set<SPaintTile *> &alreadyRecursed)
{
	if (alreadyRecursed.find(pTile) == alreadyRecursed.end())
	{
		alreadyRecursed.insert(pTile);
		if (!pTile->Frozen)
			displaceOne(pTile, displace);
	}
	if (recurs > 0)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (pTile->Voisins[i])
				recursDisplace(pTile->Voisins[i], displace, recurs - 1, alreadyRecursed);
		}
	}
}

bool CPaintCore::opDisplace(uint zone, uint patch, uint u, uint v, uint displace, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (u >= orderS(zi, patch) || v >= orderT(zi, patch)) { err = "tile out of range"; return false; }
	if (displace > 15) { err = "displace out of range"; return false; }
	SPaintTile *t = metaAt(zi, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u));
	if (!t) { err = "tile not in grid"; return false; }
	if (t->Frozen) { err = "tile frozen"; return false; }
	m_StrokeSets = 0;
	noteEditTile(zi, t->TileId);
	// The plugin's displace put rode the same brush recursion as the tile put (PutDisplace ->
	// RecursTile depth brushValue[brushSize]); depth 0 == the historical single-tile behavior.
	std::set<SPaintTile *> alreadyRecursed;
	recursDisplace(t, displace, ZP_BRUSH_VALUE[m_BrushSize], alreadyRecursed);
	applyChanges();
	endStroke();
	if (!m_StrokeSets) { err = "tile empty or unresolvable layer 0"; return false; }
	return true;
}

bool CPaintCore::opRawTile(uint zone, uint patch, uint u, uint v, int tile, int rot, std::string &err)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (u >= orderS(zi, patch) || v >= orderT(zi, patch)) { err = "tile out of range"; return false; }
	SPaintTile *t = metaAt(zi, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u));
	if (!t) { err = "tile not in grid"; return false; }
	if (t->Frozen) { err = "tile frozen"; return false; }
	m_StrokeSets = 0;
	noteEditTile(zi, t->TileId);
	CTileDescP desc;
	desc.setTile(1, 0, 0, CTileIdx(tile, rot & 3), CTileIdx(), CTileIdx());
	setTile(zi, t->TileId, desc, NULL, true);
	applyChanges();
	endStroke();
	return true;
}

bool CPaintCore::opFillDisplace(uint zone, uint patch, uint displace, std::string &err)
{
	if (!m_Bank) { err = "no tile bank loaded"; return false; }
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) { err = "unknown zone id"; return false; }
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) { err = "patch out of range"; return false; }
	if (m_Zones[zi].In.Frozen) { err = "zone frozen"; return false; }
	if (displace > 15) { err = "displace out of range"; return false; }
	m_StrokeSets = 0;
	noteEditPatch(zi, patch);
	uint numU = orderS(zi, patch), numV = orderT(zi, patch);
	for (uint v = 0; v < numV; ++v)
	for (uint u = 0; u < numU; ++u)
	{
		SPaintTile *t = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u];
		if (t->TileId >= 0 && !t->Frozen) displaceOne(t, displace);
	}
	applyChanges();
	endStroke();
	return true;
}

// ---------------------------------------------------------------------------------------------
// Seam legality report: every adjacent non-empty tile pair must agree on the shared corner
// tile sets after rotation adjustment (the GetBorderDesc invariant the transition machinery
// maintains). Illegal pairs are printed; the count is returned.

uint CPaintCore::checkSeams(uint zone, FILE *out)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return 0;
	uint illegal = 0, pairs = 0;
	SZone &z = m_Zones[zi];
	for (size_t k = 0; k < z.Meta.size(); ++k)
	{
		SPaintTile *t = &z.Meta[k];
		if (t->TileId < 0) continue;
		CTileDescP desc;
		getTileIdx(zi, t->TileId, desc);
		if (desc.isEmpty()) continue;
		CTileSetIdx corner[4];
		NL3D::CTileSet::TFlagBorder border[4][3];
		CTileDescP idx;
		for (int c = 0; c < 4; ++c) { corner[c].TileSet = -1; corner[c].Rotate = 0; }
		if (!getBorderDesc(t, corner, border, &idx)) continue; // unresolvable (stale bank refs)
		for (uint e = 0; e < 4; ++e)
		{
			SPaintTile *nb = t->Voisins[e];
			if (!nb) continue;
			CTileDescP ndesc;
			getTileIdx((uint)nb->Zone, nb->TileId, ndesc);
			if (ndesc.isEmpty()) continue;
			CTileSetIdx ncorner[4];
			NL3D::CTileSet::TFlagBorder nborder[4][3];
			CTileDescP nidx;
			for (int c = 0; c < 4; ++c) { ncorner[c].TileSet = -1; ncorner[c].Rotate = 0; }
			if (!getBorderDesc(nb, ncorner, nborder, &nidx)) continue;
			++pairs;
			// Shared corners: my edge e endpoints (corners e, (e+1)&3) vs the neighbor's edge
			// (2+e+rotate)&3 endpoints. The legality criterion is TILE SET identity at the
			// shared corners: a set discontinuity with no transition covering it is a visible
			// crack. Rotation is deliberately NOT compared: full-tile rotations of
			// non-oriented sets are free across rotated seams (authored corpus zones carry
			// them; PropagateBorder only constrains rotation within an active repaint).
			int edge = (2 + (int)e + t->Rotate[e]) & 3;
			CTileSetIdx a1 = ncorner[(edge + 1) & 3];
			CTileSetIdx a2 = ncorner[edge];
			if (corner[e].TileSet != a1.TileSet || corner[(e + 1) & 3].TileSet != a2.TileSet)
			{
				++illegal;
				fprintf(out, "ILLEGAL seam: zone %u tile %d,%d edge %u vs zone %u tile %d,%d: (set %d / %d) vs (set %d / %d)\n",
				        zone, (int)t->U, (int)t->V, e, m_Zones[nb->Zone].In.ZoneId, (int)nb->U, (int)nb->V,
				        corner[e].TileSet, corner[(e + 1) & 3].TileSet, a1.TileSet, a2.TileSet);
			}
		}
	}
	fprintf(out, "SEAMS zone %u: %u adjacent non-empty pairs, %u illegal\n", zone, pairs, illegal);
	return illegal;
}

bool CPaintCore::dumpClosure(uint zone, uint patch, sint32 s, sint32 t, FILE *out)
{
	uint zi = (uint)-1;
	for (size_t i = 0; i < m_Zones.size(); ++i)
		if (m_Zones[i].In.ZoneId == zone) { zi = (uint)i; break; }
	if (zi == (uint)-1) return false;
	if (patch >= m_Zones[zi].In.EvalRp->Patches.size()) return false;
	sint32 os = (sint32)orderS(zi, patch), ot = (sint32)orderT(zi, patch);
	if (s < 0 || s > os || t < 0 || t > ot) return false;
	sint32 tu = std::min(std::max(s - 1, (sint32)0), os - 1);
	sint32 tv = std::min(std::max(t - 1, (sint32)0), ot - 1);
	if (s < os && t < ot) { tu = s; tv = t; }
	SPaintTile *tile = &m_Zones[zi].Meta[patch * ZP_NUM_TILE_SEL + tv * ZP_MAX_TILE_IN_PATCH + tu];
	if (tile->TileId < 0) return false;
	int ds = (int)(s - tile->U), dt = (int)(t - tile->V);
	int vid = (ds == 0 && dt == 0) ? 0 : (ds == 0 && dt == 1) ? 1 : (ds == 1 && dt == 1) ? 2 : 3;
	std::vector<SColorSlot> slots;
	vertexClosure(zi, tile, vid, slots);
	for (size_t i = 0; i < slots.size(); ++i)
	{
		const SZone &sz = m_Zones[slots[i].ZoneIdx];
		const NL3D::CBezierPatch &bp = (*sz.In.Patches)[slots[i].Patch].Patch;
		float sos = (float)orderS(slots[i].ZoneIdx, (uint)slots[i].Patch);
		float sot = (float)orderT(slots[i].ZoneIdx, (uint)slots[i].Patch);
		NLMISC::CVector pos = bp.eval((float)slots[i].S / sos, (float)slots[i].T / sot);
		fprintf(out, "CLOSURE zone %u patch %d s %d t %d pos %.3f %.3f %.3f\n",
		        sz.In.ZoneId, (int)slots[i].Patch, (int)slots[i].S, (int)slots[i].T, pos.x, pos.y, pos.z);
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Preload flush (myThread preloadTiles port, over every tile set).

void CPaintCore::preloadTiles(NL3D::IDriver *driver)
{
	if (!m_Bank || !m_Landscape || !driver) return;
	for (sint ts = 0; ts < m_Bank->getTileSetCount(); ++ts)
	{
		const NL3D::CTileSet *tileSet = m_Bank->getTileSet(ts);
		sint tl;
		for (tl = 0; tl < tileSet->getNumTile128(); ++tl)
			m_Landscape->flushTiles(driver, (uint16)tileSet->getTile128(tl), 1);
		for (tl = 0; tl < tileSet->getNumTile256(); ++tl)
			m_Landscape->flushTiles(driver, (uint16)tileSet->getTile256(tl), 1);
		for (tl = 0; tl < NL3D::CTileSet::count; ++tl)
			m_Landscape->flushTiles(driver, (uint16)tileSet->getTransition(tl)->getTile(), 1);
	}
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

void CPaintCore::markGeomDirty(uint zoneId)
{
	m_GeomDirty.insert(zoneId);
}

bool CPaintCore::geomDirty(uint zoneId) const
{
	return m_GeomDirty.count(zoneId) != 0;
}

bool CPaintCore::isZoneDirty(uint zoneId) const
{
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId != zoneId)
			continue;
		// Contract: frozen zones are never dirty. A frozen instance sharing a painted
		// unfrozen home's carrier must not report the carrier's dirt as its own.
		if (m_Zones[i].In.Frozen)
			return false;
		if (propsDirty((uint)i))
			return true;
		if (geomDirty(zoneId))
			return true;
		const SCarrier &car = m_Carriers[m_Zones[i].Carrier];
		if (!car.AnyUnfrozen)
			return false;
		std::vector<uint8> cur;
		if (car.SnapLeaf)
			encodeRPatchMesh(*car.Pristine, cur);
		else
			encodeRpoChunk(*car.Pristine, cur);
		return cur != car.OriginalBytes;
	}
	return false;
}

bool CPaintCore::anyZoneDirty(const std::vector<uint> &zoneIds) const
{
	// Dedup by carrier so multi-instance zones of one file count once
	std::set<uint> carriers;
	std::set<uint> zoneIdxs;
	for (size_t z = 0; z < zoneIds.size(); ++z)
	{
		for (size_t i = 0; i < m_Zones.size(); ++i)
		{
			if (m_Zones[i].In.ZoneId != zoneIds[z])
				continue;
			carriers.insert(m_Zones[i].Carrier);
			zoneIdxs.insert((uint)i);
			break;
		}
	}
	// Prop appdata lives outside carriers
	for (std::set<uint>::const_iterator it = zoneIdxs.begin(); it != zoneIdxs.end(); ++it)
	{
		if (propsDirty(*it))
			return true;
	}
	for (std::set<uint>::const_iterator it = carriers.begin(); it != carriers.end(); ++it)
	{
		const SCarrier &car = m_Carriers[*it];
		if (!car.AnyUnfrozen)
			continue;
		std::vector<uint8> cur;
		if (car.SnapLeaf)
			encodeRPatchMesh(*car.Pristine, cur);
		else
			encodeRpoChunk(*car.Pristine, cur);
		if (cur != car.OriginalBytes)
			return true;
	}
	return false;
}

void CPaintCore::markZonesSaved(const std::vector<uint> &zoneIds)
{
	// Geometry writes went straight into the chunk tree, so once the file is on disk they are
	// committed - there is no re-encode baseline to refresh, only the flag to drop.
	for (size_t z = 0; z < zoneIds.size(); ++z)
		m_GeomDirty.erase(zoneIds[z]);
	std::set<uint> carriers;
	for (size_t z = 0; z < zoneIds.size(); ++z)
	{
		for (size_t i = 0; i < m_Zones.size(); ++i)
		{
			if (m_Zones[i].In.ZoneId != zoneIds[z])
				continue;
			carriers.insert(m_Zones[i].Carrier);
			// Re-baseline export props with the saved file
			readPropSnap(m_Zones[i].In.Node, m_Zones[i].PropSnap);
			break;
		}
	}
	for (std::set<uint>::const_iterator it = carriers.begin(); it != carriers.end(); ++it)
	{
		SCarrier &car = m_Carriers[*it];
		if (!car.AnyUnfrozen)
			continue;
		// Prefer the already-written leaf payload when writeBack ran; else re-encode.
		if (car.SnapLeaf)
			car.OriginalBytes = car.SnapLeaf->Value;
		else
			encodeRpoChunk(*car.Pristine, car.OriginalBytes);
	}
}

void CPaintCore::stashOriginalBytes(std::map<const void *, std::vector<uint8> > &out) const
{
	out.clear();
	for (size_t c = 0; c < m_Carriers.size(); ++c)
	{
		const SCarrier &car = m_Carriers[c];
		const void *key = car.SnapLeaf ? (const void *)car.SnapLeaf : (const void *)car.Rpo;
		if (!key)
			continue;
		out[key] = car.OriginalBytes;
	}
}

void CPaintCore::restoreOriginalBytes(const std::map<const void *, std::vector<uint8> > &in)
{
	for (size_t c = 0; c < m_Carriers.size(); ++c)
	{
		SCarrier &car = m_Carriers[c];
		const void *key = car.SnapLeaf ? (const void *)car.SnapLeaf : (const void *)car.Rpo;
		if (!key)
			continue;
		std::map<const void *, std::vector<uint8> >::const_iterator it = in.find(key);
		if (it != in.end())
			car.OriginalBytes = it->second;
	}
}

void CPaintCore::revertZones(const std::vector<uint> &zoneIds)
{
	std::set<uint> carriers;
	for (size_t z = 0; z < zoneIds.size(); ++z)
	{
		for (size_t i = 0; i < m_Zones.size(); ++i)
		{
			if (m_Zones[i].In.ZoneId != zoneIds[z])
				continue;
			carriers.insert(m_Zones[i].Carrier);
			break;
		}
	}
	for (std::set<uint>::const_iterator it = carriers.begin(); it != carriers.end(); ++it)
	{
		SCarrier &car = m_Carriers[*it];
		if (!car.AnyUnfrozen || !car.Pristine)
			continue;
		std::string err;
		bool ok = false;
		if (car.SnapLeaf)
			ok = decodeRPatchMesh(nlVectorData(car.OriginalBytes), car.OriginalBytes.size(),
			                      *car.Pristine, err);
		else
			ok = decodeRpoChunk(nlVectorData(car.OriginalBytes), car.OriginalBytes.size(),
			                    *car.Pristine, err);
		if (!ok)
			fprintf(stderr, "WARNING: revertZones: re-decode failed: %s\n", err.c_str());
	}
	// Landscape mirror is rebuilt on working-set change; no per-tile apply needed here.
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
				fprintf(out, " tile %d %d: num=%u flags=0x%04x noise=%u l0=(%d,r%d) l1=(%d,r%d) l2=(%d,r%d)\n",
				        u, v, t.Num, t.Flags, t.Noise,
				        t.Layer[0].Tile, t.Layer[0].Rotate,
				        t.Layer[1].Tile, t.Layer[1].Rotate,
				        t.Layer[2].Tile, t.Layer[2].Rotate);
			}
			for (int v = 0; v < ot + 1; ++v)
			for (int u = 0; u < os + 1; ++u)
				fprintf(out, " color %d %d: 0x%08x\n", u, v, up.Colors[u + v * (os + 1)]);
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

// ---------------------------------------------------------------------------------------------
// Last-edit marker (see paint_core.h). Set by the op layer at the point the artist AIMED at,
// not at every tile a transition solve happens to rewrite.

void CPaintCore::noteEditAt(const NLMISC::CVector &pos, float radius)
{
	m_HaveLastEdit = true;
	m_LastEditPos = pos;
	m_LastEditRadius = radius > 0.f ? radius : 0.f;
}

void CPaintCore::noteEditTile(uint zoneIdx, sint32 tileId)
{
	const SPaintTile *t = metaAt(zoneIdx, tileId);
	if (!t) return;
	noteEditAt(t->Center, t->Radius);
}

void CPaintCore::noteEditPatch(uint zoneIdx, uint patch)
{
	if (zoneIdx >= m_Zones.size()) return;
	const std::vector<NL3D::CPatchInfo> *patches = m_Zones[zoneIdx].In.Patches;
	if (!patches || patch >= patches->size()) return;
	const NL3D::CBezierPatch &bp = (*patches)[patch].Patch;
	const NLMISC::CVector center = bp.eval(0.5f, 0.5f);
	float radius = 0.f;
	for (uint v = 0; v < 4; ++v)
	{
		const float d = (bp.Vertices[v] - center).norm();
		if (d > radius) radius = d;
	}
	noteEditAt(center, radius);
}

bool CPaintCore::lastEditPos(NLMISC::CVector &pos, float &radius) const
{
	if (!m_HaveLastEdit) return false;
	pos = m_LastEditPos;
	radius = m_LastEditRadius;
	return true;
}

void CPaintCore::getTile(uint zone, sint32 tileId, CTileDescP &desc) const
{
	// public overload: zone is a zone ID. Untrusted input (script `rot` op, window
	// pick): bounds-check before the raw accessors index pristine arrays.
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId == zone)
		{
			if (tileId < 0) { desc.setEmpty(); return; }
			uint patch = (uint)tileId / ZP_NUM_TILE_SEL;
			uint tile = (uint)tileId % ZP_NUM_TILE_SEL;
			uint v = tile / ZP_MAX_TILE_IN_PATCH;
			uint u = tile % ZP_MAX_TILE_IN_PATCH;
			const PIPELINE::MAX::NELPATCH::SRPatchMesh *rpo = pristineOf((uint)i);
			if (!rpo || patch >= rpo->Patches.size()
			    || u >= orderS((uint)i, patch) || v >= orderT((uint)i, patch))
			{
				desc.setEmpty();
				return;
			}
			getTileIdx((uint)i, tileId, desc);
			return;
		}
	}
	desc.setEmpty();
}

bool CPaintCore::getColor(uint zone, uint patch, sint32 s, sint32 t, uint32 &color) const
{
	for (size_t i = 0; i < m_Zones.size(); ++i)
	{
		if (m_Zones[i].In.ZoneId == zone)
		{
			if (patch >= pristineOf((uint)i)->Patches.size()) return false;
			const SRpoPatch &up = pristineOf((uint)i)->Patches[patch];
			sint32 os = (1 << up.NbTilesU) + 1, ot = (1 << up.NbTilesV) + 1;
			if (s < 0 || s >= os || t < 0 || t >= ot) return false;
			color = up.Colors[s + t * os];
			return true;
		}
	}
	return false;
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
