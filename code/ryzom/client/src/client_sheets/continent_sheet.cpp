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
#include "continent_sheet.h"
#include "game_share/georges_helper.h"
//
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"
//
#include "nel/misc/smart_ptr.h"

using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;

//=========================================================================
CContinentParameters::CContinentParameters()
{
	Indoor = false;

	EntitySunContributionPower = 0.0f;
	EntitySunContributionMaxThreshold = 0.0f;
	LandscapePointLightMaterial.set( 0, 0, 0, 255 );

	FogStart = 0.0f;
	FogEnd = 0.0f;
	RootFogStart = 0.0f,
	RootFogEnd = 0.0f;

	for( uint32 i = 0; i < EGSPD::CSeason::Invalid; ++i )
	{
		TileColorMono[i] = false;
		TileColorFactor[i] = 0.0f;
		StaticLightingFactor[i] = 0.0f;
		ForceDisplayedSeason[i] = (EGSPD::CSeason::TSeason)i;
	}
}

//=========================================================================
CContinentSheet::CContinentSheet()
{
	Type = CEntitySheet::CONTINENT;
}

//=========================================================================
void CContinentParameters::build(const NLGEORGES::UFormElm &item)
{
	// Name.
	item.getValueByName (Name, "Name");

	// WorldMap
	item.getValueByName (WorldMap, "WorldMap");

	//Zone min/max
	item.getValueByName (ZoneMin, "ZoneMin");
	item.getValueByName (ZoneMax, "ZoneMax");

	// Load Light Landscape Day.
	const UFormElm *elm;
	if (item.getNodeByName (&elm, "LightLandscapeDay") && elm)
		LandscapeLightDay.build(*elm);

	// Load Light Landscape Night.
	if (item.getNodeByName (&elm, "LightLandscapeNight") && elm)
		LandscapeLightNight.build(*elm);

	// Load Light Landscape Dusk.
	if (item.getNodeByName (&elm, "LightLandscapeDusk") && elm)
	{
		if (!LandscapeLightDusk.build(*elm)) LandscapeLightDusk.blend(LandscapeLightDay, LandscapeLightNight, 0.5f);
	}
	else LandscapeLightDusk.blend(LandscapeLightDay, LandscapeLightNight, 0.5f);


	// Load Landscape PointLightMaterial
	if (item.getNodeByName (&elm, "LandscapePointLightMaterial") && elm)
		CGeorgesHelper::convert (LandscapePointLightMaterial, *elm);
	else
		LandscapePointLightMaterial= CRGBA::White;

	// Load Light Entity Day.
	if (item.getNodeByName (&elm, "LightEntityDay") && elm)
		EntityLightDay.build(*elm);


	// Load Light Entity Night.
	if (item.getNodeByName (&elm, "LightEntityNight") && elm)
		EntityLightNight.build(*elm);

	// Load Light Entity Dusk
	if (item.getNodeByName (&elm, "LightEntityDusk") && elm)
	{
		if (!EntityLightDusk.build(*elm)) EntityLightDusk.blend(EntityLightDay, EntityLightNight, 0.5f);
	}
	else EntityLightDusk.blend(EntityLightDay, EntityLightNight, 0.5f);

	// Load Light Root Day.
	if (item.getNodeByName (&elm, "LightRootDay") && elm)
		RootLightDay.build(*elm);

	// Load Light Root Night.
	if (item.getNodeByName (&elm, "LightRootNight") && elm)
		RootLightNight.build(*elm);

	// Load Light Root Dusk
	if (item.getNodeByName (&elm, "LightRootDusk") && elm)
	{
		if (!RootLightDusk.build(*elm)) RootLightDusk.blend(RootLightDay, RootLightNight, 0.5f);
	}
	else RootLightDusk.blend(RootLightDay, RootLightNight, 0.5f);


	// Load Entity sun contribution power.
	item.getValueByName (EntitySunContributionPower, "EntitySunPower");

	// Load Entity sun contribution max threshold.
	item.getValueByName (EntitySunContributionMaxThreshold, "EntitySunMaxThreshold");

	// Load PACS RBank.
	item.getValueByName (PacsRBank, "PacsRBank");

	// Load PACS RBank.
	item.getValueByName (PacsGR, "PacsGR");

	// Load IG list filename.
	item.getValueByName (LandscapeIG, "LandscapeIG");

	// Load Sky day filename.
	item.getValueByName (SkyDay, "SkyDay");

	// Load Sky night filename.
	item.getValueByName (SkyNight, "SkyNight");

	// Load Sky fog part filename.
	item.getValueByName (SkyFogPartName, "SkyFogPart");

	// Load Sky ig filename.
	item.getValueByName (BackgroundIGName, "SkyIg");

	// Load IG for canopy for each season
	for (uint season = 0; season < EGSPD::CSeason::Invalid; ++season)
	{
		item.getValueByName(CanopyIGfileName[season], (EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season) + "CanopyIG").c_str());
	}

	// Load the form
	string filename;
	if (item.getValueByName (filename, "Ecosystem"))
	{
		UFormLoader *loader = UFormLoader::createLoader();
		if (loader)
		{
			// Load the form
			CSmartPtr<UForm> form = loader->loadForm (filename.c_str ());

			// Form loaded ?
			if (form)
			{
				// Root node
				elm = &form->getRootNode ();

				// MicroVeget.
				elm->getValueByName (MicroVeget, "MicroVeget");

				// Small bank.
				elm->getValueByName (SmallBank, "SmallBank");

				// Far bank.
				elm->getValueByName (FarBank, "FarBank");

				// Coarse mesh texture.
				elm->getValueByName (CoarseMeshMap, "CoarseMeshMap");
			}
			else
			{
				nlwarning("CContinent::build : Can't load form %s.", filename.c_str());
			}
			UFormLoader::releaseLoader(loader);
		}
	}

	// Load all "Zone Constructible".
	if(item.getNodeByName(&elm, "ZCs") && elm)
	{
		// Get number of ZC
		uint nbZC;
		if(elm->getArraySize(nbZC))
		{
			// For each village
			for(uint i = 0; i < nbZC; ++i)
			{
				// Village pointer
				const UFormElm *zc;
				if(elm->getArrayNode(&zc, i) && zc)
				{
					// Get the zone associated.
					CZC zone;
					zone.EnableRuins= true;
					if(!zc->getValueByName(zone.Name, "Zone"))
						nlwarning("CContinent::build : key 'Zone' not found.");
					if(!zc->getValueByName(zone.ForceLoadDist, "ForceLoadDist"))
						nlwarning("CContinent::build : key 'ForceLoadDist' not found.");
					if(!zc->getValueByName(zone.LoadDist, "LoadDist"))
						nlwarning("CContinent::build : key 'LoadDist' not found.");
					if(!zc->getValueByName(zone.UnloadDist, "UnloadDist"))
						nlwarning("CContinent::build : key 'UnloadDist' not found.");
					if(!zc->getValueByName(zone.EnableRuins, "EnableRuins"))
						nlwarning("CContinent::build : key 'EnableRuins' not found.");

					ZCList.push_back(zone);
				}
			}
		}
		else
			nlwarning("CContinent::build : it seems 'ZCs' is not an array.");
	}

	// build fog map descriptor
	buildFogMapBuild(item);

	// Load Sky ig filename.
	item.getValueByName (Indoor, "Indoor");


	// Load the landmark list
	/*
	if(item.getNodeByName (&elm, "LandMarks") && elm)
	{
		// Get number of village
		uint numLandMarks;
		nlverify (elm->getArraySize (numLandMarks));
		LandMarks.resize(numLandMarks);

		// For each village
		for(uint k = 0; k < numLandMarks; ++k)
		{
			// Village pointer
			const UFormElm *landMarkForm;
			if (elm->getArrayNode (&landMarkForm, k) && landMarkForm)
			{
				LandMarks[k].build(*landMarkForm);
			}
		}
	}*/

	// LocalizedName
	item.getValueByName (LocalizedName, "LocalizedName");

	// Micro-life primitives
	if(item.getNodeByName(&elm, "MicroLifePrimitives") && elm)
	{
		// Get number of prims
		uint numPrims;
		if(elm->getArraySize(numPrims))
		{
			// For each prims
			for(uint i = 0; i < numPrims; ++i)
			{
				std::string primFilename;
				if (elm->getArrayValue(primFilename, i))
				{
					MicroLifeZones.push_back(primFilename);
				}
			}
		}
		else
			nlwarning("CContinent::build : it seems 'ZCs' is not an array.");
	}

	// Read Season related parameters
	for (uint season = 0; season < EGSPD::CSeason::Invalid; ++season)
	{
		item.getValueByName(TileColorMono[season], (EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season) + "TileColorMono").c_str());
		item.getValueByName(TileColorFactor[season], (EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season ) + "TileColorFactor").c_str());
		item.getValueByName(StaticLightingFactor[season], (EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season ) + "StaticLightingFactor").c_str());
		item.getValueByName(SkySheet[season],  ("SkySheet" + EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season )).c_str());

		// A continent may want to force some season instead another. read it
		ForceDisplayedSeason[season]= (EGSPD::CSeason::TSeason)season;
		string	strSeason;
		item.getValueByName(strSeason, ("DisplayedSeasonFor" + EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason)season)).c_str());
		EGSPD::CSeason::TSeason		readSeason= ForceDisplayedSeason[season]= EGSPD::CSeason::fromString(strSeason);
		if(readSeason<EGSPD::CSeason::Invalid)
			ForceDisplayedSeason[season]= readSeason;
	}
}


