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

#ifndef NL_U_LANDSCAPE_H
#define NL_U_LANDSCAPE_H

#include "nel/3d/u_material.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "height_map.h"

#include <string>

namespace NLMISC
{
	class IProgressCallback;
}

namespace NL3D
{

class CZone;

using	NLMISC::CVector;
using	NLMISC::CRGBA;

struct CTileAddedInfo
{
	uint64			TileID;
	NLMISC::CVector Corners[4];
	NLMISC::CVector Center;
	NLMISC::CVector	Normal;
};

/** Callback to know when a tile of a landscape has been added/removed
  */
struct ULandscapeTileCallback
{
	virtual ~ULandscapeTileCallback() {}
	virtual void tileAdded(const CTileAddedInfo &infos) = 0;
	virtual void tileRemoved(uint64 id) = 0;
};

// ***************************************************************************
/**
 * Game Interface for manipulate Landscape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class ULandscape
{
protected:

	/// \name Object
	/// protected because created/deleted by UScene.
	// @{
	ULandscape() {}
	virtual	~ULandscape() {}
	// @}


public:

	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath) =0;
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile) =0;
	/// Flush the tiles
	virtual void	flushTiles (NLMISC::IProgressCallback &progress) =0;
	/// Postfix tile filename
	virtual void	postfixTileFilename (const char *postfix) =0;
	/// Postfix vegetable filename
	virtual void	postfixTileVegetableDesc (const char *postfix) =0;
	/// Load all Zones around a position. This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius) =0;
	/** Load all Zones around a position. This is a blocking call.
	 *	\zonesAdded array of name of the zones added, without extension (eg: "150_EM").
	 */
	virtual	void	loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded) =0;
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius) =0;
	/** Delete old zones, or load new zones, around a position. new Zones are loaded async.
	 *	This method add or remove only one zone at a time.
	 *	\zoneRemoved name of the zone removed, without extension (eg: "150_EM"). "" if none.
	 *	\zoneAdded name of the zone added, without extension (eg: "150_EM"). "" if none.
	 *  \validZones subset of zones that can be loaded (NULL for default set)
	 */
	virtual	void	refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved, const std::vector<uint16> *validZoneIds = NULL) =0;
	/// Delete old zones, or load new zones, around a position, until it is finished. This is a blocking call.
	virtual	void	refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded,
		std::vector<std::string> &zonesRemoved, NLMISC::IProgressCallback &progress, const std::vector<uint16> *validZoneIds = NULL) =0;
	/** Get list of zones currently loaded in landscape.
	 *	\zonesLoaded array of name of the zones added, without extension (eg: "150_EM").
	 */
	virtual	void	getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const =0;
	// invalidate all tiles (this forces the tile callback to be called again)
	virtual void    invalidateAllTiles() = 0;
	// Remove all zones forcing at next pass a massive load
	virtual void	removeAllZones() = 0;
	// @}


	/// \name Lighting
	// @{
	/**
	  *  Setup the light color use for static illumination.
	  *  NB: This setup will be visible only for new texture far/near computed (when player move or see dynamic lighting).
	  *
	  *  \param diffuse is the color of the diffuse componante of the lighting.
	  *  \param ambiant is the color of the ambiante componante of the lighting.
	  *  \param multiply is the multiply factor. Final color is (diffuse*multiply*shading+ambiant*(1.0-shading))
	  */
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply) =0;

	/**	 Setup the equivalent material diffuse component used for both Static and Dynamic PointLights.
	  *	 Default is White.
	  */
	virtual	void	setPointLightDiffuseMaterial(CRGBA diffuse) =0;
	/**	 \see setPointLightDiffuseMaterial
	  */
	virtual	CRGBA	getPointLightDiffuseMaterial () const =0;


	/** Set the frequency of lighting update. If freq==1, ALL patchs are updated each second.
	 *	e.g: if 1/20, then every 20 seconds, all patchs are updated.
	 *	If you set 0, no update will be done at all (this is the default setup!!).
	 */
	virtual	void	setUpdateLightingFrequency(float freq) =0;

	/** update the lighting of ALL patch (slow method). NB: work even if UpdateLightingFrequency==0
	 *	Additionaly, vegetables are also ALL updated. WARNING!! If Scene Lighting is enabled (usual case),
	 *	vegetable lighting setup take last Sun setup at UScene::render(). Hence, you should force the new lighting
	 *	setup with ULandscape::setupVegetableLighting(), before calling updateLightingAll().
	 */
	virtual	void	updateLightingAll() =0;


	/** Set additive Lighting. Disabled by default.
	  * \param enable is true to activbe additive, false to disactive it.
	  */
	virtual	void	enableAdditive (bool enable) =0;

	/** Get additive Lighting
	  * \return true to if additive is actived, else false.
	  */
	virtual	bool	isAdditiveEnabled () const =0;


	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre) =0;
	/// Get threshold.
	virtual	float	getThreshold () const =0;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear) =0;
	/// Get tile near distance.
	virtual	float	getTileNear () const =0;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv) =0;
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision () =0;
	/// Set all zones monochromatic or colored
	virtual	void 	setTileColor (bool mono, float factor) =0;
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos) =0;

	/// show the landscape. visible by default.
	virtual	void		show() =0;
	/// hide the landscape. It is nor refined, nor rendered (=> take 0 CPU time).
	virtual	void		hide() =0;

	/** if true, the refine Center is auto computed each frame from Camera Position. Else must be given by
	 *	setRefineCenterUser()
	 *	Default to true.
	 */
	virtual void			setRefineCenterAuto(bool mode) =0;
	virtual bool			getRefineCenterAuto() const =0;
	/** \see setRefineCenterAuto
	  */
	virtual void			setRefineCenterUser(const CVector &refineCenter) =0;
	virtual const CVector	&getRefineCenterUser() const =0;
	// @}


	/// \name HeightField DeltaZ.
	// @{
	/// return the HeightField DeltaZ for the 2D position. (0,0,dZ) is returned.
	virtual	CVector		getHeightFieldDeltaZ(float x, float y) const =0;
	/** set the HeightField data. NB: take lot of place in memory.
	 * only one is possible. You should setup this heightfield around the zones which will be loaded.
	 * It is applied only when a zone is loaded, so you should setup it 2km around the user, each time you move too far
	 * from a previous place (eg 160m from last setup).
	 */
	virtual	void		setHeightField(const CHeightMap &hf) =0;
	// @}


	/// Micro-Vegetation.
	// @{

	/** enable the vegetable management in landscape. Valid only if Hardware support VertexShader.
	 */
	virtual	void		enableVegetable(bool enable) =0;

	/** load a texture for the vegetable, lookup in CPath
	 */
	virtual	void		loadVegetableTexture(const std::string &textureFileName) =0;

	/**	setup lighting ambient and diffuse for vegetable.
	 */
	virtual	void		setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight) =0;

	/** set the vegetable Wind for animation.
	 *	All thoses variables may be modified each frame without penalty.
	 *
	 *	\param windDir is the direction of the wind. NB: only XY direction is kept.
	 *	\param windFreq is the frequency for the animation (speed)
	 *	\param windPower is the power of the wind, and is a factor (0..1) of Bend
	 *	\param windBendMin is a value in (0..1) which indicate how much the vegetables are bended at minimum
	 *	(for very powerfull wind)
	 */
	virtual	void		setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin) =0;

	/** set the frequency of Vegetable lighting update. If freq==1, ALL lighted igs are updated each second.
	 *	e.g: if 1/20, then every 20 seconds, all Igs are updated.
	 *	If you set 0, no update will be done at all (this is the default setup!!).
	 */
	virtual	void		setVegetableUpdateLightingFrequency(float freq) =0;

	/** Set a density ratio [0, 1] to possibly reduce the amount of micro vegetable drawn. Default to 1
	 */
	virtual	void		setVegetableDensity(float density) =0;

	/** Get the density ratio [0, 1] of micro vegetable drawn. Default to 1
	 */
	virtual	float		getVegetableDensity() const =0;

	// @}


	/// \name Dynamic Lighting management
	// @{

	/** For Vegetable Dynamic ligthing only: this is an approximate color of all vegetables.
	 *	Default is (180, 180, 180).
	 */
	virtual	void		setDLMGlobalVegetableColor(CRGBA gvc) =0;

	/** see setDLMGlobalVegetableColor()
	 */
	virtual	CRGBA		getDLMGlobalVegetableColor() const =0;

	// @}

	/// \name ShadowMapping
	// @{
	/** By default, map shadow receiving is disabled. This enabled shadow for this model.
	 *	Fails if the model don't support dynamic Map Shadow Receiving (eg Particle system)
	 */
	virtual void			enableReceiveShadowMap(bool state) =0;
	/// true if the instance receive shadow. By default false
	virtual bool			canReceiveShadowMap() const =0;
	// @}

	/// \name Tile added/removed callback
	// @{
	// Add a new callback to know when a tile is added/removed.
	virtual	void					addTileCallback(ULandscapeTileCallback *cb) = 0;
	// Remove a tile callback
	virtual	void					removeTileCallback(ULandscapeTileCallback *cb) = 0;
	// Check cb is a tile callback
	virtual	bool					isTileCallback(ULandscapeTileCallback *cb) = 0;
	// @}

	// modify ZBuffer test of landscape material
	virtual	void					setZFunc(UMaterial::ZFunc val) = 0;

	/// \name getZone
	// @{
	// Get a zone pointer.
	virtual const CZone*	getZone (sint zoneId) const = 0;
	// @}

	/// \name raytrace
	// @{
	/** Get the ray collision against the TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 */
	virtual float			getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end) = 0;
	// @}

};


} // NL3D


#endif // NL_U_LANDSCAPE_H

/* End of u_landscape.h */
