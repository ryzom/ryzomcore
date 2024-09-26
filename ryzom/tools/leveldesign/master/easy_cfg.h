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

#ifndef __EASY_CFG_H__
#define __EASY_CFG_H__

#include <string>
#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class CConfigFile;
}

struct IEasyCFG
{

	IEasyCFG ();
	~IEasyCFG ();

	bool openRead (const std::string &filename);
	bool openWrite (const std::string &filename);
	void close ();

	// Read/Write int
	sint32 getInt (const std::string &sVarName);
	void putInt (const std::string &sVarName, sint32 sVarValue);
	// Read/Write string
	std::string getStr (const std::string &sVarName);
	void putStr (const std::string &sVarName, const std::string &sVarValue);
	// Read/Write boolean
	bool getBool (const std::string &sVarName);
	void putBool (const std::string &sVarName, bool sVarValue);
	// Put a comment line (in write mode only)
	void putCommentLine (const std::string &sComments);

	virtual bool load (const std::string &filename) = 0;
	virtual bool save (const std::string &filename) = 0;


	NLMISC::CConfigFile *cf;
	FILE *f;
};

#endif // __EASY_CFG_H__