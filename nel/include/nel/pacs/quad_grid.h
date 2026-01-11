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

#ifndef NLPACS_QUAD_GRID_H
#define NLPACS_QUAD_GRID_H

#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include <list>
#include <vector>


namespace NLPACS
{


using NLMISC::CVector;


// ***************************************************************************
/**
 * This container is a simple grid, used to quickly find elements. His purpose is similiar to CQuadTree, but
 * it is a simple grid, so test are in O(1), not in O(log n). It is perfect for local lookup (like in collisions).
 * Use it if you want to select small area, not large. Also, for best use, elements should have approximatively the
 * same size, and this size should be little smaller than the size of a grid element...
 *
 * By default, the quad grid is aligned on XY. (unlike the quadtree!!!)
 *
 * Unlike the quadtree, the quadgrid is NOT geographicly delimited, ie, its limits "tiles"!! This is why no "center"
 * is required. As a direct consequence, when you select something, you are REALLY not sure that what you select is not
 * a mile away from your selection :) ....
 *
 * Also, for memory optimisation, no bbox is stored in the quadgrid. Hence no particular selection is made on the Z
 * components...
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template<class T>	class	CQuadGrid
{
public:
	/// Iterator of the contener
	class	CIterator;
	friend class	CIterator;

public:

	/// Default constructor, use axes XY!!!, has a size of 16, and EltSize is 1.
	CQuadGrid();

	/// dtor.
	~CQuadGrid();

	/// \name Initialization
	//@{
	/** Change the base matrix of the quad grid. For exemple this code init the grid tree in the plane XZ:
	  * \code
	  * CQuadGrid			grid;
	  * NLMISC::CMatrix		tmp;
	  * NLMISC::CVector		I(1,0,0);
	  * NLMISC::CVector		J(0,0,1);
	  * NLMISC::CVector		K(0,-1,0);
	  *
	  * tmp.identity();
	  * tmp.setRot(I,J,K, true);
	  * quadTree.changeBase (tmp);
	  * \endcode
	  *
	  * \param base Base of the quad grid
	  */
	void			changeBase(const NLMISC::CMatrix& base);

	/** Init the container. container is first clear() ed.
	  *
	  * \param size is the width and the height of the initial quad tree, in number of square.
	  *		For performance view, this should be a power of 2, and <=32768. (eg: 256,512, 8, 16 ...)
	  *	\param eltSize is the width and height of an element. Must be >0. Notice that the quadgrid MUST be square!!
	  */
	void			create(uint size, float eltSize);
	//@}

	/// \name Container operation
	//@{
	/// Clear the container. Elements are deleted, but the quadgrid is not erased.
	void			clear();

	/** Erase an interator from the container
	  *
	  * \param it is the iterator to erase.
	  * \return if element is currently selected, the next selected element is returned, (or end()).
	  * if the element is not selected, end() is returned.
	  */
	CIterator		erase(CIterator it);

	/** Insert a new element in the container.
	  *
	  * \param bboxmin is the corner of the bounding box of the element to insert with minimal coordinates.
	  * \param bboxmax is the corner of the bounding box of the element to insert with maximal coordinates.
	  * \param val is a reference on the value to insert.
	  */
	CIterator		insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val);
	//@}


	/// \name Selection
	//@{
	/** Clear the selection list
	  */
	void			clearSelection();

	/** Select all the container
	  */
	void			selectAll();

	/** Select element intersecting a bounding box. Clear the selection first.
	  *
	  * \param bboxmin is the corner of the bounding box used to select
	  * \param bboxmax is the corner of the bounding box used to select
	  */
	void			select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax);


	/** Return the first iterator of the selected element list. begin and end are valid till the next insert.
	  */
	CIterator		begin();

	/** Return the end iterator of the selected element list. begin and end are valid till the next insert.
	  */
	CIterator		end();
	//@}


// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:// Classes.

	class	CBaseNode
	{
	public:
		CBaseNode	*Prev,*Next;	// For selection.
		CBaseNode() {Prev= Next= NULL;}
	};

	class	CNode : public CBaseNode
	{
	public:
		T		Elt;
		uint16	x0,x1;			// The location of the elt in the grid. Used for erase().
		uint16	y0,y1;
	};

	class	CQuadNode
	{
	public:
		std::list<CNode*>	Nodes;
	};

private:// Atttributes.
	std::vector<CQuadNode>	_Grid;
	sint				_Size;
	sint				_SizePower;
	float				_EltSize;
	NLMISC::CMatrix		_ChangeBasis;
	// Selection.
	CBaseNode			_Selection;


