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




#ifndef RYAI_ENTITY_LIST_LINK_H
#define RYAI_ENTITY_LIST_LINK_H

//--------------------------------------------------------------------------
// Advanced class definitions
//--------------------------------------------------------------------------

// NOTE: We define CAIEntityPhysical here to avoid header inclusion circularity

//	class CAIEntityPhysical;

//--------------------------------------------------------------------------
// The list link class for lists of CAIEntityPhysical
//--------------------------------------------------------------------------

//------------------------------------------------------------------
// Sub class for linked list linkage
template	<class	T>
class	CEntityListLink
{
public:
	//------------------------------------------------
	// ctor & dtor
	inline CEntityListLink()	:	_entity(NULL)
	{
		_next=this;
		_prev=this;		
	}
	
	inline CEntityListLink(T *entity)	:	_entity(entity)
	{
		_next=this;
		_prev=this;		
	}

	inline	~CEntityListLink()
	{
		unlink();
	}

	inline	void	setEntity(T	*entityPtr)
	{
#ifdef NL_DEBUG		
		nlassert(entityPtr);
#endif
		_entity=entityPtr;
	}
	
	//------------------------------------------------
	// read accessors
	inline	CEntityListLink<T> *	const	&	next()	const
	{
		return _next;
	}

	inline	CEntityListLink<T> *	const	&	prev()	const
	{
		return _prev;
	}
	
	inline	T	*entity() const
	{
		return	_entity;
	}

	//------------------------------------------------
	// link & unlink
	inline	void link(CEntityListLink &predecessor)
	{
		nlassert(_entity);

		//	unlink (do not refresh prev and next as its done below).
		_prev->_next=_next;
		_next->_prev=_prev;
		
		//	link
		_prev=&predecessor;
		_next=predecessor._next;
		_prev->_next=this;
		_next->_prev=this;
	}

	inline bool unlinked() const
	{
		return	_next==this;	//_next==this; // AlKy modification.
	}
	
	inline	void	unlink()
	{
#ifdef NL_DEBUG
		_entity=NULL;
#endif
		if (unlinked())
			return;
			
		_prev->_next=_next;
		_next->_prev=_prev;
		_next=this;
		_prev=this;
	}

private:
	CEntityListLink	*_next;
	CEntityListLink	*_prev;
	T	*_entity;
};

//--------------------------------------------------------------------------

#endif

