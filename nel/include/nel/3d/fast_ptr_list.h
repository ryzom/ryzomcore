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

#ifndef NL_FAST_PTR_LIST_H
#define NL_FAST_PTR_LIST_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


class CFastPtrListBase;


// ***************************************************************************
/**
 * See CFastPtrListBase. Each class you want to insert in a CFastPtrList should have a CFastPtrListNode.
 */
class CFastPtrListNode
{
public:
	CFastPtrListNode() {_Owner= NULL;}
	~CFastPtrListNode() {unlink();}
	// No-op const copy
	CFastPtrListNode(const CFastPtrListNode &/* o */) {_Owner= NULL;}

	// If linked to a list, remove me from it.
	void			unlink();

	// linked?
	bool			isLinked() const {return _Owner!=NULL;}

	// No-op operator=
	CFastPtrListNode	&operator=(const CFastPtrListNode &/* o */)
	{
		return *this;
	}


private:
	friend class CFastPtrListBase;
	CFastPtrListBase	*_Owner;
	uint32				_IndexInOwner;
};


// ***************************************************************************
/**
 * This class store actually an array of void*, for very fast acces (list is slower because of RAM access).
 *	CFastPtrListBase advantages are the insert() and erase() are in O(1)
 *	Overhead Cost is 8 bytes per node + 4 bytes in the _Nodes array.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CFastPtrListBase
{
public:
	/// Constructor
	CFastPtrListBase() {}
	CFastPtrListBase(const CFastPtrListBase &/* o */) {}
	~CFastPtrListBase();

	/// insert an element in the list through its Node, unlinking older if necessary
	void			insert(void *element, CFastPtrListNode *node);
	/// erase an element in the list through its Node. No-op if the list does not have this element
	void			erase(CFastPtrListNode *node);

	/// Get the head on the array of elements. NULL if none
	void			**begin() { if(_Elements.empty()) return NULL; else return &_Elements[0];}
	/// get the number of elements
	uint			size() const {return (uint)_Elements.size();}
	bool			empty() const {return _Elements.empty();}

	/// clear the list
	void			clear();

	// operator= is noop. Cant do it because nodes keep a ptr on me!!
	CFastPtrListBase	&operator=(const CFastPtrListBase &/* o */) {return *this;}

// **************
private:
	// The 2 lists of same size. Splitted in 2 lists for optimum _Elements accessing.
	std::vector<void*>				_Elements;
	std::vector<CFastPtrListNode*>	_Nodes;
};


// ***************************************************************************
/** Type Safe version of CFastPtrListBase
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
template< class T >
class CFastPtrList : public CFastPtrListBase
{
public:
	/// Constructor
	CFastPtrList() {}
	~CFastPtrList() {}

	/// insert an element in the list through its Node, unlinking older if necessary
	void			insert(T *element, CFastPtrListNode *node) {CFastPtrListBase::insert(element, node);}
	/// erase an element in the list through its Node, unlinking older if necessary
	void			erase(CFastPtrListNode *node) {CFastPtrListBase::erase(node);}

	/// Get the head on the array of elements. NULL if none
	T				**begin() {return (T**)CFastPtrListBase::begin();}
	/// get the number of elements
	uint			size() const {return CFastPtrListBase::size();}
	bool			empty() const {return CFastPtrListBase::empty();}

	/// clear the list
	void			clear() {CFastPtrListBase::clear();}

};


} // NL3D


#endif // NL_FAST_PTR_LIST_H

/* End of fast_ptr_list.h */
