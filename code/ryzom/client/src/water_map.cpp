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
#include "water_map.h"
#include "continent_manager.h"
#include "user_entity.h"
#include "misc.h"
#include "global.h"
#include "client_cfg.h"
//
#include "r2/editor.h"
#include "r2/island_collision.h"
//
#include "nel/3d/u_scene.h"
#include "nel/3d/u_driver.h"
//
#include "nel/misc/path.h"
#include "nel/misc/file.h"


extern NL3D::UScene *Scene;
extern NL3D::UDriver *Driver;
extern CContinentManager ContinentMngr;

bool DisplayWaterMap = false;


using namespace NLMISC;

H_AUTO_DECL(RZ_WaterMap)

// *********************************************************************************************************
CWaterMap::CWaterMap()
{
	H_AUTO_USE(RZ_WaterMap)
	// Construct
	_CellSize = 0.f;
	_Width = 0;
	_Height = 0;

}

// *********************************************************************************************************
void CWaterMap::init(const NLMISC::CVector2f &minCorner, const NLMISC::CVector2f &maxCorner, float cellSize /*=20.f*/)
{
	H_AUTO_USE(RZ_WaterMap)
	release();
	if (cellSize <= 0.f)
	{
		nlwarning("Invalid cell size");
		return;
	}
	_MinCorner = minCorner;
	_MaxCorner = maxCorner;
	if (_MinCorner.y > _MaxCorner.y) std::swap(_MinCorner.y, _MaxCorner.y);
	if (_MinCorner.y > _MaxCorner.y) std::swap(_MinCorner.y, _MaxCorner.y);
	_CellSize = cellSize;
	_Width = (uint) ceilf((_MaxCorner.x - _MinCorner.x) / _CellSize);
	_Height = (uint) ceilf((_MaxCorner.y - _MinCorner.y) / _CellSize);
	_Grid.resize(_Width * _Height, 0);
	// init index 0 as an empty list of water surface
	_WaterInfoVect.push_back(CWaterInfo());
	_WaterInfoVect.back().Height = 0.f;
	_WaterInfoVect.back().SplashEnabled = false;
	_WaterInfoIndexVectVect.push_back(TWaterInfoIndexVect(1, 0));
	Scene->setWaterCallback(this);
}

// *********************************************************************************************************
void CWaterMap::release()
{
	H_AUTO_USE(RZ_WaterMap)
	NLMISC::contReset(_Grid);
	NLMISC::contReset(_WaterInfoVect);
	NLMISC::contReset(_WaterInfoIndexVectVect);
	_CellSize = 0.f;
	_Width = 0;
	_Height = 0;
	Scene->setWaterCallback(NULL);
}

// *********************************************************************************************************
bool CWaterMap::getWaterHeight(const NLMISC::CVector2f &pos, float &height, bool &splashEnabled)
{
	H_AUTO_USE(RZ_WaterMap)
	if (_Grid.empty()) return false;
	float x = (pos.x - _MinCorner.x) / _CellSize;
	float y = (pos.y - _MinCorner.y) / _CellSize;
	sint ix = (sint) x;
	sint iy = (sint) y;
	if (ix < 0 || ix >= _Width) return false;
	if (iy < 0 || iy >= _Height) return false;
	const TWaterInfoIndexVect &wiiv = _WaterInfoIndexVectVect[_Grid[ix + iy * _Width]];
	if (wiiv.empty()) return false;
	// search if there's an intersection with one of the water surfaces
	for(uint k = 0; k < wiiv.size(); ++k)
	{
		const CWaterInfo &wi = _WaterInfoVect[wiiv[k]];
		if (wi.Shape.Vertices.empty()) continue;
		// test intersection with each surface, and pick the first match
		if (wi.Shape.contains(NLMISC::CVector2f(x, y)))
		{
			height = wi.Height;
			splashEnabled = wi.SplashEnabled;
			return true;
		}
	}
	return false;
}

