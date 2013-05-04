// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#include "stdpch.h"
#include "micro_life_manager.h"
#include "sheet_manager.h"
#include "misc.h"
#include "continent_manager.h"
#include "user_entity.h"
#include "weather.h"
#include "water_map.h"
//
#include "client_sheets/flora_sheet.h"
//
#include "nel/misc/polygon.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/line.h"
//
#include "nel/ligo/primitive.h"
//
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
//
#include "nel/misc/check_fpu.h"

using namespace std::rel_ops;
using namespace NLMISC;

extern NLLIGO::CLigoConfig	LigoConfig;
extern CContinentManager	ContinentMngr;
extern NL3D::ULandscape		*Landscape;
extern NL3D::UDriver		*Driver;
extern NL3D::UScene			*Scene;
extern CUserEntity			*Userentity;
extern NL3D::UMaterial		GenericMat;

#if !FINAL_VERSION
	bool DisplayMicroLifeZones = false;
#endif

#ifdef NL_DEBUG
	extern bool DisplayMicroLifeActiveTiles = false;
#endif

H_AUTO_DECL(RZ_MicroLifeManager)


// ********************************************************************************************
CMicroLifeManager::CMicroLifeManager()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	_CellSize = 0.f;
	_GridWidth = 0;
	_GridHeight = 0;
	_Noise.Abs  = 0.f;
	_Noise.Rand = 1.f;
}

// ********************************************************************************************
CMicroLifeManager &CMicroLifeManager::getInstance()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	static CMicroLifeManager instance;
	return instance;
}

// ********************************************************************************************
void CMicroLifeManager::init(const NLMISC::CVector2f &minCorner, const NLMISC::CVector2f &maxCorner, float cellSize /* = 30.f*/)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	release();
	if (!Landscape) return;
	Landscape->addTileCallback(this);
	if (cellSize == 0.f)
	{
		nlwarning("Bad cell size");
		return;
	}
	if (minCorner.x >= maxCorner.x)
	{
		nlwarning("Corners not well ordered");
		return;
	}
	_CellSize = cellSize;
	//
	NLMISC::CVector2f minCornerFinal = minCorner;
	NLMISC::CVector2f maxCornerFinal = maxCorner;
	if (minCornerFinal.x > maxCornerFinal.x) std::swap(minCornerFinal.x, maxCornerFinal.x);
	if (minCornerFinal.y > maxCornerFinal.y) std::swap(minCornerFinal.y, maxCornerFinal.y);
	//
	_GridWidth  = (uint) (floorf(maxCornerFinal.x - minCornerFinal.x) / cellSize);
	_GridHeight = (uint) (floorf(maxCornerFinal.y - minCornerFinal.y) / cellSize);
	_MinCorner = minCornerFinal;
	_Grid.resize(_GridWidth * _GridHeight, 0);

	_Noise.Frequency = 10.1346541f / cellSize;
}

///////////////////
// BUILD PROCESS //
///////////////////

// To quickly know if a tile is inside a zone of micro-life, we subdivide the world into a grid. Each grid cell
// may be overlapped by [0, n] polygons of a micro-life zone. We don't want to store a full list for each cell of
// the grid, because such lists are repeated several times (it would end up wasting too much space). What we just need to keep is
// a set of possible polygon lists that can overlap a grid cell. Afterward, we just need a grid of index into
// the list of possible polygon lists.
// To do that : - we first store complete lists in each grid cells : this is done by several calls to drawPolyInBuildGrid
//              - we search for redundant lists and store the result as index in the final grid. This is done in packBuildGrid


// *********************************************************************************************************************************
bool CMicroLifeManager::CGridOverlapPolyInfo::operator < (const CMicroLifeManager::CGridOverlapPolyInfo &other) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (PrimitiveIndex != other.PrimitiveIndex) return PrimitiveIndex < other.PrimitiveIndex;
	if (Sheet != other.Sheet) return Sheet < other.Sheet;
	if (IsExcludePoly != other.IsExcludePoly) return !other.IsExcludePoly; // want to test exclude polys first
	if (IsFullyCovered != other.IsFullyCovered) return other.IsFullyCovered;
	return Poly < other.Poly;
}

// *********************************************************************************************************************************
bool CMicroLifeManager::CGridOverlapPolyInfo::operator == (const CMicroLifeManager::CGridOverlapPolyInfo &other) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	return PrimitiveIndex == other.PrimitiveIndex &&
		   Sheet == other.Sheet &&
		   IsExcludePoly == other.IsExcludePoly &&
		   IsFullyCovered == other.IsFullyCovered &&
		   Poly == other.Poly;
}

