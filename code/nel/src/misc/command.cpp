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

#include "nel/misc/command.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

//ICommand::TCategorySet* ICommand::_Categories;
ICommand::TCommand *ICommand::LocalCommands = NULL;
bool ICommand::LocalCommandsInit = false;
//set<std::string>		ICommand::_CommandsDisablingControlChar;

NLMISC_SAFE_SINGLETON_IMPL(CCommandRegistry);

ICommand::ICommand(const char *categoryName, const char *commandName, const char *commandHelp, const char *commandArgs)
{
	// self registration

	if (!LocalCommandsInit)
	{
		LocalCommands = new TCommand;
		LocalCommandsInit = true;
	}

	TCommand::iterator comm = LocalCommands->find(commandName);

	if (comm != LocalCommands->end ())
	{
		nlinfo("command with same name: %s", commandName);
		// 2 commands have the same name
		// nlstopex (("There are 2 commands that have the same name in the project (command name '%s'), skip the second definition", commandName));
	}
	else
	{
		// insert the new command in the map
		//nlinfo ("add command '%s'", commandName);
		CategoryName = categoryName;
		HelpString = commandHelp;
		CommandArgs = commandArgs;
		_CommandName = commandName;
		Type = Command;
		(*LocalCommands)[commandName] = this;
	}

	if (INelContext::isContextInitialised())
	{
		// directly register this command
		CCommandRegistry::getInstance().registerCommand(this);
	}
}

ICommand::~ICommand()
{
	// self deregistration

	// find the command

	for (TCommand::iterator comm = LocalCommands->begin(); comm != LocalCommands->end(); comm++)
	{
		if ((*comm).second == this)
		{
			//printf("remove command\n");
			LocalCommands->erase (comm);

			// delete local commands if all gone
			if (!LocalCommands->size())
			{
				delete LocalCommands;
				LocalCommands = NULL;
				LocalCommandsInit = false;
			}

			// Yoyo: if no nlinfo()/nlwarning() (thus no createDebug(), thus no new CApplicationContext)
			// done in the .dll, it is possible that the nel context is never initialized
			if (INelContext::isContextInitialised())
			{
				CCommandRegistry::getInstance().unregisterCommand(this);
			}


			return;
		}
	}
	// commands is not found
//	nlstop;
}

void CCommandRegistry::registerCommand(ICommand *command)
{
	if (_Commands.find(command->getName()) != _Commands.end())
	{
//		nlwarning("There are 2 commands that have the same name in the project (command name '%s'), skip the second definition", command->getName().c_str());
		return;
	}
	_Commands[command->getName()] = command;
	_Categories.insert(command->CategoryName);
}

void CCommandRegistry::unregisterCommand(ICommand *command)
{
	for (TCommand::iterator comm = _Commands.begin(); comm != _Commands.end(); ++comm)
	{
		if (comm->second == command)
		{
			//printf("remove command\n");
			_Commands.erase (comm);
			return;
		}
	}
	//nlwarning("CCommandRegistry::unregisterCommand : the command '%s' is not registered", command->getName().c_str());
}

void CCommandRegistry::registerNamedCommandHandler(ICommandsHandler *handler, const std::string &className)
{
	const std::string &name = handler->getCommandHandlerName();
	if (_CommandsHandlers.getB(name) != NULL)
	{
		nlwarning("CCommandRegistry : a commands handler with the name '%s' already exist, ignoring new candidat", name.c_str());
		return;
	}
	_CommandsHandlers.add(name, handler);

	TCommandsHandlersClass::iterator it = _CommandsHandlersClass.find(className);

	if (it == _CommandsHandlersClass.end())
	{
		nlinfo("CCommandRegistry : adding commands handler for class '%s'", className.c_str());
	}

	// register the class and commands name
	TCommandHandlerClassInfo &chci = _CommandsHandlersClass[className];

	// add an instance to the counter
	++chci.InstanceCount;
	// store the command list
	TCommandHandlerClassInfo::TCommandsInfo commands;
	handler->fillCommandsHandlerList(commands);
	nlassert(chci._Commands.empty() || chci._Commands == commands);

	if (chci._Commands.empty())
		std::swap(chci._Commands, commands);

}

