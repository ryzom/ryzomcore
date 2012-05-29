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

#include "color_modifier.h"
#include <nel/misc/bitmap.h>


/** Calc new value of a component after luminosity and contrast have been applied.
  * As with photoshop, we deal we a contrast that goes from -100 to 100
  */
static uint8 inline CalcBrightnessContrast(uint8 intensity, float luminosity, float contrast, uint8 meanGrey)
{
	float fContrast = 0.01f * (contrast + 100.f);
	float result = luminosity + (float) meanGrey + fContrast * ((float) intensity - (float) meanGrey);
	NLMISC::clamp(result, 0, 255);
	return (uint8) result;
}

///=================================================================================================
void CColorModifier::convertBitmap(NLMISC::CBitmap &destBitmap, const NLMISC::CBitmap &srcBitmap, 
	const NLMISC::CBitmap &maskBitmap, float &retDeltaHue) const
{	
	/// make sure all bitmap have the same size
	nlassert(destBitmap.getWidth() == srcBitmap.getWidth() && srcBitmap.getWidth() == maskBitmap.getWidth()
			 && destBitmap.getHeight() == srcBitmap.getHeight() && srcBitmap.getHeight() == maskBitmap.getHeight());

	float h, s, l;
	uint8 grey;
	evalBitmapStats(srcBitmap, maskBitmap, h, s, l, grey);
	//nlinfo("Bitmap stats : (H, L, S) = (%g, %g, %g)", h, s, l);
	float deltaH = Hue - h;
	retDeltaHue= deltaH;
	
	
	const NLMISC::CRGBA  *src   = (NLMISC::CRGBA *) &srcBitmap.getPixels()[0];
	const NLMISC::CRGBA  *mask =  (NLMISC::CRGBA *) &maskBitmap.getPixels()[0];
		  NLMISC::CRGBA  *dest =  (NLMISC::CRGBA *) &destBitmap.getPixels()[0];
	

	for (uint y = 0; y < srcBitmap.getHeight(); ++y)
	{
		for (uint x = 0; x < srcBitmap.getWidth(); ++x)
		{			
			if (src->convertToHLS(h, l, s)) // achromatic ?
			{
				h = 0;
			}

			NLMISC::CRGBA result;

			result.buildFromHLS(h + deltaH, l + Lightness, s + Saturation);

			/// apply contrasts
			result.R = CalcBrightnessContrast(result.R, Luminosity, Contrast, grey);
			result.G = CalcBrightnessContrast(result.G, Luminosity, Contrast, grey);
			result.B = CalcBrightnessContrast(result.B, Luminosity, Contrast, grey);

			// blend to the destination by using the mask alpha			
			result.blendFromui(*dest, result, mask->R);
			

			/// keep alpha from the source			
			dest->R = result.R;
			dest->G = result.G;
			dest->B = result.B;
			dest->A = src->A;

						
			++ mask;
			++ src;
			++ dest;
		}
	}
/*	nlinfo("=========================================================");
	for (uint x = 0; x < srcBitmap.getWidth(); ++x)
	{
		nlinfo("src->A = %d dest A = %d", ((NLMISC::CRGBA &) srcBitmap.getPixels()[4 * x]).A, ((NLMISC::CRGBA &) destBitmap.getPixels()[4 * x]).A);
	}*/
}

///=================================================================================================
void CColorModifier::evalBitmapStats(const NLMISC::CBitmap &srcBitmap,
									 const NLMISC::CBitmap &maskBitmap,
									 float &H,
									 float &S,
									 float &L,
									 uint8 &greyLevel
									)
{

	nlassert(srcBitmap.getWidth() == maskBitmap.getWidth()
			 && srcBitmap.getHeight() == maskBitmap.getHeight());

	float hWeight = 0;
	float weight = 0;

	float hTotal  = 0;	
	float lTotal  = 0;	
	float sTotal  = 0;	
	float gTotal  = 0;	

	const NLMISC::CRGBA *src = (NLMISC::CRGBA *) &srcBitmap.getPixels()[0];
	const NLMISC::CRGBA *mask = (NLMISC::CRGBA *) &maskBitmap.getPixels()[0];

	for (uint y = 0; y < srcBitmap.getHeight(); ++y)
	{
		for (uint x = 0; x < srcBitmap.getWidth(); ++x)
		{
			float h, l, s;
		
			float intensity = mask->R * (1.f / 255.f);			
			bool achromatic = src->convertToHLS(h, l, s);

			float grey = 0.299f * src->R + 0.587f * src->G + 0.114f * src->B;
			
			lTotal  += intensity * l;			
			sTotal  += intensity * s;			
			gTotal  += intensity * grey;

			weight += intensity;
			if (!achromatic)
			{
				// takes the 360 - 0 loop in account.				
				if (hWeight != 0.f)
				{
					float hMean = hTotal / hWeight;
					if (fabs(h - 360.f - hMean) < fabs(h - hMean))
					{
						h -= 360.f;
					}
				}				
				hTotal  += h * intensity;	
				hWeight += intensity;
			}			

			++mask;
			++src;
		}
	}

	H = (hWeight != 0) ? hTotal / hWeight : 0.f;
	if (H < 0.f) H += 360.f;
	S = (weight != 0) ? sTotal / weight : 0.f;
	L = (weight != 0) ? lTotal / weight : 0.f;	
	greyLevel = (weight != 0) ? (uint8) (gTotal / weight) : 0;
}
