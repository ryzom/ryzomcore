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


#ifndef	NL_QUAD_TREE_H
#define	NL_QUAD_TREE_H

#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include <list>
#include <vector>


namespace	NL3D
{



/**
  * class:	CQuadTree.
  *
  *
  * A template CQuadTree.
  *
  *
  * This first implementation support real-time quad node split, but never merge the quad node.
  * The possibility to merge (delete) empty quads, when an element erase occurs, will be added later.
  *
  * The quadtree is geometrically delimited. By default, his size is 1*1, centered on (0,0,0). If an element
  * which is out this zone is inserted, then it will ALWAYS be considered selected in select*() methods.
  * By default, the quad tree is aligned on XZ.
  *
  * Sample code using CQuadTree:
  * \code
  * // My quad tree
  * CQuadTree<myType> quadTree;
  *
  * // My min and max BoundingBox corner of each element
  * CVector minPoint[elementCount]=...;
  * CVector maxPoint[elementCount]=...;
  *
  * // My values
  * myType value[elementCount]=...;
  *
  * // Init the quadTree with recursions depth = 6 (so max 64*64 cells)
  * // centered in (0,0,0) with a max size of 10 in the plane XZ
  * quadTree.create (6, CVector (0.f, 0.f, 0.f), 10.f);
  *
  * // Insert element in the quadTree
  * for (int i=0; i<elementCount; i++)
  *		quadTree.insert (minPoint[i], maxPoint[i], value[i]);
  *
  * // [...]
  *
  * // Clear the selection
  * quadTree.clearSelection ();
  *
  * // Select an element with the X axis as a 3d ray
  * quadTree.selectRay (CVector (0,0,0), CVector (1,0,0));
  *
  *	// Get first selected nodes..
  * CQuadTree<myType>::CIterator it=quadTree.begin();
  * while (it!=quadTree.end())
  * {
  * 	// Check what you want...
  *
  *		// Next selected element
  * 	it++;
  * }
  * \endcode
  */
template<class T>	class	CQuadTree
{

public:
	/// Iterator of the contener
	class	CIterator;
	friend class	CIterator;
	/// Const iterator of the contener
	class	CConstIterator;
	friend class	CConstIterator;

public:

	/// Default constructor, use axes XZ
	CQuadTree();

	/// dtor.
	~CQuadTree();

	/// \name Initialization
	//@{
	/** Change the base matrix of the quad tree. For exemple this code init the quad tree in the plane XY:
	  * \code
	  * CQuadTree			quadTree;
	  * NLMISC::CMatrix		tmp;
	  * NLMISC::CVector		I(1,0,0);
	  * NLMISC::CVector		J(0,0,-1);
	  * NLMISC::CVector		K(0,1,0);
	  *
	  * tmp.identity();
	  * tmp.setRot(I,J,K, true);
	  * quadTree.changeBase (tmp);
	  * \endcode
	  *
	  * \param base Base of the quad tree
	  */
	void changeBase(const NLMISC::CMatrix& base);

	/** Init the container
	  *
	  * \param DepthMax is the max depth in the tree. The max cell count is (1<<DepthMax)^2
	  * \param center is the center of the quad tree
	  * \param size is the width and the height of the initial quad tree.
	  */
	void		create(uint DepthMax, const NLMISC::CVector& center, float size);
	//@}


	/// \name Container operation
	//@{
	/// Clear the container. Elements are deleted, and the quadtree too (create() is undone)
	void		clear();

	/** Erase all elements from the container
	  */
	void		eraseAll();

	/** Erase an interator from the container
	  *
	  * \param it is the iterator to erase.
	  */
	void		erase(CIterator it);

	/** Insert a new element in the container. The bounding box of the element MUST be included in the bounding box of the quadtree.
	  *
	  * \param bboxmin is the corner of the bounding box of the element to insert with minimal coordinates.
	  * \param bboxmax is the corner of the bounding box of the element to insert with maximal coordinates.
	  * \param val is a reference on the value to insert
	  */
	CIterator	insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val);
	//@}


