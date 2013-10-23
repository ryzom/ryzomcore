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

#include "../user_entity.h"
#include "../water_map.h"
#include "../interface_v3/interface_manager.h"
//
#include "tool.h"
#include "editor.h"
#include "island_collision.h"
#include "verbose_clock.h"
#include "../continent_manager.h"
#include "../global.h"
//
#include "nel/misc/file.h"
#include "nel/misc/polygon.h"
#include "nel/misc/progress_callback.h"
//
#include "nel/3d/texture_file.h"

extern CContinentManager ContinentMngr;

using namespace NL3D;
using namespace NLMISC;

namespace R2
{

const char *CIslandCollision::_AccessibilityTextureId = "*accessibility_texture*";

const uint CIslandCollision::MapSizeDivisor = 4;


// gives accessible places for content creation as a texture
class CAccessibilityTexture : public NL3D::ITexture
{
public:
	NLMISC_DECLARE_CLASS(CAccessibilityTexture);
	//
	CAccessibilityTexture();
	~CAccessibilityTexture();
	CAccessibilityTexture(CIslandCollision *islandCol);
	//
	virtual void doGenerate(bool /* async */ = false)
	{
		const NLMISC::CArray2D<sint16> &hm = _IslandCol->getHeightMap();
		if (hm.empty())
		{
			resize(1, 1);
		}
		else
		{
			uint width  = hm.getWidth() / CIslandCollision::MapSizeDivisor;
			uint height = hm.getHeight() / CIslandCollision::MapSizeDivisor;
			width = std::max(1u, width);
			height = std::max(1u, height);
			resize(NLMISC::raiseToNextPowerOf2(width), NLMISC::raiseToNextPowerOf2(height));
			CRGBA *dest = (CRGBA *) &getPixels(0)[0];
			for (uint y = 0; y < height; ++y)
			{
				CRGBA *curr = dest;
				for (uint x = 0; x < width; ++x)
				{
					uint8 level = 0;
					//bool water = false;
					for (uint sy = 0; sy < CIslandCollision::MapSizeDivisor; ++sy)
					{
						for (uint sx = 0; sx < CIslandCollision::MapSizeDivisor; ++sx)
						{
							uint srcX = (CIslandCollision::MapSizeDivisor * x) + sx;
							uint srcY = (CIslandCollision::MapSizeDivisor * y) + sy;
							if (srcX < hm.getWidth() &&
								srcY < hm.getHeight())
							{
								if (hm(srcX, srcY) >= 0x7ffe)
								{
									//if (hm(srcX, srcY) == 0x7ffe) water = true;
									level = 255;
								}
							}
						}
					}
					//*curr++ = CRGBA(water ? 0 : 255, water ? 0 : 255, 255, level);
					*curr++ = CRGBA(255, 255, 255, level);
				}
				dest += getWidth();
			}
		}
	}
private:
	CIslandCollision *_IslandCol;
};


// *********************************************************************************************************
CAccessibilityTexture::CAccessibilityTexture() : _IslandCol(NULL)
{
	nlassert(0);
}

// *********************************************************************************************************
CAccessibilityTexture::CAccessibilityTexture(CIslandCollision *islandCol) : _IslandCol(islandCol)
{
	// nlwarning new accessiblity texture created
}

// *********************************************************************************************************
CAccessibilityTexture::~CAccessibilityTexture()
{
	nlwarning("Accessibility texture removed");
}

// *********************************************************************************************************
CIslandCollision::CIslandCollision()
{
	_LastPackedIslandTestPos.set(0.f, 0.f);
	_PackedIslandLoadingFailure = false;
	_EntryPointsLoadingFailure = false;
	_WorldToAccessibilityTexMat.scale(0.f);
	_AccessibilityTexture = NULL;
}

// *********************************************************************************************************
void CIslandCollision::releaseAccessibilityTexture()
{
	//H_AUTO(R2_CIslandCollision_releaseAccessibilityTexture)
	delete _AccessibilityTexture;
	_AccessibilityTexture = NULL;
	CViewRenderer &vr = *CViewRenderer::getInstance();
	vr.setExternalTexture(_AccessibilityTextureId, NULL, 0, 0, 0, 0);
}

// *********************************************************************************************************
void CIslandCollision::release()
{
	//H_AUTO(R2_CIslandCollision_release)
	_PackedIsland = NULL;
	_LastPackedIslandTestPos.set(0.f, 0.f);
	_PackedIslandLoadingFailure = false;
}


// *********************************************************************************************************
CIslandCollision::~CIslandCollision()
{
	release();
}

// *********************************************************************************************************
CRefCountedPackedWorld *CIslandCollision::getPackedIsland()
{
	//H_AUTO(R2_CIslandCollision_getPackedIsland)
	if (_PackedIsland) return _PackedIsland;
	if (_PackedIslandLoadingFailure) return NULL;
	// get the island under the player
	updateCurrPackedIsland();
	return _PackedIsland;
}


// TMP TMP
inline sint clamped(sint value, sint size)
{
	clamp(value, 0, size - 1);
	return value;
}
// *********************************************************************************************************
R2::CScenarioEntryPoints::CCompleteIsland *CIslandCollision::getCurrIslandDesc()
{
	//H_AUTO(R2_CIslandCollision_getCurrIslandDesc)
	if (!getPackedIsland()) return NULL;
	// TMP TMP
	static volatile bool dumpPACSCorrepondance = false;
	if (dumpPACSCorrepondance)
	{
		CBitmap out;
		CBitmap out2;
		sint width = _IslandDesc.XMax - _IslandDesc.XMin;
		sint height = _IslandDesc.YMax - _IslandDesc.YMin;
		const uint zoom = 4;
		out.resize(width * zoom, height * zoom);
		out2.resize(width * zoom, height * zoom);
		CRGBA *dest = (CRGBA *) &out.getPixels(0)[0];
		CRGBA *dest2 = (CRGBA *) &out2.getPixels(0)[0];
		NLPACS::UGlobalPosition pacsPos;
		for (uint y = 0; y < height * zoom; ++y)
		{
			for (uint x = 0; x < width * zoom; ++x)
			{
				sint ix = (sint) (x / zoom);
				sint iy = (sint) (y / zoom);
				// get value from height map
				dest->R = _HeightMap(ix, iy) >= 0x7ffe ? 0 : 255;
				// get value from pacs
				CTool::TRayIntersectionType interType = CTool::getPacsType(CVector((float) x / zoom + _IslandDesc.XMin, (float) y / zoom + _IslandDesc.YMin, 0.f), 100000.f, pacsPos);
				dest->G = interType == CTool::ValidPacsPos ? 255 : 0;
				dest->B = 0;
				dest->A = 255;
				*dest2 = *dest;
				bool accessible = _HeightMap(clamped((sint) ((x + 0.5f) / zoom), width), iy) < 0x7ffe &&
								  _HeightMap(clamped((sint) ((x - 0.5f) / zoom), width), iy) < 0x7ffe &&
								  _HeightMap(ix, clamped((sint) ((y + 0.5f) / zoom), height)) < 0x7ffe &&
								  _HeightMap(ix, clamped((sint) ((y - 0.5f) / zoom), height)) < 0x7ffe;
				dest2->R = accessible ? 255 : 0;
				++ dest;
				++ dest2;
			}
		}
		try
		{
			{
				COFile f("pacs_test.tga");
				out.writeTGA(f, 0, true);
			}
			{
				COFile f("pacs_test2.tga");
				out2.writeTGA(f, 0, true);
			}
		}
		catch(...)
		{
		}
	}
	return &_IslandDesc;
}

// *********************************************************************************************************
const NLMISC::CArray2D<sint16> &CIslandCollision::getHeightMap()
{
	//H_AUTO(R2_CIslandCollision_getHeightMap)
	// force an island reloadif needed
	getPackedIsland();
	return _HeightMap;
}

// *********************************************************************************************************
bool CIslandCollision::findEmptyPlace(const CVector2f &startPos, NLMISC::CVector &dest)
{
	//H_AUTO(R2_CIslandCollision_findEmptyPlace)
	//static volatile bool dumpSearch = false;
	//CBitmap dump;
	getPackedIsland(); // force a rebuild of packed island
	if (_EntryPointsLoadingFailure) return false;
	enum TDir { Top = 0, Right, Bottom, Left };
	sint32 currLength = 1;
	sint32 leftToGo = currLength;
	CVector2f currPos = startPos;
	TDir dir = Top;
	/*if (dumpSearch)
	{
		dump.resize(_HeightMap.getWidth(), _HeightMap.getHeight());
		CRGBA *dest = (CRGBA *) &dump.getPixels()[0];
		for (uint y = 0; y < _HeightMap.getHeight(); ++y)
		{
			for (uint x = 0; x < _HeightMap.getWidth(); ++x)
			{
				if (_HeightMap(x, y) <= 0x7ffe) *dest++ = CRGBA::Magenta;
				else (dest++)->set((uint8) (127 + _HeightMap(x, y)),
								   (uint8) (127 + _HeightMap(x, y)),
								   (uint8) (127 + _HeightMap(x, y)), 255);
			}
		}
	}*/
	// if start pos is not in the current island, use the player pos instead
	if (!(currPos.x >= _IslandDesc.XMin && currPos.y >= _IslandDesc.YMin &&
		  currPos.x <= _IslandDesc.XMax && currPos.y <= _IslandDesc.YMax))
	{
		currPos = CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y);
	}
	// TMP TMP, avoid to reuse the same place twice
	/*static NLMISC::CArray2D<uint8> used;
	static volatile bool doTest = false;
	if (used.getWidth() != _HeightMap.getWidth() ||
		used.getHeight() != _HeightMap.getHeight())
	{
		used.init(_HeightMap.getWidth(), _HeightMap.getHeight(), 0);
	}*/
	for (;;)
	{
		/*if (dumpSearch)
		{
			CRGBA *dest = (CRGBA *) &dump.getPixels()[0];
			dest += x + y * dump.getWidth();
			*dest = CRGBA::Green;
		}*/
		switch(dir)
		{
			case Top:
				currPos.y += 1.f;
				-- leftToGo;
				if (leftToGo == 0)
				{
					dir = Right;
					leftToGo = currLength;
				}
			break;
			case Right:
				currPos.x += 1.f;
				-- leftToGo;
				if (leftToGo == 0)
				{
					dir = Bottom;
					++ currLength;
					leftToGo = currLength;
				}
			break;
			case Bottom:
				currPos.y -= 1.f;
				-- leftToGo;
				if (leftToGo == 0)
				{
					dir = Left;
					leftToGo = currLength;
				}
			break;
			case Left:
				currPos.x -= 1.f;
				-- leftToGo;
				if (leftToGo == 0)
				{
					dir = Top;
					++ currLength;
					leftToGo = currLength;
				}
			break;
		}
		// if out of map then abort
		if (!(currPos.x >= _IslandDesc.XMin && currPos.y >= _IslandDesc.YMin &&
		      currPos.x <= _IslandDesc.XMax && currPos.y <= _IslandDesc.YMax))
			break;

		if (isValidPos(currPos))
		{
			//if (!used(x, y)) // TMP TMP
			{
				CTool::TRayIntersectionType interType = CTool::computeWorldMapIntersection(currPos.x, currPos.y, dest);
				if (interType == CTool::ValidPacsPos)
				{
					/*if (doTest)
					{
						used(x, y) = 1;
					}*/
					/*if (dumpSearch)
					{
						COFile f("search.tga");
						dump.writeTGA(f);
					}*/
					return true;
				}
			}
		}
	}
	/*if (dumpSearch)
	{
		COFile f("search.tga");
		dump.writeTGA(f);
	}*/
	return false;
}

