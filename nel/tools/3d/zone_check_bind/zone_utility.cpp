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


#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/zone.h"
#include <iostream>
#include <vector>
#include <set>


using namespace NL3D;
using namespace NLMISC;
using namespace std;



/*******************************************************************\
						getZoneCoordByName()
\*******************************************************************/
bool getZoneCoordByName(const char * name, uint16& x, uint16& y)
{
	uint i;
	
	std::string zoneName(name);

	// y
	string::size_type ind1 = zoneName.find("_");
	if(ind1 == string::npos || ind1>=zoneName.length())
	{
		nlwarning("bad file name");
		return false;
	}
	std::string ystr = zoneName.substr(0,ind1);
	for(i=0; i<ystr.length(); i++)
	{
		if(!isdigit(ystr[i]))
		{
			nlwarning("y code size is not a 2 characters code");
			return false;
		}
	}
	NLMISC::fromString(ystr, y);

	// x
	x = 0;
	uint ind2 = (uint)zoneName.length();
	if((ind2-ind1-1)!=2)
	{
		nlwarning("x code size is not a 2 characters code");
		return false;
	}
	std::string xstr = zoneName.substr(ind1+1,ind2-ind1-1);
	for(i=0; i<xstr.length(); i++)
	{
		if (isalpha(xstr[i]))
		{
			x *= 26;
			x += (tolower(xstr[i])-'a');
		}
		else
		{
			nlwarning("invalid");
			return false;
		}
	}
	return true;
}

/*******************************************************************\
						getLettersFromNum()
\*******************************************************************/
void getLettersFromNum(uint16 num, std::string& code)
{
	if(num>26*26) 
	{
		nlwarning("zone index too high");
		return;
	}
	code.resize(0);
	uint16 remainder = num%26;
	code += 'A' + num/26;
	code += 'A' + remainder;
}

/*******************************************************************\
						getZoneNameByCoord()
\*******************************************************************/
void getZoneNameByCoord(uint16 x, uint16 y, std::string& zoneName)
{
	// y str
	char stmp[10];
	sprintf(stmp,"%d",y);
	std::string ystrtmp = std::string(stmp);

	// x str
	std::string xstrtmp;
	getLettersFromNum(x, xstrtmp);

	// name
	zoneName = ystrtmp;
	zoneName +="_";
	zoneName +=xstrtmp;
}


