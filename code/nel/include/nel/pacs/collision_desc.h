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

#ifndef NL_COLLISION_DESC_H
#define NL_COLLISION_DESC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include "nel/pacs/u_collision_desc.h"
#include <vector>

namespace NLPACS
{

/**
 * Description of the contact of a collision
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionDesc: public UCollisionDesc
{
public:
	// XChg contact normal 0 and 1
	void XChgContactNormals ()
	{
		NLMISC::CVectorD tmp=ContactNormal0;
		ContactNormal0=ContactNormal1;
		ContactNormal1=tmp;
	}
};


/**
 * Ident of a surface.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSurfaceIdent
{
public:
	/// the surface mesh instance Id.
	sint32				RetrieverInstanceId;
	/// the surface Id of this surface mesh instance. -1 if Wall/impossible to walk through.
	sint32				SurfaceId;

	bool operator==(const CSurfaceIdent &o) const
	{
		return RetrieverInstanceId==o.RetrieverInstanceId && SurfaceId==o.SurfaceId;
	}

	bool operator!=(const CSurfaceIdent &o) const
	{
		return !(*this==o);
	}


public:
	CSurfaceIdent() {}
	CSurfaceIdent(sint32 retInstance, sint32 surfId) : RetrieverInstanceId(retInstance), SurfaceId(surfId) {}

	void				serial(NLMISC::IStream &f)	{ f.serial(RetrieverInstanceId, SurfaceId); }
};


/**
 * Description of the contact of a collision against a surface (interior/zones).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionSurfaceDesc
{
public:
	NLMISC::CVectorD	ContactNormal;
	double				ContactTime;

	/// To which surface we have collided.
	CSurfaceIdent		ContactSurface;
};


typedef	std::vector<CCollisionSurfaceDesc>	TCollisionSurfaceDescVector;



} // NLPACS


#endif // NL_COLLISION_DESC_H

/* End of collision_desc.h */
