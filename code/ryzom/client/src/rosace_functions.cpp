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



#include "stdpch.h"


/////////////
// INCLUDE //
/////////////
// Client.
#include "rosace_functions.h"
#include "functions.h"


///////////
// USING //
///////////


////////////
// EXTERN //
////////////


////////////
// GLOBAL //
////////////
// Rosace.
CRosace rosace;


///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// initRosace :
// Initialize Rosace.
//-----------------------------------------------
void initRosace()
{
	CRosacePage page1(4);
	CRosacePage page2(2);
	page1[0].texture(string("burnedtreeflower2.tga"));
	page1[1].texture(string("burnedtreeflower2.tga"));
	page1[2].texture(string("burnedtreeflower2.tga"));
	page1[3].texture(string("burnedtreeflower2.tga"));

	page2[0].texture(string("bush-flower.tga"));
	page2[1].texture(string("bush-flower.tga"));

	page1[0].callback(0);
	page1[1].callback(0);
	page1[2].callback(0);
	page1[3].callback(0);

	page2[0].callback(0);
	page2[1].callback(0);

	CRosaceContext context1(2);
	context1[0] = page1;
	context1[1] = page2;

	rosace.add("1", context1);
	rosace.select("1");
}// initRosace //
