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

#include "std3d.h"

#include "nel/3d/landscape_user.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/progress_callback.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Landscape )
H_AUTO_DECL( NL3D_Render_Landscape_updateLightingAll )
H_AUTO_DECL( NL3D_Load_Landscape )

#define	NL3D_HAUTO_UI_LANDSCAPE						H_AUTO_USE( NL3D_UI_Landscape )
#define	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL	H_AUTO_USE( NL3D_Render_Landscape_updateLightingAll )
#define	NL3D_HAUTO_LOAD_LANDSCAPE					H_AUTO_USE( NL3D_Load_Landscape )

// ***************************************************************************
CLandscapeUser::~CLandscapeUser()
{

	// must ensure all loading is ended
	removeAllZones();

	// then delete
	_Scene->deleteModel(_Landscape);
	_Landscape= NULL;
}


// ****************************************************************************
void	CLandscapeUser::setZonePath(const std::string &zonePath)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_ZoneManager.setZonePath(zonePath);
}

// ****************************************************************************
void CLandscapeUser::invalidateAllTiles()
{
	_Landscape->Landscape.invalidateAllTiles();
}

// ****************************************************************************
void	CLandscapeUser::loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	// Release all OLD tiles
	_Landscape->Landscape.releaseAllTiles();

	// Clear the bank
	_Landscape->Landscape.TileBank.clear ();

	// First, load the banks.
	//=======================
	CIFile bankFile(CPath::lookup(tileBankFile));
	_Landscape->Landscape.TileBank.serial(bankFile);
	// All textures path are relative!
	_Landscape->Landscape.TileBank.makeAllPathRelative();
	// Use DDS!!!
	_Landscape->Landscape.TileBank.makeAllExtensionDDS();
	// No absolute path
	_Landscape->Landscape.TileBank.setAbsPath ("");

	CIFile farbankFile(CPath::lookup(farBankFile));
	_Landscape->Landscape.TileFarBank.serial(farbankFile);
	bankFile.close();
	farbankFile.close();
}

// ****************************************************************************

void	CLandscapeUser::flushTiles (NLMISC::IProgressCallback &progress)
{
	// After loading the TileBank, and before initTileBanks(), must load the vegetables descritpor
	_Landscape->Landscape.TileBank.loadTileVegetableDescs();

	// init the TileBanks descriptors
	if ( ! _Landscape->Landscape.initTileBanks() )
	{
		nlwarning( "You need to recompute bank.farbank for the far textures" );
	}

	// Count tiles
	uint tileCount = 0;
	sint	ts;
	for (ts=0; ts<_Landscape->Landscape.TileBank.getTileSetCount (); ts++)
	{
		CTileSet *tileSet=_Landscape->Landscape.TileBank.getTileSet (ts);
		tileCount += tileSet->getNumTile128();
		tileCount += tileSet->getNumTile256();
		tileCount += CTileSet::count;
	}

	// Second, temporary, flushTiles.
	//===============================
	uint tile = 0;
	for (ts=0; ts<_Landscape->Landscape.TileBank.getTileSetCount (); ts++)
	{
		CTileSet *tileSet=_Landscape->Landscape.TileBank.getTileSet (ts);
		sint tl;
		for (tl=0; tl<tileSet->getNumTile128(); tl++)
		{
			// Progress bar
			progress.progress ((float)tile/(float)tileCount);
			tile++;

			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTile128(tl), 1);
		}
		for (tl=0; tl<tileSet->getNumTile256(); tl++)
		{
			// Progress bar
			progress.progress ((float)tile/(float)tileCount);
			tile++;

			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTile256(tl), 1);
		}
		for (tl=0; tl<CTileSet::count; tl++)
		{
			// Progress bar
			progress.progress ((float)tile/(float)tileCount);
			tile++;

			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
		}
	}
}

// ****************************************************************************

void	CLandscapeUser::loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zonesAdded.clear();

	_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	while (_ZoneManager.isLoading())
	{
		CZoneManager::SZoneManagerWork Work;
		if (_ZoneManager.isWorkComplete(Work))
		{
			nlassert(!Work.ZoneRemoved);
			if (Work.ZoneAdded)
			{
				if (Work.Zone == (CZone*)-1)
				{
					nlwarning ("Can't load zone %s", Work.NameZoneAdded.c_str ());
				}
				else
				{
					_Landscape->Landscape.addZone (*Work.Zone);

					delete Work.Zone;
					std::string zoneadd = Work.NameZoneAdded;
					zoneadd = zoneadd.substr(0, zoneadd.find('.'));
					zonesAdded.push_back(zoneadd);
				}
			}
		}
		else
		{
			nlSleep (1);
		}
		_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	}
	// Yoyo: must check the binds of the zones.
	_Landscape->Landscape.checkBinds();
}

