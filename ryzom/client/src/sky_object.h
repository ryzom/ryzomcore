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

#ifndef CL_SKY_OBJECT_H
#define CL_SKY_OBJECT_H


#include "nel/misc/bitmap.h"
//
#include "nel/3d/u_instance.h"
#include "nel/3d/u_particle_system_instance.h"
//
#include "client_sheets/sky_object_sheet.h"
//
#include "time_client.h"

namespace NL3D
{
	class UAnimationSet;
	class UPlayListManager;
	class UPlayManager;
}

class CSkyObject
{
public:
	////////////////////////////////////////////////
	// tells how a color is computed in the shape //
	////////////////////////////////////////////////
	class CColorInfo
	{
	public:
		NLMISC::CBitmap	*Map;			// color computed from a map depending on hour & weather (NULL if color is unseted)
		TSkyColorMode	Mode;  // how the color is to be used
	public:
		CColorInfo() : Map(NULL) {}
		/** Init color map from its name. Eventually load the bitmap if itsn't found in the map
		  * \param bitmapByName already build bitmap, sorted by their name
		  * \param buildBitmap list of used bitmap (to be completed if required bitmap id not in "bitmapByName")
		  */
		void init(const CSkyObjectSheet::CColorInfoSheet &ci,
				  std::map<std::string, NLMISC::CBitmap *> &bitmapByName,
				  std::vector<NLMISC::CBitmap *> &buildBitmap);
		// compute color depending on hour & weather & fog color
		NLMISC::CRGBA computeColor(float dayPart, float weatherLevel, NLMISC::CRGBA fogColor);
	};
	////////////////////////////////////////////////////////////////////////////////
	// tells how a color gradient is computed in the shape (-> sky dome gradient) //
	////////////////////////////////////////////////////////////////////////////////
	class CColorGradientInfo
	{
	public:
		sint32 TargetTextureStage; // the texture stage to which the gradient must be applied.
		/** each bitmap in the following list  gives the gradient depending on light level. The V coordinate gives the gradient values, end U gives the light level
		  * each bitmap match a weather state. First bitmap maps to weather value = 0 and last bitmap maps to weather value = 1
		  * for intermediary weather values, the two nearest bitmap are blended to get the value of the gradient
		  */

		std::vector<NLMISC::CBitmap *> WeatherToGradient;
		NLMISC::CBitmap Slice0[2]; // 2 column for slice 0
		NLMISC::CBitmap Slice1[2]; // 2 columns for slice 1
		NLMISC::CBitmap Final;
	public:
		// ctor
		CColorGradientInfo() : TargetTextureStage(0) {}
		/** Init from a sheet.
		  * Init color gradient from its bitmap names. Eventually load the bitmap if itsn't found in the map
		  * \param bitmapByName already build bitmap, sorted by their name
		  * \param buildBitmap list of used bitmap (to be completed if required bitmaps are not in "bitmapByName")
		  */
		void init(const CSkyObjectSheet::CColorGradientInfoSheet &cgi,
				  std::map<std::string, NLMISC::CBitmap *> &bitmapByName,
				  std::vector<NLMISC::CBitmap *> &buildBitmap);
		void setup(NL3D::UInstance instance, float dayPart, float weatherLevel, NLMISC::CBitmap &gradientCache, NLMISC::CBitmap &gradientCacheBlurred);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NL3D::UInstance					Instance; // Instance of object to be displayed
	NL3D::UParticleSystemInstance	PS;	   // If the object is also a particle system, keep a pointer on it.
	//
	CColorInfo			DiffuseColor;  // map for diffuse color (or fx color), depending on light level and weather
	CColorInfo			ParticleEmitters; // map whose R channel control emitters activation
	CColorInfo			ConstantColor[SKY_MAX_NUM_STAGE]; // map for constant color at each stage
	CColorGradientInfo	ColorGradient;
	TSkyRefColor		RefColor;	// tells which color info is used to test if object is visible (when alpha is 0, the object must be hidden)
	// cache last colors
	NLMISC::CRGBA		LastDiffuseColor;
	NLMISC::CRGBA		LastParticleEmittersColor;
	NLMISC::CRGBA		LastConstantColor[SKY_MAX_NUM_STAGE];
	// cache last gradient
	NLMISC::CBitmap		GradientCache;
	NLMISC::CBitmap		GradientCacheBlurred;
	NLMISC::CUV			TexPanner[SKY_MAX_NUM_STAGE];
	NLMISC::CUV			OffsetFactor[SKY_MAX_NUM_STAGE];
	NLMISC::CBitmap		*FXUserParams[SKY_MAX_NUM_FX_USER_PARAMS];

	// texture scaling depending on weather and time
	NLMISC::CBitmap		*OffsetUBitmap[SKY_MAX_NUM_STAGE];
	NLMISC::CBitmap		*OffsetVBitmap[SKY_MAX_NUM_STAGE];

	std::string			Name;
	bool				Active;
	//
	bool				VisibleInMainScene;
	bool				VisibleInEnvMap;
public:
	// ctor
	CSkyObject() : Instance(NULL),
				   PS(NULL),
				   Active(false),
				   VisibleInMainScene(true),
				   VisibleInEnvMap(true)
	{
		LastDiffuseColor.set(0, 0, 0, 0);
		LastParticleEmittersColor.set(0, 0, 0, 0);
		for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
		{
			LastConstantColor[k].set(0, 0, 0, 0);
			OffsetUBitmap[k] = NULL;
			OffsetVBitmap[k] = NULL;
			TexPanner[k].set(0.f, 0.f);
			OffsetFactor[k].set(1.f, 1.f);
		}
		for(uint k = 0; k < SKY_MAX_NUM_FX_USER_PARAMS; ++k)
		{
			FXUserParams[k] = NULL;
		}
	}
	// dtor
	~CSkyObject();
	/** Init sky object from a sheet (main or fallback version)
	  * \param sheet to init from
	  * \param instance Instance of the object in the scene
	  * \param bitmapByName already build bitmap, sorted by their name
	  * \param buildBitmap list of used bitmap (to be completed if required bitmap id not in "bitmapByName")
	  */
	void init(const CSkyObjectSheet::CVersionSheet &sheet,
		      NL3D::UInstance instance,
			  std::map<std::string, NLMISC::CBitmap *> &bitmapByName,
			  std::vector<NLMISC::CBitmap *> &builtBitmaps,
			  bool	visibleInMainScene,
			  bool	visibleInEnvMap
			 );
	/** setup the given instance to reflect the given hour & weather
	  * \param duskSetup true if the setup os for the dusk (dawn otherwise)
	  * \return true if the object is visible
      */
	bool setup(const CClientDate &date, const CClientDate &animationDate, float numHoursInDay, float weatherLevel, NLMISC::CRGBA fogColor, bool envMapScene);
};


#endif
