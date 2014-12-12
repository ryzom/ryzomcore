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


/////////////
// INCLUDE //
/////////////
// 3D Interface
#include "nel/3d/u_instance.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_water.h"
#include "nel/3d/u_landscape.h"

// Client.
#include "light_cycle_manager.h"
#include "client_cfg.h"
#include "ig_client.h"
#include "user_entity.h"
#include "pacs_client.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "continent.h"
#include "ig_enum.h"
#include "ig_callback.h"
#include "pacs_client.h"
#include "sound_manager.h"
#include "weather_manager_client.h"
#include "misc.h"
#include "interface_v3/interface_manager.h"

//#include "sound_manager.h"	// \todo GUIGUI : uncomment after new FE done and class modified


H_AUTO_DECL(RZ_LightCycleManager)

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace std;


////////////
// EXTERN //
////////////
extern UDriver		*Driver;
extern UScene		*Scene;
extern UScene		*SceneRoot;

extern EGSPD::CSeason::TSeason   CurrSeason;


////////////
// EXTERN //
////////////
extern ULandscape		*Landscape;

static uint diffColors(CRGBA c1, CRGBA c2)
{
	return (uint) maxof(abs((sint) c1.R - c2.R), abs((sint) c1.G - c2.G), abs((sint) c1.B - c2.B));
}

//-----------------------------------------------
float CLightCycleDesc::getNightTransitionLength() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	return NightTransitionEndHour >= NightTransitionStartHour ? NightTransitionEndHour - NightTransitionStartHour
		                                                       : NightTransitionEndHour + NumHours - NightTransitionStartHour;
}

//-----------------------------------------------
float CLightCycleDesc::getDawnTransitionLength() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	return DawnTransitionEndHour >= DawnTransitionStartHour ? DawnTransitionEndHour - DawnTransitionStartHour
		                                                    : DawnTransitionEndHour + NumHours - DawnTransitionStartHour;
}

//-----------------------------------------------
CLightCycleManager::CLightCycleManager() : 
					_Hour(0),
					_Touched(true),
                                        _ValidDesc(false),
					_LightLevel(0),
					_WeatherLighting(0),
					_LastWeatherLighting(0),
					_UpdateFreq(0),
					_State(StateUnknown),
					_PrevState(StateUnknown)
{
	H_AUTO_USE(RZ_LightCycleManager)
}


//-----------------------------------------------
void CLightCycleManager::getLandscapeLightColor(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &ambiant)
{
	H_AUTO_USE(RZ_LightCycleManager)
	nlassert(_LightLevel >= 0 && _LightLevel <= 1.f);

	if(ContinentMngr.cur())
	{
		switch(_State)
		{
			case DayToNight:
				if (_LightLevel <= _Desc.DuskRatio)
				{
					// day->dusk
					uint level = (uint) (256.f * (_Desc.DuskRatio != 0.f ? _LightLevel / _Desc.DuskRatio : 0.f));
					diffuse.blendFromui(ContinentMngr.cur()->LandscapeLightDay.Diffuse, ContinentMngr.cur()->LandscapeLightDusk.Diffuse, level);
					ambiant.blendFromui(ContinentMngr.cur()->LandscapeLightDay.Ambiant, ContinentMngr.cur()->LandscapeLightDusk.Ambiant, level);
				}
				else
				{
					// dusk->night
					uint level = (uint) (256.f * (_Desc.DuskRatio != 1.f ? (_LightLevel -  _Desc.DuskRatio) / (1.f - _Desc.DuskRatio) : 0.f));
					diffuse.blendFromui(ContinentMngr.cur()->LandscapeLightDusk.Diffuse, ContinentMngr.cur()->LandscapeLightNight.Diffuse, level);
					ambiant.blendFromui(ContinentMngr.cur()->LandscapeLightDusk.Ambiant, ContinentMngr.cur()->LandscapeLightNight.Ambiant, level);
				}
			break;
			default:
				uint level = (uint) (256.f * _LightLevel);
				diffuse.blendFromui(ContinentMngr.cur()->LandscapeLightDay.Diffuse, ContinentMngr.cur()->LandscapeLightNight.Diffuse, level);
				ambiant.blendFromui(ContinentMngr.cur()->LandscapeLightDay.Ambiant, ContinentMngr.cur()->LandscapeLightNight.Ambiant, level);
			break;
		}
	}
}