	/// \name Selection
	//@{
	/** Clear the selection list
	  */
	void		clearSelection();

	/** Select all the container
	  */
	void		selectAll();

	/** Select element intersecting a bounding box
	  *
	  * \param bboxmin is the corner of the bounding box used to select
	  * \param bboxmax is the corner of the bounding box used to select
	  */
	void		select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax);

	/** Select element with multiple planes. Intersect with a polytope convex made of planes.
	  * The normals of planes must be directed outwards the polytope.
	  *
	  * \param BVolume is a plane vector
	  */
	void		select(const std::vector<NLMISC::CPlane> &BVolume);

	/** Select element with a ray
	  *
	  * \param source is a point in the ray
	  * \param dir is the direction off the ray
	  */
	void		selectRay(const NLMISC::CVector& source, const NLMISC::CVector& dir);

	/** Select element with a segment
	  *
	  * \param source is the source of the segment
	  * \param dest is the destination of the segment
	  */
	void		selectSegment(const NLMISC::CVector& source, const NLMISC::CVector& dest);

	/** Return the first iterator of the selected element list. begin and end are valid till the next insert.
	  */
	CIterator	begin();

	/** Return the end iterator of the selected element list. begin and end are valid till the next insert.
	  */
	CIterator	end();
	//@}



// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:// Classes.


	// =================
	// =================
	// CBaseNode.
	// =================
	// =================
	// Links fo an element node class.
	class			CBaseNode
	{
	public:
		// for the Selection list.
		CBaseNode	*Prev, *Next;
		// for the quadnode list. A node MAY be pointed by 4 quad (each having the same level).
		CBaseNode	*QuadPrevs[4];
		CBaseNode	*QuadNexts[4];


	public:
		CBaseNode()
		{
			Prev=Next=NULL;
			QuadPrevs[0]= QuadPrevs[1]= QuadPrevs[2]= QuadPrevs[3]= NULL;
			QuadNexts[0]= QuadNexts[1]= QuadNexts[2]= QuadNexts[3]= NULL;
		}
		virtual ~CBaseNode() {}		// Empty destructor, but declare it as virtual...
		void	clear()				// update links.
		{
			// On le retire de la selection.
			if(Prev)	Prev->Next= Next;
			if(Next)	Next->Prev= Prev;
			Prev=Next=NULL;
			// On le retire des listes dans les quads.
			for(uint i=0;i<4;i++)
			{
				if(QuadPrevs[i])	{nlassert(QuadPrevs[i]->QuadNexts[i]==this); QuadPrevs[i]->QuadNexts[i]= QuadNexts[i];}
				if(QuadNexts[i])	{nlassert(QuadNexts[i]->QuadPrevs[i]==this); QuadNexts[i]->QuadPrevs[i]= QuadPrevs[i];}
				QuadPrevs[i]=NULL;
				QuadNexts[i]=NULL;
			}
		}
		bool	isSelected()	// return true if Prev is not NULL!!!
		{
			return Prev!=NULL;
		}
	};


	// =================
	// =================
	// CNode.
	// =================
	// =================
	// An element node class.
	class			CNode : public CBaseNode
	{
	public:
		// A base node, plus the Value.
		T			Value;
		CNode(const T &val) : Value(val) {}
	};


	// =================
	// =================
	// CQuadNode.
	// =================
	// =================
	class				CQuadNode
	{
	public:
		uint			Level;
		NLMISC::CVector			BBoxMin, BBoxMax;
		bool			BBoxNeverRescale;
		CQuadNode		*Sons[4];
		CBaseNode		RootNode;		// First element of the element list in this quad.
		uint			ListIndex;		// [0,3]. index of which list to follow in "Node.QuadNexts[]".
		/* Topology of sons (top view: axe x/z):
			0--1
			|  |
			2--3
		*/


	public:
		// ============================================================================================
		CQuadNode()
		{
			Level=0; Sons[0]= Sons[1]= Sons[2]= Sons[3]= NULL;
			ListIndex= 0;
			BBoxNeverRescale=true;
		}
		// ============================================================================================
		bool	isLeaf()
		{
			return Sons[0]==NULL;
		}
		// ============================================================================================
		void	clear()
		{
			// delete items in this quad node
			CBaseNode	*p;
			while( (p=RootNode.QuadNexts[ListIndex]) )
			{
				p->clear();	// clear the links. => RootNode.QuadNexts[ListIndex] is implicitly modified
				delete p;	// delete this element
			}

			// delete quad children
			for(uint i=0;i<4;i++)
			{
				if(Sons[i])
				{
					Sons[i]->clear();
					delete Sons[i];
					Sons[i]= NULL;
				}
			}
		}
		// ============================================================================================
		void	split()
		{
			nlassert(isLeaf());
			sint	i;

			for(i=0;i<4;i++)
			{
				Sons[i]= new CQuadNode;
				Sons[i]->Level= Level+1;
				Sons[i]->ListIndex= i;
			}
			// Middle compute.
			NLMISC::CVector		MidLeft(0,0,0), MidRight(0,0,0), MidTop(0,0,0), MidBottom(0,0,0), Middle(0,0,0);
			MidLeft.x  = BBoxMin.x; MidLeft.z	= (BBoxMin.z + BBoxMax.z)/2;
			MidRight.x = BBoxMax.x; MidRight.z	= (BBoxMin.z + BBoxMax.z)/2;
			MidTop.x   = (BBoxMin.x + BBoxMax.x)/2; MidTop.z   = BBoxMin.z;
			MidBottom.x= (BBoxMin.x + BBoxMax.x)/2; MidBottom.z= BBoxMax.z;
			Middle.x   = MidTop.x; Middle.z = MidLeft.z;
			// Sons compute.
			// Don't care of Y.
			Sons[0]->BBoxMin = BBoxMin; Sons[0]->BBoxMax = Middle;
			Sons[1]->BBoxMin = MidTop ; Sons[1]->BBoxMax = MidRight;
			Sons[2]->BBoxMin = MidLeft; Sons[2]->BBoxMax = MidBottom;
			Sons[3]->BBoxMin = Middle;  Sons[3]->BBoxMax = BBoxMax;

		}


		// ============================================================================================
		// This is a quadtree, so those tests just test against x/z. (the base of the box).
		bool	includeBoxQuad(const NLMISC::CVector &boxmin, const NLMISC::CVector &boxmax)
		{
			if( BBoxMin.x<= boxmin.x && BBoxMax.x>= boxmax.x &&
				BBoxMin.z<= boxmin.z && BBoxMax.z>= boxmax.z)
				return true;
			else
				return false;
		}
		// ============================================================================================
		bool	intersectBoxQuad(const NLMISC::CVector &boxmin, const NLMISC::CVector &boxmax)
		{
			// inequality and equality is very important, to ensure that a element box will not fit in too many quad boxes.
			if(boxmin.x > BBoxMax.x)	return false;
			if(boxmin.z > BBoxMax.z)	return false;
			if(boxmax.x <= BBoxMin.x)	return false;
			if(boxmax.z <= BBoxMin.z)	return false;
			return true;
		}
		// ============================================================================================
		bool	intersectBox(const NLMISC::CVector &boxmin, const NLMISC::CVector &boxmax)
		{
			// inequality and equality is very important, to ensure that a element box will not fit in too many quad boxes.
			if(boxmin.x > BBoxMax.x)	return false;
			if(boxmin.y > BBoxMax.y)	return false;
			if(boxmin.z > BBoxMax.z)	return false;
			if(boxmax.x <= BBoxMin.x)	return false;
			if(boxmax.y <= BBoxMin.y)	return false;
			if(boxmax.z <= BBoxMin.z)	return false;
			return true;
		}
		// ============================================================================================
		bool	intersectBox(std::vector<NLMISC::CPlane> &BVolume)
		{
			const	NLMISC::CVector	&b1=BBoxMin;
			const	NLMISC::CVector	&b2=BBoxMax;

			for(sint i=0;i<(int)BVolume.size();i++)
			{
				const	NLMISC::CPlane	&plane=BVolume[i];
				// If only one of the box vertex is IN the plane, then all the box is IN this plane.
				if(plane* NLMISC::CVector(b1.x, b1.y, b1.z)<=0) continue;
				if(plane* NLMISC::CVector(b1.x, b1.y, b2.z)<=0) continue;
				if(plane* NLMISC::CVector(b1.x, b2.y, b1.z)<=0) continue;
				if(plane* NLMISC::CVector(b1.x, b2.y, b2.z)<=0) continue;
				if(plane* NLMISC::CVector(b2.x, b1.y, b1.z)<=0) continue;
				if(plane* NLMISC::CVector(b2.x, b1.y, b2.z)<=0) continue;
				if(plane* NLMISC::CVector(b2.x, b2.y, b1.z)<=0) continue;
				if(plane* NLMISC::CVector(b2.x, b2.y, b2.z)<=0) continue;
				// If ALL box vertices are OUT of this plane, then the box is OUT of the entire volume.
				return false;
			}
			// TODO. This is a simple box detection. The box is not really clipped and sometimes, the box will said
			// it intersect but it is not the case... Here, We should test the real box volume, against BVolume.
			// But this is more expensive...
			return true;
		}

		// ============================================================================================
		void	addElement(CNode *newNode)
		{
			nlassert(newNode->QuadNexts[ListIndex]==NULL);
			newNode->QuadPrevs[ListIndex]= &RootNode;
			newNode->QuadNexts[ListIndex]= RootNode.QuadNexts[ListIndex];
			if(RootNode.QuadNexts[ListIndex])
				RootNode.QuadNexts[ListIndex]->QuadPrevs[ListIndex]= newNode;
			RootNode.QuadNexts[ListIndex]= newNode;
		}
		// ============================================================================================
		// Insertion of a node in this quad node, or his sons...
		void	insert(const NLMISC::CVector &boxmin, const NLMISC::CVector &boxmax, uint wantdepth, CNode *newNode)
		{
			if(Level==0)
			{
				// all out of quadtree items are in the root node
				if(!includeBoxQuad(boxmin, boxmax))
				{
					// expand Y-axe of quadnode BBox
					if(BBoxNeverRescale)
					{
						BBoxMin.y= boxmin.y;
						BBoxMax.y= boxmax.y;
						BBoxNeverRescale= false;
					}
					else
					{
						BBoxMin.y= std::min(boxmin.y, BBoxMin.y);
						BBoxMax.y= std::max(boxmax.y, BBoxMax.y);
					}
					addElement(newNode);
					return;
				}
			}

			// If at least one part of the item is not in the node of this quad, exit.
			if(!intersectBoxQuad(boxmin, boxmax))
				return;

			// If we are inserting there or its children, we have to resize the Y-axe BBox of quadnode.
			if(BBoxNeverRescale)
			{
				BBoxMin.y= boxmin.y;
				BBoxMax.y= boxmax.y;
				BBoxNeverRescale= false;
			}
			else
			{
				BBoxMin.y= std::min(boxmin.y, BBoxMin.y);
				BBoxMax.y= std::max(boxmax.y, BBoxMax.y);
			}

			// If we are at the right level, we only have to insert it in this node.
			if(wantdepth==Level)
			{
					addElement(newNode);
			}
			else
			{
				// If the quad is a leaf, we need to split it (because we are not yet at the right level).
				if(isLeaf())
					split();

				// And we are looking to put the item in one of these nodes.
				Sons[0]->insert(boxmin, boxmax, wantdepth, newNode);
				Sons[1]->insert(boxmin, boxmax, wantdepth, newNode);
				Sons[2]->insert(boxmin, boxmax, wantdepth, newNode);
				Sons[3]->insert(boxmin, boxmax, wantdepth, newNode);
			}

		}


		// ============================================================================================
		// Selection.
		void	selectLocalNodes(CBaseNode	&selroot)
		{
			CBaseNode	*p= RootNode.QuadNexts[ListIndex];
			while(p)
			{
				if(!p->isSelected())
				{
					p->Prev= &selroot;
					p->Next= selroot.Next;
					if(selroot.Next)
						selroot.Next->Prev= p;
					selroot.Next= p;
				}
				p=p->QuadNexts[ListIndex];
			}
		}
		// ============================================================================================
		void	selectAll(CBaseNode	&selroot)
		{
			selectLocalNodes(selroot);
			if(!isLeaf())
			{
				Sons[0]->selectAll(selroot);
				Sons[1]->selectAll(selroot);
				Sons[2]->selectAll(selroot);
				Sons[3]->selectAll(selroot);
			}
		}
		// ============================================================================================
		void	select(CBaseNode &selroot, const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax)
		{
			// TODO:
			// there is a bug with level0: bbox is not expanded to contain items.
			if(!intersectBox(bboxmin, bboxmax))
				return;
			selectLocalNodes(selroot);
			if(!isLeaf())
			{
				Sons[0]->select(selroot, bboxmin, bboxmax);
				Sons[1]->select(selroot, bboxmin, bboxmax);
				Sons[2]->select(selroot, bboxmin, bboxmax);
				Sons[3]->select(selroot, bboxmin, bboxmax);
			}
		}
		// ============================================================================================
		void		select(CBaseNode &selroot, std::vector<NLMISC::CPlane> &BVolume)
		{
			// TODO:
			// there is a bug with level0: bbox is not expanded to contain items.
			if(!intersectBox(BVolume))
				return;
			selectLocalNodes(selroot);
			if(!isLeaf())
			{
				Sons[0]->select(selroot, BVolume);
				Sons[1]->select(selroot, BVolume);
				Sons[2]->select(selroot, BVolume);
				Sons[3]->select(selroot, BVolume);
			}
		}
	};



