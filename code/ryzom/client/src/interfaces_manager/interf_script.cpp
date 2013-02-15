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
// Include //
/////////////
#include "interf_script.h"


//////////////
// Function //
//////////////
//-----------------------------------------------
// getFloat :
// Get a "float" in the script.
//-----------------------------------------------
float getFloat()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		return (float) atof(ptr);
	return 0.f;
}// getFloat //

//-----------------------------------------------
// getInt :
// Get an "int" in the script.
//-----------------------------------------------
sint getInt()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		return atoi(ptr);
	return 0;
}// getInt //

//-----------------------------------------------
// getRGBA :
// Get all value for a RGBA in the script.
//-----------------------------------------------
CRGBA getRGBA()
{
	char delimiter[] = "[] \t";
	uint8 rgba[4]   = {255,255,255,255};
	char *ptr;

	for(uint rgbaIndex = 0; rgbaIndex<4; ++rgbaIndex)
	{
		ptr = strtok(NULL, delimiter);
		if(ptr != NULL)
			rgba[rgbaIndex] = atoi(ptr);
		else
			break;
	}
	return CRGBA(rgba[0], rgba[1], rgba[2], rgba[3]);
}// getRGBA //



//-----------------------------------------------
// getVector :
// Get all values for a vector in the script.
//-----------------------------------------------
std::vector<float> getVectorOfFloat(uint8 nbCol)
{
	std::vector<float> vect;
	float val;
	char delimiter[] = "[] \t";

	char *ptr;

	for (uint8 i = 0 ; i< nbCol ; ++i)
	{
		ptr = strtok(NULL, delimiter);
		if(ptr != NULL)
		{
			val = (float)atof(ptr);
			if (val != 0.0f)
				vect.push_back(val);
		}
		else
			break;
	};


	return vect;
}// getVector //


//-----------------------------------------------
// getHotSpot :
// Get the Hot Spot of a control.
//-----------------------------------------------
CControl::THotSpot getHotSpot()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if     (strcmp(ptr, "TL") == 0)
			return CControl::HS_TL;
		else if(strcmp(ptr, "TM") == 0)
			return CControl::HS_TM;
		else if(strcmp(ptr, "TR") == 0)
			return CControl::HS_TR;
		else if(strcmp(ptr, "ML") == 0)
			return CControl::HS_ML;
		else if(strcmp(ptr, "MR") == 0)
			return CControl::HS_MR;
		else if(strcmp(ptr, "BL") == 0)
			return CControl::HS_BL;
		else if(strcmp(ptr, "BM") == 0)
			return CControl::HS_BM;
		else if(strcmp(ptr, "BR") == 0)
			return CControl::HS_BR;
	}

	// Return Middle Middle if it's middle middle or unknown.
	return CControl::HS_MM;
}// getHotSpot //

//-----------------------------------------------
// getBGMode :
// Get the display mode for the background.
//-----------------------------------------------
COSD::TBG getBGMode()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if     (strcmp(ptr, "none") == 0)
			return COSD::BG_none;
		else if(strcmp(ptr, "plain") == 0)
			return COSD::BG_plain;
		else if(strcmp(ptr, "stretch") == 0)
			return COSD::BG_stretch;
	}

	// Default is no background.
	return COSD::BG_none;
}// getBGMode //

//-----------------------------------------------
// getBGMode2 :
// Get the display mode for the background.
//-----------------------------------------------
CButtonBase::TBG getBGMode2()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if     (strcmp(ptr, "none") == 0)
			return CButtonBase::BG_none;
		else if(strcmp(ptr, "plain") == 0)
			return CButtonBase::BG_plain;
		else if(strcmp(ptr, "stretch") == 0)
			return CButtonBase::BG_stretch;
	}

	// Default is no background.
	return CButtonBase::BG_none;
}// getBGMode2 //

//-----------------------------------------------
// getTBMode :
// Get the display mode for the Title Bar.
//-----------------------------------------------
COSD::TTB getTBMode()
{
	char delimiter[] = "[] \t";
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if     (strcmp(ptr, "none") == 0)
			return COSD::TB_none;
		else if(strcmp(ptr, "plain") == 0)
			return COSD::TB_plain;
		else if(strcmp(ptr, "stretch") == 0)
			return COSD::TB_stretch;
	}

	// Default is no background.
	return COSD::TB_none;
}// getTBMode //