// *********************************************************************************************************
void CWaterMap::waterSurfaceAdded(const NLMISC::CPolygon2D &shape, const NLMISC::CMatrix &worldMatrix, bool splashEnabled, bool usesSceneWaterEnvMap)
{
	if (ClientCfg.R2EDEnabled)
	{
		R2::getEditor().getIslandCollision().waterSurfaceAdded(shape, worldMatrix);
	}
	H_AUTO_USE(RZ_WaterMap)
	if (usesSceneWaterEnvMap) ++WaterEnvMapRefCount;
	if (_Grid.empty()) return;
	if (_CellSize == 0.f) return;
	float height = worldMatrix.getPos().z;
	// transform the water shape in grid coordinates
	CWaterInfo wi;
	uint numVerts = (uint)shape.Vertices.size();
	wi.Shape.Vertices.resize(numVerts);
	wi.SplashEnabled = splashEnabled;
	NLMISC::CMatrix toGridMatrix;
	toGridMatrix.scale(NLMISC::CVector(1.f / _CellSize, 1.f / _CellSize, 1.f));
	toGridMatrix.translate(- _MinCorner);
	toGridMatrix = toGridMatrix * worldMatrix;
	for(uint k = 0; k < numVerts; ++k)
	{
		wi.Shape.Vertices[k] = toGridMatrix *  shape.Vertices[k];
	}
	// see if water surface already added
	for(uint k = 0; k < _WaterInfoVect.size(); ++k)
	{
		if (_WaterInfoVect[k].Height == height && _WaterInfoVect[k].Shape.Vertices.size() == shape.Vertices.size())
		{
			if (std::equal(wi.Shape.Vertices.begin(), wi.Shape.Vertices.end(), _WaterInfoVect[k].Shape.Vertices.begin()))
			{
				// already inserted -> do nothing
				return;
			}
		}
	}
	// insert new CWaterInfo in the list
	wi.Height = height;
	_WaterInfoVect.push_back(wi);
	// build rasters
	NLMISC::CPolygon2D::TRasterVect rasters;
	sint minY;
	//wi.Shape.computeBordersLarge(rasters, minY);
	wi.Shape.computeOuterBorders(rasters, minY);
	if (!rasters.empty())
	{
		sint numRasters = (sint) rasters.size();
		for(sint y = 0; y < numRasters; ++y)
		{
			if ((y + minY) < 0 || (y + minY) >= _Height) continue; // outisde of map
			sint lastX = rasters[y].second + 1;
			for (sint x = rasters[y].first; x < lastX; ++x)
			{
				if (x < 0 || x >= _Width) continue; // outside of map
				const TWaterInfoIndexVect *currWI = &_WaterInfoIndexVectVect[_Grid[x + (y + minY) * _Width]];
				// see if there's already a list of water surfaces that match the list for that grid cell
				// such a list should contains the surfaces already found for that grid cell + the new one
				sint goodList = -1;
				for(uint k = 0; k < _WaterInfoIndexVectVect.size(); ++k)
				{
					if (_WaterInfoIndexVectVect[k].size() == currWI->size() + 1)
					{
						if (std::equal(_WaterInfoIndexVectVect[k].begin(), _WaterInfoIndexVectVect[k].begin() + currWI->size(), currWI->begin()) &&
							_WaterInfoIndexVectVect[k].back() == _WaterInfoVect.size() - 1)
						{
							// this list match what we want
							goodList = k;
							break;
						}
					}
				}
				if (goodList == -1)
				{
					// must create a new list
					_WaterInfoIndexVectVect.push_back(TWaterInfoIndexVect());
					// vector has grown up, so currWI pointer becomes invalid -> rebuild it
					currWI = &_WaterInfoIndexVectVect[_Grid[x + (y + minY) * _Width]];
					_WaterInfoIndexVectVect.back().resize(currWI->size() + 1);
					std::copy(currWI->begin(), currWI->end(), _WaterInfoIndexVectVect.back().begin());
					_WaterInfoIndexVectVect.back().back() = (uint16)_WaterInfoVect.size() - 1;
					goodList = (sint)_WaterInfoIndexVectVect.size() - 1;
				}
				_Grid[x + (y + minY) * _Width] = goodList; // reassign new list
			}
		}
	}
}