// =================
// =================
// Attributes/Methods/iterators..
// =================
// =================
private:// Attributes.
	CQuadNode	_QuadRoot;
	CBaseNode	_Selection;
	uint		_DepthMax;
	float		_Size;
	NLMISC::CMatrix		_ChangeBasis;

private:// Methods.



public:
	// CLASS const_iterator.
	class const_iterator
	{
	public:
		const_iterator()	{_Ptr=NULL;}
		const_iterator(CNode *p) : _Ptr(p) {}
		const_iterator(const CIterator& x) : _Ptr(x._Ptr) {}

		const T&	operator*() const
			{return _Ptr->Value; }
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
		friend class CQuadTree<T>;
		friend class CIterator;
	};

	// CLASS CIterator
	class CIterator : public const_iterator
	{
	public:
		CIterator()			{const_iterator::_Ptr=NULL;}
		CIterator(CNode *p) : const_iterator(p) {}
		T&	operator*() const
			{return const_iterator::_Ptr->Value; }
		// Doesn't work...
		/*T*	operator->() const
			{return (&**this); }*/
		CIterator& operator++()
			{const_iterator::_Ptr = (CNode*)(const_iterator::_Ptr->Next); return (*this); }
		CIterator operator++(int)
			{CIterator tmp = *this; ++*this; return (tmp); }
		CIterator& operator--()
			{const_iterator::_Ptr = (CNode*)(const_iterator::_Ptr->Prev); return (*this); }
		CIterator operator--(int)
			{CIterator tmp = *this; --*this; return (tmp); }
		bool operator==(const const_iterator& x) const
			{return (const_iterator::_Ptr == x._Ptr); }
		bool operator!=(const const_iterator& x) const
			{return (!(*this == x)); }
	protected:
		friend class CQuadTree<T>;
	};

};