// *********************************************************************************************************
void CIslandCollision::updateCurrPackedIsland()
{
	//H_AUTO(R2_CIslandCollision_updateCurrPackedIsland)
	CVectorD playerPos = UserEntity->pos();
	CVector2f newPos((float) playerPos.x, (float) playerPos.y);
	if (fabsf(newPos.x - _LastPackedIslandTestPos.x) >= 80.f ||
		fabsf(newPos.y - _LastPackedIslandTestPos.y) >= 80.f ||
		(!_PackedIsland && !_PackedIslandLoadingFailure)
	   )
	{
		_LastPackedIslandTestPos = newPos;
		CScenarioEntryPoints &sep = CScenarioEntryPoints::getInstance();
		loadEntryPoints();
		CVectorD playerPos = UserEntity->pos();
		CScenarioEntryPoints::CCompleteIsland *island = sep.getCompleteIslandFromCoords(CVector2f((float) playerPos.x, (float) playerPos.y));
		if (!island)
		{
			_PackedIslandLoadingFailure = true;
			_PackedIsland = NULL;
			return;
		}
		if (_PackedIsland && _PackedIslandName == island->Island) return; // same than before, no-op
		reloadPackedIsland(*island); // first island or entered a new island -> reload
	}
}

// *********************************************************************************************************
CPackedWorld *CIslandCollision::reloadPackedIsland(const CScenarioEntryPoints::CCompleteIsland &islandDesc)
{
	//H_AUTO(R2_CIslandCollision_reloadPackedIsland)
	_PackedIsland = NULL;
	_PackedIslandLoadingFailure = false;
	_WorldToAccessibilityTexMat.scale(0.f);
	_HeightMap.clear();
	{
		CVerboseClock vbc("Reload packed island");
		try
		{
			CIFile f(CPath::lookup(islandDesc.Island + ".packed_island"));
			CRefCountedPackedWorld::TSmartPtr newIsland(new CRefCountedPackedWorld);
			f.serial(*newIsland);
			_PackedIsland = newIsland;
			_PackedIslandName = islandDesc.Island;
			_IslandDesc = islandDesc;
		}
		catch (const Exception &)
		{
			_PackedIslandLoadingFailure = true;
		}
		if (!_PackedIslandLoadingFailure)
		{
			// additionnaly, try to load the heightmap
			try
			{
				CIFile f(CPath::lookup(islandDesc.Island + ".island_hm"));
				f.serialCheck(NELID("MHSI"));
				f.serial(_HeightMap);
			}
			catch(const Exception &e)
			{
				nlwarning(e.what());
				_HeightMap.clear();
			}
		}
		if (!_HeightMap.empty())
		{
			{
				CVerboseClock vbc("Force water surfaces loading");
				NLMISC::IProgressCallback dummyProgress;
				// force to load all water surfaces & add them
				ContinentMngr.forceUpdateStreamable(UserEntity->pos(), dummyProgress);
			}
			mergeWaterSurfaces();
		}
		releaseAccessibilityTexture();
		_AccessibilityTexture = new NL3D::CTextureUser(new CAccessibilityTexture(this));
		_AccessibilityTexture->setFilterMode(UTexture::Nearest, UTexture::NearestMipMapOff);
		_AccessibilityTexture->setWrapS(UTexture::Clamp);
		_AccessibilityTexture->setWrapT(UTexture::Clamp);
		// make this texture accessible in the ui view renderer from a simple filename
		// this allow to display it in the world map

		uint width = std::max(_HeightMap.getWidth() / MapSizeDivisor, 1u);
		uint height = std::max(_HeightMap.getHeight() / MapSizeDivisor, 1u);
		uint realWidth = NLMISC::raiseToNextPowerOf2(width);
		uint realHeight = NLMISC::raiseToNextPowerOf2(height);
		CViewRenderer &vr = *CViewRenderer::getInstance();
		// create / update texture
		vr.setExternalTexture(_AccessibilityTextureId, _AccessibilityTexture, realWidth, realHeight, width, height);
		if (vr.getTextureIdFromName(_AccessibilityTextureId) == -1)
		{
			vr.createTexture(_AccessibilityTextureId, 0, 0, width, height);
			nlwarning("Creating accessibility texture with id %d ", (int) vr.getTextureIdFromName(_AccessibilityTextureId));
		}
		else
		{
			vr.updateTexturePos(_AccessibilityTextureId, 0, 0, width, height);
		}
		// scale resumes to dividing by real coords, because heightmap resolution is hardcoded to 1 meter
		CVector scale (1.f / std::max(_HeightMap.getWidth(), 1u),
					   1.f / std::max(_HeightMap.getHeight(), 1u),
					   0.f);
		_WorldToAccessibilityTexMat.setScale(scale);
		_WorldToAccessibilityTexMat.setPos(CVector((float) - islandDesc.XMin * scale.x, (float) - islandDesc.YMin * scale.y, 0.f));
		// cropped version of matrix
		scale.x *= (float) width / realWidth;
		scale.y *= (float) height / realHeight;
		_WorldToAccessibilityTexMatCropped.setScale(scale);
		_WorldToAccessibilityTexMatCropped.setPos(CVector((float) - islandDesc.XMin * scale.x, (float) - islandDesc.YMin * scale.y, 0.f));
	}
	return _PackedIsland;
}

