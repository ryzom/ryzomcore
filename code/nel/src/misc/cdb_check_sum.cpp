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

#include "stdmisc.h"
#include "nel/misc/cdb_check_sum.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC{

/*
 * Constructor
 */

CCDBCheckSum::CCDBCheckSum()
{
	//arbitrary values
	_Sum = 0;
	_Factor = 55665;
	_Const1 = 52845;
	_Const2 = 22719;
};

///add an uint8 to the sum
void CCDBCheckSum::add(uint8 el)
{
	uint32 cipher = (el ^ (_Factor >> 8));
	_Factor = (cipher + _Factor) * _Const1 + _Const2;
	_Sum += cipher;
}

}

