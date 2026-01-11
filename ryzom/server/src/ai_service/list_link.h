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



#ifndef RYAI_LIST_LINK_H
#define RYAI_LIST_LINK_H

//---------------------------------------------------------------------------------
// CListLink<>
//---------------------------------------------------------------------------------
// A link for linking objects into lists
// The only valid constructor requires a pointer to the parent class that the
// list link represents

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355)
#endif // NL_OS_WINDOWS

template <class C>
class CListLink
{
public:
	inline CListLink(C *parent) : _next(this), _prev(this), _parent(parent)
	{
	}

	inline virtual ~CListLink()
	{
		unlink();
	}
	
	inline void linkAfter(CListLink<C> *other)
	{
		#ifdef NL_DEBUG
			nlassert(_next==this);
			nlassert(_prev==this);
//			nlassert(other->_next->_prev==other->_prev);
//			nlassert(other->_prev->_next==other->_next);
		#endif
		_next=other->_next;
		_prev=other;

		_prev->_next=this;
		_next->_prev=this;
	}

	inline void linkBefore(CListLink<C> *other)
	{
		linkAfter(other->prev);
	}

	inline void unlink()
	{
		#ifdef NL_DEBUG
//			if (_next==this)
//				nlerror("unlink() called for object that is not linked");
//			nlassert (_next->_prev==_prev);
//			nlassert (_prev->_next==_next);
		#endif

		_next->_prev=_prev;
		_prev->_next=_next;

		#ifdef NL_DEBUG
			_next=this;
			_prev=this;
		#endif
	}

	inline C *next()
	{
		return _next->_parent;
	}

	inline C *prev()
	{
		return _prev->_parent;
	}

private:
	CListLink<C> *_next;
	CListLink<C> *_prev;
	C *_parent;
};

//---------------------------------------------------------------------------------
#endif