// *********************************************************************************************************************************
bool CMicroLifeManager::CGridOverlapPolyInfoVector::operator < (const CMicroLifeManager::CGridOverlapPolyInfoVector &other) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (V.size() != other.V.size()) return V.size() < other.V.size();
	for(uint k = 0; k < V.size(); ++k)
	{
		if (V[k] != other.V[k]) return V[k] < other.V[k];
	}
	return false;
}

// *********************************************************************************************************************************
bool CMicroLifeManager::CGridOverlapPolyInfoVector::operator == (const CMicroLifeManager::CGridOverlapPolyInfoVector &other) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (V.size() != other.V.size()) return false;
	return std::equal(V.begin(), V.end(), other.V.begin());
}


// ***************************************************************************************************************************
void CMicroLifeManager::drawPolyInBuildGrid(const std::vector<NLLIGO::CPrimVector> &primPoly,
											uint primitiveIndex,
											CMicroLifeManager::TBuildGrid &buildGrid,
											const CFloraSheet *sheet,
											bool isExcludeTri)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	NLMISC::CPolygon poly;
	poly.Vertices.resize(primPoly.size());
	std::copy(primPoly.begin(), primPoly.end(), poly.Vertices.begin());
	std::list<NLMISC::CPolygon> convexPolys;
	if (!poly.toConvexPolygons(convexPolys, NLMISC::CMatrix::Identity))
	{
		nlwarning("Can't convert to convex polys");
		return;
	}
	NLMISC::CPolygon2D poly2D;
	for(std::list<NLMISC::CPolygon>::iterator it = convexPolys.begin(); it != convexPolys.end(); ++it)
	{
		poly2D.Vertices.resize(it->Vertices.size());
		// convert poly in cell units
		for(uint k = 0; k < it->Vertices.size(); ++k)
		{
			poly2D.Vertices[k].x = (it->Vertices[k].x - _MinCorner.x) / _CellSize;
			poly2D.Vertices[k].y = (it->Vertices[k].y - _MinCorner.y) / _CellSize;
			//nlinfo("poly2D.Vertices[k].x = %.1f, poly2D.Vertices[k].y = %.1f", poly2D.Vertices[k].x, poly2D.Vertices[k].y);
		}
		// rasterize poly
		NLMISC::CPolygon2D::TRasterVect outerBorders;
		NLMISC::CPolygon2D::TRasterVect innerBorders;
		sint outerMinY;
		sint innerMinY;
		poly2D.computeOuterBorders(outerBorders, outerMinY);
		poly2D.computeInnerBorders(innerBorders, innerMinY);
		if (outerBorders.empty()) continue; // no contribution for that poly
		CGridOverlapPolyInfo gti;
		gti.IsExcludePoly = isExcludeTri;
		gti.Sheet = sheet;
		gti.Poly = *it;
		gti.PrimitiveIndex = primitiveIndex;
		sint maxY = std::min(outerMinY + (sint)outerBorders.size(), (sint)_GridHeight);
		sint startY = std::max(0, outerMinY);
		for (sint y = startY; y < maxY; ++y)
		{
			sint maxX = std::min(outerBorders[y - startY].second, (sint) (_GridWidth - 1));
			for (sint x = std::max((sint) 0, outerBorders[y - startY].first); x <= maxX; ++x)
			{
				nlassert(x >= 0);
				nlassert(y >= 0);
				nlassert(x < (sint) _GridWidth);
				nlassert(y < (sint) _GridHeight);
				gti.IsFullyCovered = false;
				// see if primitive covers entirely this grid cell
				if (y >= innerMinY && y < innerMinY + (sint) innerBorders.size())
				{
					if (x >= innerBorders[y - innerMinY].first && x <= innerBorders[y - innerMinY].second)
					{
						gti.IsFullyCovered = true;
					}
				}
				buildGrid[x + y * _GridWidth].V.push_back(gti);
			}
		}
	}
}

// predicate to remove a cell that is fully covered by an exclude poly
class CCleanFullyCoveredGridCellPred
{
public:
	uint			  PrimitiveIndex;
public:
	bool operator()(const CMicroLifeManager::CGridOverlapPolyInfo &info) const
	{
		FPU_CHECKER
		return info.PrimitiveIndex == PrimitiveIndex;
	}
};


