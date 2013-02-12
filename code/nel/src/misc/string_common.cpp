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

#include "nel/misc/string_common.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

string addSlashR (string str)
{
	string formatedStr;
	// replace \n with \r\n
	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] == '\n' && i > 0 && str[i-1] != '\r')
		{
			formatedStr += '\r';
		}
		formatedStr += str[i];
	}
	return formatedStr;
}

string removeSlashR (string str)
{
	string formatedStr;
	// replace \n with \r\n
	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] != '\r')
			formatedStr += str[i];
	}
	return formatedStr;
}

}
