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

#ifndef UT_LIGO
#define UT_LIGO

#include <nel/ligo/primitive.h>

#include "ut_ligo_primitive.h"
// Add a line here when adding a new test CLASS

struct CUTLigo : public Test::Suite
{
	CUTLigo()
	{
		add(auto_ptr<Test::Suite>(new CUTLigoPrimitive));
		// Add a line here when adding a new test CLASS
	}
};

#endif