// *********************************************************************************************************
void CIslandCollision::mergeWaterSurfaces()
{
	//H_AUTO(R2_CIslandCollision_mergeWaterSurfaces)
	CVerboseClock vbc("Merge water surfaces");
	nlassert(_PackedIsland);
	std::vector<CTriangle> tris;
	std::vector<CTriangle> clippedTris;
	NLMISC::CPolygon2D worldPoly;
	NLMISC::CPolygon clippedPoly;
	NLMISC::CPolygon clippedPoly2D;
	for (std::list<CWaterSurf>::iterator it = _WaterSurfs.begin(); it != _WaterSurfs.end(); ++it)
	{
		CVerboseClock vbc("Merge single water surface");
		// Select a list of candidate triangles from the packed landscape
		// These must be inside the water surface 2D projected (convex) shape
		worldPoly = it->Shape;
		for (uint k = 0; k < worldPoly.Vertices.size(); ++k)
		{
			worldPoly.Vertices[k] = it->WorldMatrix * worldPoly.Vertices[k];
		}
		_PackedIsland->select(worldPoly, tris);
		clippedTris.clear();
		NLMISC::CPlane waterPlane;
		waterPlane.make(CVector::K, it->WorldMatrix.getPos());
		// now clip by water surface to get tris that are below the water
		for (uint k = 0; k < tris.size(); ++k)
		{
			clippedPoly.Vertices.resize(3);
			clippedPoly.Vertices[0] = tris[k].V0;
			clippedPoly.Vertices[1] = tris[k].V1;
			clippedPoly.Vertices[2] = tris[k].V2;
			clippedPoly.clip(&waterPlane, 1);
			{
				// this part is under water, thus inaccessible
				// add contribution to heightmap
				clippedPoly2D.Vertices.resize(clippedPoly.Vertices.size());
				std::copy(clippedPoly.Vertices.begin(), clippedPoly.Vertices.end(), clippedPoly2D.Vertices.begin());
				addInaccessibleRegion(clippedPoly2D);
			}
		}
	}
}