private:// Methods.

	// return the coordinates on the grid of what include the bbox.
	void		selectQuads(CVector bmin, CVector bmax, sint &x0, sint &x1, sint &y0, sint &y1)
	{
		CVector		bminp, bmaxp;
		bminp= bmin;
		bmaxp= bmax;
		bmin.minof(bminp, bmaxp);
		bmax.maxof(bminp, bmaxp);
		bmin/= _EltSize;
		bmax/= _EltSize;
		x0= (sint)(floor(bmin.x));
		x1= (sint)(ceil(bmax.x));
		y0= (sint)(floor(bmin.y));
		y1= (sint)(ceil(bmax.y));

		// Very special case where the bbox.size==0 AND position is JUST on an edge of a case.
		if(x0==x1)
			x1++;
		if(y0==y1)
			y1++;

		// Manage tiling.
		if(x1-x0>=_Size)
			x0=0, x1= _Size;
		else
		{
			x0&= _Size-1;
			x1&= _Size-1;
			if(x1<=x0)
				x1+=_Size;
		}
		if(y1-y0>=_Size)
			y0=0, y1= _Size;
		else
		{
			y0&= _Size-1;
			y1&= _Size-1;
			if(y1<=y0)
				y1+=_Size;
		}
	}

	// If not done, add the node to the selection.
	void		addToSelection(CNode	*ptr)
	{
		if(ptr->Prev==NULL)
		{
			// Append to front of the list.
			ptr->Prev= &_Selection;
			ptr->Next= _Selection.Next;
			if(_Selection.Next)
				_Selection.Next->Prev= ptr;
			_Selection.Next= ptr;
		}
	}

	// Try to add each node of the quad node list.
	void		addQuadNodeToSelection(CQuadNode	&quad)
	{
		typename std::list<CNode*>::iterator	itNode;
		for(itNode= quad.Nodes.begin();itNode!=quad.Nodes.end();itNode++)
		{
			addToSelection(*itNode);
		}
	}


public:
	// CLASS const_iterator.
	class const_iterator
	{
	public:
		const_iterator()	{_Ptr=NULL;}
		const_iterator(CNode *p) : _Ptr(p) {}
		const_iterator(const CIterator& x) : _Ptr(x._Ptr) {}

		const T&	operator*() const
			{return _Ptr->Elt; }
		// Doesn't work...
		/*const T*	operator->() const
			{return (&**this); }*/
		const_iterator& operator++()
			{_Ptr = (CNode*)(_Ptr->Next); return (*this); }
		const_iterator operator++(int)
			{const_iterator tmp = *this; ++*this; return (tmp); }
		const_iterator& operator--()
			{_Ptr = (CNode*)(_Ptr->Prev); return (*this); }
		const_iterator operator--(int)
			{const_iterator tmp = *this; --*this; return (tmp); }
		bool operator==(const const_iterator& x) const
			{return (_Ptr == x._Ptr); }
		bool operator!=(const const_iterator& x) const
			{return (!(*this == x)); }
	protected:
		CNode	*_Ptr;
		friend class CQuadGrid<T>;
		friend class CIterator;
	};

	// CLASS CIterator
	class CIterator : public const_iterator
	{
	public:
		CIterator()			{this->_Ptr=NULL;}
		CIterator(CNode *p) : const_iterator(p) {}
		T&	operator*() const
			{return this->_Ptr->Elt; }
		// Doesn't work...
		/*T*	operator->() const
			{return (&**this); }*/
		CIterator& operator++()
			{this->_Ptr = (CNode*)(this->_Ptr->Next); return (*this); }
		CIterator operator++(int)
			{CIterator tmp = *this; ++*this; return (tmp); }
		CIterator& operator--()
			{this->_Ptr = (CNode*)(this->_Ptr->Prev); return (*this); }
		CIterator operator--(int)
			{CIterator tmp = *this; --*this; return (tmp); }
		bool operator==(const const_iterator& x) const
			{return (this->_Ptr == x._Ptr); }
		bool operator!=(const const_iterator& x) const
			{return (!(*this == x)); }
	protected:
		friend class CQuadGrid<T>;
	};


};


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// Template CQuadGrid implementation.
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// Init.
// ***************************************************************************


