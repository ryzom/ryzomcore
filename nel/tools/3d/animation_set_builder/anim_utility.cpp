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


#include <iostream>
#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"


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


