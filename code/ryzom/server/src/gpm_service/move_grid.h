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
#include "nel/misc/stream.h"

/**
 * A list of object that must have Next and Previous pointers
 * \param T the type of item
 * \param TPtr a pointer to T to use in list (useful for smartpointer)
 */
template<class TT>
class CMoveGridObjectList
{
public:
	TT									*Head;
	TT									*Tail;

	CMoveGridObjectList() : Head(NULL), Tail(NULL) {}

	void	insertAtHead(TT *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Next = Head;
		if (object->Next != NULL)
			object->Next->Previous = object;
		else
			Tail = object;
		Head = object;
	}

	void	insertAtTail(TT *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Previous = Tail;
		if (object->Previous != NULL)
			object->Previous->Next = object;
		else
			Head = object;
		Tail = object;
	}

	void	remove(TT *object)
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

	TT			*getHead() { return (TT*)Head; }
	TT			*getTail() { return (TT*)Tail; }
};

/**
 * Move grid, allows to select moving entities fast (template parameter CSIZE is in millimeter)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
template<typename T, int CELLS, int CSIZE>
class CMoveGrid
{
public:
	class CIterator;
	friend class CIterator;

protected:
	class CCellNode;
	class CNode;

public:

	/// Constructor
	CMoveGrid();

	/// Destructor
	~CMoveGrid();

	/// Clear
	void		clear();


	/// Insert an element in move grid. Return an iterator towards the inserted object
	CIterator	insert(const T &object, const NLMISC::CVector &position);


	/// Move an object in grid.
	void		move(CIterator &it, const NLMISC::CVector &position);


	/// Remove
	void		remove(CIterator &it);
	void		insertNode(CIterator &it, sint x, sint y);
	void		removeNode(CIterator &it);


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

	/// round value so it is a half of a segment
	double		round(double v)
	{
		const double	INV = 1000.0 / (double)CSIZE;
		return (floor(v*INV)+0.5)*(double)CSIZE*0.001;
	}

	/// Serial
	void		serial(NLMISC::IStream &f);

protected:
	sint		convert(float v)
	{
		const float	INV = 1000.0f / (float)CSIZE;
		return (sint)(v*INV);
	}

	sint		convertGrid(float v)
	{
		const float	INV = 1000.0f / (float)CSIZE;
		return (sint)(v*INV)&(CELLS-1);
	}

	sint		convertGrid(sint v)
	{
		return v&(CELLS-1);
	}

	void		select(sint X, sint Y);


protected:

	/// A node that contains an objects.
	typedef CMoveGridObjectList<CNode>		TNodeList;
	/// A node that is a cell, containing a list of node inside this cell.
	typedef CMoveGridObjectList<CCellNode>	TCellNodeList;

	class CCellNode
	{
	public:
		sint32		X, Y;
		sint32		GridX, GridY;

		TNodeList	NodeList;

		CCellNode	*Previous, *Next;
		CCellNode	*Selection;

		CCellNode() : X(0), Y(0), GridX(0), GridY(0), Previous(NULL), Next(NULL), Selection(NULL) {}

		void		serial(NLMISC::IStream &f)
		{
			f.serial(X, Y, GridX, GridY);
		}
	};

	class CNode
	{
	public:
		CNode		*Previous, *Next;
		CCellNode	*Root;
		T			Object;

		CNode() : Previous(NULL), Next(NULL), Root(NULL) {}

		void		serial(NLMISC::IStream &f)
		{
			f.serial(Object);
		}
	};

	/// The map of cell nodes
	TCellNodeList	_Grid[CELLS][CELLS];

	/// The first selected cell node
	CCellNode		*_Selection;

	/// The allocator of nodes
	NLMISC::CBlockMemory<CNode>		_NodeAllocator;
	/// The allocator of cell nodes
	NLMISC::CBlockMemory<CCellNode>	_CellNodeAllocator;

public:
	class CIterator
	{
		friend class CMoveGrid<T, CELLS, CSIZE>;
	public:
		CIterator(CNode *node = NULL) : _Node(node) {}
		CIterator(const CIterator &it) : _Node(it._Node) {}

		T			& operator * () { return _Node->Object; }

		CIterator	& operator ++ ()
		{ 
			if (_Node->Next != NULL)
			{
				_Node = _Node->Next;
			}
			else
			{
				CCellNode	*nextCell = _Node->Root->Selection;
				_Node = NULL;
				// iterate till we have a non empty selected cell
				while (nextCell != NULL && (_Node = nextCell->NodeList.getHead()) == NULL)
					nextCell = nextCell->Selection;
			}
			return *this;
		}
		CIterator	operator ++ (int)
		{ 
			CIterator ret(_Node);
			++(*this);
			return ret;
		}

		bool		operator == (const CIterator &it) const { return it._Node == _Node; }
		bool		operator != (const CIterator &it) const { return !(*this == it); }

		CIterator	operator = (const CIterator &it) { _Node = it._Node; return *this; }

	protected:
		CNode	*_Node;
	};
};

//
// TEMPLATE IMPLEMENTATION
//



// Serial
template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::serial(NLMISC::IStream &f)
{
	sint	version = f.serialVersion(0);
	f.serialCheck((sint32)CELLS);
	f.serialCheck((sint32)CSIZE);

	if (f.isReading())
	{
		clear();
		sint	i, j;
		uint32	nc, nn;

		for (i=0; i<CELLS; ++i)
		{
			for (j=0; j<CELLS; ++j)
			{
				TCellNodeList	&list = _Grid[i][j];
				CCellNode		*cellNode;
				CNode			*node;

				f.serial(nc);
				while (nc-- > 0)
				{
					cellNode = _CellNodeAllocator.allocate();
					f.serial(*cellNode);
					list.insertAtTail(cellNode);

					f.serial(nn);
					while (nn-- > 0)
					{
						node = _NodeAllocator.allocate();
						f.serial(*node);
						cellNode->NodeList.insertAtTail(node);
						node->Root = cellNode;
					}
				}
			}
		}
	}
	else
	{
		sint	i, j;
		uint32	nc, nn;

		for (i=0; i<CELLS; ++i)
		{
			for (j=0; j<CELLS; ++j)
			{
				TCellNodeList	&list = _Grid[i][j];
				CCellNode		*cellNode;
				CNode			*node;

				cellNode = list.getHead();
				nc = 0;
				while (cellNode != NULL)
					++nc, cellNode = cellNode->Next;

				f.serial(nc);
				cellNode = list.getHead();
				while (cellNode != NULL)
				{
					f.serial(*cellNode);

					node = cellNode->NodeList.getHead();
					nn = 0;
					while (node != NULL)
						++nn, node = node->Next;

					f.serial(nn);
					node = cellNode->NodeList.getHead();
					while (node != NULL)
					{
						f.serial(*node);
						node = node->Next;
					}

					cellNode = cellNode->Next;
				}
			}
		}
	}
}


//
template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::clear()
{
	sint	i, j;

	for (i=0; i<CELLS; ++i)
	{
		for (j=0; j<CELLS; ++j)
		{
			TCellNodeList	&list = _Grid[i][j];
			CCellNode		*cellNode;
			CNode			*node;
			while ((cellNode = list.getHead()) != NULL)
			{
				while ((node = cellNode->NodeList.getHead()) != NULL)
				{
					cellNode->NodeList.remove(node);
					_NodeAllocator.free(node);
				}
				list.remove(cellNode);
				_CellNodeAllocator.free(cellNode);
			}
		}
	}

	clearSelection();
}

//
template<typename T, int CELLS, int CSIZE>
CMoveGrid<T, CELLS, CSIZE>::CMoveGrid()
{
	_Selection = NULL;
}


//
template<typename T, int CELLS, int CSIZE>
CMoveGrid<T, CELLS, CSIZE>::~CMoveGrid()
{
	clear();
}

//
template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::insertNode(CIterator &it, sint x, sint y)
{
	sint	gridX, gridY;
	gridX = convertGrid(x);
	gridY = convertGrid(y);

	CCellNode *cellNode = _Grid[gridX][gridY].getHead();
	while (cellNode != NULL && (cellNode->X != x || cellNode->Y != y))
		cellNode = cellNode->Next;

	if (cellNode == NULL)
	{
		cellNode = _CellNodeAllocator.allocate();
		_Grid[gridX][gridY].insertAtHead(cellNode);
		cellNode->X = x;
		cellNode->Y = y;
		cellNode->GridX = gridX;
		cellNode->GridY = gridY;
	}

	it._Node->Root = cellNode;
	cellNode->NodeList.insertAtHead(it._Node);
}

//
template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::removeNode(CIterator &it)
{
	it._Node->Root->NodeList.remove(it._Node);
}

//
template<typename T, int CELLS, int CSIZE>
typename::CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::insert(const T &object, const NLMISC::CVector &position)
{
	CNode	*node = _NodeAllocator.allocate();

	sint	X, Y;

	X = convert(position.x);
	Y = convert(position.y);
	node->Object = object;

	CIterator it(node);
	insertNode(it, X, Y);

	return CIterator(node);
}

template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::move(CIterator &it, const NLMISC::CVector &position)
{
	sint	X, Y;
	X = convert(position.x);
	Y = convert(position.y);

	CNode	*node = it._Node;

	if (X == node->Root->X && Y == node->Root->Y)
		return;

	removeNode(it);
	insertNode(it, X, Y);
}

template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::remove(CIterator &it)
{
	CNode	*node = it._Node;

	removeNode(it);
	it._Node = NULL;

	_NodeAllocator.free(node);
}


template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::select(const NLMISC::CVector &position)
{
	select(convert(position.x), convert(position.y));
}

template<typename T, int CELLS, int CSIZE>
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

template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::select(sint x, sint y)
{
	sint	gx = convertGrid(x),
			gy = convertGrid(y);

	CCellNode	*cellNode = _Grid[gx][gy].getHead();
	while (cellNode != NULL && (cellNode->X != x || cellNode->Y != y))
		cellNode = cellNode->Next;

	if (cellNode != NULL && cellNode->NodeList.getHead() != NULL)
	{
		cellNode->Selection = _Selection;
		_Selection = cellNode;
	}
}

template<typename T, int CELLS, int CSIZE>
typename::CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::begin()
{
	return (_Selection != NULL) ? CIterator(_Selection->NodeList.getHead()) : CIterator(NULL);
}

template<typename T, int CELLS, int CSIZE>
typename::CMoveGrid<T, CELLS, CSIZE>::CIterator	CMoveGrid<T, CELLS, CSIZE>::end()
{
	return CIterator(NULL);
}

template<typename T, int CELLS, int CSIZE>
void	CMoveGrid<T, CELLS, CSIZE>::clearSelection()
{
	CCellNode	*cellNode = _Selection;
	_Selection = NULL;

	while (cellNode != NULL)
	{
		CCellNode	*nd = cellNode;
		cellNode = cellNode->Selection;
		nd->Selection = NULL;
	}
}


#endif // NL_MOVE_GRID_H

/* End of move_grid.h */
