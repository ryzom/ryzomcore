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
	virtual	~CLandscapeUser() NL_OVERRIDE;
	// @}


	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath) NL_OVERRIDE;
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile) NL_OVERRIDE;
	/// Flush the tiles
	virtual void	flushTiles (NLMISC::IProgressCallback &progress) NL_OVERRIDE;
	/// Postfix tile filename
	virtual void	postfixTileFilename (const char *postfix) NL_OVERRIDE;
	/// Postfix vegetable filename
	virtual void	postfixTileVegetableDesc (const char *postfix) NL_OVERRIDE;
	/// Load all Zones around a position. Call at init only!! (no zone must exist before). This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius) NL_OVERRIDE;
	virtual	void	loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded) NL_OVERRIDE;
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius) NL_OVERRIDE;
	virtual	void	refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved, const std::vector<uint16> *validZoneIds = NULL) NL_OVERRIDE;
	/// Delete old zones, or load new zones, around a position, until it is finished. This is a blocking call.
	virtual	void	refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved,
		NLMISC::IProgressCallback &progress, const std::vector<uint16> *validZoneIds = NULL) NL_OVERRIDE;
	virtual	void	getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const NL_OVERRIDE;
	virtual void    invalidateAllTiles() NL_OVERRIDE;
	virtual void	removeAllZones() NL_OVERRIDE;
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
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply) NL_OVERRIDE;

	virtual	void	setPointLightDiffuseMaterial(CRGBA diffuse) NL_OVERRIDE;
	virtual	CRGBA	getPointLightDiffuseMaterial () const NL_OVERRIDE;


	virtual	void	setUpdateLightingFrequency(float freq) NL_OVERRIDE;

	virtual	void	updateLightingAll() NL_OVERRIDE;

	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre) NL_OVERRIDE;
	/// Get threshold.
	virtual	float	getThreshold () const NL_OVERRIDE;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear) NL_OVERRIDE;
	/// Get tile near distance.
	virtual	float	getTileNear () const NL_OVERRIDE;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv) NL_OVERRIDE;
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision () NL_OVERRIDE;
	/// Set all zones monochromatic or colored
	virtual	void 	setTileColor (bool monochrome, float factor) NL_OVERRIDE { _ZoneManager.setZoneTileColor(monochrome, factor); }
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos) NL_OVERRIDE;

	virtual	void		show() NL_OVERRIDE
	{
		_Landscape->show();
	}
	virtual	void		hide() NL_OVERRIDE
	{
		_Landscape->hide();
	}

	virtual	void		enableAdditive (bool enable) NL_OVERRIDE;
	virtual	bool		isAdditiveEnabled () const NL_OVERRIDE;

	virtual	void			setRefineCenterAuto(bool mode) NL_OVERRIDE;
	virtual bool			getRefineCenterAuto() const NL_OVERRIDE;
	virtual void			setRefineCenterUser(const CVector &refineCenter) NL_OVERRIDE;
	virtual const CVector	&getRefineCenterUser() const NL_OVERRIDE;

	// @}


	/// \name HeightField DeltaZ.
	// @{
	virtual	CVector		getHeightFieldDeltaZ(float x, float y) const NL_OVERRIDE;
	virtual	void		setHeightField(const CHeightMap &hf) NL_OVERRIDE;
	// @}

	/// Micro-Vegetation.
	// @{
	virtual	void		enableVegetable(bool enable) NL_OVERRIDE;
	virtual	void		loadVegetableTexture(const std::string &textureFileName) NL_OVERRIDE;
	virtual	void		setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight) NL_OVERRIDE;
	virtual	void		setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin) NL_OVERRIDE;
	virtual	void		setVegetableUpdateLightingFrequency(float freq) NL_OVERRIDE;
	virtual	void		setVegetableDensity(float density) NL_OVERRIDE;
	virtual	float		getVegetableDensity() const NL_OVERRIDE;
	// @}


	/// \name Dynamic Lighting management
	// @{
	virtual	void		setDLMGlobalVegetableColor(CRGBA gvc) NL_OVERRIDE;
	virtual	CRGBA		getDLMGlobalVegetableColor() const NL_OVERRIDE;
	// @}

	/// \name ShadowMapping
	// @{
	virtual void			enableReceiveShadowMap(bool state) NL_OVERRIDE;
	virtual bool			canReceiveShadowMap() const NL_OVERRIDE;
	// @}

	/// \name TileCallback
	// @{
	virtual	void					addTileCallback(ULandscapeTileCallback *cb) NL_OVERRIDE;
	virtual	void					removeTileCallback(ULandscapeTileCallback *cb) NL_OVERRIDE;
	virtual	bool					isTileCallback(ULandscapeTileCallback *cb) NL_OVERRIDE;
	// @}

	// modify ZBuffer test of landscape material
	virtual	void					setZFunc(UMaterial::ZFunc val) NL_OVERRIDE;

	/// \name getZone
	// @{
	// Get a zone pointer.
	virtual const CZone*	getZone (sint zoneId) const NL_OVERRIDE;
	// @}

	/// \name raytrace
	// @{
	virtual float			getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end) NL_OVERRIDE;
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
