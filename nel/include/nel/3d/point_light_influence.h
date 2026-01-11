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

#ifndef NL_POINT_LIGHT_INFLUENCE_H
#define NL_POINT_LIGHT_INFLUENCE_H

#include "nel/misc/types_nl.h"

namespace NL3D {


class CPointLight;


/**
 * A pointLight Influence. Filled by Static Lighting system (landscape / Igs).
 *	The influence may be lower than 1, because landscape/Igs look into a grid (on patchs or on pacs surfaces)
 *	and do biLinear to avoid lighting poping.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightInfluence
{
public:
	// The light of interest
	CPointLight		*PointLight;
	// The factor of influence (0..1) of this light, computed by the static light setup.
	float			Influence;
	// Internal Use only. Used by CLightingManager
	float			BkupInfluence;
	// Internal Use only. Used by CLightingManager
	float			DistanceToModel;

	// To sort by influence.
	bool	operator<(const CPointLightInfluence &sl) const
	{
		// sort in growing order
		return Influence>sl.Influence;
	}

};



} // NL3D


#endif // NL_POINT_LIGHT_INFLUENCE_H

/* End of point_light_influence.h */
