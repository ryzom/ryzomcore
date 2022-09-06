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



#ifndef RYAI_KEYWORD_H
#define RYAI_KEYWORD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"


//---------------------------------------------------------------------------------
// CKeywordMask
//---------------------------------------------------------------------------------
// A bit mask with one bit per keyword for a given keyword set

class CKeywordMask
{
public:
	inline CKeywordMask(uint32 mask=0)
	{
		_mask=mask;
	}

	inline CKeywordMask(const CKeywordMask &other)
	{
		_mask=other._mask;
	}

	// equivalence operators: == !=
	inline bool operator==(const CKeywordMask &other) const
	{
		return _mask == other._mask;
	}
	inline bool operator!=(const CKeywordMask &other) const
	{
		return _mask != other._mask;
	}

	// basic bitwise operators: & |	~
	inline CKeywordMask operator&(const CKeywordMask &other) const
	{
		return CKeywordMask( _mask & other._mask );
	}
	inline CKeywordMask operator|(const CKeywordMask &other) const
	{
		return CKeywordMask( _mask | other._mask );
	}
	inline CKeywordMask operator~()	const
	{
		return CKeywordMask( ~_mask );
	}

	// basic bitwise operators with assignment: &= |= =
	inline CKeywordMask operator&=(const CKeywordMask &other)
	{
		_mask &= other._mask;
		return *this;
	}
	inline CKeywordMask operator|=(const CKeywordMask &other)
	{
		_mask |= other._mask;
		return *this;
	}
	inline CKeywordMask operator=(const CKeywordMask &other)
	{
		_mask = other._mask;
		return *this;
	}

	// handy utility methods: clear() isEmpty()
	inline void clear()
	{
		_mask = 0;
	}
	inline bool isEmpty() const
	{
		return _mask == 0;
	}

	uint32  asUint() const { return _mask; }

private:
	uint32 _mask;

	friend class CKeywordSet;
};

//---------------------------------------------------------------------------------
// CKeywordFilter
//---------------------------------------------------------------------------------
// A set of keyword masks representing:
// - 'must include all of' keywords
// - 'must include at least one of'	keywords
// - 'must not include any of' keywords
// used for filter test operations with keyword masks

class CKeywordFilter
{
public:
	inline CKeywordFilter()	{}
	inline CKeywordFilter(const CKeywordFilter &other)
	{	 
		*this=other;
	}

	inline bool test(const CKeywordMask &mask) const
	{
		return 
			(mask & (_includeAll|_notInclude) ) == _includeAll &&
			( _includeAny.isEmpty() || !(mask & _includeAny).isEmpty() );
	}

	inline const CKeywordFilter &operator=(const CKeywordFilter &other)
	{
		_includeAll=other._includeAll;
		_includeAny=other._includeAny;
		_notInclude=other._notInclude;
		return *this;
	}

	inline const CKeywordFilter &operator+=(const CKeywordFilter &other)
	{
		_includeAll|=other._includeAll;
		_includeAny|=other._includeAny;
		_notInclude|=other._notInclude;
		return *this;
	}

	inline const CKeywordFilter &operator-=(const CKeywordFilter &other)
	{
		_includeAll&=~other._includeAll;
		_includeAny&=~other._includeAny;
		_notInclude&=~other._notInclude;
		return *this;
	}

	// handy utility methods: clear() isEmpty()
	inline void clear()
	{
		_includeAll.clear();	
		_includeAny.clear();
		_notInclude.clear();
	}
	inline bool isEmpty() const
	{
		return (_includeAll|_includeAny|_notInclude).isEmpty();
	}

private:
	CKeywordMask _includeAll;
	CKeywordMask _includeAny;
	CKeywordMask _notInclude;

	friend class CKeywordSet;
};

//---------------------------------------------------------------------------------
// CKeywordSet
//---------------------------------------------------------------------------------
// A set of keywords 
// - a vector of keyword names (limited to 32)
// - methods for generating keyword masks and filters from strings
// - methods for converting masks and filters back to strings (for debug)

class CKeywordSet
{
public:
	void clear();
	void addKeywords(const std::string &keywords);

	bool	stringToMask(std::string s, CKeywordMask &result) const;
	bool	stringToFilter(std::string s, CKeywordFilter &result) const ;

	std::string		maskToString(CKeywordMask mask) const;
	std::string		filterToString(CKeywordFilter filter) const;

	std::string		toString();	// build the valid wor list into a blank-separated string

private:
	std::vector<std::string> _words;
};
 
//---------------------------------------------------------------------------------
#endif
