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



#ifndef RY_DIR_LIGHT_SETUP_H
#define RY_DIR_LIGHT_SETUP_H

#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"


namespace NLGEORGES
{
	class UFormElm;
}

// Setup for a directionnal light
struct CDirLightSetup
{
	NLMISC::CRGBA Ambiant;
	NLMISC::CRGBA Diffuse;
	NLMISC::CRGBA Specular;
	NLMISC::CVector Direction;
	// default ctor
	CDirLightSetup();
	// ctor
	CDirLightSetup(NLMISC::CRGBA ambiant,
			       NLMISC::CRGBA diffuse,
				   NLMISC::CRGBA specular,
				   const NLMISC::CVector &dir)
				  : Ambiant(ambiant), Diffuse(diffuse), Specular(specular), Direction(dir)
	{
	}
	// blend
	void blend(const CDirLightSetup &setup0, const CDirLightSetup &setup1, float blendFactor);
	// modulate
	void modulate(float level);
	/** load from sheet.
	  * \return true if the loading was ok
	  */
	bool build(const NLGEORGES::UFormElm &elm);
	//
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(Ambiant, Diffuse, Specular, Direction);
	}

};



#endif

