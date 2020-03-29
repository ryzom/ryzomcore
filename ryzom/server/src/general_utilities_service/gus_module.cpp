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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_module.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// methods IModule
	//-----------------------------------------------------------------------------

	IModule::IModule()
	{
	}

	IModule::~IModule()
	{
	}

	
	//-----------------------------------------------------------------------------
	// utility Routines
	//-----------------------------------------------------------------------------

	NLMISC::CSString extractNamedParameter(const NLMISC::CSString& argName,NLMISC::CSString rawArgs)
	{
		while (!rawArgs.empty())
		{
			NLMISC::CSString keyword;
			NLMISC::CSString args;
			keyword= rawArgs.firstWord(true);
			rawArgs=rawArgs.strip();
			if (rawArgs[0]=='(')
			{
				args=rawArgs.matchDelimiters(true).stripBlockDelimiters();
			}

			if (keyword==argName)
			{
				return args;
			}
		}

		return NLMISC::CSString();
	}

	NLMISC::CSString extractNamedPathParameter(const NLMISC::CSString& argName,NLMISC::CSString rawArgs)
	{
		return NLMISC::CPath::standardizePath(extractNamedParameter(argName,rawArgs).unquoteIfQuoted(),false);
	}
}