// ***************************************************************************************************************************
// pack the build grid so that each cell only contains index into a small set of poly list, instead of a full poly list
// (a lot of cells share the same poly list)
void CMicroLifeManager::packBuildGrid(TBuildGrid									  &buildGrid,
									  CMicroLifeManager::TPossibleOverlapingPolyLists &finalPossibleLists
								     )
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	// the already possible lists
	std::set<CGridOverlapPolyInfoVector> possibleLists;
	// for each cell :  - remove all polys of the same zone if there's an exclude tri that covers the whole grid
	//                  - compute the set of possible lists
	for(uint k = 0; k < buildGrid.size(); ++k)
	{
		CGridOverlapPolyInfoVector &gopiv = buildGrid[k];
		bool restart = false;
		do
		{
			restart = false;
			for(std::vector<CGridOverlapPolyInfo>::iterator it = gopiv.V.begin(); it != gopiv.V.end(); ++it)
			{
				if (it->IsExcludePoly && it->IsFullyCovered)
				{
					// remove everything about that primitive
					CCleanFullyCoveredGridCellPred pred;
					pred.PrimitiveIndex = it->PrimitiveIndex;
					gopiv.V.erase(std::remove_if(gopiv.V.begin(), gopiv.V.end(), pred), gopiv.V.end());
					restart = true; // must restart scan from start of list because iterator is now invalid
					break;
				}
			}
		}
		while (restart);
		// insert in possible lists
		possibleLists.insert(gopiv);
	}
	// second pass : flatten the set of possible lists to build an index for each grid cell
	nlassert(possibleLists.size() <= 65536);
	TPossibleOverlapingPolyLists flatSet(possibleLists.size());
	std::copy(possibleLists.begin(), possibleLists.end(), flatSet.begin());
	_Grid.resize(buildGrid.size());
	for(uint k = 0; k < buildGrid.size(); ++k)
	{
		TPossibleOverlapingPolyLists::const_iterator it = std::lower_bound(flatSet.begin(), flatSet.end(), buildGrid[k]);
		nlassert(it != flatSet.end());
		nlassert(*it == buildGrid[k]);
		_Grid[k] = (uint16) (it - flatSet.begin());
	}
	finalPossibleLists.swap(flatSet);
}

// ***************************************************************************************************************************
// read a .primitive file and add its content to a build grid
void CMicroLifeManager::addPrimitiveToBuildGrid(const std::string &fileName, uint &primitiveIndex, CMicroLifeManager::TBuildGrid &buildGrid)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	// read the file into memory and parse to generate 'prims' data tree
	NLLIGO::CPrimitives prims;
	NLMISC::CIFile fileIn;
	std::string path = NLMISC::CPath::lookup(fileName, false, true);
	if (path.empty()) return;
	if (!fileIn.open (path))
	{
		nlwarning("Can't open %s", path.c_str());
		return;
	}
	// Xml stream
	NLMISC::CIXml xmlIn;
	xmlIn.init (fileIn);
	// Read it
	if (!prims.read(xmlIn.getRootNode(), path.c_str(), LigoConfig))
	{
		nlwarning ("Error reading file %s", path.c_str());
		return;
	}
	// get each son zone
	for(uint k = 0; k < prims.RootNode->getNumChildren(); ++k)
	{
		NLLIGO::IPrimitive *child;
		if (!(prims.RootNode->getChild(child, k) && child)) continue;
		//
		std::string className;
		// make sure it is a 'micro_life' primitive
		if (!child->getPropertyByName("class", className))
		{
			nlwarning("Can't get class for child %d of primitive %s", (int) k, path.c_str());
			continue;
		}
		if (NLMISC::nlstricmp(className.c_str(), "micro_life") != 0) continue; // only deals with micro life
		// read flora sheet in the primitive
		std::string formName;
		if (!child->getPropertyByName("form", formName))
		{
			nlwarning("Can't get form name for child %d of primitive %s", (int) k, path.c_str());
			continue;
		}
		const CEntitySheet *sheet = SheetMngr.get(NLMISC::CSheetId(formName));
		if (!sheet)
		{
			nlwarning("Can't get sheet %s", formName.c_str());
			continue;
		}
		const CFloraSheet *floraSheet = dynamic_cast<const CFloraSheet *>(sheet);
		if (!floraSheet)
		{
			nlwarning("Sheet %s has bad type. Flora sheet wanted", formName.c_str());
			continue;
		}
		//
		for(uint m = 0; m < child->getNumChildren(); ++m)
		{
			NLLIGO::IPrimitive *mlZone;
			if (!(child->getChild(mlZone, m) && mlZone)) continue;
			// make sure it is a 'micro_life_zone'
			if (!mlZone->getPropertyByName("class", className))
			{
				nlwarning("Can't get class for child %d of primitive %s", (int) m, path.c_str());
				continue;
			}
			if (NLMISC::nlstricmp("micro_life_zone", className) != 0) continue;

			NLLIGO::CPrimZone *zone = dynamic_cast<NLLIGO::CPrimZone *>(mlZone);
			if (!zone)
			{
				nlwarning("Child %d of primitive %s is not a zone", (int) k, path.c_str());
				continue;
			}
			drawPolyInBuildGrid(zone->VPoints, primitiveIndex, buildGrid, floraSheet, false);
			// remove exlcusion zones
			for(uint l = 0; l < zone->getNumChildren(); ++l)
			{
				NLLIGO::IPrimitive *exclPrim;
				if (zone->getChild(exclPrim, l) && exclPrim)
				{
					if (!exclPrim->getPropertyByName("class", className))
					{
						nlwarning("Can't get class of sub-zone %d for child %d of primitive %s", (int) l, (int) m, path.c_str());
						continue;
					}
				}
				if (NLMISC::nlstricmp("micro_life_exclude_zone", className) == 0)
				{
					NLLIGO::CPrimZone *exclZone = dynamic_cast<NLLIGO::CPrimZone *>(exclPrim);
					if (!zone)
					{
						nlwarning("Child %d of child %d of primitive %s is not a zone", (int) l, (int) m, path.c_str());
						continue;
					}
					// draw exclude polygon
					drawPolyInBuildGrid(exclZone->VPoints, primitiveIndex, buildGrid, floraSheet, true);
				}
			}
			++ primitiveIndex;
		}
	}
}

