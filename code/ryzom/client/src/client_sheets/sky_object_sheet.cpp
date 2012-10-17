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
#include "sky_object_sheet.h"

using namespace NLMISC;

/////////////////////////////////
// CSkyObjectSheet::CColorInfo //
/////////////////////////////////

// *****************************************************************************************************
void CSkyObjectSheet::CColorInfoSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	item.getValueByName(MapName, (prefix + "MapName").c_str());
	uint32 mode;
	item.getValueByName(mode, (prefix + "ColorMode").c_str());
	Mode = (TSkyColorMode) mode;
}

// *****************************************************************************************************
void CSkyObjectSheet::CColorInfoSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(MapName);
	f.serialEnum(Mode);
}

/////////////////////////////////////////
// CSkyObjectSheet::CColorGradientInfo //
/////////////////////////////////////////

// *****************************************************************************************************
void CSkyObjectSheet::CColorGradientInfoSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	item.getValueByName(TargetTextureStage, (prefix + "TargetTextureStage").c_str());
	const NLGEORGES::UFormElm *elm = NULL;
	if(item.getNodeByName (&elm, (prefix + "WeatherToGradient").c_str()) && elm)
	{
		uint numBitmaps;
		nlverify (elm->getArraySize (numBitmaps));
		WeatherToGradient.resize(numBitmaps);
		// For each sky object
		for(uint k = 0; k < numBitmaps; ++k)
		{
			elm->getArrayValue(WeatherToGradient[k], k);
		}
	}
}

// *****************************************************************************************************
void CSkyObjectSheet::CColorGradientInfoSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(TargetTextureStage);
	f.serialCont(WeatherToGradient);
}

///////////////////////////////
// CSkyObjectSheet::CVersion //
///////////////////////////////

// *****************************************************************************************************
void CSkyObjectSheet::CVersionSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	item.getValueByName(ShapeName, (prefix + "ShapeName").c_str());
	item.getValueByName(TransparencyPriority, (prefix + "TransparencyPriority").c_str());
	DiffuseColor.build(item, prefix + "DiffuseColor.");
	ParticleEmitters.build(item, prefix + "ParticleEmitters.");
	for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
	{
		// build constant color infos
		ConstantColor[k].build(item, prefix + toString("ConstantColor%d.", (int) k));
		// build tex panner
		item.getValueByName(TexPanner[k].U, (prefix + toString("PannerU%d", k)).c_str());
		item.getValueByName(TexPanner[k].V, (prefix + toString("PannerV%d", k)).c_str());
		// texture offset
		item.getValueByName(OffsetFactor[k].U, (prefix + toString("OffsetFactorU%d", k)).c_str());
		item.getValueByName(OffsetFactor[k].V, (prefix + toString("OffsetFactorV%d", k)).c_str());
		// texture scaling depending on weather & hour
		item.getValueByName(OffsetUBitmap[k], (prefix + toString("OffsetUBitmap%d", k)).c_str());
		item.getValueByName(OffsetVBitmap[k], (prefix + toString("OffsetVBitmap%d", k)).c_str());
	}
	uint32 refColor = 0;
	item.getValueByName(refColor, (prefix + "RefColor").c_str());
	RefColor = (TSkyRefColor) refColor;
	ColorGradient.build(item, prefix + "ColorGradient.");
	for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
	{
		item.getValueByName(FXUserParamBitmap[k], (prefix + toString("UserParam%d", (int) k)).c_str());
	}
}

// *****************************************************************************************************
void CSkyObjectSheet::CVersionSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(ShapeName);
	f.serial(TransparencyPriority);
	f.serial(DiffuseColor);
	f.serial(ParticleEmitters);
	for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
	{
		f.serial(ConstantColor[k]);
		f.serial(TexPanner[k]);
		f.serial(OffsetFactor[k]);
		f.serial(FXUserParamBitmap[k]);
		f.serial(OffsetUBitmap[k]);
		f.serial(OffsetVBitmap[k]);
	}
	f.serialEnum(RefColor);
	f.serial(ColorGradient);
}

/////////////////////
// CSkyObjectSheet //
/////////////////////

// *****************************************************************************************************
CSkyObjectSheet::CSkyObjectSheet()
{
	VisibleInMainScene = true;
	VisibleInEnvMap = true;
}

// *****************************************************************************************************
void CSkyObjectSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	Std.build(item, prefix + "StdVersion.");
	FallbackPass[0].build(item, prefix + "FallbackPass0.");
	FallbackPass[1].build(item, prefix + "FallbackPass1.");
	item.getValueByName(VisibleInMainScene, (prefix + "VisibleInMainScene").c_str());
	item.getValueByName(VisibleInEnvMap, (prefix + "VisibleInEnvMap").c_str());
}

// *****************************************************************************************************
void CSkyObjectSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Std);
	f.serial(FallbackPass[0]);
	f.serial(FallbackPass[1]);
	f.serial(VisibleInMainScene);
	f.serial(VisibleInEnvMap);
}
