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

#ifndef NL_ORDERING_TABLE_H
#define NL_ORDERING_TABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/smart_ptr.h"
#include <vector>

namespace NL3D
{

// ***************************************************************************
/**
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
template<class T> class COrderingTable
{

public:

	COrderingTable();
	~COrderingTable();

	// copy ctor. NB : Allocator is not shared from source, and bahave like when reset(0) is called.
	COrderingTable(const COrderingTable<T> &other);

	// assignement operator. NB : Allocator is not shared from source, and bahave like when reset(0) is called.
	COrderingTable &operator=(const COrderingTable<T> &other);


	/**
	 * Initialization.
	 * The ordering table has a range from 0 to nNbEntries-1
	 */
	void init( uint32 nNbEntries );

	/**
	 * Just return the number of entries in the ordering table
	 */
	uint32 getSize();

	/**
	 * Put the ordering table to empty
	 *	\param maxElementToInsert prepare allocator for insert by setting maximum insert() that will arise.
	 */
	void reset(uint maxElementToInsert);

	/** Share allocator between 2 or more ordering tables. So that calling reset will give the max number of insert
      * for both tables. This is useful if several table are used for sorting (example : sort by priority with one table per possible priority)
	  * NB : the table of "source table" becomes the used allocator
	  */
	void shareAllocator(COrderingTable<T> &sourceTable) { _Allocator = sourceTable._Allocator; }

	/**
	 * Insert an element in the ordering table
	 *	NB: element is inserted in front of the list at nEntryPos (for optim consideration)
	 *	NB: nlassert in debug if num of insert() calls exceed value passed in reset()
	 *	NB: nlassert in debug if nEntryPos is => getSize()
	 */
	void insert( uint32 nEntryPos, T *pValue );

	/**
	 * Traversing operations
	 *
	 *	OrderingTable<Face> ot;
	 *	ot.begin();
	 *	while( ot.get() != NULL )
	 *	{
	 *		Face *pF = ot.get();
	 *		// Do the treatment you want here
	 *		ot.next();
	 *	}
	 */
	void begin();

	/**
	 * Get the currently selected element.
	 */
	T* get();

	/**
	 * Move selection pointer to the next element
	 */
	void next();

// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:

	struct CNode
	{
		T *val;
		CNode *next;

		CNode()
		{
			val = NULL;
			next = NULL;
		}
	};

	class CAllocator : public NLMISC::CRefCount
	{
	public:
		std::vector<CNode> NodePool;
		CNode			   *CurAllocatedNode;
		CAllocator() : CurAllocatedNode(NULL) {}
	};
	// a raw allocator of node.
	NLMISC::CSmartPtr<CAllocator>	_Allocator;


	uint32 _nNbElt;
	CNode* _Array;
	CNode* _SelNode;

};

// ***************************************************************************
template<class T> COrderingTable<T>::COrderingTable()
{
	_nNbElt = 0;
	_Array = NULL;
	_SelNode = NULL;
	_Allocator = new CAllocator;
}

// ***************************************************************************
template<class T> COrderingTable<T>::~COrderingTable()
{
	delete [] _Array;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::init( uint32 nNbEntries )
{
	delete [] _Array;
	_nNbElt = nNbEntries;
	if (nNbEntries == 0) return;
	_Array = new CNode[_nNbElt];
	reset(0);
}

// ***************************************************************************
template<class T> uint32 COrderingTable<T>::getSize()
{
	return _nNbElt;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::reset(uint maxElementToInsert)
{
	// reset allocation
	maxElementToInsert= std::max(1U, maxElementToInsert);
	_Allocator->NodePool.resize(maxElementToInsert);
	_Allocator->CurAllocatedNode= &_Allocator->NodePool[0];

	// reset OT.
	for( uint32 i = 0; i < _nNbElt-1; ++i )
	{
		_Array[i].val = NULL;
		_Array[i].next = &_Array[i+1];
	}
	_Array[_nNbElt-1].val  = NULL;
	_Array[_nNbElt-1].next = NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::insert( uint32 nEntryPos, T *pValue )
{
#ifdef NL_DEBUG
	// check not so many calls to insert()
	nlassert( !_Allocator->NodePool.empty() && _Allocator->CurAllocatedNode < (&_Allocator->NodePool[0])+_Allocator->NodePool.size() );
	// check good entry size
	nlassert( nEntryPos < _nNbElt );
#endif
	// get the head list node
	CNode *headNode = &_Array[nEntryPos];
	// alocate a new node
	CNode *nextNode = _Allocator->CurAllocatedNode++;
	// fill this new node with data of head node
	nextNode->val= headNode->val;
	nextNode->next= headNode->next;
	// and replace head node with new data: consequence is pValue is insert in front of the list
	headNode->val= pValue;
	headNode->next= nextNode;
	// NB: prec of headNode is still correclty linked to headNode.
}

// ***************************************************************************
template<class T> void COrderingTable<T>::begin()
{
	_SelNode = &_Array[0];
	if( _SelNode->val == NULL )
		next();
}

// ***************************************************************************
template<class T> T* COrderingTable<T>::get()
{
	if( _SelNode != NULL )
		return _SelNode->val;
	else
		return NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::next()
{
	_SelNode = _SelNode->next;
	while( ( _SelNode != NULL )&&( _SelNode->val == NULL ) )
		_SelNode = _SelNode->next;
}


// ***************************************************************************
template <class T>
inline COrderingTable<T>::COrderingTable(const COrderingTable<T> &other)
{
	_nNbElt = 0;
	_Array = NULL;
	_SelNode = NULL;
	*this = other;
}


// ***************************************************************************
template <class T>
inline COrderingTable<T> &COrderingTable<T>::operator=(const COrderingTable<T> &other)
{
	_Allocator = new CAllocator;
	init(other._nNbElt);
	return *this;
}



} // NL3D


#endif // NL_ORDERING_TABLE_H

/* End of ordering_table.h */