// *********************************************************************************************************
void CIslandCollision::addInaccessibleRegion(const NLMISC::CPolygon2D &poly)
{
	//H_AUTO(R2_CIslandCollision_addInaccessibleRegion)
	static NLMISC::CPolygon2D localPoly;
	localPoly = poly;
	for (uint k = 0; k < localPoly.Vertices.size(); ++k)
	{
		localPoly.Vertices[k].x -= (float) _IslandDesc.XMin;
		localPoly.Vertices[k].y -= (float) _IslandDesc.YMin;
	}
	static NLMISC::CPolygon2D::TRasterVect borders;
	borders.clear();
	sint minY;
	localPoly.computeOuterBorders(borders, minY);
	for (sint y = minY; y < (sint) (minY + borders.size()); ++y)
	{
		if (y < 0 || y >= (sint) _HeightMap.getHeight()) continue;
		for (sint x = borders[y - minY].first; x <= borders[y - minY].second; ++x)
		{
			if (x < 0 || x >= (sint) _HeightMap.getWidth()) continue;
			{
				_HeightMap(x, y) = 0x7ffe;
			}
		}
	}
}

// *********************************************************************************************************
void CIslandCollision::loadEntryPoints()
{
	//H_AUTO(R2_CIslandCollision_loadEntryPoints)
	if (_EntryPointsLoadingFailure) return;
	CScenarioEntryPoints &sep = CScenarioEntryPoints::getInstance();
	if (sep.getCompleteIslands().empty())
	{
		// load the entry points
		try
		{
			sep.loadCompleteIslands();
		}
		catch (const NLMISC::EStream &e)
		{
			_EntryPointsLoadingFailure = true;
			nlwarning(e.what());
		}
	}
}