//-----------------------------------------------
bool CLightCycleManager::setLightDesc(const CLightCycleDesc &desc)
{
	H_AUTO_USE(RZ_LightCycleManager)
	if (desc.NightTransitionStartHour > desc.NumHours)
	{
		nlwarning ("NightTransitionStartHour (%d) > NumHours (%d)", desc.NightTransitionStartHour, desc.NumHours);
		_State = StateUnknown;
		return false;
	}

	if (desc.NightTransitionEndHour > desc.NumHours)
	{
		nlwarning ("NightTransitionEndHour (%d) > NumHours (%d)", desc.NightTransitionEndHour, desc.NumHours);
		_State = StateUnknown;
		return false;
	}
	//
	if (desc.DawnTransitionStartHour > desc.NumHours)
	{
		nlwarning ("DawnTransitionStartHour (%d) > NumHours (%d)", desc.DawnTransitionStartHour, desc.NumHours);
		_State = StateUnknown;
		return false;
	}
	if (desc.DawnTransitionEndHour > desc.NumHours)
	{
		nlwarning ("DawnTransitionEndHour (%d) > NumHours (%d)", desc.DawnTransitionEndHour, desc.NumHours);
		_State = StateUnknown;
		return false;
	}
	//
	// test if intervals are not overlapping
	if (!(desc.DawnTransitionStartHour >= desc.NightTransitionEndHour
		  || 	desc.DawnTransitionEndHour <= desc.NightTransitionStartHour
		 )
	   )
	{
		nlwarning("!(DawnTransitionStartHour (%d) >= NightTransitionEndHour (%d) || DawnTransitionEndHour (%d) <= NightTransitionStartHour (%d))",
		          desc.DawnTransitionStartHour, desc.NightTransitionEndHour, desc.DawnTransitionEndHour, desc.NightTransitionStartHour);
		_State = StateUnknown;
		return false;
	}
	//

	//
	_Desc = desc;
	_ValidDesc = true;
	_Hour = 0;
	updateState();
	return true;
}

//-----------------------------------------------
float CLightCycleManager::getUpdateDuration() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	if (!_ValidDesc || _UpdateFreq == 0.f) return 0.f;
	return (1.f / _UpdateFreq) / _Desc.RealDayLength * _Desc.NumHours;
}


//-----------------------------------------------
std::string	CLightCycleManager::getStateString() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	switch(_State)
	{
		case NightToDay: return "night->day";
		case DayToNight: return "day->night";
		case Day: return "day";
		case Night: return "night";
		default:
			return "???";
	}
	return "???";
}

//-----------------------------------------------
void CLightCycleManager::updateState()
{
	H_AUTO_USE(RZ_LightCycleManager)
	_PrevState = _State;
	if(isInInterval(_Desc.DawnTransitionStartHour, _Desc.DawnTransitionEndHour, _Hour))
	{
		// night->day
		_State = NightToDay;
		return;
	}
	else if(isInInterval(_Desc.DawnTransitionEndHour, _Desc.NightTransitionStartHour, _Hour))
	{
		// day
		_State = Day;
		return;
	}
	else if(isInInterval(_Desc.NightTransitionStartHour, _Desc.NightTransitionEndHour, _Hour))
	{
		// day->night
		_State = DayToNight;
		return;
	}
	else if(isInInterval(_Desc.NightTransitionEndHour, _Desc.DawnTransitionStartHour, _Hour))
	{
		// night
		if( _PrevState != Night && _PrevState != StateUnknown)
		{
			CInterfaceManager * pIM = CInterfaceManager::getInstance();
			if( pIM )
			{
				CAHManager::getInstance()->runActionHandler("set",NULL,"dblink=UI:VARIABLES:NIGHT_WARNING_WANTED|value=1");
			}
		}
		_State = Night;
		return;
	}
	_State = StateUnknown;
	return;
}
//

