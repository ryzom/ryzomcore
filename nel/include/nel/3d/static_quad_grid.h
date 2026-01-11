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

#ifndef NL_STATIC_QUAD_GRID_H
#define NL_STATIC_QUAD_GRID_H

#include "nel/misc/common.h"
#include "nel/3d/quad_grid.h"


namespace NL3D
{


// ***************************************************************************
/**
 * This class is builded from a CQuadGrid, and is to be used when:
 *		- sizeof(T) is small (ie a pointer)
 *		- no dynamic insertion are made (builded from a CQuadGrid)
 *		- selection is made with a point, not a BBox.
 *
 *	Because elements are duplicated all over cells and only one cell can be selected at a time.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
template<class T>
class CStaticQuadGrid
{
public:

	/// Constructor
	CStaticQuadGrid();
	/// dtor.
	~CStaticQuadGrid();


	/// \name Initialization
	//@{
	/** build from a CQuadGrid. Elements are copied, not referenced
	 *	Elements may be duplicated in all cells they lie into.
	 *	quadGrid selection is cleared
	 */
	void			build(CQuadGrid<T> &quadGrid);

	/** Clear all the container. Elements are deleted, and the quadgrid is erased.
	 */
	void			clear();

	//@}


	/// \name Selection
	//@{
	/** Select elements at a given point
	  *	Speed is in O(1), because the array of the cell is returned. NULL if size==0.
	  *
	  * \param point is the point used to select
	  *	\param numElts number of elements returned
	  * \return a ptr on array of elements
	  */
	const T			*select(const NLMISC::CVector &point, uint &numElts);
	//@}


// ****************************
private:
	class	CQuadNode
	{
	public:
		T				*Nodes;
		uint			NumNodes;

		CQuadNode()
		{
			Nodes= NULL; NumNodes=0;
		}
	};

private:// Atttributes.
	// The big continous array of list of elements.
	std::vector<T>			_Elements;
	std::vector<CQuadNode>	_Grid;
	sint				_Size;
	sint				_SizePower;
	float				_EltSize;
	NLMISC::CMatrix		_ChangeBasis;


	// return the coordinates on the grid of what include the bbox.
	void		selectPoint(CVector point, sint &x0, sint &y0)
	{
		point/= _EltSize;
		x0= (sint)(floor(point.x));
		y0= (sint)(floor(point.y));

		x0&= _Size-1;
		y0&= _Size-1;
	}


};


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// Template CStaticQuadGrid implementation.
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// Init.
// ***************************************************************************


// ***************************************************************************
template<class T>
CStaticQuadGrid<T>::CStaticQuadGrid()
{
	_SizePower=4;
	_Size=1<<_SizePower;
	_EltSize=1;
	_ChangeBasis.identity();
	_Grid.resize(_Size * _Size);
}
// ***************************************************************************
template<class T>
CStaticQuadGrid<T>::~CStaticQuadGrid()
{
	clear();
}


// ***************************************************************************
template<class T>
void			CStaticQuadGrid<T>::clear()
{
	// Just clear all vectors
	_Elements.clear();
	_Grid.clear();

	// reset
	_SizePower=4;
	_Size=1<<_SizePower;
	_EltSize=1;
	_ChangeBasis.identity();
	_Grid.resize(_Size * _Size);
}


// ***************************************************************************
template<class T>
void			CStaticQuadGrid<T>::build(CQuadGrid<T> &quadGrid)
{
	clear();
	NLMISC::contReset(_Grid);

	// Copy from quadGrid, and init quads
	_Size= quadGrid.getSize();
	_SizePower= NLMISC::getPowerOf2(_Size);
	_EltSize= quadGrid.getEltSize();
	_ChangeBasis= quadGrid.getBasis();
	_Grid.resize(_Size * _Size);

	NLMISC::CMatrix		invBasis= _ChangeBasis.inverted();

	// Count number of elements per cell, and total copies of elements
	uint	totalDupElt= 0;
	sint	x,y;
	for(y=0; y<_Size; y++)
	{
		for(x=0; x<_Size; x++)
		{
			// Select the center of the case
			CVector	pos;
			pos.x= (x+0.5f)*_EltSize;
			pos.y= (y+0.5f)*_EltSize;
			pos.z= 0.f;
			// mul by invBasis
			pos= invBasis * pos;
			quadGrid.select(pos, pos);

			// Count elements.
			uint	n= 0;
			typename CQuadGrid<T>::CIterator	it;
			for(it= quadGrid.begin(); it!=quadGrid.end(); it++)
			{
				n++;
			}

			// store.
			_Grid[y*_Size + x].NumNodes= n;
			totalDupElt+= n;
		}
	}

	// Resize array copy.
	_Elements.resize(totalDupElt);


	// Then reparse all array, filling _Elements and setup quadNodes ptr.
	uint	curDupElt= 0;
	for(y=0; y<_Size; y++)
	{
		for(x=0; x<_Size; x++)
		{
			// Select the center of the case
			CVector	pos;
			pos.x= (x+0.5f)*_EltSize;
			pos.y= (y+0.5f)*_EltSize;
			pos.z= 0.f;
			// mul by invBasis
			pos= invBasis * pos;
			quadGrid.select(pos, pos);

			// Setup quadNode ptr.
			if (curDupElt < _Elements.size())
				_Grid[y*_Size + x].Nodes= &_Elements[curDupElt];

			// For all elements.
			typename CQuadGrid<T>::CIterator	it;
			for(it= quadGrid.begin(); it!=quadGrid.end(); it++)
			{
				// Copy elt in array.
				_Elements[curDupElt]= *it;

				// Next elt in array.
				curDupElt++;
			}
		}
	}


	// clean up.
	quadGrid.clearSelection();
}


// ***************************************************************************
template<class T>
const T			*CStaticQuadGrid<T>::select(const NLMISC::CVector &pointIn, uint &numElts)
{
	CVector		point= _ChangeBasis * pointIn;

	// Select the case.
	sint	x, y;
	selectPoint(point, x, y);

	// get ref on the selection
	CQuadNode	&quadNode= _Grid[y*_Size + x];
	numElts= quadNode.NumNodes;

	return quadNode.Nodes;
}


} // NL3D


#endif // NL_STATIC_QUAD_GRID_H

/* End of static_quad_grid.h */
