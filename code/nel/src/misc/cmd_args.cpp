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
#include "nel/misc/cmd_args.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

CCmdArgs::CCmdArgs()
{
#ifdef NL_VERSION
	_Version = NL_VERSION;
#endif

	// add help
	addArg("h", "help", "", "Display this help");

	// add version
	addArg("v", "version", "", "Display version of this program");
}

void CCmdArgs::addArg(const TArg &arg)
{
	_Args.push_back(arg);
}

void CCmdArgs::addArg(const std::string &shortName, const std::string &longName, const std::string &helpName, const std::string &helpDescription, bool onlyOnce)
{
	TArg arg;
	arg.shortName = shortName;
	arg.longName = longName;
	arg.helpName = helpName;
	arg.helpDescription = helpDescription;
	arg.onlyOnce = onlyOnce;
	arg.found = false;
	arg.required = false;

	addArg(arg);
}

void CCmdArgs::addAdditionalArg(const std::string &helpName, const std::string &helpDescription, bool onlyOnce, bool required)
{
	TArg arg;
	arg.helpName = helpName;
	arg.helpDescription = helpDescription;
	arg.onlyOnce = onlyOnce;
	arg.found = false;
	arg.required = required;

	addArg(arg);
}

bool CCmdArgs::haveArg(const std::string &argName) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// return true if long arg found
		if (arg.shortName == argName) return arg.found;
	}

	return false;
}

std::vector<std::string> CCmdArgs::getArg(const std::string &argName) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// return values if short arg found
		if (arg.shortName == argName && arg.found) return arg.values;
	}

	// return an empty vector
	return std::vector<std::string>();
}

bool CCmdArgs::haveLongArg(const std::string &argName) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// return true if long arg found
		if (arg.longName == argName) return arg.found;
	}

	return false;
}

std::vector<std::string> CCmdArgs::getLongArg(const std::string &argName) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// return values if long arg found
		if (arg.longName == argName && arg.found) return arg.values;
	}

	// return an empty vector
	return std::vector<std::string>();
}

bool CCmdArgs::needAdditionalArg() const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// they don't have any short or long name, but need a name in help
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty() && arg.required && !arg.found)
			return true;
	}

	return false;
}

bool CCmdArgs::haveAdditionalArg() const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// they don't have any short or long name, but need a name in help
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty() && arg.found)
			return true;
	}

	return false;
}

bool CCmdArgs::haveAdditionalArg(const std::string &name) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// they don't have any short or long name, but need a name in help
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty() && arg.helpName == name && arg.found)
			return true;
	}

	return false;
}

std::vector<std::string> CCmdArgs::getAdditionalArg(const std::string &name) const
{
	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// they don't have any short or long name, but need a name in help
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty() && arg.helpName == name)
			return arg.values;
	}

	// return an empty vector
	return std::vector<std::string>();
}

bool CCmdArgs::parse(const std::string &args)
{
	std::vector<std::string> argv;

#ifdef NL_OS_WINDOWS
	wchar_t str[4096];
	uint len = GetModuleFileNameW(NULL, str, 4096);

	// first argument should be full path to executable
	if (len && len < 4096)
		argv.push_back(wideToUtf8(str));
#endif

	// convert string with arguments to array
	explodeArguments(args, argv);

	return parse(argv);
}

bool CCmdArgs::parse(int argc, char **argv)
{
	// convert C strings to STL strings
	std::vector<std::string> args;

	for(sint i = 0; i < argc; ++i)
	{
#ifdef NL_OS_MAC
		// get rid of -psn_* arguments under OS X
		if (strncmp(argv[i], "-psn_", 5) == 0) continue;
#endif

		args.push_back(argv[i]);
	}

	return parse(args);
}