// ****************************************************************************
void	CLandscapeUser::refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved,
											  NLMISC::IProgressCallback &progress, const std::vector<uint16> *validZoneIds)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zonesAdded.clear();
	zonesRemoved.clear();
	std::string		za, zr;

	_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius, validZoneIds);
	refreshZonesAround (pos, radius, za, zr);

	// Zone to load
	uint zoneToLoad = _ZoneManager.getNumZoneLeftToLoad ();
	while (_ZoneManager.isLoading() || _ZoneManager.isRemoving())
	{
		// Progress bar
		if (zoneToLoad != 0)
			progress.progress ((float)(zoneToLoad-_ZoneManager.getNumZoneLeftToLoad ())/(float)zoneToLoad);

		refreshZonesAround (pos, radius, za, zr);

		// some zone added or removed??
		if(!za.empty())
			zonesAdded.push_back(za);
		if(!zr.empty())
			zonesRemoved.push_back(zr);

		_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);

		if (_ZoneManager.isLoading())
			nlSleep (0);
	}
}

// ***************************************************************************
void	CLandscapeUser::getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const
{
	// Build the list of zoneId.
	std::vector<uint16>	zoneIds;
	_Landscape->Landscape.getZoneList(zoneIds);

	// transcript
	zoneLoaded.clear();
	zoneLoaded.resize(zoneIds.size());
	for(uint i=0;i<zoneLoaded.size();i++)
	{
		CLandscape::buildZoneName(zoneIds[i], zoneLoaded[i]);
	}
}

// ****************************************************************************
void	CLandscapeUser::loadAllZonesAround(const CVector &pos, float radius)
{
	std::vector<std::string>	dummy;
	loadAllZonesAround(pos, radius, dummy);
}


// ****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	std::string	dummy1, dummy2;
	refreshZonesAround(pos, radius, dummy1, dummy2);
}
// ****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved, const std::vector<uint16> *validZoneIds)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zoneRemoved.clear();
	zoneAdded.clear();
	CZoneManager::SZoneManagerWork Work;
	// Check if new zone must be added to landscape
	if (_ZoneManager.isWorkComplete(Work))
	{
		if (Work.ZoneAdded)
		{
			if (Work.Zone == (CZone*)-1)
			{
				//nlwarning ("Can't load zone %s", Work.NameZoneAdded.c_str ());
			}
			else
			{
				_Landscape->Landscape.addZone(*Work.Zone);

				// Yoyo: must check the binds of the new inserted zone.
				try
				{
					_Landscape->Landscape.checkBinds(Work.Zone->getZoneId());
				}
				catch (const EBadBind &e)
				{
					nlwarning ("Bind error : %s", e.what());
					nlstopex(("Zone Data Bind Error. Please send a report. You may continue but it should crash!"));
				}

				delete Work.Zone;
				zoneAdded = Work.NameZoneAdded;
				zoneAdded = zoneAdded.substr(0, zoneAdded.find('.'));
			}
		}

		// Check if a zone must be removed from landscape
		if (Work.ZoneRemoved)
		{
			_Landscape->Landscape.removeZone (Work.IdZoneToRemove);
			zoneRemoved = Work.NameZoneRemoved;
			zoneRemoved = zoneRemoved.substr(0, zoneRemoved.find('.'));
		}
	}

	_ZoneManager.checkZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius, validZoneIds);
}

// ****************************************************************************
void CLandscapeUser::removeAllZones()
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	// Ensure Async Loading is ended
	CZoneManager::SZoneManagerWork Work;
	while (_ZoneManager.isLoading())
	{
		if (_ZoneManager.isWorkComplete(Work))
		{
			// Zone to add?
			if (Work.ZoneAdded)
			{
				if (Work.Zone == (CZone*)-1)
				{
					nlwarning ("Can't load zone %s", Work.NameZoneAdded.c_str ());
				}
				else
				{
					// just discard it! cause will be all cleared below
					delete Work.Zone;
				}
			}

			// Zone to remove?
			if (Work.ZoneRemoved)
			{
				_Landscape->Landscape.removeZone (Work.IdZoneToRemove);
			}
		}
		else
		{
			nlSleep(1);
		}
	}

	// Then do a full clear
	_Landscape->Landscape.clear();
	_ZoneManager.clear();
}

// ****************************************************************************
void	CLandscapeUser::setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setupStaticLight(diffuse, ambiant, multiply);
}



// ****************************************************************************
void	CLandscapeUser::setThreshold (float thre)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setThreshold(thre);
}
// ****************************************************************************
float	CLandscapeUser::getThreshold () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getThreshold();
}
// ****************************************************************************
void	CLandscapeUser::setTileNear (float tileNear)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setTileNear(tileNear);
}
// ****************************************************************************
float	CLandscapeUser::getTileNear () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getTileNear();
}
// ****************************************************************************
void	CLandscapeUser::setTileMaxSubdivision (uint tileDiv)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setTileMaxSubdivision(tileDiv);
}
// ****************************************************************************
uint	CLandscapeUser::getTileMaxSubdivision ()
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getTileMaxSubdivision();
}


