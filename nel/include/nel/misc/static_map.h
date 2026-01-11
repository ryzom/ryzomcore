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

#ifndef NL_STATIC_MAP_H
#define NL_STATIC_MAP_H

#include "types_nl.h"
#include "common.h"
#include "stream.h"
#include "debug.h"

#include <map>

// With NeL Memory Debug, use new
#ifndef NL_USE_DEFAULT_MEMORY_MANAGER
# ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
#  define NL_OV_USE_NEW_ALLOCATOR
# endif // NLMISC_HEAP_ALLOCATION_NDEBUG
#endif // NL_USE_DEFAULT_MEMORY_MANAGER

namespace NLMISC {


// ***************************************************************************
/**
 * Implemented with a std::vector
 * Use it not like a map : begin by adding all your values with add()/del()/fromMap()
 * and then call endAdd() that performs a slow sort on the vector and then call find()
 * to find the element you want. If you have not called endAdd() its done in find(),
 * but take care that endAdd() is slow.
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date October 2003
 */
template<class Key, class Typ, class Comp = std::less<Key> >	class CStaticMap
{
public:
	typedef Key						key_type;
	typedef Typ						data_type;
	typedef Typ						mapped_type;
	typedef std::pair<Key, Typ>		value_type;
	typedef Comp					key_compare;

	class value_compare : public std::binary_function<value_type, value_type, bool>
	{
		friend class CStaticMap<Key, Typ, Comp>;
	public:
		bool operator()(const value_type& __x, const value_type& __y) const
		{
			return _CompareFunc(__x.first, __y.first);
		}
	protected :
		Comp _CompareFunc;
	protected :
		// Constructor
		value_compare(Comp __c) : _CompareFunc(__c) {}
	};

private:

	std::vector<value_type> _Data;
	bool					_DataSorted;
	Comp					_CompFunc;


public:

	typedef typename std::vector<value_type>::reference					reference;
	typedef typename std::vector<value_type>::const_reference			const_reference;
	typedef typename std::vector<value_type>::iterator					iterator;
	typedef typename std::vector<value_type>::const_iterator			const_iterator;
	typedef typename std::vector<value_type>::reverse_iterator			reverse_iterator;
	typedef typename std::vector<value_type>::const_reverse_iterator	const_reverse_iterator;
	typedef typename std::vector<value_type>::size_type					size_type;
	typedef typename std::vector<value_type>::difference_type			difference_type;

  // allocation/deallocation

	CStaticMap() : _DataSorted(true)
	{
	}

	explicit CStaticMap (const Comp& /* __comp */) : _DataSorted(true)
	{
	}

	CStaticMap (const_iterator __first, const_iterator __last)
	{
		_DataSorted = false;
		_Data.insert(__first, __last);
		endAdd();
	}

	CStaticMap (const_iterator __first, const_iterator __last, const Comp& __comp)
		: _CompFunc(__comp)
	{
		_DataSorted = false;
		_Data.insert(__first, __last);
		endAdd();
	}

	CStaticMap(const CStaticMap<Key, Typ, Comp>& __x)
		: _Data(__x._Data) , _DataSorted(__x._DataSorted), _CompFunc(__x._CompFunc)
	{
	}

	CStaticMap<Key, Typ, Comp>& operator= (const CStaticMap<Key, Typ, Comp>& __x)
	{
		_Data = __x._Data;
		endAdd();
		return *this;
	}

	// accessors:

	key_compare				key_comp() const		{ return _Data.key_comp(); }
	value_compare			value_comp() const		{ return value_compare(_CompFunc); }
	iterator				begin()					{ endAdd(); return _Data.begin(); }
	const_iterator			begin() const			{ endAdd(); return _Data.begin(); }
	iterator				end()					{ endAdd(); return _Data.end(); }
	const_iterator			end() const				{ endAdd(); return _Data.end(); }
	reverse_iterator		rbegin()				{ endAdd(); return _Data.rbegin(); }
	const_reverse_iterator	rbegin() const			{ endAdd(); return _Data.rbegin(); }
	reverse_iterator		rend()					{ endAdd(); return _Data.rend(); }
	const_reverse_iterator	rend() const			{ endAdd(); return _Data.rend(); }
	bool					empty() const			{ return _Data.empty(); }
	size_type				size() const			{ return _Data.size(); }
	size_type				max_size() const		{ return _Data.max_size(); }

	Typ& operator[](const key_type& __k)
	{
		iterator __i = find(__k);
		// The key MUST exist no automatic insertion done in this class
		nlassert(__i != end());
		return (*__i).second;
	}

	void swap (CStaticMap<Key, Typ, Comp>& __x)
	{
		_Data.swap (__x._Data);
		_DataSorted = false;
	}

	// Add an element in the static map

	void reserve(size_type n)
	{
		_Data.reserve(n);
	}

	void add(const value_type& __v)
	{
		_DataSorted = false;
		_Data.push_back (__v);
	}

	void fromMap (const std::map<Key, Typ, Comp> &m)
	{
		_DataSorted = false;
		_Data.reserve(m.size());
		typename std::map<Key,Typ,Comp>::const_iterator itEnd = m.end();
		typename std::map<Key,Typ,Comp>::const_iterator it = m.begin();
		for (; it != itEnd; it++)
			_Data.push_back (std::pair<Key, Typ>(it->first, it->second));
	}

	void endAdd()
	{
		if (_DataSorted) return;
		_DataSorted = true;
		sort (_Data.begin(), _Data.end(), value_comp());	// Sort the vector
	}

	// Delete an element from the static map

	void del(iterator __position)
	{
		nlassert(_DataSorted);
		_Data.erase (__position);
	}

	size_type del(const key_type& __x)
	{
		endAdd();
		return _Data.erase (__x);
	}

	void del(iterator __first, iterator __last)
	{
		nlassert(_DataSorted);
		_Data.erase (__first, __last);
	}

	void clear()
	{
		_Data.clear();
	}

	// map operations:

	iterator find(const key_type& __x)
	{
		endAdd();
		value_type __v(__x, Typ());
		iterator it = lower_bound((iterator)_Data.begin(), (iterator)_Data.end(), __v, value_comp());
		if ((it != end()) && (!value_comp()(*it,__v) && !value_comp()(__v,*it)))
			return it;
		else
			return end();
	}

	const_iterator find(const key_type& __x) const
	{
		endAdd();
		value_type __v(__x, Typ());
		iterator it = lower_bound((const_iterator)_Data.begin(), (const_iterator)_Data.end(), __v, value_comp());
		if ((it != end()) && (!value_comp()(*it,__v) && !value_comp()(__v,*it)))
			return it;
		else
			return end();
	}

	size_type count(const key_type& __x) const
	{
		endAdd();
		return find(__x) == _Data.end() ? 0 : 1;
	}
};


} // NLMISC


#endif // NL_STATIC_MAP_H

/* End of static_map.h */