// *********************************************************************************************************
bool CIslandCollision::isValidPoly(const NLMISC::CPolygon2D &poly)
{
	//H_AUTO(R2_CIslandCollision_isValidPoly)
	if (poly.Vertices.empty()) return true;
	// test against heightmap
	R2::CScenarioEntryPoints::CCompleteIsland *islandDesc = getCurrIslandDesc();
	if (!islandDesc) return true;
	// apply offset to polygon to  avoid overflow (coords of rasterized poly can be 32000 at most)
	static NLMISC::CPolygon2D offsetedPoly;
	offsetedPoly.Vertices.resize(poly.Vertices.size());
	for(uint k = 0; k < poly.Vertices.size(); ++k)
	{
		const CVector2f &src = poly.Vertices[k];
		offsetedPoly.Vertices[k].set(src.x - (float) islandDesc->XMin, src.y - (float) islandDesc->YMin);

		static volatile bool doAssert = false;
		if (doAssert)
		{
			nlassert(offsetedPoly.Vertices[k].x >= -32000.f); // coordinate too big !
			nlassert(offsetedPoly.Vertices[k].x < 32000.f);   // coordinate too big !
			nlassert(offsetedPoly.Vertices[k].y >= -32000.f); // coordinate too big !
			nlassert(offsetedPoly.Vertices[k].y < 32000.f);   // coordinate too big !
		}
		//
		if (offsetedPoly.Vertices[k].x < -32000.f) return false;
		if (offsetedPoly.Vertices[k].x >= 32000.f) return false;
		if (offsetedPoly.Vertices[k].y < -32000.f) return false;
		if (offsetedPoly.Vertices[k].y >= 32000.f) return false;
	}
	static NLMISC::CPolygon2D::TRasterVect borders;
	sint minY;
	offsetedPoly.computeOuterBorders(borders, minY);

	const CArray2D<sint16> &hm = getHeightMap();
	// TMP TMP
	static volatile bool dumpTestBitmap = false;
	if (dumpTestBitmap)
	{
		CBitmap bm;
		bm.resize(hm.getWidth(), hm.getHeight());
		CRGBA *dest = (CRGBA *) &bm.getPixels(0)[0];
		for (sint y = 0; y < (sint) hm.getHeight(); ++y)
		{
			for (sint x = 0; x < (sint) hm.getWidth(); ++x)
			{
				if (hm(x, y) >= 0x7ffe)
				{
					*dest = CRGBA::Red;
				}
				else
				{
					dest->set(0, hm(x, y) & 255, 0, 255);
				}
				++ dest;
			}
		}
		//
		for (sint y = 0; y < (sint) borders.size(); ++y)
		{
			if (y + minY < 0 || y + minY >= (sint) bm.getHeight()) continue;
			sint xmin = borders[y].first;
			sint xmax = borders[y].second;
			if (xmin > xmax) continue;
			if (xmax < 0 || xmin >= (sint) hm.getWidth()) continue;
			clamp(xmin, 0, (sint) (hm.getWidth() - 1));
			clamp(xmax, 0, (sint) (hm.getWidth() - 1));
			for (sint x = xmin; x <= xmax; ++x)
			{
				CRGBA *dest = (CRGBA *) &bm.getPixels(0)[0];
				dest += x + ((y + minY)) * hm.getWidth();
				dest->B = 255;
			}
		}
		try
		{
			COFile of("test_poly.tga");
			bm.writeTGA(of);
		}
		catch(...)
		{
		}
	}
	for (sint y = 0; y < (sint) borders.size(); ++y)
	{
		if (y + minY < 0 || y + minY >= (sint) hm.getHeight()) return false;
		sint xmin = borders[y].first;
		sint xmax = borders[y].second;
		if (xmin > xmax) continue;
		if (xmax < 0 || xmin >= (sint) hm.getWidth()) return false;
		clamp(xmin, 0, (sint) (hm.getWidth() - 1));
		clamp(xmax, 0, (sint) (hm.getWidth() - 1));
		for (sint x = xmin; x <= xmax; ++x)
		{
			if (hm(x, y + minY) >= 0x7ffe) return false;
		}
	}
	return true;
}

