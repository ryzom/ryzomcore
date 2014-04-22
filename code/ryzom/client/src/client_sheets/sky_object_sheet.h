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

#ifndef CL_SKY_OBJECT_SHEET_H
#define CL_SKY_OBJECT_SHEET_H


#include "nel/misc/stream.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/uv.h"
//
#include "nel/georges/u_form_elm.h"
//
#include <string>


const uint SKY_MAX_NUM_STAGE = 4;
const uint SKY_MAX_NUM_FX_USER_PARAMS = 4;

enum TSkyRefColor { NoColorRef = 0, DiffuseColorRef, ConstantColor0Ref, ConstantColorLastRef = ConstantColor0Ref + SKY_MAX_NUM_STAGE, ParticleEmittersColorRef = ConstantColorLastRef  }; // color serves as a reference to see if object must be hiden. This is the case when the alpha is 0
enum TSkyColorMode { Unused = 0, BitmapColor, FogColor, BitmapColorModulatedByFogColor };


// Sheet describing an object in the sky
class CSkyObjectSheet
{
public:
	////////////////////////////////////////////////
	// tells how a color is computed in the shape //
	////////////////////////////////////////////////
	class CColorInfoSheet
	{
	public:
		std::string MapName;
		TSkyColorMode	Mode;
	public:
		// Build from an external script
		void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
		/// Serialize sheet into binary data file.
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	////////////////////////////////////////////////////////////////////////////////
	// tells how a color gradient is computed in the shape (-> sky dome gradient) //
	////////////////////////////////////////////////////////////////////////////////
	class CColorGradientInfoSheet
	{
	public:
		sint32 TargetTextureStage; // the texture stage to which the gradient must be applied.
		/** each bitmap in the following list  gives the gradient depending on weather valuet level. The V coordinate gives the gradient values, and U is offseted by the weather value
		  * each bitmap match a weather state. First bitmap maps to light level = 0 and last bitmap maps to light level = 1
		  * for intermediary weather values, the two nearest bitmap are blended to get the value of the gradient
		  */

		std::vector<std::string> WeatherToGradient;
	public:
		// ctor
		CColorGradientInfoSheet() : TargetTextureStage(0) {}
		// Build from an external script
		void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
		/// Serialize sheet into binary data file.
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A version of a sky object. If materials are too complex, a fallback version may be used instead (on gpu with 2 stages) //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CVersionSheet
	{
	public:
		std::string				ShapeName;
		uint8					TransparencyPriority;
		TSkyRefColor			RefColor;
		CColorInfoSheet			DiffuseColor;
		CColorInfoSheet			ParticleEmitters;
		CColorInfoSheet			ConstantColor[SKY_MAX_NUM_STAGE];
		CColorGradientInfoSheet	ColorGradient;
		NLMISC::CUV				TexPanner[SKY_MAX_NUM_STAGE];
		NLMISC::CUV				OffsetFactor[SKY_MAX_NUM_STAGE];
		std::string				FXUserParamBitmap[SKY_MAX_NUM_FX_USER_PARAMS];
		// texture scaling : each bitmap gives a scaling factor depending on weather and the hour of the day
		std::string				OffsetUBitmap[SKY_MAX_NUM_STAGE];
		std::string				OffsetVBitmap[SKY_MAX_NUM_STAGE];
	public:
		// ctor
		CVersionSheet() : TransparencyPriority(0), RefColor(NoColorRef)
		{
			for(uint k = 0; k < SKY_MAX_NUM_STAGE; ++k)
			{
				TexPanner[k].set(0.f, 0.f);
			}

		}
		// Build from an external script
		void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
		/// Serialize sheet into binary data file.
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CVersionSheet	Std;			 // standard version of the object
	CVersionSheet	FallbackPass[2]; // up to 2 fallback versions (for multi-pass fallback)
	bool			VisibleInMainScene;
	bool			VisibleInEnvMap;
public:
	// ctor
	CSkyObjectSheet();
	// Build from an external script
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