bool CCmdArgs::parse(const std::vector<std::string> &argv)
{
	// no parameters
	if (argv.empty()) return false;

	// first argument is always the program name
	_ProgramName = CFile::getFilename(argv.front());
	_ProgramPath = CPath::makePathAbsolute(CPath::standardizePath(CFile::getPath(argv.front())), CPath::getCurrentPath(), true);

	// current path
	_StartupPath = CPath::standardizePath(CPath::getCurrentPath());

	// set process name for logs
	CLog::setProcessName(_ProgramName);

	// arguments count
	uint argc = argv.size();

	// process each argument
	for (uint i = 1; i < argc; i++)
	{
		std::string name = argv[i];

#ifdef NL_OS_WINDOWS
		// support / and - under Windows, arguments should be at least 2 characters
		if (name.size() > 1 && (name[0] == '-' || name[0] == '/'))
#else
		if (name.size() > 1 && name[0] == '-')
#endif
		{
			// it's a long name if using --
			bool useLongName = name[0] == '-' && name[1] == '-';

			// extract argument name
			name = name.substr(useLongName ? 2:1);

			std::string value;

			if (useLongName)
			{
				// look if using = to define value
				std::string::size_type pos = name.find('=');

				if (pos != std::string::npos)
				{
					// value is second part, name the first one
					value = name.substr(pos+1);
					name = name.substr(0, pos);
				}
			}
			else if (name.length() > 1)
			{
				value = name.substr(1);
				name = name.substr(0, 1);
			}

			bool found = false;

			// process each argument definition
			for(uint j = 0; j < _Args.size(); ++j)
			{
				TArg &arg = _Args[j];

				// only process arguments of the right type
				if ((useLongName && name != arg.longName) || (!useLongName && name != arg.shortName)) continue;

				// already get the only once argument
				if (arg.found && arg.onlyOnce)
				{
					// the last one is the only kept, so discard previous ones
					arg.values.clear();
				}

				// argument is found
				found = arg.found = true;

				// another argument is required
				if (!arg.helpName.empty())
				{
					// if the value hasn't be specified by =
					if (value.empty() && i+1 < argc)
					{
						// take next argument
						value = argv[++i];
					}

					// add argument value if not empty
					if (!value.empty())
					{
						arg.values.push_back(value);
					}
				}

				break;
			}

			if (!found)
			{
				printf("Warning: Argument %s not recognized, skip it!\n", name.c_str());
			}
		}
		else
		{
			// process each argument definition
			for(uint j = 0, len = _Args.size(); j < len; ++j)
			{
				TArg &arg = _Args[j];

				// only process arguments that don't have a name
				if (!arg.shortName.empty() || !arg.longName.empty()) continue;

				// already get the only once argument
				if (arg.found && arg.onlyOnce) continue;

				arg.found = true;

				// in fact, if there are more than one required arguments, all arguments are added in first one to simplify
				arg.values.push_back(name);

				break;
			}
		}
	}

	// process version
	if (haveLongArg("version"))
	{
		displayVersion();
		return false;
	}

	// process help if requested or if required arguments are missing
	if (haveLongArg("help") || needAdditionalArg())
	{
		displayHelp();
		return false;
	}

	return true;
}

void CCmdArgs::displayHelp()
{
	// display program name
	printf("Usage: %s ", _ProgramName.c_str());

	// display optional parameters
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// only short argument is displayed
		if (!arg.shortName.empty())
		{
			printf("[-%s", arg.shortName.c_str());

			// a parameter is required
			if (!arg.helpName.empty())
			{
				printf("<%s>", arg.helpName.c_str());
			}

			printf("]");
		}
	}

	// display required arguments
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// they don't have any short or long name, but need a name in help
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty())
		{
			printf(" %c%s", arg.required ? '<':'[', arg.helpName.c_str());

			// if support more than once argument
			if (!arg.onlyOnce) printf("...");

			printf("%c", arg.required ? '>':']');
		}
	}

	printf("\n");

	if (!_Description.empty())
	{
		printf("\n%s\n", _Description.c_str());
	}

	printf("\nWhere options are:\n");

	// display details on each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// not an optional argument
		if (arg.shortName.empty() && arg.longName.empty()) continue;

		// 2 spaces
		printf("  ");

		std::vector<std::string> syntaxes;

		// display short argument
		if (!arg.shortName.empty())
		{
			// and it's required argument
			if (!arg.helpName.empty())
			{
				syntaxes.push_back(toString("-%s <%s>", arg.shortName.c_str(), arg.helpName.c_str()));
			}
			else
			{
				syntaxes.push_back(toString("-%s", arg.shortName.c_str()));
			}
		}

		// display long argument
		if (!arg.longName.empty())
		{
			if (!arg.helpName.empty())
			{
				// display first syntax for long argument, --arg <value>
				syntaxes.push_back(toString("--%s <%s>", arg.longName.c_str(), arg.helpName.c_str()));
			}
			else
			{
				syntaxes.push_back(toString("--%s", arg.longName.c_str()));
			}
		}

		for(uint j = 0; j < syntaxes.size(); ++j)
		{
			if (j > 0)
			{
				printf("%s ", (j == syntaxes.size() - 1) ? " or":",");
			}

			printf("%s", syntaxes[j].c_str());
		}

		// display argument description
		if (!arg.helpDescription.empty())
		{
			printf(" : %s", arg.helpDescription.c_str());
		}

		printf("\n");
	}

	// process each argument
	for(uint i = 0; i < _Args.size(); ++i)
	{
		const TArg &arg = _Args[i];

		// only display required arguments
		if (arg.shortName.empty() && arg.longName.empty() && !arg.helpName.empty() && !arg.helpDescription.empty())
		{
			printf("  %s : %s\n", arg.helpName.c_str(), arg.helpDescription.c_str());
		}
	}
}

void CCmdArgs::displayVersion()
{
	// display a verbose version string
#ifdef BUILD_DATE
	printf("%s %s (built on %s)\nCopyright (C) %s\n", _ProgramName.c_str(), _Version.c_str(), BUILD_DATE, COPYRIGHT);
#endif
}

}; // NAMESPACE NLMISC