// ============================================================================================
// ============================================================================================
// Template CQuadTree implementation. Construction/Destruction.
// ============================================================================================
// ============================================================================================


// ============================================================================================
template<class T>	CQuadTree<T>::CQuadTree()
{
	_Selection.Next= NULL;
	_DepthMax= 0;
	_QuadRoot.BBoxMin.set(-0.5, 0, -0.5);
	_QuadRoot.BBoxMax.set( 0.5, 0,  0.5);
	_Size=1;

	_ChangeBasis.identity();
}
// ============================================================================================
template<class T>	void CQuadTree<T>::changeBase(const NLMISC::CMatrix& base)
{
	_ChangeBasis=base;
}
// ============================================================================================
template<class T>	CQuadTree<T>::~CQuadTree NL_TMPL_PARAM_ON_METHOD_1(T)()
{
	clear();
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::clear()
{
	_QuadRoot.clear();
	_Selection.Next= NULL;
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::create(uint DepthMax, const NLMISC::CVector& center, float size)
{
	NLMISC::CVector mycenter=_ChangeBasis*center;
	clear();
	_DepthMax= DepthMax;
	_Size= size;
	_QuadRoot.BBoxMin= mycenter-NLMISC::CVector(size/2, 0 , size/2);
	_QuadRoot.BBoxMax= mycenter+NLMISC::CVector(size/2, 0 , size/2);
}


// ============================================================================================
// ============================================================================================
// Template CQuadTree implementation.  Element Insertion/Deletion.
// ============================================================================================
// ============================================================================================


// ============================================================================================
template<class T>	void		CQuadTree<T>::eraseAll()
{
	CIterator				it;
	std::vector<CIterator>	its;

	// First, make a copy of all elements.
	selectAll();
	for(it= begin();it!=end();it++)
	{
		its.push_back(it);
	}

	// Then erase them. Must do it OUTSIDE the select loop.
	for(sint i=0;i<(sint)its.size();i++)
	{
		erase(its[i]);
	}
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::erase(CIterator it)
{
	CNode	*p=it._Ptr;
	if(p)
	{
		// Clear links.
		p->clear();

		// delete it!!
		delete p;
	}
}
// ============================================================================================
template<class T>	typename CQuadTree<T>::CIterator	CQuadTree<T>::insert(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax, const T &val)
{
	NLMISC::CVector myboxmin2=_ChangeBasis*bboxmin;
	NLMISC::CVector myboxmax2=_ChangeBasis*bboxmax;
	NLMISC::CVector myboxmin (std::min (myboxmin2.x, myboxmax2.x), std::min (myboxmin2.y, myboxmax2.y), std::min (myboxmin2.z, myboxmax2.z));
	NLMISC::CVector myboxmax (std::max (myboxmin2.x, myboxmax2.x), std::max (myboxmin2.y, myboxmax2.y), std::max (myboxmin2.z, myboxmax2.z));

	CNode	*newNode=new CNode(val);

	nlassert(myboxmax.x>=myboxmin.x);
	nlassert(myboxmax.y>=myboxmin.y);
	nlassert(myboxmax.z>=myboxmin.z);

	float	boxsize= std::max(myboxmax.x-myboxmin.x, myboxmax.z-myboxmin.z );
	// Prevent float precision problems. Increase bbox size a little.
	boxsize*=1.01f;
	// We must find the level quad which is just bigger.
	float	wantsize=_Size;
	uint	wantdepth=0;
	while(boxsize<wantsize/2 && wantdepth<_DepthMax)
	{
		wantsize/=2;
		wantdepth++;
	}

	_QuadRoot.insert(myboxmin, myboxmax, wantdepth, newNode);

	return CIterator(newNode);
}


// ============================================================================================
// ============================================================================================
// Template CQuadTree implementation. Quad Selection, element iteration.
// ============================================================================================
// ============================================================================================


// ============================================================================================
template<class T>	void		CQuadTree<T>::clearSelection()
{
	CBaseNode	*p;
	while((p=_Selection.Next))
	{
		// We are removing this node from selection, which will implicitly modify _Selection.Next.
		if(p->Prev)	p->Prev->Next= p->Next;
		if(p->Next)	p->Next->Prev= p->Prev;
		p->Prev=p->Next=NULL;
	}
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::selectAll()
{
	clearSelection();
	_QuadRoot.selectAll(_Selection);
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::select(const NLMISC::CVector &bboxmin, const NLMISC::CVector &bboxmax)
{
	NLMISC::CVector myboxmin2=_ChangeBasis*bboxmin;
	NLMISC::CVector myboxmax2=_ChangeBasis*bboxmax;
	NLMISC::CVector bboxminCopy (std::min (myboxmin2.x, myboxmax2.x), std::min (myboxmin2.y, myboxmax2.y), std::min (myboxmin2.z, myboxmax2.z));
	NLMISC::CVector bboxmaxCopy (std::max (myboxmin2.x, myboxmax2.x), std::max (myboxmin2.y, myboxmax2.y), std::max (myboxmin2.z, myboxmax2.z));

	clearSelection();
	_QuadRoot.select(_Selection, bboxminCopy, bboxmaxCopy);
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::select(const std::vector<NLMISC::CPlane> &BVolume)
{
	std::vector<NLMISC::CPlane> BVolumeCopy;
	BVolumeCopy.resize (BVolume.size());
	for (int i=0; i<(int)BVolumeCopy.size(); i++)
	{
		BVolumeCopy[i]=BVolume[i]*((_ChangeBasis).inverted());
	}

	clearSelection();
	_QuadRoot.select(_Selection, BVolumeCopy);
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::selectRay(const NLMISC::CVector& source, const NLMISC::CVector& dir)
{
	NLMISC::CMatrix mat;
	mat.identity ();

	// Set a wrong matrix
	NLMISC::CVector vTmp=dir^((fabs(vTmp*NLMISC::CVector(1,0,0))>0.f)?NLMISC::CVector(1,0,0):NLMISC::CVector(0,1,0));
	mat.setRot (dir, vTmp, dir^vTmp);

	// Normalize it Yoyo!
	mat.normalize (NLMISC::CMatrix::XYZ);

	// Get the planes..
	std::vector<NLMISC::CPlane> BVolume;
	BVolume.reserve (4);

	// Setup the planes
	NLMISC::CPlane plane;
	plane.make (mat.getJ(), source);
	BVolume.push_back (plane);
	plane.make (mat.getK(), source);
	BVolume.push_back (plane);
	plane.make (-mat.getJ(), source);
	BVolume.push_back (plane);
	plane.make (-mat.getK(), source);
	BVolume.push_back (plane);

	// Select the nodes
	select (BVolume);
}
// ============================================================================================
template<class T>	void		CQuadTree<T>::selectSegment(const NLMISC::CVector& source, const NLMISC::CVector& dest)
{
	NLMISC::CMatrix mat;
	mat.identity ();

	// Set a wrong matrix
	NLMISC::CVector dir=dest-source;
	NLMISC::CVector vTmp=dir^((fabs(vTmp*NLMISC::CVector(1,0,0))>0.f)?NLMISC::CVector(1,0,0):NLMISC::CVector(0,1,0));
	mat.setRot (dir, vTmp, dir^vTmp);

	// Normalize it Yoyo!
	mat.normalize (NLMISC::CMatrix::XYZ);

	// Get the planes..
	std::vector<NLMISC::CPlane> BVolume;
	BVolume.reserve (4);

	// Setup the planes
	NLMISC::CPlane plane;
	plane.make (mat.getJ(), source);
	BVolume.push_back (plane);
	plane.make (mat.getK(), source);
	BVolume.push_back (plane);
	plane.make (-mat.getJ(), source);
	BVolume.push_back (plane);
	plane.make (-mat.getK(), source);
	BVolume.push_back (plane);
	plane.make (mat.getI(), dest);
	BVolume.push_back (plane);
	plane.make (-mat.getI(), source);
	BVolume.push_back (plane);

	// Select the nodes
	select (BVolume);
}
// ============================================================================================
template<class T>	typename CQuadTree<T>::CIterator	CQuadTree<T>::begin()
{
	return (CNode*)(_Selection.Next);
}
// ============================================================================================
template<class T>	typename CQuadTree<T>::CIterator	CQuadTree<T>::end()
{
	return CIterator(NULL);
}


}

#endif

