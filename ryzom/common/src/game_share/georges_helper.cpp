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

#include "georges_helper.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/georges/u_form_elm.h"


//=========================================================================
/*static*/ bool CGeorgesHelper::convert(NLMISC::CVector &dest,const NLGEORGES::UFormElm &src)
{
	NLMISC::CVector temp;
	if ( src.getValueByName (temp.x, "X") &&
		src.getValueByName (temp.y, "Y") &&
		src.getValueByName (temp.z, "Z") )
	{
		dest = temp;
		return true;
	}
	return false;
}

//=========================================================================
/*static*/ bool CGeorgesHelper::convert(NLMISC::CRGBA &dest,const NLGEORGES::UFormElm &src)
{
	NLMISC::CRGBA temp;
	if ( src.getValueByName (temp.R, "R") &&
		src.getValueByName (temp.G, "G") &&
		src.getValueByName (temp.B, "B") &&
		src.getValueByName (temp.A, "A") )
	{
		dest = temp;
		return true;
	}
	return false;
}


//=========================================================================
bool CGeorgesHelper::getValueByName(NLMISC::CVector &dest, const NLGEORGES::UFormElm &item, const char *name)
{
	const NLGEORGES::UFormElm *pElt;
	if(item.getNodeByName (&pElt, name) && pElt)
		return CGeorgesHelper::convert(dest, *pElt);
	else
		return false;
}

//=========================================================================
bool CGeorgesHelper::getValueByName(NLMISC::CRGBA &dest, const NLGEORGES::UFormElm &item, const char *name)
{
	const NLGEORGES::UFormElm *pElt;
	if(item.getNodeByName (&pElt, name) && pElt)
		return CGeorgesHelper::convert(dest, *pElt);
	else
		return false;
}
