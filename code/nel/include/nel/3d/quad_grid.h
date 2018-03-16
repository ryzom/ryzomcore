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

#ifndef NL_QUAD_GRID_H
#define NL_QUAD_GRID_H

#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/common.h"
#include "nel/misc/polygon.h"
#include "nel/misc/grid_traversal.h"

#include <limits>
#include <vector>
#include <map>

namespace NL3D
{


using NLMISC::CVector;

// ***************************************************************************
// Default Size of a block for allocation of elements and elements node list in the grid.
#define	NL3D_QUAD_GRID_ALLOC_BLOCKSIZE	16


// base class containing structure common to all quad grids
class CQuadGridBase
{
protected:
	static NLMISC::CPolygon2D				_ScaledPoly;		// for polygon selection
	static NLMISC::CPolygon2D::TRasterVect  _PolyBorders;		// for polygon selection
	static std::vector<uint>				_AlreadySelected;	// During some selection operations, mark the cells that have already been visited.
																// may happen if the selection shape can overlap itself due to the grid vrapiing.
																// A cell is known to be selected if its uint "timestamp" is equal to _SelectStamp.
	static uint								_SelectStamp;		// Incremented at each selection, value used to mark selected cells.
};

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
 * For maximum allocation speed Efficiency, it uses a CBlockMemory<CNode> to allocate elements at insert().
 * DefaultBlockSize is 16, but you can change it at construction.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template<class T>	class	CQuadGrid : public CQuadGridBase
{
public:
	/// Iterator of the contener
	class	CIterator;
	friend class	CIterator;

public:
	typedef std::vector<uint> TSelectionShape;

	/// Default constructor, use axes XY!!!, has a size of 16, and EltSize is 1.
	CQuadGrid(uint memoryBlockSize= NL3D_QUAD_GRID_ALLOC_BLOCKSIZE);

	/// dtor.
	~CQuadGrid();

	// operator= don't modify the block memory blocksize
	CQuadGrid<T>	&operator=(const CQuadGrid<T> &o);

	// Copy ctor
	CQuadGrid(const CQuadGrid<T> &o);

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

	// Get creation parameters.
	const NLMISC::CMatrix	&getBasis() const {return _ChangeBasis;}
	uint					getSize() const {return _Size;}
	float					getEltSize() const {return _EltSize;}
	//@}

	/// \name Container operation
	//@{
	/** Clear the container. Elements are deleted, but the quadgrid is not erased.
	 *	Speed is in O(Nelts)
	 */
	void			clear();

	/** Erase an interator from the container
	  *	Speed is in O(1 * L*H) where L*H is the number of squares surrounded by the element
	  *
	  * \param it is the iterator to erase.
	  * \return if element is currently selected, the next selected element is returned, (or end()).
	  * if the element is not selected, end() is returned.
	  */
	CIterator		erase(CIterator it);

	/** Insert a new element in the container.
	  *	Speed is in O(1 * L*H) where L*H is the number of squares surrounded by the element
	  *
	  *	Warning! : bboxmin and bboxmax are multiplied by matrix setuped by changeBase. This work for any
	  *	matrix with 90deg rotations (min and max are recomputed internally), but not with any rotation (43 degrees ...)
	  *	because of the nature of AABBox. To do this correclty you should compute the bbox min and max in the
	  *	basis given in changeBase, and insert() with multiplying min and max with inverse of this basis.
	  *	eg:
	  *		CMatrix					base= getSomeBase();
	  *		CMatrix					invBase= base.inverted();
	  *		// create quadGrid.
	  *		CQuadGrid<CTriangle>	quadGrid;
	  *		quadGrid.changeBase(base);
	  *		quadGrid.create(...);
	  *		// Insert a triangle tri correctly
	  *		CAABBox					bbox;
	  *		bbox.setCenter(base * tri.V0);
	  *		bbox.extend(base * tri.V1);
	  *		bbox.extend(base * tri.V2);
	  *		quadGrid.insert(invBase*bbox.getMin(), invBase*bbox.getMax(), tri);
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
	  *	Speed is in O(Nelts)
	  */
	void			clearSelection();

	/** Select all the container.
	  *	Speed is in O(Nelts)
	  */
	void			selectAll();

	/** Select element intersecting a bounding box. Clear the selection first.
	  *	Speed is in O(Nelts * L*H), where L*H is the number of squares surrounded by the selection
	  *
	  * \param bboxmin is the corner of the bounding box used to select
	  * \param bboxmax is the corner of the bounding box used to select
	  */
	void			select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax);