void CCommandRegistry::unregisterNamedCommandHandler(ICommandsHandler *handler, const std::string &className)
{
	if (_CommandsHandlers.getA(handler) == NULL)
		return;

	_CommandsHandlers.removeWithB(handler);

	// update the handler class commands tables
	TCommandsHandlersClass::iterator it = _CommandsHandlersClass.find(className);
	if (it != _CommandsHandlersClass.end())
	{
		--(it->second.InstanceCount);

		if (it->second.InstanceCount == 0)
		{
			nlinfo("CCommandRegistry : removing commands handler for class '% s'", className.c_str());
			_CommandsHandlersClass.erase(it);
		}
	}
}


bool ICommand::execute (const std::string &commandWithArgs, CLog &log, bool quiet, bool human)
{
	try
	{
		return CCommandRegistry::getInstance().execute(commandWithArgs, log, quiet, human);
	}
	catch(const exception &e)
	{
		log.displayNL("Command '%s' thrown an exception :", commandWithArgs.c_str());
		log.displayNL(e.what());

		return false;
	}
}

struct TCommandParams
{
	string			CommandName;
	string			RawCommandString;
	vector<string>	CommandArgs;
};

bool CCommandRegistry::execute (const std::string &commandWithArgs, CLog &log, bool quiet, bool human)
{
	if (!quiet)
	{
		log.displayNL ("Executing command : '%s'", commandWithArgs.c_str());
	}

	// true to indicate that '"', ';' and '\' are special character sequence control
	bool allowControlChar= true;
	// Start of each command in the command line
	string::size_type commandBegin = 0;

	// convert the buffer into string vector
	vector<TCommandParams>	commands;
	bool firstArg = true;
	uint i = 0;
	for(;;)
	{
		// skip whitespace
		for(;;)
		{
			if (i == commandWithArgs.size())
			{
				goto end;
			}
			if (commandWithArgs[i] != ' ' && commandWithArgs[i] != '\t' && commandWithArgs[i] != '\n' && commandWithArgs[i] != '\r')
			{
				break;
			}
			i++;
		}

		// get param
		string arg;
		if (allowControlChar && commandWithArgs[i] == '\"')
		{
			// starting with a quote "
			i++;
			for(;;)
			{
				if (i == commandWithArgs.size())
				{
					if (!quiet) log.displayNL ("Missing end quote character \"");
					return false;
				}
				if (commandWithArgs[i] == '"')
				{
					i++;
					break;
				}
				if (commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						if (!quiet) log.displayNL ("Missing character after the backslash \\ character");
						return false;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						default:
							if (!quiet) log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return false;
					}
					i++;
				}
				else
				{
					arg += commandWithArgs[i++];
				}
			}
		}
		else
		{
			// normal word
			for(;;)
			{
				if (allowControlChar && commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						if (!quiet) log.displayNL ("Missing character after the backslash \\ character");
						return false;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						case ';':	arg += ';'; break; // ;
						default:
							if (!quiet) log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return false;
					}
				}
				else if (allowControlChar && commandWithArgs[i] == ';')
				{
					// command separator
					break;
				}
				else
				{
					arg += commandWithArgs[i];
				}

				i++;

				if (i == commandWithArgs.size() || commandWithArgs[i] == ' ' || commandWithArgs[i] == '\t' || commandWithArgs[i] == '\n' || commandWithArgs[i] == '\r')
				{
					break;
				}
			}
		}

		if (!arg.empty())
		{
			if (firstArg)
			{
				// the first arg is the command
				TCommandParams cp;
				cp.CommandName = arg;
				commands.push_back(cp);
				firstArg = false;

				// does this command disable control char for remaining params?
				if(!isControlCharForCommandEnabled(arg))
					allowControlChar= false;
			}
			else
			{
				commands[commands.size()-1].CommandArgs.push_back (arg);
			}
		}

		// separator
		if (i < commandWithArgs.size() && allowControlChar && commandWithArgs[i] == ';')
		{
			// store the raw command
			if (!commands.empty() && commands.back().RawCommandString.empty())
				commands.back().RawCommandString = string(commandWithArgs.begin()+commandBegin, commandWithArgs.begin()+i);
			firstArg = true;
			i++;
			commandBegin = i;
		}
	}