//-----------------------------------------------
bool CLightCycleManager::isInTransition() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	return isInInterval(_Desc.DawnTransitionStartHour, _Desc.DawnTransitionEndHour, _Hour)
		   || isInInterval(_Desc.NightTransitionStartHour, _Desc.DawnTransitionEndHour, _Hour);
}

//-----------------------------------------------
void CLightCycleManager::setHour(float hour, const CWeatherManagerClient &wm, NLMISC::CRGBA lightningColor)
{
	H_AUTO_USE(RZ_LightCycleManager)
	if (!_ValidDesc) return;
	if (hour < 0) hour = 0;
	if (hour > _Desc.NumHours)
	{
		hour -= ::floorf(hour / _Desc.NumHours) * _Desc.NumHours;
	}
	float updateDuration = getUpdateDuration();
	if (fabs(_Hour - hour) >= (4.f * updateDuration))
	{
		touch();
	}
	_Hour = hour;
	_LightLevel = getLightLevel(hour);
	clamp(_LightLevel, 0.f, 1.f); // should not be necessary, but we avoid imprecisions
	updateState();

	_WeatherLighting = wm.getCurrWeatherState().Lighting;

	// Change water lighting
	NL3D::UWaterHeightMapManager::setBlendFactor(Driver, _LightLevel);

	if (_PrevState != _State)
	{
		if (_State == Night || _State == Day)
		{
			// a transiation has ended, so release textures used for the blend
			NL3D::UWaterHeightMapManager::releaseBlendTextures();
		}
	}

	// directionnal lights
	// (override the global light by the light in the weather manager : during bad weather, the scene can be darker)
	setupCanopyLight(_WeatherLighting);
	setupMainLight(_WeatherLighting);

	// landscape lighting
	CRGBA diffuse, ambiant;
	getLandscapeLightColor(diffuse, ambiant);

	bool colorTouched = diffuse != _LastDiffuse || ambiant != _LastAmbient;
	if (colorTouched)
	{
		_LastDiffuse = diffuse;
		_LastAmbient = ambiant;

		// change landscape
		if (Landscape)
		{
			if(ContinentMngr.cur())
				Landscape->setupStaticLight(diffuse, ambiant, ContinentMngr.cur()->StaticLightingFactor[CurrSeason]);
			else
				Landscape->setupStaticLight(diffuse, ambiant, 1.f);
			_UpdateFreq = getLandscapePatchUpdateFreq();
			if (_Touched)
			{
				Landscape->updateLightingAll();
				Landscape->setUpdateLightingFrequency(0);
			}
			else
			{
				Landscape->setUpdateLightingFrequency(_UpdateFreq);
			}
		}
	}
	_Touched = false;


	// Set the Sun color only if not indoor
	if (ContinentMngr.cur()->Indoor)
	{
		Scene->setSunAmbient(CRGBA(150, 150, 150, 255));
	}
	else
	{
		CRGBA color;
		color.add(_LastDiffuse, lightningColor);
		Scene->setLightGroupColor (LightGroupDay, color);
		float nightLevel = _LightLevel*255.f;
		clamp (nightLevel, 0, 255);
		color.set ((uint8)nightLevel, (uint8)nightLevel, (uint8)nightLevel);
		Scene->setLightGroupColor (LightGroupNight, color);
	}

	if (Landscape)
	{
		if (!isInInterval(_Desc.DawnTransitionStartHour, _Desc.DawnTransitionEndHour + 4.f * updateDuration, hour)
			&& !isInInterval(_Desc.NightTransitionStartHour, _Desc.NightTransitionEndHour + 4.f * updateDuration, hour)
		   )
		{
			Landscape->setUpdateLightingFrequency(0);
		}
	}

/*	if (SoundMngr)
	{
		float ratio;
		if(isInDayInterval(_Desc.DawnTransitionStartHour, _Desc.DawnTransitionEndHour, _Desc.NumHours, hour, ratio))
		{
			// night->day
			SoundMngr->setDayNightRatio (1.0f-ratio);
		}
		else if(isInDayInterval(_Desc.DawnTransitionEndHour, _Desc.NightTransitionStartHour, _Desc.NumHours, hour, ratio))
		{
			// day
			SoundMngr->setDayNightRatio (0.0f);
		}
		else if(isInDayInterval(_Desc.NightTransitionStartHour, _Desc.NightTransitionEndHour, _Desc.NumHours, hour, ratio))
		{
			// day->night
			SoundMngr->setDayNightRatio (ratio);
		}
		else if(isInDayInterval(_Desc.NightTransitionEndHour, _Desc.DawnTransitionStartHour, _Desc.NumHours, hour, ratio))
		{
			// night
			SoundMngr->setDayNightRatio (1.0f);
		}
	}
*/
}


