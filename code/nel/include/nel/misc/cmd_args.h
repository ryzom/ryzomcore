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

#ifndef NL_CMD_ARGS_H
#define NL_CMD_ARGS_H

//
// Includes
//
#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"

namespace NLMISC
{

class CCmdArgs
{
public:
	/// Sets the command line and init _Args variable. You must call this before calling main()
	void setArgs (int argc, const char **argv);

	/// Sets the command line and init _Args variable. You must call this before calling main()
	void setArgs (const char *args);

	/// Returns arguments of the program pass from the user to the program using parameters (ie: "myprog param1 param2")
	const NLMISC::CVectorSString    &getArgs () const { return _Args; }

	/// Returns true if the argument if present in the command line (ie: haveArg('p') will return true if -p is in the command line)
	bool                                                    haveArg (char argName) const;

	/** Returns the parameter linked to an option
	 * getArg('p') will return toto if -ptoto is in the command line
	 * getArg('p') will return C:\Documents and Settings\toto.tmp if -p"C:\Documents and Settings\toto.tmp" is in the command line
	 * It'll thrown an Exception if the argName is not found
	 */
	std::string                                             getArg (char argName) const;

	/// return true if named long arg is present on the commandline
	/// eg haveLongArg("toto") returns true if "--toto" or "--toto=xxx" can be found on commandline
	bool                                                    haveLongArg (const char* argName) const;

	/// returns the value associated with the given named argument
	/// both "--toto=xxx" and "--toto xxx" are acceptable
	/// quotes round arguments are stripped
	std::string                                             getLongArg (const char* argName) const;
protected:
	/// Array of arguments pass from the command line
	NLMISC::CVectorSString                          _Args;
}; // class CCmdArgs

}; // NAMESPACE NLMISC

#endif // NL_CMD_ARGS_H

