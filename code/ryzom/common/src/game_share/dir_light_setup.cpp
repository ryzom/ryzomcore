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
#include "dir_light_setup.h"
#include "georges_helper.h"
#include "nel/georges/u_form_elm.h"


//-----------------------------------------------
CDirLightSetup::CDirLightSetup() :
							Ambiant(0, 0, 0),
							Diffuse(255, 255, 255),
							Specular(0, 0, 0),
							Direction(1.f, 0.f, 0.f)
{
}

//-----------------------------------------------
void CDirLightSetup::blend(const CDirLightSetup &setup0, const CDirLightSetup &setup1, float blendFactor)
{
	uint uiFactor = (uint) (256.f * blendFactor);
	Ambiant.blendFromui(setup0.Ambiant, setup1.Ambiant, uiFactor);
	Diffuse.blendFromui(setup0.Diffuse, setup1.Diffuse, uiFactor);
	Specular.blendFromui(setup0.Specular, setup1.Specular, uiFactor);
	Direction = (blendFactor * setup0.Direction + (1.f - blendFactor) * setup1.Direction).normed();
}

//-----------------------------------------------
void CDirLightSetup::modulate(float level)
{
	uint uiLevel = (uint) (256.f * level);
	Ambiant.modulateFromui(Ambiant, uiLevel);
	Diffuse.modulateFromui(Diffuse, uiLevel);
	Specular.modulateFromui(Specular, uiLevel);
}

//-----------------------------------------------
bool CDirLightSetup::build(const NLGEORGES::UFormElm &item)
{
	NLMISC::CRGBA amb, dif, spe;
	NLMISC::CVector dir;

	const NLGEORGES::UFormElm *pElt;
	// Light Direction
	if (item.getNodeByName (&pElt, ".Direction") && pElt)
	{
		if (!CGeorgesHelper::convert(dir, *pElt)) return false;
	}
	// Light Ambiant
	if (item.getNodeByName (&pElt, ".Ambiant") && pElt)
	{
		if (!CGeorgesHelper::convert(amb, *pElt)) return false;
	}

	// Light Diffuse
	if (item.getNodeByName (&pElt, ".Diffuse") && pElt)
	{
		if (!CGeorgesHelper::convert(dif, *pElt)) return false;
	}

	// Light Specular
	if (item.getNodeByName (&pElt, ".Specular") && pElt)
	{
		if (!CGeorgesHelper::convert(spe, *pElt)) return false;
	}

	Ambiant = amb;
	Diffuse = dif;
	Specular = spe;
	Direction = dir;

	return true;
}