// *********************************************************************************************************
void CWaterMap::waterSurfaceRemoved(bool usesSceneWaterEnvMap)
{
	H_AUTO_USE(RZ_WaterMap)
	if (usesSceneWaterEnvMap) --WaterEnvMapRefCount;
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


// *********************************************************************************************************
void CWaterMap::dump(const std::string &filename)
{
	H_AUTO_USE(RZ_WaterMap)
	if (_Grid.empty())
	{
		nlwarning("Grid not built");
		return;
	}
	NLMISC::CBitmap bm;
	bm.resize(_Width, _Height, NLMISC::CBitmap::RGBA);
	NLMISC::CRGBA *pix = (NLMISC::CRGBA *) bm.getPixels(0).getPtr();
	for(uint x = 0; x < _Width; ++x)
	{
		for(uint y = 0; y < _Height; ++y)
		{
			if (_Grid[x + y *_Width] == 0)
			{
				pix[x + y * _Width] = CRGBA(127, 127, 127);
			}
			else
			{
				pix[x + y * _Width] = DebugCols[_Grid[x + y *_Width] % NumDebugCols];
			}
		}
	}
	// merge with world map if present
	if (ContinentMngr.cur())
	{
		if (!ContinentMngr.cur()->WorldMap.empty())
		{
			std::string path = CPath::lookup(ContinentMngr.cur()->WorldMap, false);
			if (!path.empty())
			{
				CIFile stream;
				if (stream.open(path))
				{
					CBitmap worldMap;
					if (worldMap.load(stream))
					{
						worldMap.flipV();
						worldMap.convertToType(CBitmap::RGBA);
						worldMap.resample(bm.getWidth(), bm.getHeight());
						bm.blend(bm, worldMap, 127, true);
					}
				}
			}
		}
	}
	//
	drawDisc(bm,  ((float) UserEntity->pos().x - _MinCorner.x) / _CellSize, ((float) UserEntity->pos().y - _MinCorner.y) / _CellSize, 2.f, CRGBA::Magenta);
	//
	NLMISC::COFile f;
	if (!f.open(filename))
	{
		nlwarning("Can't open %s for writing", filename.c_str());
		return;
	}
	bm.writeTGA(f, 24, true);
	f.close();
}

// ******************************************************************************************************************
void CWaterMap::render(const NLMISC::CVector2f &camPos, float maxDist /*=100.f*/)
{
	H_AUTO_USE(RZ_WaterMap)
	if (_Grid.empty()) return;
	Driver->setViewMatrix(Scene->getCam().getMatrix().inverted());
	NL3D::CFrustum fr;
	Scene->getCam().getFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far);
	fr.Perspective = true;
	Driver->setFrustum(fr);
	Driver->setModelMatrix(NLMISC::CMatrix::Identity);
	float userZ = UserEntity ? (float) UserEntity->pos().z : 0.f;
	for (uint x = 0; x < _Width; ++x)
	{
		for (uint y = 0; y < _Height; ++y)
		{
			uint16 index = _Grid[x + _Width * y];
			if (index == 0) continue;
			const TWaterInfoIndexVect &wii = _WaterInfoIndexVectVect[index];
			// see if cell not too far
			NLMISC::CVector2f pos(x * _CellSize + _MinCorner.x, y * _CellSize + _MinCorner.y);
			if ((camPos - pos).norm() > maxDist) continue; // too far, don't display
			// display box for each primitive type
			NLMISC::CVector cornerMin(pos.x, pos.y, userZ - 5.f);
			NLMISC::CVector cornerMax(pos.x + _CellSize, pos.y + _CellSize, userZ + 5.f);
			for(uint l = 0; l < wii.size(); ++l)
			{
				// add a bias each time to see when several primitives are overlapped
				NLMISC::CVector bias = (float) wii[l] * NLMISC::CVector(0.01f, 0.f, 0.1f);
				drawBox(cornerMin + bias,  cornerMax + bias, DebugCols[wii[l] % NumDebugCols]);
			}
		}
	}
}

#if !FINAL_VERSION

// ******************************************************************************************************************
// dump water map in a tga file
NLMISC_COMMAND(dumpWaterMap, "dump water map", "<filename>")
{
	if (args.size() != 1) return false;
	if (!ContinentMngr.cur()) return false;
	ContinentMngr.cur()->WaterMap.dump(args[0]);
	return true;
}

// ******************************************************************************************************************
// display the water map
NLMISC_COMMAND(displayWaterMap, "dump water map", "<0 = on / 1 = off>")
{
	if (args.size() != 1) return false;
	fromString(args[0], DisplayWaterMap);
	return true;
}

#endif
