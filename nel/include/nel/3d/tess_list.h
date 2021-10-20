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

#ifndef NL_TESS_LIST_H
#define NL_TESS_LIST_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


// ***************************************************************************
/** A basic list node.
 *
 */
class	CTessNodeList
{
public:
	CTessNodeList	*Prec;
	CTessNodeList	*Next;

	CTessNodeList()
	{
		Prec= NULL;
		Next= NULL;
	}
};


// ***************************************************************************
/** A basic speed list gestion, to add/remove already created Object. Object must herit from CTessNodeList.
 *	Also, provide a fast size().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template<class T> class	CTessList
{
private:
	T				*Root;
	sint			Size;

public:
	CTessList()		{Root= NULL; Size= 0;}

	sint			size() const {return Size;}
	T				*begin() {return Root;}

	void			append(T *node)
	{
		Size++;
		nlassert(node);
		// inserted???
		nlassert(node->Prec==NULL && Root!=node);
		// update Next.
		node->Next= Root;
		if(Root)
			Root->Prec= node;
		// update Prec.
		node->Prec= NULL;
		Root= node;
	}
	void			remove(T *node)
	{
		Size--;
		nlassert(node);
		// inserted???
		nlassert(node->Prec!=NULL || Root==node);
		nlassert(Size>=0);
		// update Prec.
		if(!node->Prec)
		{
			nlassert(Root==node);
			Root= (T*)node->Next;
		}
		else
		{
			node->Prec->Next= node->Next;
		}
		// update Next.
		if(node->Next)
			node->Next->Prec= node->Prec;
		node->Next= NULL;
		node->Prec= NULL;
	}

	// This only clear this list, not the elements. Also, Elements Next/Prec ptrs are NOT modified.
	void			clear()
	{
		Root= NULL;
		Size= 0;
	}

};


} // NL3D


#endif // NL_TESS_LIST_H

/* End of tess_list.h */
