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
//
#include "sky.h"
#include "client_sheets/sky_sheet.h"

using namespace NL3D;
using namespace NLMISC;


/////////////
// GLOBALS //
/////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Render_Sky )


// *************************************************************************************************
CSky::CSky()
{
	_Scene = NULL;
	_Driver = NULL;
	_IG = NULL;
	_AnimationSet = NULL;
	_NumHourInDay = 24;
	_PlayListManager = NULL;
	_PlayList = NULL;
	_AnimationSet = NULL;
	_AnimLengthInSeconds = 3.f;
	_AmbientSunLight = NULL;
	_DiffuseSunLight = NULL;
	_FogColor = NULL;
	_WaterEnvMapCameraHeight = 0.f;
	_WaterEnvMapAlpha = 255;
}

// *************************************************************************************************
CSky::~CSky()
{
	release();
}

// *************************************************************************************************
void CSky::release()
{
	if (_PlayListManager)
	{
		if (_PlayList)
		{
			_PlayListManager->deletePlayList(_PlayList);
			_PlayList = NULL;
		}
		if (_AnimationSet && _Driver)
		{
			_Driver->deleteAnimationSet(_AnimationSet);
			_AnimationSet = NULL;
		}
		if (_Scene)
		{
			_Scene->deletePlayListManager(_PlayListManager);
		}
		_PlayListManager = NULL;
	}
	for(uint k = 0; k < _Bitmaps.size(); ++k)
	{
		delete _Bitmaps[k];
	}
	_Bitmaps.clear();
	if (_Scene)
	{
		if (_IG) _IG->removeFromScene(*_Scene);
		_Objects.clear();
		_Driver->deleteScene(_Scene);
		_Scene = NULL;
		_Driver = NULL;
	}
}

// *************************************************************************************************
void CSky::init(UDriver *drv, const CSkySheet &sheet, bool forceFallbackVersion /*= false*/, float numHourInDay /*= 24.f*/, std::vector<std::string> *unsupportedObjects /*= NULL*/)
{
	release();
	if(!drv) return;
	_Driver = drv;
	// create a new scene for the sky
	_Scene = _Driver->createScene(true);
	_Scene->setupTransparencySorting(99, 1); // only sort by priority (99 of them)
	_AnimLengthInSeconds = sheet.AnimLengthInSeconds;
	// create animation set
	if (!sheet.AnimationName.empty())
	{
		_PlayListManager = _Scene->createPlayListManager();
		if (_PlayListManager)
		{
			_AnimationSet =    _Driver->createAnimationSet();
			_PlayList     =	   _PlayListManager->createPlayList(_AnimationSet);
			if (_AnimationSet && _PlayList)
			{
				uint animationID = _AnimationSet->addAnimation(sheet.AnimationName.c_str(), sheet.AnimationName.c_str());
				if (animationID != UAnimationSet::NotFound)
				{
					_AnimationSet->build();
					_PlayList->setAnimation(0, animationID);
					_PlayList->setTimeOrigin(0, 0);
					_PlayList->setWrapMode(0, UPlayList::Repeat);
				}
				else
				{
					// no animation loaded
					_PlayListManager->deletePlayList(_PlayList);
					_Scene->deletePlayListManager(_PlayListManager);
					_Driver->deleteAnimationSet(_AnimationSet);
					_PlayListManager = NULL;
					_PlayList = NULL;
					_AnimationSet = NULL;
				}
			}
		}
	}
	// load instance group of sky
	_IG = UInstanceGroup::createInstanceGroup(sheet.InstanceGroupName);
	if (!_IG)
	{
		nlwarning("Couldn't load sky ig : %s", sheet.InstanceGroupName.c_str());
		release();
		return;
	}
	_IG->addToScene(*_Scene, drv);
	_Objects.reserve(sheet.Objects.size());
	// dump name of objects in the scene
	//nlinfo("Sky scene objects : ");
	for(uint k = 0; k < _IG->getNumInstance(); ++k)
	{
		//nlinfo(_IG->getInstanceName(k).c_str());
		UInstance i = _IG->getInstance(k); // hide all instances by default
		if (!i.empty()) i.hide();
	}
	if (unsupportedObjects) unsupportedObjects->clear();
	// map name of a bitmap to the actual bitmap (for reuse of bitmaps)
	std::map<std::string, CBitmap *> buildShareBitmapByName;
	//
	for(uint k = 0; k < sheet.Objects.size(); ++k)
	{
		UInstance instance;
		instance = _IG->getByName(sheet.Objects[k].Std.ShapeName);
		// should main instance if driver supports its rendering
		// hide all the instance at start
		if (!instance.empty() && !instance.supportMaterialRendering(*drv, forceFallbackVersion))
		{
			if (unsupportedObjects)
			{
				unsupportedObjects->push_back(sheet.Objects[k].Std.ShapeName);
			}
			instance.hide();
			// build fallbacks
			for(uint l = 0; l < 2; ++l)
			{
				UInstance fallbackInstance = _IG->getByName(sheet.Objects[k].FallbackPass[l].ShapeName);
				if (!fallbackInstance.empty())
				{
					fallbackInstance.hide();
					CSkyObject so;
					so.init(sheet.Objects[k].FallbackPass[l], fallbackInstance, buildShareBitmapByName, _Bitmaps, sheet.Objects[k].VisibleInMainScene, sheet.Objects[k].VisibleInEnvMap);
					_Objects.push_back(so);
					if (_PlayList)
					{
						_PlayList->registerTransform(fallbackInstance, (sheet.Objects[k].FallbackPass[l].ShapeName + ".").c_str());
					}
				}
			}
		}
		else if (!instance.empty())
		{
			instance.hide();
			// uses main instance and hides fallback instances
			CSkyObject so;
			so.init(sheet.Objects[k].Std, instance, buildShareBitmapByName, _Bitmaps, sheet.Objects[k].VisibleInMainScene, sheet.Objects[k].VisibleInEnvMap);
			_Objects.push_back(so);
			for(uint l = 0; l < 2; ++l)
			{
				UInstance fallbackInstance = _IG->getByName(sheet.Objects[k].FallbackPass[l].ShapeName);
				if (!fallbackInstance.empty()) fallbackInstance.hide();
			}
			if (_PlayList)
			{
				_PlayList->registerTransform(instance, (sheet.Objects[k].Std.ShapeName + ".").c_str());
			}
		}
		else
		{
			nlwarning("Object not found in scene : %s", sheet.Objects[k].Std.ShapeName.c_str());
		}
	}
	// get gradient that gives sun light color
	bool alreadyBuilt;
	_AmbientSunLight = buildSharedBitmap(sheet.AmbientSunLightBitmap, buildShareBitmapByName, _Bitmaps, alreadyBuilt);
	_DiffuseSunLight = buildSharedBitmap(sheet.DiffuseSunLightBitmap, buildShareBitmapByName, _Bitmaps, alreadyBuilt);
	//
	_FogColor = buildSharedBitmap(sheet.FogColorBitmap, buildShareBitmapByName, _Bitmaps, alreadyBuilt);
	//
	_NumHourInDay = numHourInDay;
	_WaterEnvMapCameraHeight = sheet.WaterEnvMapCameraHeight;
	_WaterEnvMapAlpha= sheet.WaterEnvMapAlpha;
}

