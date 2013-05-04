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

#ifndef NL_LAYERED_ORDERING_TABLE_H
#define NL_LAYERED_ORDERING_TABLE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ordering_table.h"

namespace NL3D {


/**
 * The same as an ordering table, but it allows to have several layers for the display.
 * - Layer 0 acts as an ordering table.
 * - Layer 1 is a simple vector, with no sort.
 * - Layer 2 acts as an ordering table.
 * Typically, this can be used to sort objects that are above or below  a surface (which fit in layer 1).
 */
template <class T>
class CLayeredOrderingTable
{
public:

	/// ctor
	CLayeredOrderingTable();

	/**
	 * Initialization.
	 * The ordering tables have a range from 0 to nNbEntries-1
	 */
	void init( uint32 nNbEntries );

	/**
	 * Just return the number of entries in the ordering tables
	 */
	uint32 getSize();

	/**
	 * Put all the layers to empty
	 *	\param maxElementToInsert prepare allocator for insert by setting maximum insert() that will arise.
	 */
	void reset(uint maxElementToInsert);

	/**
	 * Insert an element in the ordering table
	 * \param layer the layer in which to insert the object. Might be 0, 1 or 2
	 * \param entry pos The position for the ordering tables. It is ignored when the layer is 1.
	 */
	void insert( uint layer, T *pValue, uint32 nEntryPos = 0 );

	/** Share allocator between 2 or more layered ordering tables. So that calling reset will give the max number of insert
      * for both tables. This is useful if several table are used for sorting (example : sort by priority with one table per possible priority)
	  * NB : the table of "source table" becomes the used allocator
	  */
	void shareAllocator(CLayeredOrderingTable<T> &sourceTable);

	/**
	 * Traversal operations
	 *
	 *	LayeredOrderingTable<Face> ot;
	 *	ot.begin();
	 *	while( ot.get() != NULL )
	 *	{
	 *		Face *pF = ot.get();
	 *		// Do the treatment you want here
	 *		ot.next();
	 *	}
	 *
	 * \param forwardTraversal true to traverse from layer 0 to layer 2 and vice versa.
	 */
	inline void begin(bool forwardTraversal = true);

	/**
	 * Get the currently selected element.
	 */
	inline T* get();

	/**
	 * Move selection pointer to the next element
	 */
	inline void next();

// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:
	typedef std::vector<T *> TTypePtVect;
	COrderingTable<T>		_Layer0;
	TTypePtVect				_Layer1;
	COrderingTable<T>		_Layer2;
	uint					_IndexInLayer1;
	uint8					_CurrLayer;
	bool					_ForwardTraversal;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
CLayeredOrderingTable<T>::CLayeredOrderingTable()  : _IndexInLayer1(0), _CurrLayer(0), _ForwardTraversal(true)
{
	// share allocator between layer 0 and 2
	_Layer2.shareAllocator(_Layer0);
}

//==================================================================
template <class T>
void CLayeredOrderingTable<T>::init( uint32 nNbEntries )
{
	_Layer0.init(nNbEntries);
	_Layer2.init(nNbEntries);
}

//==================================================================
template <class T>
uint32 CLayeredOrderingTable<T>::getSize()
{
	nlassert(_Layer0.getSize() == _Layer2.getSize());
	return _Layer0.getSize();
}


//==================================================================
template <class T>
void CLayeredOrderingTable<T>::shareAllocator(CLayeredOrderingTable<T> &sourceTable)
{
	_Layer0.shareAllocator(sourceTable._Layer0);
	_Layer2.shareAllocator(sourceTable._Layer0);
}

//==================================================================
template <class T>
void CLayeredOrderingTable<T>::reset(uint maxElementToInsert)
{
	_Layer0.reset(maxElementToInsert);
	_Layer1.clear();
	_Layer2.reset(maxElementToInsert);
}

//==================================================================
template <class T>
void CLayeredOrderingTable<T>::insert( uint layer, T *pValue, uint32 nEntryPos /* = 0 */)
{
	switch (layer)
	{
		case 0:
			_Layer0.insert(nEntryPos, pValue);
		break;
		case 1:
			_Layer1.push_back(pValue);
		break;
		case 2:
			_Layer2.insert(nEntryPos, pValue);
		break;
		default:
			nlassert(0); // invalid layer
		break;
	}
}

//==================================================================
template <class T>
inline void CLayeredOrderingTable<T>::begin(bool forwardTraversal /*= true*/)
{
	_ForwardTraversal = forwardTraversal;
	if (forwardTraversal)
	{
		_Layer0.begin();
		if (!_Layer0.get())
		{
			if (_Layer1.size() != 0)
			{
				_CurrLayer = 1;
				_IndexInLayer1 = 0;
			}
			else
			{
				_CurrLayer = 2;
				_Layer2.begin();
			}
		}
		else
		{
			_CurrLayer = 0;
		}
	}
	else
	{
		_Layer2.begin();
		if (!_Layer2.get())
		{
			if (_Layer1.size() != 0)
			{
				_CurrLayer = 1;
				_IndexInLayer1 = 0;
			}
			else
			{
				_CurrLayer = 0;
				_Layer0.begin();
			}
		}
		else
		{
			_CurrLayer = 2;
		}
	}
}

//==================================================================
template <class T>
inline T* CLayeredOrderingTable<T>::get()
{
	switch(_CurrLayer)
	{
		case 0:
			return _Layer0.get();
		break;
		case 1:
			return _Layer1[_IndexInLayer1];
		break;
		case 2:
			return _Layer2.get();
		break;
		default:
			nlassert(0);
		break;
	}
	return NULL; // avoid warning
}

//==================================================================
template <class T>
inline void CLayeredOrderingTable<T>::next()
{
	if (_ForwardTraversal)
	{
		switch(_CurrLayer)
		{
			case 0:
				_Layer0.next();
				if (_Layer0.get() == NULL)
				{
					if (_Layer1.size() != 0)
					{
						_CurrLayer = 1;
						_IndexInLayer1 = 0;
					}
					else
					{
						_CurrLayer = 2;
						_Layer2.begin();
					}
				}

			break;
			case 1:
				++ _IndexInLayer1;
				if (_IndexInLayer1 == _Layer1.size())
				{
					_CurrLayer = 2;
					_Layer2.begin();
				}
			break;
			case 2:
				_Layer2.next();
			break;
		}
	}
	else
	{
		switch(_CurrLayer)
		{

			case 2:
				_Layer2.next();
				if (_Layer2.get() == NULL)
				{
					if (_Layer1.size() != 0)
					{
						_CurrLayer = 1;
						_IndexInLayer1 = 0;
					}
					else
					{
						_CurrLayer = 0;
						_Layer0.begin();
					}
				}

			break;
			case 1:
				++ _IndexInLayer1;
				if (_IndexInLayer1 == _Layer1.size())
				{
					_CurrLayer = 0;
					_Layer0.begin();
				}
			break;
			case 0:
				_Layer0.next();
			break;
		}
	}
}





} // NL3D


#endif // NL_LAYERED_ORDERING_TABLE_H

/* End of layered_ordering_table.h */
