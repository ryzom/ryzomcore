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
#include <vector>
#include <set>
#include <iostream>


using namespace NL3D;
using namespace NLMISC;
using namespace std;


/*******************************************************************\
						getDir()
\*******************************************************************/
std::string getDir (const std::string& path)
{
	char tmpPath[512];
	strcpy (tmpPath, path.c_str());
	char* slash=strrchr (tmpPath, '/');
	if (!slash)
	{
		slash=strrchr (tmpPath, '\\');
	}

	if (!slash)
		return "";

	slash++;
	*slash=0;
	return tmpPath;
}


/*******************************************************************\
						getName()
\*******************************************************************/
std::string getName (const std::string& path)
{
	std::string dir=getDir (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *name=tmpPath;
	nlassert (dir.length()<=strlen(tmpPath));
	name+=dir.length();

	char* point=strrchr (name, '.');
	if (point)
		*point=0;

	return name;
}


/*******************************************************************\
						getExt()
\*******************************************************************/
std::string getExt (const std::string& path)
{
	std::string dir=getDir (path);
	std::string name=getName (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *ext=tmpPath;
	nlassert (dir.length()+name.length()<=strlen(tmpPath));
	ext+=dir.length()+name.length();

	return ext;
}

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



/*******************************************************************\
						getAdjacentZonesName()
\*******************************************************************/
void getAdjacentZonesName(const std::string& zoneName, std::vector<std::string>& names)
{
	uint16 x,y;
	int xtmp,ytmp;
	std::string nametmp;
	std::string empty("empty");

	names.reserve(8);
	
	getZoneCoordByName(zoneName.c_str(), x, y);

	// top left
	xtmp = x-1;
	ytmp = y-1;
	if(xtmp<0||ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// top
	xtmp = x;
	ytmp = y-1;
	if(ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// top right
	xtmp = x+1;
	ytmp = y-1;
	if(ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// left
	xtmp = x-1;
	ytmp = y;
	if(xtmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// right
	xtmp = x+1;
	ytmp = y;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom left
	xtmp = x-1;
	ytmp = y+1;
	if(xtmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom
	xtmp = x;
	ytmp = y+1;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom right 
	xtmp = x+1;
	ytmp = y+1;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);
}


/*******************************************************************\
						createZoneId()
\*******************************************************************/
uint16 createZoneId(std::string zoneName)
{
	uint16 x,y;
	getZoneCoordByName(zoneName.c_str(), x, y);
	return ((y-1)<<8) + x;
}