// ********************************************************************************************
void CMicroLifeManager::build(const std::vector<std::string> &fileNames)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	TBuildGrid buildGrid;
	buildGrid.resize(_Grid.size());
	uint currPrimitiveIndex = 0;
	for(uint l = 0; l < fileNames.size(); ++l)
	{
		addPrimitiveToBuildGrid(fileNames[l], currPrimitiveIndex, buildGrid);
	}
	// build the final grid
	packBuildGrid(buildGrid, _PossibleOverlapPolyLists);
}


// ********************************************************************************************
void CMicroLifeManager::release()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (Landscape)
	{
		if (Landscape->isTileCallback(this))
		{
			Landscape->removeTileCallback(this);
		}
	}
	// release all registered fxs
	for(TTileIDToFX::iterator it = _ActiveFXs.begin(); it != _ActiveFXs.end(); ++it)
	{
		CTimedFXManager::getInstance().remove(it->second);
	}
	_ActiveFXs.clear();
	NLMISC::contReset(_Grid);
	NLMISC::contReset(_PossibleOverlapPolyLists);
	#ifdef NL_DEBUG
		_ActiveTiles.clear();
		_ActiveTilesWithFX.clear();
	#endif
}

// ********************************************************************************************
void CMicroLifeManager::tileAdded(const NL3D::CTileAddedInfo &infos)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (_CellSize == 0.f) return; // not initialized
	#ifdef NL_DEBUG
		_ActiveTiles[infos.TileID] = infos;
	#endif
	// get coords in grid
	sint gridCoordX = (sint) floorf((infos.Center.x - _MinCorner.x) / _CellSize);
	if (gridCoordX < 0 || gridCoordX >= (sint) _GridWidth) return;
	sint gridCoordY = (sint) floorf((infos.Center.y - _MinCorner.y) / _CellSize);
	if (gridCoordY < 0 || gridCoordY >= (sint) _GridHeight) return;
	// get list of primitives over which the center of the tile is
	if( _PossibleOverlapPolyLists.empty() )
		return;	// AJM for when called during zone destructor
	const CGridOverlapPolyInfoVector &iv = _PossibleOverlapPolyLists[_Grid[gridCoordX + gridCoordY * _GridWidth]];
	if (iv.V.empty()) return;
	static std::vector<CTimedFX> fxToSpawn; // static for fast alloc
	fxToSpawn.clear();
	uint k = 0;
	do
	{
		const CFloraSheet *fs = iv.V[k].Sheet;
		if (!fs) continue;
		if (fs->getNumPlantInfos() == 0) continue;
		//
		bool inside = false;
		uint currPrimIndex = iv.V[k].PrimitiveIndex;
		do
		{
			if (iv.V[k].IsExcludePoly)
			{
				// test if center of tile is inside of the primitive
				if (iv.V[k].Poly.contains(NLMISC::CVector2f(infos.Center.x, infos.Center.y)))
				{
					inside = false;
					// jump to next primitive
					for(;;)
					{
						if (k == iv.V.size()) break;
						if (iv.V[k].PrimitiveIndex != currPrimIndex) break;
						++ k;
					}
					break;
				}
			}
			else
			{
				if (iv.V[k].IsFullyCovered)
				{
					// the primitive covers the whole cell so we are inside
					inside = true;
				}
				else
				{
					// test is center of tile is inside
					if (iv.V[k].Poly.contains(NLMISC::CVector2f(infos.Center.x, infos.Center.y)))
					{
						inside = true;
					}
				}
			}
			++k;
			if (k == iv.V.size()) break;
		}
		while(iv.V[k].PrimitiveIndex == currPrimIndex);
		if (!inside) continue; // no inside this primitive, try the next
		// Compute noise at center of the tile, and if it is over the micro-life threshold, then spawn a fx
		// To avoid that each kind of primitive be created at the same time, add an abitrary bias to the position for each primitive type
		float noise = computeUniformNoise(_Noise, infos.Center + (float) (currPrimIndex + 1) * _CellSize * 1.2564f * NLMISC::CVector::K);
		if (noise > fs->MicroLifeThreshold)
		{
			// choose a category of plant/microlife to instanciate by computing some noise around
			float mlCategory = computeUniformNoise(_Noise, infos.Center + (float) (k + 1) * _CellSize * 1.254f * NLMISC::CVector::J + 1.421f * NLMISC::CVector::I);
			uint64 plantWeightedIndex = (uint64) ((double) mlCategory * fs->getPlantInfoTotalWeight());
			const CPlantInfo *pi = fs->getPlantInfoFromWeightedIndex(plantWeightedIndex);
			if (!pi) continue;
			// get pointer on .plant from sheet
			CEntitySheet *es = SheetMngr.get(NLMISC::CSheetId(pi->SheetName));
			if (!es) continue;
			if (es->Type != CEntitySheet::PLANT) continue;
			CPlantSheet *fs = static_cast<CPlantSheet *>(es);
			const CSeasonFXSheet &sfs = fs->getFXSheet(CurrSeason);
			// if orientation is ok for that kind of plant
			float cosMax = cosf(NLMISC::degToRad(sfs.AngleMin));
			float cosMin = cosf(NLMISC::degToRad(sfs.AngleMax));
			if (cosMax < cosMin) std::swap(cosMin, cosMax);
			if (infos.Normal.z < cosMin) continue; // not valid
			if (infos.Normal.z > cosMax) continue; // not valid

			CTimedFX newFX;
#if !FINAL_VERSION
				// for debug display, tells that it was generated dynamically
				newFX.FromIG = false;
#endif
			// spawn a primitive on the quad
			float weight[3];
			// compute weight values by computing some noise values around
			weight[0] = computeUniformNoise(_Noise, infos.Center + (float) (currPrimIndex + 1) * _CellSize * 1.415f * NLMISC::CVector::I);
			weight[1] = computeUniformNoise(_Noise, infos.Center - (float) (currPrimIndex + 1) * _CellSize * 1.568f * NLMISC::CVector::I);
			weight[2] = computeUniformNoise(_Noise, infos.Center - (float) (currPrimIndex + 1) * _CellSize * 1.512f * NLMISC::CVector::J);
			bool tri =  computeUniformNoise(_Noise, infos.Center + (float) (currPrimIndex + 1) * _CellSize * 1.898f * NLMISC::CVector::I) > 0.5f; // choose a tri to use
			// if by extraordinary...
			if (weight[0] == 0.f && weight[1] == 0.f && weight[2] == 0.f)
			{
				weight[0] = 1.f;
			}
			// normalize weights
			float invTotalWeight = 1.f / (weight[0] + weight[1] + weight[2]);
			// choose one of the tri to spawn fx (the 2 tri of the quad are not planar)
			if (tri)
			{
				newFX.SpawnPosition = invTotalWeight * (weight[0] * infos.Corners[0] + weight[1] * infos.Corners[1] + weight[2] * infos.Corners[2]);
			}
			else
			{
				newFX.SpawnPosition = invTotalWeight * (weight[0] * infos.Corners[1] + weight[1] * infos.Corners[2] + weight[2] * infos.Corners[3]);
			}


			// see if the fx must be aligned on water
			if (sfs.AlignOnWater)
			{
				float height;
				bool splashEnabled;
				bool hasWater = ContinentMngr.cur()->WaterMap.getWaterHeight(NLMISC::CVector2f(newFX.SpawnPosition.x, newFX.SpawnPosition.y), height, splashEnabled);
				if (hasWater)
				{
					newFX.SpawnPosition.z = height;
				}
			}
			newFX.SpawnPosition.z += sfs.ZOffset;

			// fit K axis of model with the normal, by doing a rotation around K ^ Normal by an angle alpha
			// whose cos(alpha) is K * Normal = Normal.Z
			// K ^ Normal resolves to [-y x 0]
			// if z is near from 1.f or -1.f then no rotation is performed (because [-y x 0] tends to 0 and can't be normalized)
			CVector rotAxis; // rotation axis to match Z of model with normal
			float   angle = 0.f; // angle of rotation to match Z of model with normal
			// see if want rotation
			if (!sfs.DontRotate)
			{
				if (1.f - infos.Normal.z < 10e-4)
				{
					rotAxis = NLMISC::CVector::I; // any axis in the (x, y) plane will be ok
					angle = 0.f;
				}
				else if (infos.Normal.z + 1.f < 10e-4)
				{
					rotAxis = NLMISC::CVector::I; // any axis in the (x, y) plane will be ok
					angle = (float) NLMISC::Pi;
				}
				else
				{
					rotAxis.set(- infos.Normal.y, infos.Normal.x, 0.f);
					angle = acosf(infos.Normal.z);
				}
			}

			float angleAroundNormal = 0.f;
			// see if want rotation around normal
			if (!sfs.DontRotateAroundLocalZ)
			{
				// Once instance is positionned, rotate it around its normal for more variety
				angleAroundNormal = computeUniformNoise(_Noise, infos.Center + (float) (currPrimIndex + 1) * _CellSize * 1.1213f * NLMISC::CVector::K);
			}
			// build final rot
			if (!sfs.DontRotate && !sfs.DontRotateAroundLocalZ)
			{
				newFX.Rot = NLMISC::CQuat(infos.Normal, angleAroundNormal) * NLMISC::CQuat(rotAxis, angle);
			}
			else if (!sfs.DontRotate)
			{
				newFX.Rot = NLMISC::CQuat(rotAxis, angle);
			}
			else if (!sfs.DontRotateAroundLocalZ)
			{
				newFX.Rot = NLMISC::CQuat(NLMISC::CVector::K, angleAroundNormal);
			}
			else
			{
				// no rotation at all
				newFX.Rot = NLMISC::CQuat::Identity;
			}

			// deal with scale
			if (!sfs.WantScaling)
			{
				newFX.Scale.set(1.f, 1.f, 1.f);
			}
			else
			{
				if (sfs.UniformScale)
				{
					float scaleBlend = computeUniformNoise(_Noise, infos.Center + (float) currPrimIndex * _CellSize * 3.2371f * NLMISC::CVector::J);
					newFX.Scale = scaleBlend * sfs.ScaleMax + (1.f - scaleBlend) * sfs.ScaleMin;
				}
				else
				{
					// compute a different blend factor for each axis
					CVector scaleBlend(computeUniformNoise(_Noise, infos.Center + (float) currPrimIndex * _CellSize * 3.2371f * NLMISC::CVector::J),
									   computeUniformNoise(_Noise, infos.Center + (float) currPrimIndex * _CellSize * 2.9784f * NLMISC::CVector::J),
									   computeUniformNoise(_Noise, infos.Center + (float) currPrimIndex * _CellSize * 1.1782f * NLMISC::CVector::J));
					newFX.Scale.set(scaleBlend.x * sfs.ScaleMax.x + (1.f - scaleBlend.x) * sfs.ScaleMin.x,
						            scaleBlend.y * sfs.ScaleMax.y + (1.f - scaleBlend.y) * sfs.ScaleMin.y,
									scaleBlend.z * sfs.ScaleMax.z + (1.f - scaleBlend.z) * sfs.ScaleMin.z);

				}
			}
			newFX.FXSheet = &sfs;
			fxToSpawn.push_back(newFX);
		}
	}
	while (k != iv.V.size());
	if (!fxToSpawn.empty())
	{
		CTimedFXManager::TFXGroupHandle fxsHandle = CTimedFXManager::getInstance().	add(fxToSpawn, CurrSeason);
		#ifdef NL_DEBUG
				// make sure that tile is not inserted twice
			TTileIDToFX::iterator testIt = _ActiveFXs.find(infos.TileID);
			nlassert(testIt == _ActiveFXs.end());
		#endif
		_ActiveFXs[infos.TileID] = fxsHandle;
		#ifdef NL_DEBUG
			_ActiveTilesWithFX[infos.TileID] = infos;
		#endif
	}
}