end:
	// store the last raw command string
	if (!commands.empty() && commands.back().RawCommandString.empty())
		commands.back().RawCommandString = string(commandWithArgs.begin()+commandBegin, commandWithArgs.begin()+i);

	bool ret = true;

	for (uint u = 0; u < commands.size (); u++)
	{
		TCommandParams &cp = commands[u];
		// find the command
		// check for object name
		string::size_type pos = cp.CommandName.find(".");
		if (pos != string::npos)
		{
			// there is an object name, separate it and look in the object registry
			string objectName = cp.CommandName.substr(0, pos);
			string commandName = cp.CommandName.substr(pos+1);
			ICommandsHandler *const *ppch = _CommandsHandlers.getB(objectName);
			if (ppch != NULL)
			{
				// ok, we found the object
				ret = ret && (*ppch)->execute(commands[u].RawCommandString, commandName, commands[u].CommandArgs, log, quiet, human);
			}
			else
			{
				if (!quiet)
					log.displayNL("Command '%s' : can't found object named '%s'",
						cp.CommandName.c_str(),
						objectName.c_str());
			}
		}
		else
		{
			// this is a global command
			TCommand::iterator comm = _Commands.find(commands[u].CommandName);
			if (comm == _Commands.end ())
			{
				// the command doesn't exist
				ret = false;
				if (!quiet)
					log.displayNL("Command '%s' not found, try 'help'", commands[u].CommandName.c_str());
			}
			else
			{
				bool res = comm->second->execute (commands[u].RawCommandString, commands[u].CommandArgs, log, quiet, human);
				ret = ret & res;
				if (!res)
				{
					if (!quiet)
						log.displayNL("Bad command usage, try 'help %s'", commands[u].CommandName.c_str());
				}
			}
		}
	}

	// false if at least one command returned false
	return ret;
}


/*
 * Command name completion.
 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
 * Completes commands used with prefixes (such as "help " for example) as well.
 */
void ICommand::expand (std::string &commandName, NLMISC::CLog &log)
{
	// forward to command registry
	CCommandRegistry::getInstance().expand(commandName, log);
}

