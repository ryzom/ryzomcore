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

#ifndef NL_PRIMITIVE_BLOCK_PACS_H
#define NL_PRIMITIVE_BLOCK_PACS_H

#include "nel/misc/types_nl.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_primitive_block.h"


namespace NLPACS
{

/**
 * PACS primitive description
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CPrimitiveDesc
{
public:

	// Default constructor
	CPrimitiveDesc ();

	// The length of the 4 edges. The first is the width, the second is the depth
	// For cylinder, the first is the radius
	float							Length[2];

	// This is the height of the box or of the cylinder.
	float							Height;

	// Attenuation
	float							Attenuation;

	// Primitive type
	UMovePrimitive::TType			Type;

	// Reaction type
	UMovePrimitive::TReaction		Reaction;

	// Reaction type
	UMovePrimitive::TTrigger		Trigger;

	// Obstacle flag
	bool							Obstacle;

	// Occlusion mask
	UMovePrimitive::TCollisionMask	OcclusionMask;

	// Collision mask
	UMovePrimitive::TCollisionMask	CollisionMask;

	// Position of the primitive
	NLMISC::CVector					Position;

	// Orientation of the primitive
	float							Orientation;

	// User data
	UMovePrimitive::TUserData		UserData;

	// Serial methods
	void serial (NLMISC::IStream &s);
};

/**
 * Block of pacs primitive
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CPrimitiveBlock	: public UPrimitiveBlock
{
public:

	// Array of primitives
	std::vector<CPrimitiveDesc>		Primitives;

	// Serial methods
	void serial (NLMISC::IStream &s);

	///\name from UPrimitive block, create a P.B. from a stream
	//@{
	static UPrimitiveBlock *createPrimitiveBlock(NLMISC::IStream &src);
	static UPrimitiveBlock *createPrimitiveBlockFromFile(const std::string &fileName);
	uint						getNbPrimitive() { return (uint)Primitives.size(); }
	UMovePrimitive::TUserData	getUserData(uint nPrimNb) { nlassert(nPrimNb < Primitives.size());
															return Primitives[nPrimNb].UserData; }
	//@}
};


} // NLPACS


#endif // NL_PRIMITIVE_BLOCK_PACS_H

/* End of primitive_block.h */