// ********************************************************************************************
void CMicroLifeManager::tileRemoved(uint64 id)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
#ifdef NL_DEBUG
		CHashMap<uint64, NL3D::CTileAddedInfo>::iterator tileIt = _ActiveTiles.find(id);
		if (tileIt != _ActiveTiles.end())
		{
			_ActiveTiles.erase(tileIt);
		}
		tileIt = _ActiveTilesWithFX.find(id);
		if (tileIt != _ActiveTilesWithFX.end())
		{
			_ActiveTilesWithFX.erase(tileIt);
		}
#endif
	TTileIDToFX::iterator it = _ActiveFXs.find(id);
	if (it == _ActiveFXs.end()) return;
	// remove FX from the manager
	CTimedFXManager::getInstance().shutDown(it->second);
	_ActiveFXs.erase(it);
}

static const NLMISC::CRGBA DebugCols[] =
{
		NLMISC::CRGBA(255, 32, 32),
		NLMISC::CRGBA(32, 255, 32),
		NLMISC::CRGBA(255, 255, 32),
		NLMISC::CRGBA(32, 255, 255),
		NLMISC::CRGBA(255, 32, 255),
		NLMISC::CRGBA(255, 127, 32),
		NLMISC::CRGBA(255, 255, 255)
};
static const uint NumDebugCols = sizeof(DebugCols) / sizeof(DebugCols[0]);

