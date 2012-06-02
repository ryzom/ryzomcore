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

#ifndef NL_MATERIAL_LIGO_H
#define NL_MATERIAL_LIGO_H

#include "nel/misc/types_nl.h"
#include "zone_template.h"

// NeL include
//#include "3d/zone.h"

namespace NLLIGO
{

class CLigoError;
class CLigoConfig;

/**
 * A ligoscape material
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMaterial
{
public:

	/// Build method
	bool build (const CZoneTemplate &tplt, const CLigoConfig &config, CLigoError &errors);

	/// Return the material edge
	const CZoneEdge& getEdge () const { return _ZoneEdge; }

	/// Serial method
	void serial (NLMISC::IStream &s);

private:

	// The zone template for this material
	CZoneEdge		_ZoneEdge;
};

}

#endif // NL_MATERIAL_LIGO_H

/* End of material.h */