	// Select element intersecting a ray. Clear the selection first.
	void			selectRay(const NLMISC::CVector &rayStart, const NLMISC::CVector &rayEnd);

	/** Build a selection from a convex polygon. The resulting selection can then be used for a subsequent call
	  * to 'select'
	  */
	void			buildSelectionShape(TSelectionShape &dest, const NLMISC::CPolygon2D &poly) const;

	/** Select element intersecting a selection shape. Clear the selection first.
	  */
	void			select(const TSelectionShape &shape);


	/** Return the first iterator of the selected element list. begin and end are valid till the next insert.
	  *	Speed is in O(1)
	  */
	CIterator		begin();

	/** Return the end iterator of the selected element list. begin and end are valid till the next insert.
	  *	Speed is in O(1)
	  */
	CIterator		end();
	//@}


// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:// Classes.
	class CNode;

	/** A circular list node for the list of node per Quad element.
	 *	The root node have Node==NULL
	 */
	class	CQuadNode
	{
	public:
		CQuadNode	*Prev,*Next;
		CNode		*Node;

		CQuadNode() : Prev(NULL), Next(NULL), Node(NULL) {}

		// can't call this at ctor since copied in array
		void	initRoot()
		{
			Prev= this;
			Next= this;
			Node= NULL;
		}
	};

	/** A base node (not circular) for the list of selected or unselected node
	 */
	class	CBaseNode
	{
	public:
		CBaseNode	*Prev,*Next;	// For selection.
		bool		Selected;		// true if owned by _SelectedList, or by _UnSelectedList.
		CBaseNode() {Prev= Next= NULL;}
	};

	/** An element inserted in the quadGrid. T + Link-list variables (CBaseNode and QuadNodes)
	 */
	class	CNode : public CBaseNode
	{
	public:
		T		Elt;
		// A node can be contained in L*H squares. This vector is a place holder for each node of each quad list
		NLMISC::CObjectVector<CQuadNode>	QuadNodes;
	};

private:// Atttributes.
	std::vector<CQuadNode>	_Grid;
	sint				_Size;
	sint				_SizePower;
	float				_EltSize;
	NLMISC::CMatrix		_ChangeBasis;
	// Selection. Elements are either in _SelectedList or in _UnSelectedList
	CBaseNode			_SelectedList;		// list of elements selected.
	CBaseNode			_UnSelectedList;	// circular list of elements not selected
	// The memory for nodes
	NLMISC::CBlockMemory<CNode>					_NodeBlockMemory;


private:// Methods.


	// default constor imp
	void		initCons();

	// link a node to a root: Selected or UnSelected list
	void		linkToRoot(CBaseNode &root, CBaseNode *ptr)
	{
		ptr->Prev= &root;
		ptr->Next= root.Next;
		ptr->Prev->Next= ptr;
		if(ptr->Next)
			ptr->Next->Prev= ptr;
	}