// ********************************************************************************************
void CMicroLifeManager::dumpMLGrid(const std::string &filename)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (_Grid.empty())
	{
		nlwarning("Grid not built");
		return;
	}
	NLMISC::CBitmap bm;
	bm.resize(_GridWidth, _GridHeight, NLMISC::CBitmap::RGBA);
	NLMISC::CRGBA *pix = (NLMISC::CRGBA *) bm.getPixels(0).getPtr();
	for(uint x = 0; x < _GridWidth; ++x)
	{
		for(uint y = 0; y < _GridHeight; ++y)
		{
			if (_Grid[x + y *_GridWidth] == 0)
			{
				pix[x + y * _GridWidth] = CRGBA(127, 127, 127);
			}
			else
			{
 				pix[x + y * _GridWidth] = DebugCols[_Grid[x + y *_GridWidth] % NumDebugCols];
			}
		}
	}
	NLMISC::COFile f;
	if (!f.open(filename))
	{
		nlwarning("Can't open %s for writing", filename.c_str());
		return;
	}
	bm.writeTGA(f, 24, true);
	f.close();
}

// ********************************************************************************************
void CMicroLifeManager::renderMLZones(const NLMISC::CVector2f &camPos, float maxDist /*=1000.f*/)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_MicroLifeManager)
	if (_Grid.empty()) return;
	// no fast at all version
	Driver->setViewMatrix(Scene->getCam().getMatrix().inverted());
	NL3D::CFrustum fr;
	Scene->getCam().getFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far);
	fr.Perspective = true;
	Driver->setFrustum(fr);
	Driver->setModelMatrix(NLMISC::CMatrix::Identity);
	float userZ = UserEntity ? (float) UserEntity->pos().z : 0.f;
	for(uint k = 0; k < _PossibleOverlapPolyLists.size(); ++k)
	{
		const CGridOverlapPolyInfoVector &currPolyList = _PossibleOverlapPolyLists[k];
		for(uint l = 0; l < currPolyList.V.size(); ++l)
		{
			const std::vector<NLMISC::CVector2f> &verts = currPolyList.V[l].Poly.Vertices;
			NLMISC::CLineColor line;
			line.Color0 = DebugCols[currPolyList.V[l].PrimitiveIndex % NumDebugCols];
			line.Color0.add(line.Color0, NLMISC::CRGBA(127, 127, 127));
			line.Color1 = line.Color0;
			for(uint m = 0; m < verts.size(); ++m)
			{
				line.V0.set(verts[m].x, verts[m].y, userZ);
				line.V1.set(verts[(m + 1) % verts.size()].x, verts[(m + 1) % verts.size()].y, userZ);
				Driver->drawLine(line, GenericMat);
				line.V0.z = userZ + 5.f;
				line.V1.z = userZ + 5.f;
				Driver->drawLine(line, GenericMat);
				line.V0.set(verts[m].x, verts[m].y, userZ);
				line.V1.set(verts[m].x, verts[m].y, userZ + 5.f);
				Driver->drawLine(line, GenericMat);
			}
		}
	}
	for (uint x = 0; x < _GridWidth; ++x)
	{
		for (uint y = 0; y < _GridHeight; ++y)
		{
			const CGridOverlapPolyInfoVector &currPolyList = _PossibleOverlapPolyLists[_Grid[x + _GridWidth * y]];
			if (currPolyList.V.empty()) continue;
			// see if cell not too far
			NLMISC::CVector2f pos(x * _CellSize + _MinCorner.x, y * _CellSize + _MinCorner.y);
			if ((camPos - pos).norm() > maxDist) continue; // too far, don't display
			// display box for each primitive type
			NLMISC::CVector cornerMin(pos.x, pos.y, userZ - 5.f);
			NLMISC::CVector cornerMax(pos.x + _CellSize, pos.y + _CellSize, userZ + 5.f);
			for(uint l = 0; l < currPolyList.V.size(); ++l)
			{
				// add a bias each time to see when several primitives are overlapped
				NLMISC::CVector bias = (float) currPolyList.V[l].PrimitiveIndex * NLMISC::CVector(0.01f, 0.f, 0.1f);
				CRGBA col = DebugCols[currPolyList.V[l].PrimitiveIndex % NumDebugCols];
				if (!currPolyList.V[l].IsFullyCovered)
				{
					drawBox(cornerMin + bias,  cornerMax + bias, col);
				}
				else
				{
					col.R /= 2;
					col.G /= 2;
					col.B /= 2;
					drawBox(cornerMin + bias,  cornerMax + bias, col);
				}
			}
		}
	}
}

