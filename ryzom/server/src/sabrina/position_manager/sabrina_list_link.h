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



#ifndef RY_SABRINA_LIST_LINK_H
#define RY_SABRINA_LIST_LINK_H

//---------------------------------------------------------------------------------
// CListLink<>
//---------------------------------------------------------------------------------
// A link for linking objects into lists
// The only valid constructor requires a pointer to the parent class that the
// list link represents

#pragma warning (disable : 4355)

template <class T> class CListLink
{
public:
	inline explicit CListLink(T *parent) : _Next(this), _Prev(this), _Parent(parent)
	{
	}

	inline virtual ~CListLink()
	{
		unlink();
	}
	
	inline void linkAfter(CListLink<T> *other)
	{
		#ifdef NL_DEBUG
			nlassert(other);
			nlassert(_Next==this);
			nlassert(_Prev==this);
//			nlassert(other->_Next->_Prev==other->_Prev);
//			nlassert(other->_Prev->_Next==other->_Next);
		#endif
		_Next=other->_Next;
		_Prev=other;

		_Prev->_Next=this;
		_Next->_Prev=this;
	}

	inline void linkBefore(CListLink<T> *other)
	{
		#ifdef NL_DEBUG		
			nlassert(other);
		#endif
		linkAfter(other->prev);
	}

	inline void unlink()
	{
		#ifdef NL_DEBUG
//			if (_Next==this)
//				nlerror("unlink() called for object that is not linked");
//			nlassert (_Next->_Prev==_Prev);
//			nlassert (_Prev->_Next==_Next);
		#endif

		_Next->_Prev=_Prev;
		_Prev->_Next=_Next;

		#ifdef NL_DEBUG
			_Next=this;
			_Prev=this;
		#endif
	}

	inline CListLink<T> * const &next()
	{
		return _Next;
	}

	inline CListLink<T> * const &prev()
	{
		return _Prev;
	}

	inline T *parent()
	{
		return _Parent;
	}

private:
	CListLink<T>	*_Next;
	CListLink<T>	*_Prev;
	T				*_Parent;
};

//---------------------------------------------------------------------------------
#endif // RY_SABRINA_LIST_LINK_H
