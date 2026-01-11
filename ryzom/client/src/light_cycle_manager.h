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



#ifndef CL_LIGHT_CYCLE_MANAGER_H
#define CL_LIGHT_CYCLE_MANAGER_H

/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_light.h"

#include "game_share/dir_light_setup.h"
#include "ig_enum.h"

///////////
// USING //
///////////

namespace NL3D
{
	class UScene;
}


/// Description of the hours for days / night
struct CLightCycleDesc
{
	float RealDayLength; // real length of the day, in seconds
	float NumHours;  // number of ryzom hours in a day
	float NightTransitionStartHour; // start of night transition
	float NightTransitionEndHour;   // end   of night transition
	float DuskRatio;                // ratio for dusk transition. e.g. when set to 0.5f, that means that the dusk colors are taken when the day->night transition is at its half
	float DawnTransitionStartHour;  // start of dawn transition
	float DawnTransitionEndHour;	// end of dawn transition
	uint  MaxNumColorSteps;			// the max number of color steps
	//
	float getNightTransitionLength() const;
	float getDawnTransitionLength() const;
};


class CWeatherManagerClient;

/**
 * This class manage the weather and the time.
 * \author Guillaume PUZIN
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CLightCycleManager : public IIGObserver
{
public:
	enum TLightState { Day = 0, Night, DayToNight, NightToDay, StateUnknown };
public:
	/// ctor
	CLightCycleManager();
	/** Set the description of the light cycle
	  * This also reset the hour to 0
	  * \param desc description of lighting intervals
	  * \return true If the desc was valid (not overlapping transitions..)
	  */
	bool	setLightDesc(const CLightCycleDesc &desc);
	// get the current light desc
	const CLightCycleDesc &getLightDesc() const { return _Desc;	}
	/** Set the current hour, and change color if necessary.
	  * The caller can provide an additionnal lightning color if he want
	  * NB : the weather manager is needed if the lighting needs to be overriden in case of bad weather
	  */
	void	setHour(float hour, const CWeatherManagerClient &wm, NLMISC::CRGBA lightningColor);
	//
	float	getHour() const { return _Hour; }
	// Get the last computed light level in a call to setHour (from 0 fro day to 1 for night)
	float   getLightLevel() const { return _LightLevel; }
	/// Create driver directionnal light light
	void	create();
	/** Touch the light setup.
	  * The landscape patch are recomputed
	  */
	void	touch();
	/// Get needed landscape update freq (or 0 if the setupped lightDesc was not valid)
	float	getLandscapePatchUpdateFreq() const;

	/// Returns a string that describ the current state (day, night, ...)
	std::string	getStateString() const;

	// Returns an enum that describe the current state
	TLightState getState() const
	{
		return _State;
	}

	/// Test if a day->night or night->day transition is currently occuring
	bool		isInTransition() const;

	/// From IIGObserver
	virtual void instanceGroupLoaded(NL3D::UInstanceGroup * /* ig */) { }
	virtual void instanceGroupAdded(NL3D::UInstanceGroup *ig);
	virtual void instanceGroupRemoved(NL3D::UInstanceGroup * /* ig */) { }

	/// get the light level at the given hour
	float			getLightLevel(float hour) const;



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
private:
	float			_Hour;
	bool			_Touched;
	bool			_ValidDesc;
	NLMISC::CRGBA	_LastDiffuse, _LastAmbient;
	CLightCycleDesc _Desc;
	float			_LightLevel;
	float			_WeatherLighting, _LastWeatherLighting;
	float			_UpdateFreq;
	TLightState     _State;
	TLightState     _PrevState;
private:
	void			getLandscapeLightColor(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &ambiant);
	static  bool	isInInterval(float start, float end, float value);
	static  bool	isInDayInterval(float startHour, float endHour, float dayDuration, float hour, float &ratio);
	void			setDirLight(const CDirLightSetup &setup0, const CDirLightSetup &setup1, float level, float intensity, NL3D::UScene &scene);
	// get the number of hours for a landscape patch update
	float			getUpdateDuration() const;
	/// Set the light to render the canopy or the sky
	void			setupCanopyLight(float intensity);
	/// Set the light to render the main scene
	void			setupMainLight(float intensity);
	// Setup light for canopy or main scene
	void            setupDayToNightLight(NL3D::UScene &scene, const CDirLightSetup &dayLight, const CDirLightSetup &duskLight, const CDirLightSetup &nightLight, float lightIntensity);
	// Update the '_State' field
	void            updateState();
};


#endif // LIGHT_CYCLE_MANAGER

/* End of weather.h */