void CCommandRegistry::expand (std::string &commandName, NLMISC::CLog &log)
{
	// Take out the string before the last separator and remember it as a prefix
	string objectName;
	string::size_type lastseppos = commandName.find_last_of( " " );
	{
		// eventually use the last dot as separator
		string::size_type lastDot = commandName.find_last_of( "." );
		if (lastDot != string::npos
			&& (lastseppos == string::npos || lastDot > lastseppos))
		{
			lastseppos = lastDot;
			// store the object name to limit the matching scope
			string::size_type spcPos = commandName.find_last_of(" ", lastDot);
			if (spcPos == string::npos)
				spcPos = 0;
			else
				spcPos++;
			objectName = commandName.substr(spcPos, lastDot-spcPos);
		}
	}
	string prefix;
	bool useprefix;
	if ( lastseppos != string::npos )
	{
		prefix = commandName.substr( 0, lastseppos+1 );
		commandName.erase( 0, lastseppos+1 );
		useprefix = true;
	}
	else
	{
		useprefix = false;
	}

	string lowerCommandName = toLower(commandName);
	// Build the list of matching command names
	vector<string> matchingnames;
	{
		if (objectName.empty())
		{
			// list of global commands
			for (TCommand::iterator comm = _Commands.begin(); comm != _Commands.end(); comm++)
			{
				string first = toLower((*comm).first);
				if (first.find( lowerCommandName ) == 0)
				{
					matchingnames.push_back( (*comm).first );
				}
			}

			// list of object instance
			for (TCommandsHandlers::TAToBMap::const_iterator it(_CommandsHandlers.getAToBMap().begin()); it != _CommandsHandlers.getAToBMap().end(); ++it)
			{
				string first = toLower(it->first);
				if (first.find( lowerCommandName ) == 0)
				{
					matchingnames.push_back( it->first );
				}
			}
		}
		else
		{
			ICommandsHandler *const *pch = _CommandsHandlers.getB(objectName);
			if (pch != NULL)
			{
				// ok, an object of this name exist, lookup the class
				TCommandsHandlersClass::iterator it = _CommandsHandlersClass.find((*pch)->getCommandHandlerClassName());

				// list of class commands
				if (it != _CommandsHandlersClass.end())
				{
					TCommandHandlerClassInfo &chci = it->second;

					for (TCommandHandlerClassInfo::TCommandsInfo::iterator it(chci._Commands.begin()); it != chci._Commands.end(); ++it)
					{
						string first = toLower(it->first);
						if (first.find( lowerCommandName ) == 0)
						{
							matchingnames.push_back( it->first );
						}
					}
				}
			}
		}

	}

	// Do not complete if there is no result
	if ( matchingnames.empty() )
	{
		log.displayNL( "No matching command" );
		goto returnFromExpand;
	}

	// Complete if there is a single result
	if ( matchingnames.size() == 1 )
	{
		if (_CommandsHandlers.getAToBMap().find(matchingnames.front()) != _CommandsHandlers.getAToBMap().end())
		{
			// this is an object, complete with '.'
			commandName = matchingnames.front() + ".";
		}
		else
			commandName = matchingnames.front() + " ";
		goto returnFromExpand;
	}

	// Try to complete to the common part if there are several results
	{
		// Stop loop when a name size is i or names[i] are different
		string commonstr = commandName;
		size_t i = commandName.size();
		while ( true )
		{
			char letter = 0;
			vector<string>::iterator imn;
			for ( imn=matchingnames.begin(); imn!=matchingnames.end(); ++imn )
			{
				// Return common string if the next letter is not the same in all matching names
				if ( ((*imn).size() == i) || ( (letter!=0) && ((*imn)[i] != letter) ) )
				{
					log.displayNL( "(Matching command not unique)" );
					static string lastCommandName;
					commandName = commonstr;
					if ( lastCommandName == commandName )
					{
						// Display all the matching names
						vector<string>::iterator imn2;
						//stringstream ss;
						string str;
						//ss << "Matching commands:" << endl;
						str += "Matching commands:\n";
						for ( imn2=matchingnames.begin(); imn2!=matchingnames.end(); ++imn2 )
						{
							//ss << " " << (*imn2);
							str += " " + (*imn2);
						}
						log.displayNL( "%s", str.c_str() );
					}
					lastCommandName = commandName;
					goto returnFromExpand;
				}
				// Add the next letter to the common string if it is the same in all matching names
				else if ( letter == 0 )
				{
					letter = (*imn)[i];
				}
			}
			commonstr += letter;
			++i;
		}
	}

returnFromExpand:

	// Put back the prefix
	if ( useprefix )
	{
		commandName = prefix + commandName;
	}
}


void ICommand::serialCommands (IStream &f)
{
	CCommandRegistry::getInstance().serialCommands(f);
}

void CCommandRegistry::serialCommands (IStream &f)
{
	vector<CSerialCommand> cmd;
	for (TCommand::iterator comm = _Commands.begin(); comm != _Commands.end(); comm++)
	{
		cmd.push_back (CSerialCommand ((*comm).first, (*comm).second->Type));
	}
	f.serialCont (cmd);
}

bool ICommand::exists (std::string const &commandName)
{
	return CCommandRegistry::getInstance().exists(commandName);
}
bool CCommandRegistry::exists (std::string const &commandName)
{
	return (_Commands.find(commandName) != _Commands.end ());
}

bool CCommandRegistry::isNamedCommandHandler(const std::string &handlerName)
{
	return _CommandsHandlers.getB(handlerName) != NULL;
}