//-----------------------------------------------
// NB : interval can be reversed
bool CLightCycleManager::isInInterval(float start, float end, float value)
{
	H_AUTO_USE(RZ_LightCycleManager)
	return  start <= end ? value >= start && value < end
						 : value >= start || value < end;

}

//-----------------------------------------------
// NB : interval can be reversed
bool CLightCycleManager::isInDayInterval(float startHour, float endHour, float dayDuration, float hour, float &ratio)
{
	H_AUTO_USE(RZ_LightCycleManager)
	if (startHour <= endHour)
	{
		if (hour >= startHour && hour < endHour)
		{
			ratio = startHour != endHour ?	(hour - startHour) / (endHour - startHour)
								         :  0;
			return true;
		}
		else
			return false;
	}
	else
	{
		if (hour >= startHour || hour < endHour)
		{
			ratio = hour >= startHour ? (hour - startHour) / (dayDuration - startHour + endHour)
			                          : (hour + dayDuration - startHour) / (dayDuration - startHour + endHour);
			return true;
		}
		else
			return false;
	}
}

//-----------------------------------------------
float CLightCycleManager::getLightLevel(float hour) const
{
	H_AUTO_USE(RZ_LightCycleManager)
	float lightValue = 0.f;
	if (isInDayInterval(_Desc.NightTransitionStartHour, _Desc.NightTransitionEndHour, _Desc.NumHours, hour, lightValue))
		return lightValue;
	if (isInDayInterval(_Desc.DawnTransitionStartHour, _Desc.DawnTransitionEndHour, _Desc.NumHours, hour, lightValue))
		return 1.f - lightValue;

	// No transition, it is night or day
	if (_Desc.DawnTransitionEndHour <= _Desc.NightTransitionStartHour)
	{
		return hour >= _Desc.DawnTransitionEndHour && hour < _Desc.NightTransitionStartHour ? 0.f : 1.f;
	}
	else
	{
		return hour >= _Desc.DawnTransitionEndHour || hour < _Desc.NightTransitionStartHour ? 0.f : 1.f;
	}
}

//-----------------------------------------------
void CLightCycleManager::create()
{
	H_AUTO_USE(RZ_LightCycleManager)
	Driver->enableLight(0, true);
}

//-----------------------------------------------
void CLightCycleManager::setDirLight(const CDirLightSetup &setup0, const CDirLightSetup &setup1, float level, float intensity,NL3D::UScene &scene)
{
	H_AUTO_USE(RZ_LightCycleManager)
	CDirLightSetup resultSetup;
	resultSetup.blend(setup0, setup1, level);
	resultSetup.modulate(intensity);
	//
	scene.setSunAmbient  (resultSetup.Ambiant);
	scene.setSunDiffuse  (resultSetup.Diffuse);
	scene.setSunSpecular (resultSetup.Specular);
	CSky &sky = ContinentMngr.cur()->CurrentSky;
	if (sky.overrideSunDirection())
	{
		scene.setSunDirection(sky.calculateSunDirection());
	}
	else
	{
		scene.setSunDirection(resultSetup.Direction);
	}
}

