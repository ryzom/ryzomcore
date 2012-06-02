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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"
#include "nel/misc/sstring.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace NLMISC;


//-----------------------------------------------------------------------------
// cleanPath - convert a path to standardised format
//-----------------------------------------------------------------------------

CSString cleanPath(const CSString& path,bool addTrailingSlash)
{
	CSString result;

	// split the path up into its component elements
	CVectorSString pathComponents;
	path.unquoteIfQuoted().splitByOneOfSeparators("/\\",pathComponents,false,false,true,false,true);

	// iterate over path components collapsing '.' and '..' entries
	for (uint32 i=0;i<pathComponents.size();++i)
	{
		// skip "." entries
		if (pathComponents[i]==".")
		{
			pathComponents[i].clear();
			continue;
		}

		// deal with ".."
		if (pathComponents[i]=="..")
		{
			// run back through our component list looking for an element to remove
			uint32 j;
			for (j=i;j--;)
			{
				if (!pathComponents[j].empty())
					break;
			}
			// if we found an element then remove it and the '..' as well
			if (j!=std::numeric_limits<uint32>::max())
			{
				pathComponents[j].clear();
				pathComponents[i].clear();
			}
			continue;
		}
	}

	// treat the special case where original path started with a '/' or '//'
	if (path.left(1)=="/" || path.left(1)=="\\")
	{
		result= (path.left(2).right(1)=="/" || path.left(2).right(1)=="\\")? "//": "/";
	}

	// concatenate the path bits
	for (uint32 i=0;i<pathComponents.size();++i)
	{
		if (!pathComponents[i].empty())
		{
			result+= pathComponents[i];
			result+= '/';
		}
	}

	// if we're not supposed to have a trailing slash then get rid of the one that's added by default
	if (addTrailingSlash==false && path.right(1)!='/' && path.right(1)!='\\')
	{
		result.resize(result.size()-1);
	}

	return result;
}