bool ICommand::isCommand (const std::string &str)
{
	return CCommandRegistry::getInstance().isCommand(str);
}

bool CCommandRegistry::isCommand (const std::string &str)
{
	if (str.empty())
		return false;

	return isupper(str[0]) == 0;
}

ICommand *ICommand::getCommand(const std::string &commandName)
{
	return CCommandRegistry::getInstance().getCommand(commandName);
}

ICommand *CCommandRegistry::getCommand(const std::string &commandName)
{
	TCommand::iterator it(_Commands.find(commandName));

	if (it == _Commands.end())
		return NULL;
	else
		return it->second;
}


NLMISC_CATEGORISED_COMMAND(nel,help,"display help on a specific variable/commands or on all variables and commands", "[<variable>|<command>]")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);
//	nlassert (_Commands != NULL);

	// make sure we have a valid number of parameters
	if (args.size()>1)
		return false;

	CCommandRegistry &cr = CCommandRegistry::getInstance();

	// treat the case where we have no parameters
	if (args.empty())
	{
		// display a list of all command categories
		log.displayNL("Help commands:");
		log.displayNL("- help all");
		for (CCommandRegistry::TCategorySet::iterator it=cr._Categories.begin();it!=cr._Categories.end();++it)
		{
			log.displayNL("- help %s",it->c_str());
		}
		log.displayNL("- help <wildcard>");
		log.displayNL("- help <command name>");
		return true;
	}

	// treat the case where the supplied parameter is "all"
	if (args[0]=="all")
	{
		// display all commands
		log.displayNL("Displaying all %d variables and commands: ", cr._Commands.size());
		uint i = 0;
		for (TCommand::iterator comm = cr._Commands.begin(); comm != cr._Commands.end(); ++comm, i++)
		{
			log.displayNL("%2d %-15s: %s", i, comm->first.c_str(), comm->second->HelpString.c_str());
		}

		// display the class commands
		{
			CCommandRegistry::TCommandsHandlersClass::iterator first(cr._CommandsHandlersClass.begin()), last(cr._CommandsHandlersClass.end());
			for (; first != last; ++first)
			{
				log.displayNL("%-15s :", first->first.c_str());
				TCommandHandlerClassInfo &chci = first->second;
				{
					TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
					for (;first != last; ++first)
					{
						log.displayNL("  %-15s: %s", first->first.c_str(), first->second.CommandHelp.c_str());
					}
				}
			}
		}

		// display the named instance instance
		{
			log.displayNL("Listing named object instance : <name> : <className>");
			CCommandRegistry::TCommandsHandlers::TAToBMap::const_iterator first(cr._CommandsHandlers.getAToBMap().begin()), last(cr._CommandsHandlers.getAToBMap().end());
			for (; first != last; ++first)
			{
				log.displayNL(" %-15s: %s",
					first->first.c_str(),
					first->second->getCommandHandlerClassName().c_str());
			}
		}
		return true;
	}

	// treat the case where the supplied parameter is a category name
	{
		if (cr._Categories.find(args[0])!=cr._Categories.end())
		{
			log.displayNL("Displaying commands and variables from category: %s", args[0].c_str());
			uint i = 0;
			for (TCommand::iterator comm = cr._Commands.begin(); comm != cr._Commands.end(); ++comm)
			{
				if (comm->second->CategoryName == args[0])
				{
					log.displayNL("%2d %-15s: %s", i, comm->first.c_str(), comm->second->HelpString.c_str());
					i++;
				}
			}
			return true;
		}
	}
	// treat the case where the supplied parameter is a class name
	{
		string className = args[0].substr(0, args[0].find("."));
		if (cr._CommandsHandlersClass.find(className) != cr._CommandsHandlersClass.end())
		{
			TCommandHandlerClassInfo &chci = cr._CommandsHandlersClass[className];
			if (className != args[0])
			{
				string cmdName = args[0].substr(className.size()+1);
				// we are looking for a particular command in this class
				TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
				for (;first != last; ++first)
				{
					if (first->first == cmdName)
					{
						log.displayNL("%s::%s", className.c_str(), cmdName.c_str());
						log.displayNL("usage: <instanceName>.%s %s : %s",
							cmdName.c_str(),
							first->second.CommandArgs.c_str(),
							first->second.CommandHelp.c_str());
//						log.displayNL("  %s::%-15s: %s", className.c_str(), cmdName.c_str(), first->second.CommandHelp.c_str());
						return true;
					}
				}

			}
			else
			{
				log.displayNL("%-15s :", args[0].c_str());
				{
					TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
					for (;first != last; ++first)
					{
						log.displayNL("  %-15s: %s", first->first.c_str(), first->second.CommandHelp.c_str());
					}
				}

				// list the instance of this class
				log.displayNL(" Here is a list of the %u named instance of this class", chci.InstanceCount);
				for (CCommandRegistry::TCommandsHandlers::TAToBMap::const_iterator it=cr._CommandsHandlers.getAToBMap().begin(); it != cr._CommandsHandlers.getAToBMap().end(); ++it)
				{
					if (it->second->getCommandHandlerClassName() == args[0])
					{
						log.displayNL("  %-15s", it->first.c_str());
					}
				}
				return true;
			}
		}
	}
	// treat the case where the supplied parameter is an object name
	{
		string objName = args[0].substr(0, args[0].find("."));

		if (cr._CommandsHandlers.getB(objName) != NULL)
		{
			const string &className = (*(cr._CommandsHandlers.getB(objName)))->getCommandHandlerClassName();
			if (cr._CommandsHandlersClass.find(className) != cr._CommandsHandlersClass.end())
			{
				TCommandHandlerClassInfo &chci = cr._CommandsHandlersClass[className];
				if (objName != args[0])
				{
					// only display a particular command of this class instance
					string cmdName = args[0].substr(objName.size()+1);
					TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
					for (;first != last; ++first)
					{
						if (first->first == cmdName)
						{
							log.displayNL("%s.%s", objName.c_str(), cmdName.c_str());
							log.displayNL("usage: %s.%s %s : %s",
								objName.c_str(),
								cmdName.c_str(),
								first->second.CommandArgs.c_str(),
								first->second.CommandHelp.c_str());
//							log.displayNL("  %s.%-15s: %s", className.c_str(), cmdName.c_str(), first->second.CommandHelp.c_str());
							return true;
						}
					}
				}
				else
				{
					TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
					for (;first != last; ++first)
					{
						log.displayNL("  %-15s: %s", first->first.c_str(), first->second.CommandHelp.c_str());
					}

					return true;
				}
			}
		}
	}

	// treat the case where the supplied parameter is a wildcard
	if (args[0].find('*')!=std::string::npos || args[0].find('?')!=std::string::npos)
	{
		log.displayNL("Displaying commands, variables and objects matching wildcard: '%s'", args[0].c_str());
		log.displayNL(" Global commands and variables :");
		uint i = 0;
		for (TCommand::iterator comm = cr._Commands.begin(); comm != cr._Commands.end(); ++comm)
		{
			if (testWildCard(comm->first,args[0]))
			{
				log.displayNL("%2d %-15s: %s", i, comm->first.c_str(), comm->second->HelpString.c_str());
				i++;
			}
		}

		// display the named instance instance that match
		{
			log.displayNL(" Named objects instances : <name> : <className>");
			CCommandRegistry::TCommandsHandlers::TAToBMap::const_iterator first(cr._CommandsHandlers.getAToBMap().begin()), last(cr._CommandsHandlers.getAToBMap().end());
			for (; first != last; ++first)
			{
				if (testWildCard(first->first, args[0]))
				{
					log.displayNL("  %-15s: '%s'",
						first->first.c_str(),
						first->second->getCommandHandlerClassName().c_str());
				}
			}
		}

		// display the class commands that match
		{
			log.displayNL(" Class commands :");
			CCommandRegistry::TCommandsHandlersClass::iterator first(cr._CommandsHandlersClass.begin()), last(cr._CommandsHandlersClass.end());
			for (; first != last; ++first)
			{
				const string &className = first->first;
				TCommandHandlerClassInfo &chci = first->second;
				{
					TCommandHandlerClassInfo::TCommandsInfo::iterator first(chci._Commands.begin()), last(chci._Commands.end());
					for (;first != last; ++first)
					{
						if (testWildCard(first->first, args[0]))
						{
							log.displayNL("  %s::%-15s: %s",
								className.c_str(),
								first->first.c_str(),
								first->second.CommandHelp.c_str());
						}
					}
				}
			}
		}

		return true;
	}

	// treat the case where we're looking at help on a given command
	{
		// look in global commands
		if (cr._Commands.find(args[0]) != cr._Commands.end())
		{
			TCommand::iterator comm = cr._Commands.find(args[0]);
			log.displayNL("%s", comm->second->HelpString.c_str());

			std::vector<std::string> commandArgs;
			splitString(comm->second->CommandArgs, "\n", commandArgs);

			log.displayNL("usage: %s %s",
				comm->first.c_str(),
				commandArgs.empty() ? "":commandArgs.front().c_str());

			for(uint i = 1; i < commandArgs.size(); ++i)
				log.displayNL("%s", commandArgs[i].c_str());

			return true;
		}
		// look in the class commands
		{
			CCommandRegistry::TCommandsHandlersClass::iterator first(cr._CommandsHandlersClass.begin()), last(cr._CommandsHandlersClass.end());
			for (; first != last; ++first)
			{
				TCommandHandlerClassInfo &chci = first->second;
				{
					TCommandHandlerClassInfo::TCommandsInfo::iterator it = chci._Commands.find(args[0]);
					if (it != chci._Commands.end())
					{
						log.displayNL("%s", it->second.CommandHelp.c_str());

						std::vector<std::string> commandArgs;
						splitString(it->second.CommandArgs, "\n", commandArgs);

						log.displayNL("usage: %s %s",
							it->first.c_str(),
							commandArgs.empty() ? "":commandArgs.front().c_str());

						for(uint i = 1; i < commandArgs.size(); ++i)
							log.displayNL("%s", commandArgs[i].c_str());

						return true;
					}
				}
			}
		}
	}


	// we've failed to find a case that works so display an error message and prompt the player
	log.displayNL("'%s' is not a command, category or wildcard. Type 'help' for more information", args[0].c_str());
	return true;
}