// *************************************************************************************************
uint CSky::setup(const CClientDate &date, const CClientDate &animationDate, float weatherLevel, CRGBA fogColor, const NLMISC::CVector &sunLightDir, bool envMapScene)
{
	if (!_Scene) return 0;
	uint numVisibleObjects = 0;
	uint numObjects = (uint)_Objects.size();
	float dayPart = date.Hour / _NumHourInDay;
	clamp(dayPart, 0.f, 1.f);
	clamp(weatherLevel, 0.f, 1.f);
	for(uint k = 0; k < numObjects; ++k)
	{
		if (_Objects[k].setup(date, animationDate, _NumHourInDay, weatherLevel, fogColor, envMapScene)) ++ numVisibleObjects;
	}
	// animate objects
	if (_PlayListManager)
	{
		double globalDate = (double)date.Hour / (double)_NumHourInDay;
		//nlinfo("global date = %f", (float) globalDate);
		_PlayListManager->animate(_AnimLengthInSeconds * globalDate);
	}
	// compute sunlight for the scene
	if (_AmbientSunLight)
	{
		_Scene->setSunAmbient(_AmbientSunLight->getColor(dayPart, weatherLevel, true, false));
	}
	else
	{
		_Scene->setSunAmbient(CRGBA::White);
	}
	if (_DiffuseSunLight)
	{
		_Scene->setSunDiffuse(_DiffuseSunLight->getColor(dayPart, weatherLevel, true, false));
	}
	else
	{
		_Scene->setSunDiffuse(CRGBA::White);
	}
	_Scene->setSunSpecular(CRGBA::Black); // specular not useful for sky
	_Scene->setSunDirection(sunLightDir);
	return numVisibleObjects;
}

// *************************************************************************************************
NLMISC::CRGBA CSky::computeFogColor(const CClientDate &date, float weatherLevel) const
{
	if (!_FogColor) return CRGBA::White;
	float dayPart = date.Hour / _NumHourInDay;
	clamp(dayPart, 0.f, 1.f);
	clamp(weatherLevel, 0.f, 1.f);
	return _FogColor->getColor(dayPart, weatherLevel, true, false);
}


// *************************************************************************************************
CBitmap *buildSharedBitmap(const std::string &filename,
					 std::map<std::string, CBitmap *> &bitmapByName,
					 std::vector<CBitmap *> &builtBitmaps,
					 bool &alreadyBuilt
					)
{
	alreadyBuilt = false;
	if (filename.empty()) return NULL;
	std::string lcBMFilename = strlwr(CFile::getFilenameWithoutExtension(filename));
	std::map<std::string, CBitmap *>::iterator it = bitmapByName.find(lcBMFilename);
	if (it != bitmapByName.end())
	{
		alreadyBuilt = true;
		// bitmap already loaded so reuse it
		return it->second;
	}
	else
	{
		// load the bitmap
		std::string path = CPath::lookup(filename, false);
		if (path.empty()) return NULL;
		std::auto_ptr<CBitmap> bm(new CBitmap);
		try
		{
			CIFile f;
			f.open(path);
			if (bm->load(f, 0) == 0) return NULL;
			builtBitmaps.push_back(bm.release());
			bitmapByName[lcBMFilename] = builtBitmaps.back();
			// dump bitmap fisrt line
			return builtBitmaps.back();
		}
		catch(const EStream &)
		{
			return NULL;
		}
	}
}
