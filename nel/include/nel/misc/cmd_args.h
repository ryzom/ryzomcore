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
	CCmdArgs();

	struct TArg
	{
		std::string shortName; // short argument. Eg: o for -o
		std::string longName; // long argument. Eg: output for --output

		std::string helpName; // name of argument in help. Eg: <output directory>
		std::string helpDescription; // description of argument in help. Eg: Specifies the directory where to write generated files
		bool onlyOnce; // only one argument of this type is used, if several are provided, only the last one is kept
		bool required; // at least one argument of this type must be provided

		bool found; // all values for this argument
		std::vector<std::string> values; // all values for this argument
	};

	typedef std::vector<TArg> TArgs;

	/// Add a TArg structure to arguments list.
	void addArg(const TArg &arg);

	/// Add an full argument to arguments list.
	/// shortName is "p" of the argument is -p
	/// longName is "print" of the argument is --print
	/// helpName is the name that will be displayed in help if it's a required argument
	/// helpDescription is the description of the argument that will be displayed in help
	/// onlyOnce can be set to false, if we allows to use this argument more than once
	void addArg(const std::string &shortName, const std::string &longName, const std::string &helpName, const std::string &helpDescription, bool onlyOnce = true);

	/// Add a additional argument to arguments list.
	/// helpName is the name that will be displayed in help if it's a required argument
	/// helpDescription is the description of the argument that will be displayed in help
	/// onlyOnce can be set to false, if we allows to use this argument more than once
	/// required can be set to false, if this argument is required
	void addAdditionalArg(const std::string &name, const std::string &helpDescription, bool onlyOnce = true, bool required = true);

	/// Parse the command line from main() parameters argc and argv and process default arguments.
	bool parse(int argc, char **argv);

	/// Parse the command line from a std::string and process default arguments.
	bool parse(const std::string &args);

	/// Parse the command linefrom a std::vector<std::string> and process default arguments.
	bool parse(const std::vector<std::string> &args);

	/// Returns arguments of the program pass from the user to the program using parameters (ie: "myprog param1 param2")
	const TArgs& getArgs() const { return _Args; }

	/// Returns true if the argument if present in the command line (ie: haveArg("p") will return true if -p is in the command line)
	bool haveArg(const std::string &argName) const;

	/** Returns the parameters linked to an option
	 * getArg("p") will return toto if -ptoto is in the command line
	 * getArg("p") will return C:\Documents and Settings\toto.tmp if -p"C:\Documents and Settings\toto.tmp" is in the command line
	 */
	std::vector<std::string> getArg(const std::string &argName) const;

	/// return true if named long arg is present on the commandline
	/// eg haveLongArg("toto") returns true if "--toto" or "--toto=xxx" can be found on commandline
	bool haveLongArg(const std::string &argName) const;

	/// returns values associated with the given named argument
	/// both "--toto=xxx" and "--toto xxx" are acceptable
	/// quotes round arguments are stripped
	std::vector<std::string> getLongArg(const std::string &argName) const;

	/// return true if there are arguments that are required
	bool needAdditionalArg() const;

	/// return true if any additional args are present on the commandline
	bool haveAdditionalArg() const;

	/// return true if a specific additional arg is present on the commandline
	bool haveAdditionalArg(const std::string &name) const;

	/// Returns values of a specific additional arg
	std::vector<std::string> getAdditionalArg(const std::string &name) const;

	/// Display help of the program.
	void displayHelp();

	/// Display version of the program.
	void displayVersion();

	/// Returns program name or path passed as first parameter to parse() method
	std::string getProgramName() const { return _ProgramName; }
	std::string getProgramPath() const { return _ProgramPath; }
	std::string getStartupPath() const { return _StartupPath; }

	/// Set or get description to display in help
	void setDescription(const std::string &description) { _Description = description; }
	std::string getDescription() const { return _Description; }

	/// Set or get version to display in -v
	void setVersion(const std::string &version) { _Version = version; }
	std::string getVersion() const { return _Version; }
protected:
	std::string		_ProgramName;	// filename of the program
	std::string		_ProgramPath;	// full path of the program
	std::string		_StartupPath;	// initial startup path
	std::string		_Description;	// description of the program
	std::string		_Version;		// version of the program

	/// Array of arguments pass from the command line
	TArgs			_Args;
}; // class CCmdArgs

}; // NAMESPACE NLMISC

#endif // NL_CMD_ARGS_H

