// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef NL_MOVE_GRID_H
#define NL_MOVE_GRID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/block_memory.h"

/**
 * A list of object that must have Next and Previous pointers
 * \param T the type of item
 * \param TPtr a pointer to T to use in list (useful for smartpointer)
 */
template<class T, class TPtr = T*>
class CObjectList
{
public:
	TPtr	Head;
	TPtr	Tail;

	CObjectList() : Head(NULL), Tail(NULL) {}

	void	insertAtHead(T *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Next = Head;
		if (object->Next != NULL)
			object->Next->Previous = object;
		Head = object;
	}

	void	insertAtTail(T *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Previous = Tail;
		if (object->Previous != NULL)
			object->Previous->Next = object;
		Tail = object;
	}

	void	remove(T *object)
	{
		// if object at head
		if (object->Previous == NULL)
			Head = object->Next;
		else
			object->Previous->Next = object->Next;

		// if object at tail
		if (object->Next == NULL)
			Tail = object->Previous;
		else
			object->Next->Previous = object->Previous;

		object->Previous = NULL;
		object->Next = NULL;
	}

	T			*getHead() { return (T*)Head; }
	T			*getTail() { return (T*)Tail; }
};

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
template<typename T, int CELLS, float CSIZE>
class CMoveGrid
{
public:
	class CIterator;
	friend class CIterator;

protected:
	class CNode;

public:

	/// Constructor
	CMoveGrid();

	/// Destructor
	~CMoveGrid();


	/// Insert
	CIterator	insert(const T &object, const NLMISC::CVector &position);


	/// Move
	void		move(CIterator &it, const NLMISC::CVector &position);


	/// Remove
	void		remove(CIterator &it);


	/// select
	void		select(const NLMISC::CVector &position);

	/// select
	void		select(const NLMISC::CAABBox &bbox);

	/// begin
	CIterator	begin();

	/// end
	CIterator	end();

	/// clearSelection
	void		clearSelection();

protected:
	sint		convert(float v)
	{
		const float	INV = 1.0f / CSIZE;
		return (sint)(v*INV);
	}

	sint		convertGrid(float v)
	{
		const float	INV = 1.0f / CSIZE;
		return (sint)(v*INV)&(CELLS-1);
	}

	sint		convertGrid(sint v)
	{
		return v&(CELLS-1);
	}

	void		select(sint X, sint Y);

protected:
	class CNode
	{
	public:
		sint	X, Y;
		sint	GridX, GridY;

		CNode	*Previous, *Next;
		CNode	*Selection;
		T		Object;

		CNode() : X(0), Y(0), GridX(0), GridY(0), Previous(NULL), Next(NULL), Selection(NULL) {}
	};

	typedef CObjectList<CNode>	TNodeList;
	TNodeList	_Grid[CELLS][CELLS];

	NLMISC::CBlockMemory<CNode>	_NodeAllocator;

	CNode		*_Selection;

public:
	class CIterator
	{
		friend class CMoveGrid<T, CELLS, CSIZE>;
	public:
		CIterator(CNode *node = NULL) : _Node(node) {}
		CIterator(const CIterator &it) : _Node(it._Node) {}

		T			& operator * () { return _Node->Object; }

		CIterator	& operator ++ () { _Node = _Node->Selection; return *this; }
		CIterator	operator ++ (int) { CIterator ret(_Node); _Node = _Node->Selection; return ret; }

		bool		operator == (const CIterator &it) { return it._Node == _Node; }
		bool		operator != (const CIterator &it) { return !(*this == it); }

		CIterator	operator = (const CIterator &it) { _Node = it._Node; return *this; }

	protected:
		CNode	*_Node;
	};
};


//
// TEMPLATE IMPLEMENTATION
//


//
template<typename T, int CELLS, float CSIZE>
CMoveGrid<T, CELLS, CSIZE>::CMoveGrid()
{
	_Selection = NULL;
}


//
template<typename T, int CELLS, float CSIZE>
CMoveGrid<T, CELLS, CSIZE>::~CMoveGrid()
{
	sint	i, j;

	for (i=0; i<CELLS; ++i)
	{
		for (j=0; j<CELLS; ++j)
		{
			TNodeList	&list = _Grid[i][j];
			CNode		*node;
			while ((node = list.getHead()) != NULL)
			{
				list.remove(node);
				_NodeAllocator.free(node);
			}
		}
	}
}

//
template<typename T, int CELLS, float CSIZE>
CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::insert(const T &object, const NLMISC::CVector &position)
{
	CNode	*node = _NodeAllocator.allocate();

	node->Object = object;
	node->X = convert(position.x);
	node->Y = convert(position.y);
	node->GridX = convertGrid(node->X);
	node->GridY = convertGrid(node->Y);

	_Grid[node->GridX][node->GridY].insertAtHead(node);

	return CIterator(node);
}

template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::move(CIterator &it, const NLMISC::CVector &position)
{
	sint	X, Y;

	CNode	*node = it._Node;

	X = convert(position.x);
	Y = convert(position.y);

	if (X == node->X && Y == node->Y)
		return;

	_Grid[node->GridX][node->GridY].remove(node);

	node->X = X;
	node->Y = Y;

	node->GridX = convertGrid(X);
	node->GridY = convertGrid(Y);

	_Grid[node->GridX][node->GridY].insertAtHead(node);
}

template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::remove(CIterator &it)
{
	CNode	*node = it._Node;
	_Grid[node->GridX][node->GridY].remove(node);

	it._Node = NULL;

	_NodeAllocator.free(node);
}


template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::select(const NLMISC::CVector &position)
{
	sint	X, Y;

	select(convert(position.x), convert(position.y));
}

template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::select(const NLMISC::CAABBox &bbox)
{
	sint	x0, x1;
	sint	y0, y1;

	x0 = convert(bbox.getCenter().x-bbox.getHalfSize().x);
	x1 = convert(bbox.getCenter().x+bbox.getHalfSize().x);

	y0 = convert(bbox.getCenter().y-bbox.getHalfSize().y);
	y1 = convert(bbox.getCenter().y+bbox.getHalfSize().y);

	sint	x, y;

	for (y=y0; y<=y1; ++y)
		for (x=x0; x<=x1; ++x)
			select(x, y);
}

template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::select(sint x, sint y)
{
	sint	gx = convertGrid(x),
			gy = convertGrid(y);

	CNode	*node = _Grid[gx][gy].getHead();

	while (node != NULL)
	{
		if (node->X == x && node->Y == y && node->Selection == NULL)
		{
			node->Selection = _Selection;
			_Selection = node;
		}
		node = node->Next;
	}
}

template<typename T, int CELLS, float CSIZE>
CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::begin()
{
	return CIterator(_Selection);
}

template<typename T, int CELLS, float CSIZE>
CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::end()
{
	return CIterator(NULL);
}

template<typename T, int CELLS, float CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::clearSelection()
{
	CNode	*node = _Selection;
	_Selection = NULL;

	while (node != NULL)
	{
		CNode	*nd = node;
		node = node->Selection;
		nd->Selection = NULL;
	}
}


#endif // NL_MOVE_GRID_H

/* End of move_grid.h */
