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

#ifndef NL_LANDSCAPE_FACE_VECTOR_MANAGER_H
#define NL_LANDSCAPE_FACE_VECTOR_MANAGER_H

#include "nel/misc/types_nl.h"
#include <vector>
#include "nel/3d/index_buffer.h"


namespace NL3D {

// ***************************************************************************
/**
 * Fast Allocate blocks of faces, according to the size of the block.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeFaceVectorManager
{
public:

	/// Constructor
	CLandscapeFaceVectorManager();
	~CLandscapeFaceVectorManager();


	// Empty the Free List. All FaceVector must be deleted.
	void					purge();
	/** return an Array of Tris with this Format: NumTris, index0, index1, index2....
	 *	NB: NumTris really means number of triangles, not number of indexes (which is 3*)!
	 */
	TLandscapeIndexType					*createFaceVector(uint numTri);
	/// delete a faceVector. NB: no check.
	void					deleteFaceVector(TLandscapeIndexType *fv);


private:
	// Array of Free Blocks List. NB: actually, in place of a TLandscapeIndexType, it is a TLandscapeIndexType* we point here (for next!!)
	std::vector<TLandscapeIndexType*>	_Blocks;

	uint		getBlockIdFromNumTri(uint numTris);

};


} // NL3D


#endif // NL_LANDSCAPE_FACE_VECTOR_MANAGER_H

/* End of landscape_face_vector_manager.h */