// *********************************************************************************************************
bool CIslandCollision::isValidSegment(const NLMISC::CVector2f start, const NLMISC::CVector2f end)
{
	//H_AUTO(R2_CIslandCollision_isValidSegment)
	static NLMISC::CPolygon2D extrudedSegment;
	extrudedSegment.Vertices.resize(4);
	CVector2f right, up;
	if (end.x != start.x || end.y != start.y)
	{
		right = 0.5f * (end - start).normed();
		up.set(- right.y, right.x);
	}
	else
	{
		right.set(0.5f, 0.f);
		up.set(0.f, 0.5f);
	}
	extrudedSegment.Vertices[0] = start - right + up;
	extrudedSegment.Vertices[1] = end + right + up;
	extrudedSegment.Vertices[2] = end + right - up;
	extrudedSegment.Vertices[3] = start - right - up;
	return isValidPoly(extrudedSegment);
}

// *********************************************************************************************************
void CIslandCollision::waterSurfaceAdded(const NLMISC::CPolygon2D &shape, const NLMISC::CMatrix &worldMatrix)
{
	//H_AUTO(R2_CIslandCollision_waterSurfaceAdded)
	for (std::list<CWaterSurf>::iterator it = _WaterSurfs.begin(); it != _WaterSurfs.end(); ++it)
	{
		if (shape.Vertices.size() == it->Shape.Vertices.size())
		{
			if (std::equal(shape.Vertices.begin(), shape.Vertices.end(), it->Shape.Vertices.begin()))
			{
				float oldMat[16];
				float newMat[16];
				it->WorldMatrix.get(oldMat);
				worldMatrix.get(newMat);
				if (std::equal(oldMat, oldMat + 16, newMat))
				{
					nlwarning("Duplicate water surface, not inserting second");
					return; // already inserted
				}
			}
		}
	}
	CWaterSurf ws;
	ws.Shape = shape;
	ws.WorldMatrix = worldMatrix;
	_WaterSurfs.push_back(ws);
}

// *********************************************************************************************************
bool CIslandCollision::isValidPos(const NLMISC::CVector2f &pos)
{
	//H_AUTO(R2_CIslandCollision_isValidPos)
	if (!isValidSegment(pos - CVector2f(0.5f, 0.f), pos + CVector2f(0.5f, 0.f))) return false;
	CVector dummy;
	return CTool::computeWorldMapIntersection(pos.x, pos.y, dummy) == CTool::ValidPacsPos;
}


} // R2