// ***************************************************************************
template<class T>	CQuadGrid<T>::CQuadGrid()
{
	_SizePower=4;
	_Size=1<<_SizePower;
	_EltSize=1;
	_ChangeBasis.identity();
}
// ***************************************************************************
template<class T>	CQuadGrid<T>::~CQuadGrid()
{
	clear();
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::changeBase(const NLMISC::CMatrix& base)
{
	_ChangeBasis= base;
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::create(uint size, float eltSize)
{
	clear();

	nlassert(NLMISC::isPowerOf2(size));
	nlassert(size<=32768);
	_SizePower= NLMISC::getPowerOf2(size);
	_Size=1<<_SizePower;
	_Grid.resize(_Size*_Size);

	nlassert(eltSize>0);
	_EltSize= eltSize;
}


// ***************************************************************************
// insert/erase.
// ***************************************************************************


// ***************************************************************************
template<class T>	void		CQuadGrid<T>::clear()
{
	CIterator	it;
	selectAll();
	while( (it=begin())!=end())
	{
		erase(it);
	}

	// Clear the selection...
	_Selection.Next= NULL;
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::erase(typename CQuadGrid<T>::CIterator it)
{
	sint	x,y;
	CNode	*ptr= it._Ptr;

	if(!ptr)
		return end();

	// First erase all references to it.
	//==================================
	for(y= ptr->y0;y<ptr->y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= ptr->x0;x<ptr->x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			typename std::list<CNode*>::iterator	itNode;
			for(itNode= quad.Nodes.begin();itNode!=quad.Nodes.end();itNode++)
			{
				if((*itNode)==ptr)
				{
					quad.Nodes.erase(itNode);
					break;
				}
			}
		}
	}

	// Then delete it..., and update selection linked list.
	//=====================================================
	// if selected.
	CBaseNode	*next= NULL;
	if(ptr->Prev)
	{
		next= ptr->Next;
		if(next)
			next->Prev=ptr->Prev;
		ptr->Prev->Next= next;
	}
	delete ptr;


	return CIterator((CNode*)next);
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val)
{
	CVector		bmin,bmax;
	bmin= _ChangeBasis*bboxmin;
	bmax= _ChangeBasis*bboxmax;

	CNode	*ptr= new CNode;
	ptr->Elt= val;

	// Find which quad include the object.
	//===================================
	sint	x0,y0;
	sint	x1,y1;
	selectQuads(bmin, bmax, x0,x1, y0,y1);

	ptr->x0= uint16(x0);
	ptr->x1= uint16(x1);
	ptr->y0= uint16(y0);
	ptr->y1= uint16(y1);

	// Then for all of them, insert the node in their list.
	//=====================================================
	sint	x,y;
	for(y= ptr->y0;y<ptr->y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= ptr->x0;x<ptr->x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			quad.Nodes.push_back(ptr);
		}
	}

	return CIterator(ptr);
}


// ***************************************************************************
// selection.
// ***************************************************************************


// ***************************************************************************
template<class T>	void			CQuadGrid<T>::clearSelection()
{
	CBaseNode	*ptr= _Selection.Next;
	while(ptr)
	{
		ptr->Prev= NULL;
		CBaseNode	*next= ptr->Next;
		ptr->Next= NULL;
		ptr= next;
	}

	_Selection.Next= NULL;
}
// ***************************************************************************
template<class T>	void			CQuadGrid<T>::selectAll()
{
	clearSelection();
	for(sint i=0;i<(sint)_Grid.size();i++)
	{
		CQuadNode	&quad= _Grid[i];
		addQuadNodeToSelection(quad);
	}
}
// ***************************************************************************
template<class T>	void			CQuadGrid<T>::select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax)
{
	CVector		bmin,bmax;
	bmin= _ChangeBasis*bboxmin;
	bmax= _ChangeBasis*bboxmax;

	clearSelection();

	// What are the quads to access?
	sint	x0,y0;
	sint	x1,y1;
	selectQuads(bmin, bmax, x0,x1, y0,y1);

	sint	x,y;
	for(y= y0;y<y1;y++)
	{
		sint	xe,ye;
		ye= y &(_Size-1);
		for(x= x0;x<x1;x++)
		{
			xe= x &(_Size-1);
			CQuadNode	&quad= _Grid[(ye<<_SizePower)+xe];
			addQuadNodeToSelection(quad);
		}
	}

}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::begin()
{
	return CIterator((CNode*)(_Selection.Next));
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::end()
{
	return CIterator(NULL);
}


} // NLPACS


#endif // NLPACS_QUAD_GRID_H

/* End of quad_grid.h */
