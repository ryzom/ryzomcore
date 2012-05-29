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

#ifndef NL_LANDSCAPE_USER_H
#define NL_LANDSCAPE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_landscape.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/zone_manager.h"
#include "nel/3d/scene.h"


namespace NL3D
{


// ****************************************************************************
/**
 * ULandscape Implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeUser : public ULandscape
{
private:
	// The Scene.
	CScene				*_Scene;
	// The landscape, owned by the scene.
	CLandscapeModel		*_Landscape;
	// The zoneManager.
	CZoneManager		_ZoneManager;

public:

	/// \name Object
	// @{
	CLandscapeUser(CScene *scene)
	{
		nlassert(scene);
		_Scene= scene;
		_Landscape= (CLandscapeModel*)_Scene->createModel(LandscapeModelId);
	}
	virtual	~CLandscapeUser();
	// @}


	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath);
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile);
	/// Flush the tiles
	virtual void	flushTiles (NLMISC::IProgressCallback &progress);
	/// Postfix tile filename
	virtual void	postfixTileFilename (const char *postfix);
	/// Postfix vegetable filename
	virtual void	postfixTileVegetableDesc (const char *postfix);
	/// Load all Zones around a position. Call at init only!! (no zone must exist before). This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius);
	virtual	void	loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded);
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius);
	virtual	void	refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved, const std::vector<uint16> *validZoneIds = NULL);
	/// Delete old zones, or load new zones, around a position, until it is finished. This is a blocking call.
	virtual	void	refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved,
		NLMISC::IProgressCallback &progress, const std::vector<uint16> *validZoneIds = NULL);
	virtual	void	getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const;
	virtual void    invalidateAllTiles();
	virtual void	removeAllZones();
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
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply);

	virtual	void	setPointLightDiffuseMaterial(CRGBA diffuse);
	virtual	CRGBA	getPointLightDiffuseMaterial () const;


	virtual	void	setUpdateLightingFrequency(float freq);

	virtual	void	updateLightingAll();

	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre);
	/// Get threshold.
	virtual	float	getThreshold () const;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear);
	/// Get tile near distance.
	virtual	float	getTileNear () const;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv);
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision ();
	/// Set all zones monochromatic or colored
	virtual	void 	setTileColor (bool monochrome, float factor) { _ZoneManager.setZoneTileColor(monochrome, factor); }
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos);

	virtual	void		show()
	{
		_Landscape->show();
	}
	virtual	void		hide()
	{
		_Landscape->hide();
	}

	virtual	void		enableAdditive (bool enable);
	virtual	bool		isAdditiveEnabled () const;

	virtual	void			setRefineCenterAuto(bool mode);
	virtual bool			getRefineCenterAuto() const;
	virtual void			setRefineCenterUser(const CVector &refineCenter);
	virtual const CVector	&getRefineCenterUser() const;

	// @}


	/// \name HeightField DeltaZ.
	// @{
	virtual	CVector		getHeightFieldDeltaZ(float x, float y) const;
	virtual	void		setHeightField(const CHeightMap &hf);
	// @}

	/// Micro-Vegetation.
	// @{
	virtual	void		enableVegetable(bool enable);
	virtual	void		loadVegetableTexture(const std::string &textureFileName);
	virtual	void		setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight);
	virtual	void		setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin);
	virtual	void		setVegetableUpdateLightingFrequency(float freq);
	virtual	void		setVegetableDensity(float density);
	virtual	float		getVegetableDensity() const;
	// @}


	/// \name Dynamic Lighting management
	// @{
	virtual	void		setDLMGlobalVegetableColor(CRGBA gvc);
	virtual	CRGBA		getDLMGlobalVegetableColor() const;
	// @}

	/// \name ShadowMapping
	// @{
	virtual void			enableReceiveShadowMap(bool state);
	virtual bool			canReceiveShadowMap() const;
	// @}

	/// \name TileCallback
	// @{
	virtual	void					addTileCallback(ULandscapeTileCallback *cb);
	virtual	void					removeTileCallback(ULandscapeTileCallback *cb);
	virtual	bool					isTileCallback(ULandscapeTileCallback *cb);
	// @}

	// modify ZBuffer test of landscape material
	virtual	void					setZFunc(UMaterial::ZFunc val);

	/// \name getZone
	// @{
	// Get a zone pointer.
	virtual const CZone*	getZone (sint zoneId) const;
	// @}

	/// \name raytrace
	// @{
	virtual float			getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end);
	// @}

public:
	/// \name Accessor for CLandscapeUser.
	// @{
	CLandscapeModel		*getLandscape()
	{
		return _Landscape;
	}
	// @}

};


} // NL3D


#endif // NL_LANDSCAPE_USER_H

/* End of landscape_user.h */
