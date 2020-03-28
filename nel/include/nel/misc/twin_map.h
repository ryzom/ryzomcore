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

#ifndef NL_TWIN_MAP_H
#define NL_TWIN_MAP_H

#include "nel/misc/debug.h"
#include <map>

namespace NLMISC
{


/** Bidirectionnal association between values
  * Behaves like a map, but key can be used as value and vice-versa
  *
  * Association must be bijective or assertions are raised
  *
  *
  * Example of use :
  *
  * CTwinMap<string, int>  tm;
  * tm.add("foo", 1);
  * tm.add("bar", 2);
  * tm.add("foorbar", 3);
  * tm.get("foo");		 // returns 1
  * tm.get(1);			 // returns "foo"
  * tm.remove(1);		 // removes ("foo", 1) couple
  * tm.remove("bar")	 // removes ("bar", 2) couple
  * tm.add("foobar", 4); // assert !
  * tm.add("foo", 3);    // assrt! (3 associated with foobar)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
template <class TypeA, class TypeB>
class CTwinMap
{
public:
	typedef std::map<TypeA, TypeB> TAToBMap;
	typedef std::map<TypeB, TypeA> TBToAMap;
public:
	// add a couple in the twin map. An assertion is raised if either valueA or valueB were already present in the map
	inline void			add(const TypeA &valueA, const TypeB &valueB);
	// retrieves value of type 'TypeB' associated with 'valueA', or NULL if not found
	inline const TypeA *getA(const TypeB &valueB) const;
	// retrieves value of type 'TypeB' associated with 'valueA', or NULL if not found
	inline const TypeB *getB(const TypeA &valueA) const;
	// removes a couple from its TypeA value
	inline void			removeWithA(const TypeA &valueA);
	// removes a couple from its TypeB value
	inline void			removeWithB(const TypeB &valueB);
	// Direct read access to 'TypeA to TypeB' map
	const TAToBMap	   &getAToBMap() const { return _AToB; }
	// Direct read access to 'TypeB to TypeA' map
	const TBToAMap	   &getBToAMap() const { return _BToA; }
	// clear the twin map
	inline	void		clear();

private:
	TAToBMap _AToB;
	TBToAMap _BToA;
};

////////////////////
// IMPLEMENTATION //
////////////////////

//==================================================================================================
template <class TypeA, class TypeB>
inline void	CTwinMap<TypeA, TypeB>::clear()
{
	_AToB.clear();
	_BToA.clear();
}

//==================================================================================================
template <class TypeA, class TypeB>
inline void	CTwinMap<TypeA, TypeB>::add(const TypeA &valueA, const TypeB &valueB)
{
	nlassert(!getB(valueA));
	nlassert(!getA(valueB));
	_AToB[valueA] = valueB;
	_BToA[valueB] = valueA;
}

//==================================================================================================
template <class TypeA, class TypeB>
inline const TypeA *CTwinMap<TypeA, TypeB>::getA(const TypeB &valueB) const
{
	typename TBToAMap::const_iterator it = _BToA.find(valueB);
	if (it == _BToA.end()) return NULL;
	else return &(it->second);
}

//==================================================================================================
template <class TypeA, class TypeB>
inline const TypeB *CTwinMap<TypeA, TypeB>::getB(const TypeA &valueA) const
{
	typename TAToBMap::const_iterator it = _AToB.find(valueA);
	if (it == _AToB.end()) return NULL;
	else return &(it->second);
}

//==================================================================================================
template <class TypeA, class TypeB>
inline void	CTwinMap<TypeA, TypeB>::removeWithA(const TypeA &valueA)
{
	typename TAToBMap::iterator itA = _AToB.find(valueA);
	nlassert(itA != _AToB.end());
	typename TBToAMap::iterator itB = _BToA.find(itA->second);
	nlassert(itB != _BToA.end());
	_AToB.erase(itA);
	_BToA.erase(itB);
}

//==================================================================================================
template <class TypeA, class TypeB>
inline void	CTwinMap<TypeA, TypeB>::removeWithB(const TypeB &valueB)
{
	typename TBToAMap::iterator itB = _BToA.find(valueB);
	nlassert(itB != _BToA.end());
	typename TAToBMap::iterator itA = _AToB.find(itB->second);
	nlassert(itA != _AToB.end());
	_AToB.erase(itA);
	_BToA.erase(itB);
}



} // NLMISC



#endif
