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
#include "weather_setup_sheet_base.h"
//
#include "nel/misc/path.h"
//
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"

//=====================================================================================================================
/**Tool fct to extract a value from a sheet, and to display a warning if it failed.
  */
template <class T> static void GetWeatherFormValue(const NLGEORGES::UFormElm &item, T &destItem, const char *name)
{
	nlassert(name);
	if (!item.getValueByName(destItem, name)) nlwarning("WeatherSetup : can't get %s value, keeping default", name);
}


//==================================================================================
CWeatherStateSheet::CWeatherStateSheet()	: 	FogRatio(0),
												FogColorDay(NLMISC::CRGBA::Black),
												FogColorDusk(NLMISC::CRGBA::Black),
												FogColorNight(NLMISC::CRGBA::Black),
												FogGradientFactor(1.f),
												Lighting(1.f),
												WindIntensity(0),
												ThunderIntensity(0)

{
	for(uint k = 0; k < NumFogType; ++k)
	{
		FogNear[k] = FogFar[k] = 0.f;
	}
}



//=====================================================================================================================
void CWeatherStateSheet::build(const NLGEORGES::UFormElm &item)
{
	// Setup name for blending aproximation
	GetWeatherFormValue(item, BestSetupName, "SetupName");

	// Fog (main & canopy)
	GetWeatherFormValue(item, FogRatio, "FogRatio");
	NLMISC::clamp(FogRatio, 0.f, 1.f);
	GetWeatherFormValue(item, FogColorDay, "FogColorDay");
	GetWeatherFormValue(item, FogColorDusk, "FogColorDusk");
	GetWeatherFormValue(item, FogColorNight, "FogColorNight");
	GetWeatherFormValue(item, FogNear[0], "FogNear");
	GetWeatherFormValue(item, FogFar[0], "FogFar");
	GetWeatherFormValue(item, FogNear[1], "FogCanopyNear");
	GetWeatherFormValue(item, FogFar[1], "FogCanopyFar");
	GetWeatherFormValue(item, FogGradientFactor, "FogGradientFactor");
	NLMISC::clamp(FogRatio, 0.f, 1.f);
	//
	GetWeatherFormValue(item, Lighting, "Lighting");
	NLMISC::clamp(Lighting, 0.f, 1.f);
	// Bckgnd
	GetWeatherFormValue(item, DayBackground, "WeatherBackgroundDay");
	GetWeatherFormValue(item, NightBackground, "WeatherBackgroundNight");
	GetWeatherFormValue(item, DuskBackground, "WeatherBackgroundDusk");
	GetWeatherFormValue(item, WindIntensity, "WindIntensity");
	NLMISC::clamp(WindIntensity, 0.f, 1.f);
	// FX (form contains a single FX for now)
	std::string fxName;
	GetWeatherFormValue(item, fxName, "FXName");
	if (!fxName.empty())
	{
		fxName = NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(fxName));
		if (!fxName.empty())
		{
			FXInfos.resize(1);
			FXInfos[0].Name = fxName;
			GetWeatherFormValue(item, FXInfos[0].Ratio, "FXRatio");
			NLMISC::clamp(FXInfos[0].Ratio, 0.f, 1.f);
		}
		else
		{
			nlwarning("Invalid FX name");
		}
	}
	// Thunder intensity & color
	GetWeatherFormValue(item, ThunderIntensity, "ThunderIntensity");
	NLMISC::clamp(ThunderIntensity, 0.f, 1.f);
	GetWeatherFormValue(item, ThunderColor, "ThunderColor");
	GetWeatherFormValue(item, LocalizedName, "LocalizedName");
}



//==================================================================================
void CWeatherStateSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(BestSetupName);
	f.serial(FogRatio);
	f.serial(FogColorDay);
	f.serial(FogColorDusk);
	f.serial(FogColorNight);
	for(uint k = 0; k < NumFogType; ++k)
	{
		f.serial(FogNear[k], FogFar[k]);
	}
	f.serial(FogGradientFactor);
	//
	f.serial(Lighting);
	//
	f.serial(DayBackground);
	f.serial(DuskBackground);
	f.serial(NightBackground);
	//
	f.serial(WindIntensity);
	//
	f.serial(ThunderColor);
	f.serial(ThunderIntensity);
	//
	f.serialCont(FXInfos);
	f.serial(LocalizedName);
}

//==================================================================================
CCloudStateSheet::CCloudStateSheet() : AmbientDay(120, 150, 155),
									 DiffuseDay(220, 250, 255),
									 AmbientNight(60, 75, 125),
									 DiffuseNight(110, 125, 200),
									 AmbientDusk(210, 116, 21),
									 DiffuseDusk(234, 183, 77),
									 NumClouds(50),
									 DiffusionSpeed(8.f)
{
}

//==================================================================================
void CCloudStateSheet::build(const NLGEORGES::UFormElm &item)
{
	GetWeatherFormValue(item, AmbientDay, "CloudsAmbientDay");
	GetWeatherFormValue(item, DiffuseDay, "CloudsDiffuseDay");
	GetWeatherFormValue(item, AmbientNight, "CloudsAmbientNight");
	GetWeatherFormValue(item, DiffuseNight, "CloudsDiffuseNight");
	GetWeatherFormValue(item, AmbientDusk, "CloudsAmbientDusk");
	GetWeatherFormValue(item, DiffuseDusk, "CloudsDiffuseDusk");
	GetWeatherFormValue(item, NumClouds, "CloudsNumber");
	GetWeatherFormValue(item, DiffusionSpeed, "CloudsDiffusionSpeed");
	NLMISC::clamp(NumClouds, 0u, 255u);
}

//==================================================================================
void CCloudStateSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(AmbientDay);
	f.serial(DiffuseDay);
	f.serial(AmbientNight);
	f.serial(DiffuseNight);
	f.serial(AmbientDusk);
	f.serial(DiffuseDusk);
	f.serial(NumClouds);
	f.serial(DiffusionSpeed);
}

//==================================================================================
CWeatherSetupSheetBase::CWeatherSetupSheetBase()
{
}

void CWeatherSetupSheetBase::readGeorges (const NLGEORGES::UForm *form, const NLMISC::CSheetId &/* sheetId */)
{
	build( form->getRootNode() );
}

//==================================================================================
void CWeatherSetupSheetBase::build(const NLGEORGES::UFormElm &item)
{
	WeatherState.build(item);
	CloudState.build(item);
	std::string setupName;
	GetWeatherFormValue(item, setupName, "SetupName");
	SetupName = NLMISC::CStringMapper::map(NLMISC::strlwr(setupName));
}

//==================================================================================
void CWeatherSetupSheetBase::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(WeatherState, CloudState);
	if (f.isReading())
	{
		std::string setupName;
		f.serial(setupName);
		SetupName = NLMISC::CStringMapper::map(NLMISC::strlwr(setupName));
	}
	else
	{
		std::string setupName = NLMISC::CStringMapper::unmap(SetupName);
		f.serial(setupName);
	}
}