void CContinentParameters::buildFogMapBuild(const NLGEORGES::UFormElm &item)
{
	item.getValueByName (FogMapBuild.Map[CFogMapBuild::Day], "FogDayMap");
	item.getValueByName (FogMapBuild.Map[CFogMapBuild::Night], "FogNightMap");
	item.getValueByName (FogMapBuild.Map[CFogMapBuild::Distance], "FogDistMap");
	item.getValueByName (FogMapBuild.Map[CFogMapBuild::Depth], "FogDepthMap");
	item.getValueByName (FogMapBuild.Map[CFogMapBuild::NoPrecipitation], "NoRainMap");
	item.getValueByName (FogStart, "FogStart");
	item.getValueByName (FogEnd, "FogEnd");
	item.getValueByName (FogMapBuild.ZoneMin, "ZoneMin");
	item.getValueByName (FogMapBuild.ZoneMax, "ZoneMax");
	if (FogStart > FogEnd)
	{
		std::swap(FogStart, FogEnd);
	}
	if (!item.getValueByName (RootFogStart, "RootFogStart"))
	{
		RootFogStart = FogStart;
	}
	if (!item.getValueByName (RootFogEnd,   "RootFogEnd"))
	{
		RootFogEnd = FogEnd;
	}
	if (RootFogStart > RootFogEnd)
	{
		std::swap(RootFogStart, RootFogEnd);
	}
}


