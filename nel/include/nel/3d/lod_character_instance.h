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

#ifndef NL_LOD_CHARACTER_INSTANCE_H
#define NL_LOD_CHARACTER_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/uv.h"
#include "nel/misc/rgba.h"
#include "nel/3d/animation_time.h"


namespace NL3D
{


using NLMISC::CRGBA;
using NLMISC::CUV;


class CLodCharacterManager;


// ***************************************************************************
/**
 * An instance of a lodCharacter (stored in CSkeletonModel).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterInstance
{
public:
	/// shapeId is the id of the lod character shape to use. No-Op if not found.
	sint					ShapeId;	// -1 if disabled
	/// animId is the anim to use for this shape. No-Op if not found.
	uint					AnimId;
	/// time is the time of animation
	TGlobalAnimationTime	AnimTime;
	/// wrapMode if true, the anim loop, else just clamp
	bool					WrapMode;

	/** The precomputed alpha array
	 *	must be same size of the shape number vertices, else the
	 *	whole mesh is supposed to be opaque. see CLodCharacterShape::startBoneAlpha() for how to build this array
	 */
	std::vector<uint8>		VertexAlphas;

public:
	CLodCharacterInstance()
	{
		ShapeId= -1;
		AnimId= 0;
		AnimTime= 0;
		WrapMode= true;
		_TextureId= -1;
		_Owner= NULL;
	}

	~CLodCharacterInstance();

	/// get a ptr on the UVs.
	const CUV		*getUVs() const;

// ***************
private:
	friend class CLodCharacterManager;

	// The manager which owns us. Filled by CLodCharacterManager.
	CLodCharacterManager	*_Owner;
	// The id of the texture the manager gives to us. Filled by CLodCharacterManager.
	sint					_TextureId;
	/// The precomputed UVs. Filled by CLodCharacterManager.
	std::vector<CUV>		_UVs;

};



} // NL3D


#endif // NL_LOD_CHARACTER_INSTANCE_H

/* End of lod_character_instance.h */