	void initSelectStamps() const
	{
		if (_AlreadySelected.size() < _Grid.size())
		{
			_AlreadySelected.resize(_Grid.size(), _SelectStamp);
		}
		++ _SelectStamp;
		if (_SelectStamp == 0)
		{
			std::fill(_AlreadySelected.begin(), _AlreadySelected.end(), std::numeric_limits<uint>::max());
		}
	}

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
		// if not selected
		if(!ptr->Selected)
		{
			// remove from the unselected list.
			ptr->Prev->Next= ptr->Next;
			if(ptr->Next)
				ptr->Next->Prev= ptr->Prev;

			// Append to front of the _Selected list.
			linkToRoot(_SelectedList, ptr);

			// mark it
			ptr->Selected= true;
		}
	}

	// Try to add each node of the quad node list.
	void		addQuadNodeToSelection(CQuadNode	&quad)
	{
		// NB: the root quadNode does not contains any Node
		nlassert(quad.Node==NULL);
		CQuadNode	*qn= quad.Next;
		// until we roll all the circular list
		while(qn!=&quad)
		{
			nlassert(qn->Node);
			addToSelection(qn->Node);
			qn= qn->Next;
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
template<class T>	CQuadGrid<T>::CQuadGrid(uint memoryBlockSize) :
	_NodeBlockMemory(memoryBlockSize)
{
	initCons();
}
// ***************************************************************************
template<class T>	CQuadGrid<T>::CQuadGrid(const CQuadGrid<T> &o) : _NodeBlockMemory(o._NodeBlockMemory)
{
	// _NodeBlockMemory Copy ctor just init block memory setup (not data)
	// init default
	initCons();
	// copy
	operator=(o);
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::initCons()
{
	_ChangeBasis.identity();
	// create a valid grid with default size
	create(16, 1);
}
// ***************************************************************************
template<class T>	CQuadGrid<T> &CQuadGrid<T>::operator=(const CQuadGrid<T> &o)
{
	// this==o test
	if(this==&o)
		return *this;

	// recreate (full cleared first)
	create(o._Size, o._EltSize);
	nlassert(_Grid.size()==o._Grid.size());
	nlassert(_SelectedList.Next==NULL);
	nlassert(_UnSelectedList.Next==NULL);

	// copy basis
	_ChangeBasis= o._ChangeBasis;

	// Fill with copy of elements of other grid. Complex copy...
	std::map<const CNode*, CNode *>	srcNodeToDestNode;
	std::map<const CNode*, uint>		srcNodeToIndexInQuadNodes;
	// NB: the order of nodes in CNode::QuadNodes is not important (may be different from src to dst)
	for(uint i=0;i<_Grid.size();i++)
	{
		const CQuadNode		&quadSrcRoot= o._Grid[i];
		CQuadNode			&quadDstRoot= _Grid[i];
		const CQuadNode		*quadSrcCur= quadSrcRoot.Next;
		// until we roll all the circular list
		while(quadSrcCur!=&quadSrcRoot)
		{
			const CNode	*srcNode= quadSrcCur->Node;
			nlassert(srcNode);

			// get the dest node created for this src node
			CNode	*dstNode= NULL;
			typename std::map<const CNode*, CNode *>::iterator	it= srcNodeToDestNode.find(srcNode);
			if(it!=srcNodeToDestNode.end())
			{
				dstNode= it->second;
			}
			// else this src node had not already been created
			else
			{
				// create a new node, copy content, and resize place holder
				dstNode=_NodeBlockMemory.allocate();
				dstNode->Elt= srcNode->Elt;
				dstNode->QuadNodes.resize(srcNode->QuadNodes.size());

				// Link to _Unselected list.
				linkToRoot(_UnSelectedList, dstNode);
				// mark as not selected.
				dstNode->Selected= false;

				// insert in the map
				srcNodeToDestNode[srcNode]= dstNode;

				// insert in the map of "index in quad node array"
				srcNodeToIndexInQuadNodes[srcNode]= 0;
			}

			// get the quadnode to insert in, and increment index
			uint	index= srcNodeToIndexInQuadNodes[srcNode]++;
			nlassert(index<dstNode->QuadNodes.size());
			CQuadNode	&quadDstCur= dstNode->QuadNodes[index];

			// link
			quadDstCur.Node= dstNode;
			// insert in back of list
			quadDstCur.Next= &quadDstRoot;
			quadDstCur.Prev= quadDstRoot.Prev;
			quadDstRoot.Prev->Next= &quadDstCur;
			quadDstRoot.Prev= &quadDstCur;


			// next
			quadSrcCur= quadSrcCur->Next;
		}
	}

	return *this;
}
// ***************************************************************************
template<class T>	CQuadGrid<T>::~CQuadGrid()
{
	clear();
	_Grid.clear();
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::changeBase(const NLMISC::CMatrix& base)
{
	_ChangeBasis= base;
}
// ***************************************************************************
template<class T>	void		CQuadGrid<T>::create(uint size, float eltSize)
{
	// full clear
	clear();
	_Grid.clear();

	// recreate
	nlassert(NLMISC::isPowerOf2(size));
	nlassert(size<=32768);
	_SizePower= NLMISC::getPowerOf2(size);
	_Size=1<<_SizePower;
	_Grid.resize(_Size*_Size);
	// Init QuadNode Root (can't be done in ctor() because of vector<> copy)
	for(uint i=0;i<_Grid.size();i++)
		_Grid[i].initRoot();

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

	// Clear the 2 selection...
	_SelectedList.Next= NULL;
	_UnSelectedList.Next= NULL;
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::erase(typename CQuadGrid<T>::CIterator it)
{
	CNode	*ptr= it._Ptr;

	if(!ptr)
		return end();

	// First erase from all QuadNode list
	//==================================
	for(uint i=0;i<ptr->QuadNodes.size();i++)
	{
		CQuadNode	&qn= ptr->QuadNodes[i];
		// unlink from circular list
		qn.Next->Prev= qn.Prev;
		qn.Prev->Next= qn.Next;
	}
	ptr->QuadNodes.clear();


	// Then delete it..., and update selection linked list.
	//=====================================================
	// remove it from _SelectedList or _UnSelectedList
	CBaseNode	*next= NULL;
	next= ptr->Next;
	if(next)
		next->Prev=ptr->Prev;
	ptr->Prev->Next= next;
	// if not selected, then must return NULL
	if(!ptr->Selected)
		next= NULL;
	// delete the object.
	_NodeBlockMemory.freeBlock(ptr);


	return CIterator((CNode*)next);
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator	CQuadGrid<T>::insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val)
{
	CVector		bmin,bmax;
	bmin= _ChangeBasis*bboxmin;
	bmax= _ChangeBasis*bboxmax;

	// init the object.
	CNode	*ptr= _NodeBlockMemory.allocate();
	ptr->Elt= val;
	// Link to _Unselected list.
	linkToRoot(_UnSelectedList, ptr);
	// mark as not selected.
	ptr->Selected= false;


	// Find which quad include the object.
	//===================================
	sint	x0,y0;
	sint	x1,y1;
	selectQuads(bmin, bmax, x0,x1, y0,y1);

	// must fit at lease in one quad
	sint	wn= x1-x0;
	sint	hn= y1-y0;
	nlassert(wn>0 && hn>0);
	// NB: this allocation may be slow (don't use BlockMemory system). But STLPort smallblock alloc
	// works quite well (if <128 bytes, ie a block of 10 squares)
	ptr->QuadNodes.resize(wn*hn);

	// Then for all of them, insert the node in their list.
	//=====================================================
	sint	x,y;
	for(y= y0;y<y1;y++)
	{
		sint	xg,yg;		// x,y in grid (_Grid[])
		sint	xn,yn;		// x,y in node array (ptr->QuadNodes[])
		yg= y &(_Size-1);
		yn= y-y0;
		for(x= x0;x<x1;x++)
		{
			xg= x &(_Size-1);
			xn= x-x0;
			CQuadNode	&quadRoot= _Grid[(yg<<_SizePower)+xg];
			CQuadNode	&quadNew= ptr->QuadNodes[yn*wn+xn];
			// let point on the node created
			quadNew.Node= ptr;
			// insert in back of list
			quadNew.Next= &quadRoot;
			quadNew.Prev= quadRoot.Prev;
			quadRoot.Prev->Next= &quadNew;
			quadRoot.Prev= &quadNew;
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
	CBaseNode	*ptr= _SelectedList.Next;
	while(ptr)
	{
		// next selected.
		CBaseNode	*nextSelected= ptr->Next;

		// Link to _Unselected list.
		linkToRoot(_UnSelectedList, ptr);

		// mark as not selected.
		ptr->Selected= false;

		// next.
		ptr= nextSelected;
	}

	// the selected list is now empty.
	_SelectedList.Next= NULL;
}
// ***************************************************************************
template<class T>	void			CQuadGrid<T>::selectAll()
{
	// This is the opposite of clearSelection(). get all that are in _UnSelectedList,
	// and put them in _SelectedList
	CBaseNode	*ptr= _UnSelectedList.Next;
	while(ptr)
	{
		// next selected.
		CBaseNode	*nextUnSelected= ptr->Next;

		// Link to _Selected list.
		linkToRoot(_SelectedList, ptr);

		// mark as selected.
		ptr->Selected= true;

		// next.
		ptr= nextUnSelected;
	}

	// the Unselected list is now empty.
	_UnSelectedList.Next= NULL;
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
template<class T> void	CQuadGrid<T>::buildSelectionShape(TSelectionShape &dest, const NLMISC::CPolygon2D &poly) const
{
	dest.clear();
	sint minY;
	uint numVerts = (uint)poly.Vertices.size();
	_ScaledPoly.Vertices.resize(numVerts);
	nlassert(_EltSize != 0.f);
	float invScale = 1.f / _EltSize;
	for(uint k = 0; k < numVerts; ++k)
	{
		CVector xformPos = _ChangeBasis * CVector(poly.Vertices[k]);
		_ScaledPoly.Vertices[k].set(poly.Vertices[k].x * invScale, poly.Vertices[k].y * invScale);
	}
	_ScaledPoly.computeOuterBorders(_PolyBorders, minY);
	if (_PolyBorders.empty()) return;
	initSelectStamps();
	sint numSegs = (sint)_PolyBorders.size();
	for (sint y = 0; y < numSegs; ++y)
	{
		sint currIndex = ((minY + y) & (_Size - 1)) << _SizePower;
		for (sint x = _PolyBorders[y].first; x <= _PolyBorders[y].second; ++x)
		{
			sint currX = x & (_Size - 1);
			uint index = (uint) (currX + currIndex);
			if (_AlreadySelected[index] != _SelectStamp)
			{
				_AlreadySelected[index] = _SelectStamp; // update stamp, so that won't be selected twice if
												// there's an overlap
				dest.push_back(index);
			}
		}
	}
}

// ***************************************************************************
template<class T> void	CQuadGrid<T>::select(const TSelectionShape &shape)
{
	clearSelection();
	for (TSelectionShape::const_iterator it = shape.begin(); it != shape.end(); ++it)
	{
		addQuadNodeToSelection(_Grid[*it]);
	}
}

// ***************************************************************************
template<class T>
void CQuadGrid<T>::selectRay(const NLMISC::CVector &rayStart, const NLMISC::CVector &rayEnd)
{
	clearSelection();
	CVector localRayStart = _ChangeBasis * rayStart;
	CVector localRayEnd = _ChangeBasis * rayEnd;
	float invScale = 1.f / _EltSize;
	NLMISC::CVector2f localRayStart2f(localRayStart.x * invScale, localRayStart.y * invScale);
	NLMISC::CVector2f localRayEnd2f(localRayEnd.x * invScale, localRayEnd.y * invScale);
	NLMISC::CVector2f localRayDir = localRayEnd2f - localRayStart2f;
	initSelectStamps();
	sint x, y;
	NLMISC::CGridTraversal::startTraverse(localRayStart2f, x, y);
	do
	{
		uint index = (x & (_Size - 1)) + ((y & (_Size - 1))  << _SizePower);
		if (_AlreadySelected[index] != _SelectStamp)
		{
			_AlreadySelected[index] = _SelectStamp; // update stamp, so that won't be selected twice if
			addQuadNodeToSelection(_Grid[index]);
		}
	}
	while (NLMISC::CGridTraversal::traverse(localRayStart2f, localRayDir, x, y));
}


// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::begin()
{
	return CIterator((CNode*)(_SelectedList.Next));
}
// ***************************************************************************
template<class T>	typename CQuadGrid<T>::CIterator		CQuadGrid<T>::end()
{
	return CIterator(NULL);
}


} // NL3D


#endif // NL_QUAD_GRID_H

/* End of quad_grid.h */
