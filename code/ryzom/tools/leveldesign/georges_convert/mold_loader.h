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

#ifndef NLGEORGES_MOLD_LOADER_H
#define NLGEORGES_MOLD_LOADER_H

#include "string_ex.h"

namespace NLOLDGEORGES
{

class CMoldElt;
class CLoader;

class CMoldLoader  
{
protected:
	std::map< CStringEx, CMoldElt* > mmold;
	std::vector< CMoldElt* > vpme;
	CLoader* pl;	

public:
	CMoldLoader();
	virtual ~CMoldLoader();

	void Clear();
	void SetLoader( CLoader* const _pl );
	CMoldElt* LoadMold( const CStringEx _sxfilename );
	CMoldElt* LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ); 
//	CMoldElt* Find( const CStringEx _sxfullname ); 
};

/*
struct StringExGreater : public std::binary_function< CStringEx, CStringEx, bool > 
{
	bool operator()( const CStringEx& x, const CStringEx& y ) const
	{
		return( y <= x );
	}
};
//	std::map< CStringEx, CMoldElt*, StringExGreater > moldmap;
*/

} // NLGEORGES

#endif // NLGEORGES_MOLD_LOADER_H
