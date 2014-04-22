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

#ifndef NL_EDGE_QUAD_H
#define NL_EDGE_QUAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "exterior_mesh.h"
#include "collision_surface_temp.h"
#include <vector>

#ifdef _X
#	undef _X
#endif

namespace NLMISC
{
	class	IStream;
};

namespace NLPACS
{
class	CGlobalRetriever;

using	NLMISC::CVector;
using	NLMISC::IStream;

// ***************************************************************************
struct EEdgeQuad : public NLMISC::Exception
{
	EEdgeQuad(const std::string &reason) : Exception(reason) { }
};


// ***************************************************************************
/**
 * a quadgrid of list of edge chain.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CEdgeQuad
{
public:

	/// Constructor
	CEdgeQuad();
	/// Copy Constructor
	CEdgeQuad(const CEdgeQuad &o);
	/// Destructor
	~CEdgeQuad();
	/// operator=.
	CEdgeQuad &operator=(const CEdgeQuad &o);

	/// clear
	void			clear();


	/// build a chain quad, with a list of exterior Edges and the global retriever
	void			build(const CExteriorMesh &em,
						  const CGlobalRetriever &global,
						  CCollisionSurfaceTemp &cst,
						  uint32 thisInstance);


	/** look in the quad to select a list of chain from a bbox.
	 * NB: The outpout do not contains any redundant edge. An edge appears only one time in the result.
	 * \param bbox the area of interest.
	 * \param cst the array of CExteriorEdgeEntry to fill. contain also OChainLUT, an array for internal use. In: must be filled with 0xFFFF. Out: still filled with 0xFFFF.
	 * \return number of exterioredge found. stored in cst.ExteriorEdgeEntries (array cleared first).
	 */
	sint			selectEdges(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst) const;

	/** look in the quad to select a list of chain from a line.
	 * NB: The outpout do not contains any redundant edge. An edge appears only one time in the result.
	 * \param start the starting point of the selection segment.
	 * \param end the ending point of the selection segment.
	 * \param cst the array of CExteriorEdgeEntry to fill. contain also OChainLUT, an array for internal use. In: must be filled with 0xFFFF. Out: still filled with 0xFFFF.
	 * \return number of exterioredge found. stored in cst.ExteriorEdgeEntries (array cleared first).
	 */
	sint			selectEdges(CVector start, CVector end, CCollisionSurfaceTemp &cst) const;


	/// Get the whole set of edge entries
	const std::vector<CExteriorEdgeEntry>	&getEdgeEntries() const { return _EdgeEntries; }
	/// Get a single edge entry
	const CExteriorEdgeEntry				&getEdgeEntry(uint entry) const { return _EdgeEntries[entry]; }


	/// serial.
	void								serial(NLMISC::IStream &f);

	///
	void								removeLinks(sint32 instanceId)
	{
		uint	i;
		for (i=0; i<_EdgeEntries.size(); ++i)
		{
			if (_EdgeEntries[i].Exterior.RetrieverInstanceId == instanceId)
			{
				_EdgeEntries[i].Exterior.RetrieverInstanceId = -1;
				_EdgeEntries[i].Exterior.SurfaceId = -1;
			}
		}
	}

	///
	void								removeLinks()
	{
		uint	i;
		for (i=0; i<_EdgeEntries.size(); ++i)
		{
			_EdgeEntries[i].Exterior.RetrieverInstanceId = -1;
			_EdgeEntries[i].Exterior.SurfaceId = -1;
		}
	}

// **************************
private:

	/** W*H pointers on array of CExteriorEdgeEntry indexes. NULL if no edge in this quad.
	 * Each array is 1xuint16(LEN) + LEN*CExteriorEdgeEntryIndex(16 bits).
	 */
	std::vector<uint8*>				_Quad;
	/// The real exterior edge entries
	std::vector<CExteriorEdgeEntry>	_EdgeEntries;
	/// Width of the quadgrid.
	uint32							_Width;
	/// Height of the quadgrid.
	uint32							_Height;
	/// Postion of the chainquad.
	sint32							_X, _Y;
	/// Single memory block of CExteriorEdgeEntry chains.
	uint8							*_QuadData;
	/// size (in byte) of _QuadData.
	uint32							_QuadDataLen;


	static const	float	_QuadElementSize;	// = 4 meters.


private:

	// get local integer bounds in the grid.
	void			getGridBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const CVector &minP, const CVector &maxP) const;

};


} // NLPACS


#endif // NL_CHAIN_QUAD_H

/* End of chain_quad.h */
