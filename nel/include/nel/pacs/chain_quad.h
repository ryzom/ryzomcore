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

#ifndef NL_CHAIN_QUAD_H
#define NL_CHAIN_QUAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "chain.h"
#include "collision_surface_temp.h"
#include <vector>

// _X is defined in some versions of linux/ctype.h
#ifdef _X
#	undef _X
#endif

namespace NLPACS
{

using	NLMISC::CVector;
using	NLMISC::IStream;

// ***************************************************************************
struct EChainQuad : public NLMISC::Exception
{
	EChainQuad(const std::string &reason) : Exception(reason) { }
};


// ***************************************************************************
/**
 * a quadgrid of list of edge chain.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CChainQuad
{
public:

	/// Constructor
	CChainQuad();
	/// Copy Constructor
	CChainQuad(const CChainQuad &o);
	/// Destructor
	~CChainQuad();
	/// operator=.
	CChainQuad &operator=(const CChainQuad &o);


	/// build a chain quad, with a list of OrderedChains.
	void			build(const std::vector<COrderedChain> &ochains);


	/** look in the quad to select a list of chain from a bbox.
	 * NB: The outpout do not contains any redundant edge. A OChain appears only one time in the result.
	 * \param bbox the area of interest.
	 * \param cst the array of CEdgeChainEntry to fill. contain also OChainLUT, an array for internal use. In: must be filled with 0xFFFF. Out: still filled with 0xFFFF.
	 * \return number of edgechain found. stored in cst.EdgeChainEntries (array cleared first).
	 */
	sint			selectEdges(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst) const;

	/** look in the quad to select a list of chain from a bbox.
	 * NB: The outpout do not contains any redundant edge. A OChain appears only one time in the result.
	 * \param start the starting point of the selection segment.
	 * \param end the ending point of the selection segment.
	 * \param cst the array of CEdgeChainEntry to fill. contain also OChainLUT, an array for internal use. In: must be filled with 0xFFFF. Out: still filled with 0xFFFF.
	 * \return number of edgechain found. stored in cst.EdgeChainEntries (array cleared first).
	 */
	sint			selectEdges(CVector start, CVector end, CCollisionSurfaceTemp &cst) const;


	/// serial.
	void								serial(NLMISC::IStream &f);

	/// clear
	void			clear()
	{
		NLMISC::contReset(_Quad);
		_Width = 0;
		_Height = 0;
		_X = 0;
		_Y = 0;
		delete [] _QuadData;
		_QuadData = NULL;
		_QuadDataLen = 0;
	}


// **************************
private:

	/** W*H pointers on array of CEdgeChainEntry. NULL if no edge in this quad.
	 * Each array is 1xuint16(LEN) + LEN*CEdgeChainEntry.
	 */
	std::vector<uint8*>		_Quad;
	/// Width of the quadgrid.
	uint32					_Width;
	/// Height of the quadgrid.
	uint32					_Height;
	/// Postion of the chainquad.
	sint32					_X, _Y;
	/// Single memory block of CEdgeChainEntry chains.
	uint8					*_QuadData;
	/// size (in byte) of _QuadData.
	uint32					_QuadDataLen;


	static const	float	_QuadElementSize;	// = 4 meters.


private:
	void			addEdgeToQuadNode(std::list<CEdgeChainEntry> &quadNode, sint ochainId, sint edgeId);

	// get local integer bounds in the grid.
	void			getGridBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const CVector &minP, const CVector &maxP) const;

};


} // NLPACS


#endif // NL_CHAIN_QUAD_H

/* End of chain_quad.h */
