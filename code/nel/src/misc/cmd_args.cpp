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

//
// Includes
//
#include "stdmisc.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"

#include "nel/misc/cmd_args.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

bool CCmdArgs::haveArg(char argName) const
{
	for(uint32 i = 0; i < _Args.size(); i++)
	{
		if(_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if(_Args[i][1] == argName)
			{
				return true;
			}
		}
	}
	return false;
}

std::string CCmdArgs::getArg(char argName) const
{
	for(uint32 i = 0; i < _Args.size(); i++)
	{
		if(_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if(_Args[i][1] == argName)
			{
				/* Remove the first and last '"' : 
				-c"C:\Documents and Settings\toto.tmp" 
				will return :
				C:\Documents and Settings\toto.tmp
				*/
				uint begin = 2;
				if(_Args[i].size() < 3)
					return "";
					//throw Exception ("Parameter '-%c' is malformed, missing content", argName);

				if(_Args[i][begin] == '"')
					begin++;

				// End
				uint size = (uint)_Args[i].size();
				if(size && _Args[i][size-1] == '"')
					size--;
				size = (uint)(std::max((int)0, (int)size-(int)begin));
				return _Args[i].substr(begin, size);
			}
		}
	}
	throw Exception("Parameter '-%c' is not found in command line", argName);
}

bool CCmdArgs::haveLongArg(const char* argName) const
{
	for(uint32 i = 0; i < _Args.size(); i++)
	{
		if(_Args[i].left(2)=="--" && _Args[i].leftCrop(2).splitTo('=')==argName)
		{
			return true;
		}
	}
	return false;
}

std::string CCmdArgs::getLongArg (const char* argName) const
{
        for (uint32 i = 0; i < _Args.size(); i++)
        {
		if (_Args[i].left(2)=="--" && _Args[i].leftCrop(2).splitTo('=')==argName)
		{
			NLMISC::CSString val= _Args[i].splitFrom('=');
			if (!val.empty())
			{
				return val.unquoteIfQuoted();
			}
			if (i+1<_Args.size() && _Args[i+1].c_str()[0]!='-')
			{
				return _Args[i+1].unquoteIfQuoted();
			}

			return std::string();
		}
	}
	return std::string();
}

void CCmdArgs::setArgs(const char *args)
{
	_Args.push_back ("<ProgramName>");

	std::string sargs (args);
	std::string::size_type pos1 = 0, pos2 = 0;

	do
	{
		// Look for the first non space character
		pos1 = sargs.find_first_not_of (" ", pos2);
		if(pos1 == std::string::npos) break;

		// Look for the first space or "
		pos2 = sargs.find_first_of (" \"", pos1);
		if(pos2 != std::string::npos)
		{
			// " ?
			if(sargs[pos2] == '"')
			{
				// Look for the final \"
				pos2 = sargs.find_first_of ("\"", pos2+1);
				if(pos2 != std::string::npos)
				{
					// Look for the first space
					pos2 = sargs.find_first_of (" ", pos2+1);
				}
			}
		}

		// Compute the size of the string to extract
		std::string::difference_type length = (pos2 != std::string::npos) ? pos2-pos1 : std::string::npos;

		std::string tmp = sargs.substr (pos1, length);
		_Args.push_back (tmp);
	}
	while(pos2 != std::string::npos);
}

void CCmdArgs::setArgs(int argc, const char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
}

}; // NAMESPACE NLMISC