#ifdef NL_DEBUG
	// ********************************************************************************************
	void CMicroLifeManager::renderActiveTiles()
	{
		FPU_CHECKER
		/*
		Driver->setViewMatrix(Scene->getCam().getMatrix().inverted());
		NL3D::CFrustum fr;
		Scene->getCam().getFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far);
		fr.Perspective = true;
		Driver->setFrustum(fr);
		Driver->setModelMatrix(NLMISC::CMatrix::Identity);
		NL3D::UDriver::TPolygonMode oldPolyMode = Driver->getPolygonMode();
		Driver->setPolygonMode(NL3D::UDriver::Line);
		NL3D::UMaterial mat = Driver->createMaterial();
		mat->initUnlit();
		mat->setDoubleSided(true);
		mat->setColor(CRGBA::Green);
		for(std::hash_map<uint64, NL3D::CTileAddedInfo>::iterator it = _ActiveTiles.begin(); it != _ActiveTiles.end(); ++it)
		{
			Driver->drawLine(NLMISC::CLine(it->second.Corners[0], it->second.Corners[1]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[1], it->second.Corners[2]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[2], it->second.Corners[3]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[3], it->second.Corners[0]), *mat);
		}
		mat->setColor(CRGBA::Red);
		for(std::hash_map<uint64, NL3D::CTileAddedInfo>::iterator it = _ActiveTilesWithFX.begin(); it != _ActiveTilesWithFX.end(); ++it)
		{
			Driver->drawLine(NLMISC::CLine(it->second.Corners[0], it->second.Corners[1]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[1], it->second.Corners[2]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[2], it->second.Corners[3]), *mat);
			Driver->drawLine(NLMISC::CLine(it->second.Corners[3], it->second.Corners[0]), *mat);
		}
		Driver->deleteMaterial(mat);
		Driver->setPolygonMode(oldPolyMode);
		*/
	}