// ***************************************************************************
void	ICommand::enableControlCharForCommand(const std::string &commandName, bool state)
{
	CCommandRegistry::getInstance().enableControlCharForCommand(commandName, state);
}
void	CCommandRegistry::enableControlCharForCommand(const std::string &commandName, bool state)
{
	if(state)
		// allow, so erase from the set of those who disable
		_CommandsDisablingControlChar.erase(commandName);
	else
		// disable, so insert in the set of those who disable
		_CommandsDisablingControlChar.insert(commandName);
}

// ***************************************************************************
bool	ICommand::isControlCharForCommandEnabled(const std::string &commandName)
{
	return CCommandRegistry::getInstance().isControlCharForCommandEnabled(commandName);
}
bool	CCommandRegistry::isControlCharForCommandEnabled(const std::string &commandName)
{
	// true if not in the set
	return _CommandsDisablingControlChar.find(commandName)==_CommandsDisablingControlChar.end();
}

ICommandsHandler::ICommandsHandler()
: _ClassName(NULL)
{
}

void ICommandsHandler::registerCommandsHandler()
{
	if (_ClassName == NULL)
	{
		// store the class name for unregistering during destruction
		_ClassName = &getCommandHandlerClassName();
		CCommandRegistry::getInstance().registerNamedCommandHandler(this, *_ClassName);
	}
}

void ICommandsHandler::unregisterCommandsHandler()
{
	if (_ClassName != NULL)
	{
		CCommandRegistry::getInstance().unregisterNamedCommandHandler(this, *_ClassName);
		_ClassName = NULL;
	}
}


ICommandsHandler::~ICommandsHandler()
{
	unregisterCommandsHandler();
}


} // NLMISC
