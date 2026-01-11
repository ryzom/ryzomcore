// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_VISUAL_COLLISION_ENTITY_H
#define NL_VISUAL_COLLISION_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/3d/point_light_influence.h"
#include "nel/3d/patch.h"
#include "nel/3d/landscape_collision_grid.h"


namespace NL3D
{


class	CVisualCollisionManager;
class	IDriver;
struct	CSurfaceInfo;

// ***************************************************************************
/**
 * An entity created by CVisualCollisionManager.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionEntity
{
public:

	/// Constructor
	CVisualCollisionEntity(CVisualCollisionManager *owner);
	~CVisualCollisionEntity();


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 *	and meshes. see setSnapToRenderedTesselation() option.
	 *	pos is checked with polygons that are at least (cache dependent) at +- 10m in altitude.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos);


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 *	and meshes.
	 *	pos is checked with polygons that are at least (cache dependent) at +- 10m in altitude.
	 * \param normal the ret normal of where it is snapped. NB: if return false, not modified.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos, CVector &normal);



	/** If groundMode is true, the entity is snapped on faces with normal.z > 0. Default is true.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	void	setGroundMode(bool groundMode) {_GroundMode= groundMode;}


	/** If ceilMode is true, the entity is snapped on faces with normal.z < 0. Default is false.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	void	setCeilMode(bool ceilMode) {_CeilMode= ceilMode;}


	bool	getGroundMode() const {return _GroundMode;}
	bool	getCeilMode() const {return _CeilMode;}


	/** By default, the visual collision entity is snapped on rendered/geomorphed tesselation (true).
	 *  Use this method to change this behavior. if false, the entity is snapped to the tile level tesselation
	 *	according to noise etc...
	 */
	void	setSnapToRenderedTesselation(bool snapMode) {_SnapToRenderedTesselation= snapMode;}
	bool	getSnapToRenderedTesselation() const {return _SnapToRenderedTesselation;}


	/** Get surface information.
	 * pos is checked with polygons that are at least (cache dependent) at +- 10m in altitude.
	 * \param info will be filled with surface information if the method returns true.
	 * \return true if the surface has been found and information has been filled.
	 */
	bool	getSurfaceInfo(const CVector &pos, CSurfaceInfo &info);


	/// \name Parameters.
	// @{
	/// This is the radius of the bbox around the entity where we have correct collisions: 10m.
	static const float					BBoxRadius;
	/** Same as BBoxRadius, but for z value. This later should be greater because of NLPACS
	 *	surface quadtree imprecision. 20m
	 *	NB: Because of caching, if the pos.z passed to snapToGround() is outside of the currentBBox
	 *	with BBoxRadiuZ/2 (=> 10m), then the bbox is recomputed.
	 *	Hence, this actually means that a pos is checked with patchs that are at least at +- 10m in altitude.
	 */
	static const float					BBoxRadiusZ;
	// @}


	/// \name Static Lighting
	// @{
	/** Get the static Light Setup, using landscape under us. append lights to pointLightList.
	 *	NB: if find no landscape faces, don't modify pointLightList, set sunContribution=255, and return false
	 *	Else, use CPatch::TileLightInfluences to get lights, and use CPatch::Lumels to get sunContribution.
	 *	NB: because CPatch::Lumels encode the gouraud shading on the surface, returning lumelValue will
	 *	darken the object too much. To avoid this, the sunContribution is raised to a power (0..1).
	 *	See CVisualCollisionManager::setSunContributionPower(). Default is 0.5
	 */
	bool		getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &pos, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient);
	// @}


	/// \name Debug display
	// @{

	/// Draw lines for the landscape quadgrid collision faces under us
	void		displayDebugGrid(IDriver &drv) const;

	// @}


// ***********************
private:
	CVisualCollisionManager		*_Owner;

	bool	_CeilMode;
	bool	_GroundMode;
	bool	_SnapToRenderedTesselation;


	/// \name Landscape part.
	// @{
	/// Default capacity of _PatchQuadBlocks.
	static const uint32					_StartPatchQuadBlockSize;
	/// This is the temp array of BlockIds filled by landscape search.
	static std::vector<CPatchBlockIdent>	_TmpBlockIds;
	/// This is the temp array of PatchBlocks ptr.
	static std::vector<CPatchQuadBlock*>	_TmpPatchQuadBlocks;

	/** Array of quadBlock which are around the entity. NB: plain vector, because not so big (ptrs).
	 * NB: reserve to a big size (64), so reallocation rarely occurs.
	 * NB: this array is sorted in ascending order (comparison of CPatchBlockIdent).
	 */
	std::vector<CPatchQuadBlock*>		_PatchQuadBlocks;
	/// A quadgrid of chainlist of tileId (CVisualTileDescNode), which are around the entity.
	CLandscapeCollisionGrid				_LandscapeQuadGrid;
	/// The current BBox where we don't need to recompute the patchQuadBlocks if the entity is in
	CAABBox								_CurrentBBoxValidity;
	/// Cache for getPatchTriangleUnderUs().
	bool								_LastGPTValid;
	CVector								_LastGPTPosInput;
	CVector								_LastGPTPosOutput;
	CTrianglePatch						_LastGPTTrianglePatch;

	/// Fast "2D" test of a triangle against ray P0 P1.
	static bool	triangleIntersect2DGround(CTriangle &tri, const CVector &pos0);
	static bool	triangleIntersect2DCeil(CTriangle &tri, const CVector &pos0);

	/// Fast "2D" test of a triangle against ray P0 P1.
	bool		triangleIntersect(CTriangle &tri, const CVector &pos0, const CVector &pos1, CVector &hit);


	/// test if the new position is outside the preceding setuped bbox, and then compute tiles infos around the position.
	void		testComputeLandscape(const CVector &pos);
	/// compute tiles infos around the position.
	void		doComputeLandscape(const CVector &pos);

	/// snap to current rendered tesselation.
	void		snapToLandscapeCurrentTesselation(CVector &pos, const CTrianglePatch &tri);

	/// given a CTrianglePatch, compute Patch uv according to position.
	static void	computeUvForPos(const CTrianglePatch &tri, const CVector &pos, CUV &uv);

	/** return the best trianglePatch under what we are. NULL if not found.
	 *	Ptr is valid until next call to getPatchTriangleUnderUs()
	 *	Actually return NULL or &_LastTrianglePatch;
	 */
	CTrianglePatch		*getPatchTriangleUnderUs(const CVector &pos, CVector &res);

	// @}


};


} // NL3D


#endif // NL_VISUAL_COLLISION_ENTITY_H

/* End of visual_collision_entity.h */
