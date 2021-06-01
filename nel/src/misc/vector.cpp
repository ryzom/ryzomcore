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

#include "stdmisc.h"

#include	"nel/misc/vector.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{

const CVector	CVector::Null(0,0,0);
const CVector	CVector::I(1,0,0);
const CVector	CVector::J(0,1,0);
const CVector	CVector::K(0,0,1);


/*
 * Returns the contents as a printable string "x y z"
 */
string CVector::toString() const
{
	string str;
	str = NLMISC::toString(x) + " " + NLMISC::toString(y) + " " + NLMISC::toString(z);
	return str;
}


}
