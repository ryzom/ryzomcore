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

#ifndef R2_ISLAND_COLLISION_H
#define R2_ISLAND_COLLISION_H


#include "game_share/scenario_entry_points.h"
//
#include "nel/misc/array_2d.h"
#include "nel/misc/polygon.h"
//
#include "nel/3d/texture_user.h"
#include "nel/3d/packed_world.h"
//
#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_element.h"

namespace NL3D
{
	class CPackedWorld;
	class CPolygon2D;
	class ITexture;
}

namespace R2
{

class CRefCountedPackedWorld : public NL3D::CPackedWorld, public NLMISC::CRefCount
{
public:
	typedef NLMISC::CRefPtr<CRefCountedPackedWorld> TRefPtr;
	typedef NLMISC::CSmartPtr<CRefCountedPackedWorld> TSmartPtr;
};

// access to island heightmap for collision tests
class CIslandCollision
{
public:
	static const uint							 MapSizeDivisor; // size ratio between the island map & the accessibility texture
public:
	CIslandCollision();
	~CIslandCollision();
	void release();
	CRefCountedPackedWorld						*getPackedIsland();
	R2::CScenarioEntryPoints::CCompleteIsland	*getCurrIslandDesc();
	const NLMISC::CArray2D<sint16>				&getHeightMap();
	/** Find an empty place from a start place.
	  * The result place is suitable to create a new entity.
	  * Used by copy-paste to find a new place to create a copy of an entity
	  * Fill snapped pos and returns true on success
	  */
	bool										findEmptyPlace(const NLMISC::CVector2f &startPos, NLMISC::CVector &dest);
	// Update current available collision based on current pos
	void										updateCurrPackedIsland();
	void										loadEntryPoints();
	// test a convex polygon against the map & water, return true if all positions under
	// the polygon are valid positions. Normal of terrain is not tested.
	bool										isValidPoly(const NLMISC::CPolygon2D &poly);
	// Test if the given position is valid (0.5 meter diameter). Test also normal at that position (so good test
	// to create an entity here)
	bool										isValidPos(const NLMISC::CVector2f &pos);
	// test a segment against the heightmap & water, normal of terrain is not tested
	bool										isValidSegment(const NLMISC::CVector2f start, const NLMISC::CVector2f end);
	// accessibility texture
	NL3D::ITexture								*getAccesibilityTexture();
	const NLMISC::CMatrix						&getWorldToAccessibilityTexMat(bool cropped = false) const { return cropped ? _WorldToAccessibilityTexMatCropped : _WorldToAccessibilityTexMat; }
private:
	//	packed island
	NLMISC::CVector2f							_LastPackedIslandTestPos;
	bool										_PackedIslandLoadingFailure;
	CRefCountedPackedWorld::TSmartPtr			_PackedIsland;
	NLMISC::CSString							_PackedIslandName;
	bool										_EntryPointsLoadingFailure;
	NLMISC::CArray2D<sint16>					_HeightMap;
	CScenarioEntryPoints::CCompleteIsland		_IslandDesc;
	NL3D::CTextureUser							*_AccessibilityTexture;
	NLMISC::CMatrix								_WorldToAccessibilityTexMat;
	NLMISC::CMatrix								_WorldToAccessibilityTexMatCropped;
	static const char							*_AccessibilityTextureId;
private:
	NL3D::CPackedWorld *reloadPackedIsland(const CScenarioEntryPoints::CCompleteIsland &islandDesc);
	void releaseAccessibilityTexture();
protected:
	struct CWaterSurf
	{
		NLMISC::CPolygon2D Shape;
		NLMISC::CMatrix WorldMatrix;
	};
	std::list<CWaterSurf> _WaterSurfs;
	void mergeWaterSurfaces();
	void addInaccessibleRegion(const NLMISC::CPolygon2D &poly);
public:
	// for CWaterMap
	void waterSurfaceAdded(const NLMISC::CPolygon2D &shape, const NLMISC::CMatrix &worldMatrix);
};


} // R2

#endif