#endif

////////////////////
// DEBUG COMMANDS //
////////////////////

#ifdef NL_DEBUG
	// display micro-life active tiles
	NLMISC_COMMAND(showMLActiveTiles,"display micro-life active tiles", "<0 = off, 1 = on>")
	{
		if (args.size() != 1) return false;
		fromString(args[0], DisplayMicroLifeActiveTiles);
		return true;
	}
#endif

#if !FINAL_VERSION
	#include "continent_manager.h"

	// ******************************************************************************************************************
	// display micro-life zone on screen
	NLMISC_COMMAND(showMLZones,"display micro-life zones", "<0 = off, 1 = on>")
	{
		if (args.size() != 1) return false;
		fromString(args[0], DisplayMicroLifeZones);
		return true;
	}


	// ******************************************************************************************************************
	// dump micro-life zone in a tga file
	NLMISC_COMMAND(dumpMLZones,"display micro-life zones", "<filename>")
	{
		if (args.size() != 1) return false;
		CMicroLifeManager::getInstance().dumpMLGrid(args[0]);
		return true;
	}


	// ******************************************************************************************************************
	// reload micro-life zones
	NLMISC_COMMAND(reloadMLZones, "reload micro-life zones", "")
	{
		if (!args.empty()) return false;
		ClientSheetsStrings.memoryUncompress();
		// reload .flora
		std::vector<std::string> exts;
		exts.push_back("flora");
		NLMISC::IProgressCallback progress;
		SheetMngr.loadAllSheet(progress, true, false, true, true, &exts);
		// reload .plant 5but keep at their current adress)
		CSheetManager sheetManager;
		exts[0] = "plant";
		sheetManager.loadAllSheet(progress, true, false, false, true, &exts);
		//
		const CSheetManager::TEntitySheetMap &sm = SheetMngr.getSheets();
		for(CSheetManager::TEntitySheetMap::const_iterator it = sm.begin(); it != sm.end(); ++it)
		{
			if (it->second.EntitySheet && it->second.EntitySheet->Type == CEntitySheet::PLANT)
			{
				// find matching sheet in new sheetManager
				const CEntitySheet *other = sheetManager.get(it->first);
				if (other)
				{
					// replace data in place
					*(CPlantSheet *) it->second.EntitySheet = *(const CPlantSheet *) other;
				}
			}
		}

		//
		ClientSheetsStrings.memoryCompress();
		// reload prims
		ContinentMngr.cur()->loadMicroLife();
		if (Landscape) Landscape->invalidateAllTiles();
		return true;
	}
#endif











