//=========================================================================
void CContinentParameters::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Name);
	f.serial(PacsRBank);
	f.serial(PacsGR);
	f.serial(LandscapeIG);
	f.serial(SkyDay);
	f.serial(SkyNight);
	f.serial(SkyFogPartName);
	f.serial(BackgroundIGName);
	//
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		f.serial(CanopyIGfileName[k]);
	}
	//
	f.serial(MicroVeget);
	f.serial(SmallBank);
	f.serial(FarBank);
	f.serial(CoarseMeshMap);
	f.serial(EntitySunContributionPower);
	f.serial(EntitySunContributionMaxThreshold);
	f.serial(LandscapeLightDay);
	f.serial(LandscapeLightDusk);
	f.serial(LandscapeLightNight);
	f.serial(LandscapePointLightMaterial);
	f.serial(EntityLightDay);
	f.serial(EntityLightDusk);
	f.serial(EntityLightNight);
	f.serial(RootLightDay);
	f.serial(RootLightDusk);
	f.serial(RootLightNight);
	//
	f.serialCont(ZCList);
	//
	f.serial(FogMapBuild);
	f.serial(FogStart);
	f.serial(FogEnd);
	f.serial(RootFogStart);
	f.serial(RootFogEnd);
	f.serial(Indoor);
	//
	f.serial(WorldMap);
	//f.serialCont(LandMarks);
	f.serial(LocalizedName);
	//
	f.serialCont(MicroLifeZones);
	//
	f.serial(ZoneMin);
	f.serial(ZoneMax);
	//
	for (uint i = 0; i < EGSPD::CSeason::Invalid; ++i)
	{
		f.serial(TileColorMono[i]);
		f.serial(TileColorFactor[i]);
		f.serial(StaticLightingFactor[i]);
		f.serial(SkySheet[i]);
		f.serialEnum(ForceDisplayedSeason[i]);
	}
}

//=========================================================================

void CContinentParameters::CZC::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial (Name);
	f.serial (ForceLoadDist);
	f.serial (LoadDist);
	f.serial (UnloadDist);
	f.serial (EnableRuins);
}

//=========================================================================
void CContinentSheet::build(const NLGEORGES::UFormElm &item)
{
	Continent.build(item);
	const UFormElm *elm;
	// Load the village list
	if(item.getNodeByName (&elm, "Villages") && elm)
	{
		// Get number of village
		uint numVillage;
		nlverify (elm->getArraySize (numVillage));
		Villages.resize(numVillage);

		// For each village
		for(uint k = 0; k < numVillage; ++k)
		{
			// Village pointer
			const UFormElm *villageForm;
			if (elm->getArrayNode (&villageForm, k) && villageForm)
			{
				Villages[k].build(*villageForm);
				elm->getArrayNodeName(Villages[k].Name, k);
			}
		}
	}
	// load the weather functions
	// Build season descriptor
	static const char *seasonFuncName[] =
	{
		"SpringWeatherFunction",
		"SummerWeatherFunction",
		"AutumnWeatherFunction",
		"WinterWeatherFunction"
	};
	// added - 1 because there is an invalid season
	nlctassert(sizeof(seasonFuncName) / sizeof(seasonFuncName[0]) == EGSPD::CSeason::Invalid );

	// Load weather functions & sky sheets
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		const NLGEORGES::UFormElm *elm;
		if (item.getNodeByName(&elm, seasonFuncName[k]) && elm)
		{
			WeatherFunction[k].build(*elm);
		}
	}

}

//=========================================================================
void CContinentSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Continent);
	f.serialCont(Villages);
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		f.serial(WeatherFunction[k]);
	}
}

//=========================================================================
/*void CLandMark::build(const NLGEORGES::UFormElm &item)
{
	item.getValueByName(Pos.x, "X");
	item.getValueByName(Pos.y, "Y");
	item.getValueByName(TitleTextID, "Name");
}

//=========================================================================
void CLandMark::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Pos, TitleTextID);
}

*/
