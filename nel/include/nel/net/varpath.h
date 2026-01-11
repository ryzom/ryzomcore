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

#ifndef NL_VARPATH_H
#define NL_VARPATH_H

#include "nel/misc/types_nl.h"

class CVarPath
{
public:

	CVarPath (const std::string &raw) : RawVarPath(raw)
	{
		decode ();
	}

	void decode ();

	std::vector<std::pair<std::string, std::string> > Destination;

	void display ();

	/// returns true if there's no more . in the path
	bool isFinal ();

	bool empty ()
	{
		return Destination.empty();
	}

private:

	std::string getToken ();

	std::string RawVarPath;

	uint32 TokenPos;

};


#endif // NL_VARPATH_H

/* End of varpath.h */
