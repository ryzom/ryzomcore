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

#ifndef NL_U_PRIMITIVE_BLOCK_H
#define NL_U_PRIMITIVE_BLOCK_H

#include <string>
#include "nel/pacs/u_move_primitive.h"

namespace NLMISC
{
	class IStream;
}

namespace NLPACS
{

/**
 * Block of pacs primitives, user interface.
 * They can be created from a stream (*.pacs_prim)
 * A primitive block can be instanciated in a move container.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class UPrimitiveBlock
{
public:
	// dtor
	virtual ~UPrimitiveBlock() {}
	/** Create a primitive block from a stream.
	  * This may raise exception if loading failed
	  */
	static UPrimitiveBlock *createPrimitiveBlock(NLMISC::IStream &src);
	/** Create a primitive block from its file name.
	  * This may raise exceptions if loading failed.
	  */
	static UPrimitiveBlock *createPrimitiveBlockFromFile(const std::string &fileName);

	/// get the number of primitives in the block
	virtual uint						getNbPrimitive() = 0;

	/// return the user data for a primitive of the block
	virtual UMovePrimitive::TUserData	getUserData(uint nPrimNb) = 0;
};

}

#endif