// ****************************************************************************
std::string	CLandscapeUser::getZoneName(const CVector &pos)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _ZoneManager.getZoneName((uint)pos.x, (uint)(-pos.y), 0, 0).first;
}


// ****************************************************************************
CVector		CLandscapeUser::getHeightFieldDeltaZ(float x, float y) const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getHeightFieldDeltaZ(x,y);
}

// ****************************************************************************
void		CLandscapeUser::setHeightField(const CHeightMap &hf)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setHeightField(hf);
}


// ****************************************************************************
void		CLandscapeUser::enableVegetable(bool enable)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.enableVegetable(enable);
}

// ****************************************************************************
void		CLandscapeUser::loadVegetableTexture(const std::string &textureFileName)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;
	_Landscape->Landscape.loadVegetableTexture(textureFileName);
}

// ****************************************************************************
void		CLandscapeUser::setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setupVegetableLighting(ambient, diffuse, directionalLight);
}

// ****************************************************************************
void		CLandscapeUser::setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setVegetableWind(windDir, windFreq, windPower, windBendMin);
}

// ****************************************************************************
void		CLandscapeUser::setVegetableUpdateLightingFrequency(float freq)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setVegetableUpdateLightingFrequency(freq);
}


// ****************************************************************************
void		CLandscapeUser::setUpdateLightingFrequency(float freq)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setUpdateLightingFrequency(freq);
}


// ****************************************************************************
void		CLandscapeUser::enableAdditive (bool enable)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->enableAdditive(enable);
}
// ****************************************************************************
bool		CLandscapeUser::isAdditiveEnabled () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->isAdditive ();
}

// ****************************************************************************
void		CLandscapeUser::setPointLightDiffuseMaterial(CRGBA diffuse)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setPointLightDiffuseMaterial(diffuse);
}
// ****************************************************************************
CRGBA		CLandscapeUser::getPointLightDiffuseMaterial () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getPointLightDiffuseMaterial();
}

// ****************************************************************************
void		CLandscapeUser::setDLMGlobalVegetableColor(CRGBA gvc)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setDLMGlobalVegetableColor(gvc);
}
// ****************************************************************************
CRGBA		CLandscapeUser::getDLMGlobalVegetableColor() const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getDLMGlobalVegetableColor();
}
// ****************************************************************************
void		CLandscapeUser::updateLightingAll()
{
	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL;
	_Landscape->Landscape.updateLightingAll();
}
// ****************************************************************************
void		CLandscapeUser::postfixTileFilename (const char *postfix)
{
	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL;
	_Landscape->Landscape.TileBank.postfixTileFilename (postfix);
}
// ****************************************************************************
void		CLandscapeUser::postfixTileVegetableDesc (const char *postfix)
{
	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL;
	_Landscape->Landscape.TileBank.postfixTileVegetableDesc (postfix);
}

// ***************************************************************************
void		CLandscapeUser::enableReceiveShadowMap(bool state)
{
	_Landscape->enableReceiveShadowMap(state);
}

// ***************************************************************************
bool		CLandscapeUser::canReceiveShadowMap() const
{
	return _Landscape->canReceiveShadowMap();
}

// ***************************************************************************
void		CLandscapeUser::setRefineCenterAuto(bool mode)
{
	_Landscape->setRefineCenterAuto(mode);
}

// ***************************************************************************
void		CLandscapeUser::setRefineCenterUser(const CVector &refineCenter)
{
	_Landscape->setRefineCenterUser(refineCenter);
}

// ***************************************************************************
bool		CLandscapeUser::getRefineCenterAuto() const
{
	return _Landscape->getRefineCenterAuto();
}

// ***************************************************************************
const CVector	&CLandscapeUser::getRefineCenterUser() const
{
	return _Landscape->getRefineCenterUser();
}


// ***************************************************************************
void CLandscapeUser::addTileCallback(ULandscapeTileCallback *cb)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.addTileCallback(cb);
}

// ***************************************************************************
void CLandscapeUser::removeTileCallback(ULandscapeTileCallback *cb)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.removeTileCallback(cb);
}

// ***************************************************************************
bool CLandscapeUser::isTileCallback(ULandscapeTileCallback *cb)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.isTileCallback(cb);
}

// ***************************************************************************
void CLandscapeUser::setZFunc(UMaterial::ZFunc val)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setZFunc((CMaterial::ZFunc)val);
}

// ***************************************************************************
const CZone*	CLandscapeUser::getZone (sint zoneId) const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getZone(zoneId);
}

// ***************************************************************************
void		CLandscapeUser::setVegetableDensity(float density)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setVegetableDensity(density);
}

// ***************************************************************************
float		CLandscapeUser::getVegetableDensity() const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getVegetableDensity();
}

// ***************************************************************************
float CLandscapeUser::getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getRayCollision(start, end);
}

} // NL3D
