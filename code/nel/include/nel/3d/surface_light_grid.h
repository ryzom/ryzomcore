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

#ifndef NL_SURFACE_LIGHT_GRID_H
#define NL_SURFACE_LIGHT_GRID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vector.h"
#include "nel/3d/point_light_influence.h"


namespace NL3D
{


using	NLMISC::CVector;


class	CIGSurfaceLight;


// ***************************************************************************
/**
 * Dynamic objects lighted by IG's Static Lights lookup into this grid.
 *	Builded by CInstanceLighter.
 *	Used by CIGSurfaceLight
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CSurfaceLightGrid
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:

	/// We support only 2 light per corner. Should never be changed.
	enum	{NumLightPerCorner= 2};


	struct	CCellCorner
	{
		/// Contribution of Sun.
		uint8	SunContribution;
		/// Light that influence this point. 0xFF if none. if Light[i]==0xFF, then Light[j] with j>i is disabled too
		uint8	Light[NumLightPerCorner];
		/** Id of the ambiant Light to take for this corner. Ambient light are stored too in ig->getPointLigths()
		 *	If 0xFF => take Ambient of the sun.
		 */
		uint8	LocalAmbientId;

		void		serial(NLMISC::IStream &f)
		{
			uint	ver= f.serialVersion(1);
			nlassert(NumLightPerCorner==2);

			if(ver>=1)
			{
				f.serial(LocalAmbientId);
			}
			else if(f.isReading())
			{
				LocalAmbientId= 0xFF;
			}
			f.serial(SunContribution);
			f.serial(Light[0]);
			f.serial(Light[1]);
		}
	};


public:
	// Origin of the grid. Size is (Width-1) * CIGSurfaceLight.getCellSize().
	NLMISC::CVector2f					Origin;
	uint32								Width;
	uint32								Height;
	NLMISC::CObjectVector<CCellCorner>	Cells;


public:

	/// Constructor
	CSurfaceLightGrid();

	// Get Infos from the grid
	void		getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &localPos, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, CIGSurfaceLight &igsl, NLMISC::CRGBA &localAmbient) const;

	// serial
	void		serial(NLMISC::IStream &f);

};


} // NL3D


#endif // NL_SURFACE_LIGHT_GRID_H

/* End of surface_light_grid.h */