//-----------------------------------------------
float CLightCycleManager::getLandscapePatchUpdateFreq() const
{
	H_AUTO_USE(RZ_LightCycleManager)
	if(!_ValidDesc)
		return 0.f;

	if(ContinentMngr.cur() == 0)
		return 0.f;

	// transition duration in seconds
	float dt = std::min(_Desc.getDawnTransitionLength(), _Desc.getNightTransitionLength()) * _Desc.RealDayLength / _Desc.NumHours;
	uint numStep = std::max(diffColors(ContinentMngr.cur()->LandscapeLightDay.Diffuse, ContinentMngr.cur()->LandscapeLightNight.Diffuse),
					 	    diffColors(ContinentMngr.cur()->LandscapeLightDay.Ambiant, ContinentMngr.cur()->LandscapeLightNight.Ambiant)
						   );
	numStep = std::min(numStep, _Desc.MaxNumColorSteps);
	return (float) numStep / dt;
}

//-----------------------------------------------
void CLightCycleManager::touch()
{
	H_AUTO_USE(RZ_LightCycleManager)
	_Touched = true;
}


//-----------------------------------------------
void CLightCycleManager::instanceGroupAdded(NL3D::UInstanceGroup * /* ig */)
{
	H_AUTO_USE(RZ_LightCycleManager)
}


//-----------------------------------------------
// rootLight :
// Set the light to render the root.
//-----------------------------------------------
void CLightCycleManager::setupCanopyLight(float intensity)
{
	H_AUTO_USE(RZ_LightCycleManager)
	if(!SceneRoot)
		return;

	if(ContinentMngr.cur())
	{
		switch (_State)
		{
			case DayToNight: // blend form day to night with dusk transition
				setupDayToNightLight(*SceneRoot, ContinentMngr.cur()->RootLightDay, ContinentMngr.cur()->RootLightDusk, ContinentMngr.cur()->RootLightNight, intensity);
			break;
			default: // blend from night to day with no other transition
				setDirLight(ContinentMngr.cur()->RootLightDay, ContinentMngr.cur()->RootLightNight, _LightLevel, intensity, *SceneRoot);
			break;
		}
	}
}// setupCanopyLight //

//-----------------------------------------------
// mainLight :
// Set the light to render the main scene
//-----------------------------------------------
void CLightCycleManager::setupMainLight(float intensity)
{
	H_AUTO_USE(RZ_LightCycleManager)
	if(!Scene)
		return;
	if(ContinentMngr.cur())
	{
		switch (_State)
		{
			case DayToNight: // blend form day to night with dusk transition
				setupDayToNightLight(*Scene, ContinentMngr.cur()->EntityLightDay, ContinentMngr.cur()->EntityLightDusk, ContinentMngr.cur()->EntityLightNight, intensity);
			break;
			default: // blend from night to day with no other transition
				setDirLight(ContinentMngr.cur()->EntityLightDay, ContinentMngr.cur()->EntityLightNight, _LightLevel, intensity, *Scene);
			break;
		}
	}
}// setupMainLight //

//-----------------------------------------------
void CLightCycleManager::setupDayToNightLight(NL3D::UScene &scene, const CDirLightSetup &dayLight, const CDirLightSetup &duskLight, const CDirLightSetup &nightLight, float lightIntensity)
{
	H_AUTO_USE(RZ_LightCycleManager)
	float blendFactor;
	if (_LightLevel <= _Desc.DuskRatio)
	{
		blendFactor = _Desc.DuskRatio != 0 ? _LightLevel / _Desc.DuskRatio : 0.f;
		setDirLight(dayLight,
				    duskLight,
					blendFactor,
					lightIntensity,
					scene
			   );
	}
	else
	{
		blendFactor = _Desc.DuskRatio != 1.f ? (_LightLevel -  _Desc.DuskRatio) / (1.f - _Desc.DuskRatio) : 0.f;
		setDirLight(duskLight,
				    nightLight,
					blendFactor,
					lightIntensity,
					scene
			   );
	}
}
