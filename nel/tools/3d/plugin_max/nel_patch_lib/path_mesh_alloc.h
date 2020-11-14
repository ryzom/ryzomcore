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

#ifndef NL_PATH_MESH_ALLOC_H
#define NL_PATH_MESH_ALLOC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include <vector>
#include <list>

template <class T>
class CArrayElement : public NLMISC::CRefCount
{
public:
	CArrayElement (uint defaultSize)
	{
		_Array.reserve (defaultSize);
		_Allocated=false;
	}
	bool									_Allocated;
	std::vector<T>							_Array;
};

/**
 * TODO Class description
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CPathMeshAlloc
{
public:

	/// Constructor
	CPathMeshAlloc(uint defaultSize)
	{
		_DefaultSize=defaultSize;
		_BlockAllocated=0;
	}

	// Allocate a vector
	std::vector<T>* allocate ()
	{
		_BlockAllocated++;

		// Some trace
		nldebug ("Allocate, %d blocks %d max\n", _BlockAllocated, _ArrayList.size());

		// Look for a free element
		ListArray::iterator	ite=_ArrayList.begin();

		// for each element
		while (ite!=_ArrayList.end())
		{
			// Find one ?
			if (!((*ite)->_Allocated))
			{
				(*ite)->_Allocated=true;
				return &((*ite)->_Array);
			}
			ite++;
		}

		// ** Not find, add an entry
		
		// Create
		CArrayElement<T>	*pElement=new CArrayElement<T> (_DefaultSize);

		// Push back the enrty
		_ArrayList.push_back (NLMISC::CSmartPtr<CArrayElement<T> > (pElement));
		ite=_ArrayList.end();
		ite--;
		(*ite)->_Allocated=true;
		return &((*ite)->_Array);
	}

	// Free a vector
	void free (std::vector<T>* ptr)
	{
		_BlockAllocated--;

		// Some trace
		nldebug ("Allocate, %d blocks %d max\n", _BlockAllocated, _ArrayList.size());

		// Look for the good array
		ListArray::iterator	ite=_ArrayList.begin();

		// for each element
		while (ite!=_ArrayList.end())
		{
			// Find one ?
			if (&((*ite)->_Array)==ptr)
			{
				(*ite)->_Allocated=false;
				return;
			}
			ite++;
		}

		// No, should be somewhere
		nlassert (0);		// no!
	}

private:
	// Typedef 
typedef std::list< NLMISC::CSmartPtr<CArrayElement<T> > > ListArray;

	uint										_DefaultSize;
	uint										_BlockAllocated;
	ListArray									_ArrayList;
};

#endif // NL_PATH_MESH_ALLOC_H

/* End of path_mesh_alloc.h */
