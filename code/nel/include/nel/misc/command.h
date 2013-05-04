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

#ifndef NL_COMMAND_H
#define NL_COMMAND_H

#include "types_nl.h"
#include "twin_map.h"

#include <string>
#include <map>
#include <set>
#include <vector>
//#include <sstream>
#include <istream>

#include "stream.h"
#include "config_file.h"
#include "log.h"


namespace NLMISC {

/** WARNING:
 *   This is standard Unix linker behavior: object files
 *   that are not referenced from outside are discarded. The
 *   file in which you run your constructor is thus simply
 *   thrown away by the linker, which explains why the constructor
 *   is not run.
 */

/**
 * Create a function that can be call in realtime.
 *
 * Example:
 * \code
	// I want to create a function that computes the square of the parameter and display the result
	NLMISC_COMMAND(square,"display the square of the parameter","<value>")
	{
		// check args, if there s not the right number of parameter, return bad
		if(args.size() != 1) return false;
		// get the value
		uint32 val;
		fromString(args[0], val);
		// display the result on the displayer
		log.displayNL("The square of %d is %d", val, val*val);
		return true;
	}
 * \endcode
 *
 * Please use the same casing than for the function (first letter in lower case and after each word first letter in upper case)
 * ie: myFunction, step, orderByName, lookAtThis
 *
 * System extended by Sadge July 2004
 *  - NLMISC_CATEGORISED_COMMAND now takes a 4th 'category' parameter which is used by 'help' system to organise cmmands
 *  - All default commands added by NeL are categorised as "nel"
 *  - All commands created using the NLMISC_COMMAND macro are are categorised as "commands"
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
#define NLMISC_COMMAND(__name,__help,__args) NLMISC_CATEGORISED_COMMAND(commands,__name,__help,__args)
#define NLMISC_CATEGORISED_COMMAND(__category,__name,__help,__args) \
struct __category##_##__name##Class: public NLMISC::ICommand \
{ \
	__category##_##__name##Class() : NLMISC::ICommand(#__category,#__name,__help,__args) { } \
	virtual bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human); \
}; \
__category##_##__name##Class __category##_##__name##Instance; \
bool __category##_##__name##Class::execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)

/** Helper to declare a command as friend of a class.
 *	Useful when you want to declare debug command that access private class method or data.
*/
#define NLMISC_COMMAND_FRIEND(__name) friend struct commands_##__name##Class
#define NLMISC_CATEGORISED_COMMAND_FRIEND(__category,__name) friend struct __category##_##__name##Class

/**
 * Create a function that can be call in realtime. Don't use this class directly but use the macro NLMISC_COMMAND
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class ICommand
{
public:

	/// Constructor
	ICommand(const char *categoryName, const char *commandName, const char *commandHelp, const char *commandArgs);

	virtual ~ICommand();

	// quiet means that we don't display anything else than the value
	// human means that we want the value in a human readable if possible
	virtual bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human = true) = 0;

	std::string CategoryName;
	std::string HelpString;
	std::string CommandArgs;

	// is it a variable or a classic command?
	enum TType { Unknown, Command, Variable };
	TType Type;

	// static members

	typedef std::map<std::string, ICommand *> TCommand;
//	typedef std::set<std::string> TCategorySet;

	static TCommand		*LocalCommands;
//	static TCategorySet	*LocalCategories;
//	static TCommand		*LocalCommands;
	static bool			LocalCommandsInit;

	/// Executes the command and display output to the log
	/// \param quiet true if you don't want to display the "executing the command ..."
	static bool execute (const std::string &commandWithArgs, NLMISC::CLog &log, bool quiet = false, bool human = true);

	/** Command name completion.
	 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
     * Completes commands used with prefixes (such as "help " for example) as well.
	 */
	static void	expand (std::string &commandName, NLMISC::CLog &log=*InfoLog);

	static void serialCommands (IStream &f);

	/// returns true if the command exists
	static bool exists (std::string const &commandName);

	/// if the string begin with an upper case, it s a variable, otherwise, it s a command
	static bool isCommand (const std::string &str);

	/// Retrieve the interface over command object for the given command name.
	static ICommand *getCommand(const std::string &commandName);

	const std::string &getName () const { return _CommandName; }

	/** declare a command to "enable control char". By default all commands "enable control char"
	 *
	 *	eg: if enableControlCharForCommand("region", false) is called, then the command:
	 *
	 *	region hello; "i am busy" \never disturb me please
	 *
	 *	won't treat '"', '\' and ';' as special control character.
	 *	Thus the final list of args will be (separated by '/' here for clarity):
	 *
	 *	hello;/"i/am/busy"/\never/disturb/me/please
	 */
	static void	enableControlCharForCommand(const std::string &commandName, bool state);

	/// see enableControlCharForCommand()
	static bool	isControlCharForCommandEnabled(const std::string &commandName);

protected:

	std::string _CommandName;

};


/** Struct to host data for one object command
 * \author Boris 'SoniX' Boucher
 * \author Nevrax France
 * \date 2005
 */
struct TCommandHandlerInfo
{
	/// The help string of the command
	std::string		CommandHelp;
	/// The argument required for the command
	std::string		CommandArgs;

	/// A comparison operator need for STL container storage
	bool operator ==(const TCommandHandlerInfo &other) const
	{
		return (CommandHelp == other.CommandHelp) && (CommandArgs == other.CommandArgs);
	}
};

/** Struct to host data for all the commands of an object class
 * \author Boris 'SoniX' Boucher
 * \author Nevrax France
 * \date 2005
 */
struct TCommandHandlerClassInfo
{
	/// Number of instance of object of this class
	uint32			InstanceCount;

	/// The list of command available on this class of object.
	typedef std::map<std::string, TCommandHandlerInfo>	TCommandsInfo;
	TCommandsInfo	_Commands;

	/// Constructor.
	TCommandHandlerClassInfo()
		: InstanceCount(0)
	{}
};


/** Base class for command handler.
 *	Command handler are a mean to build object that support NeL commands
 *	_Commands are associated to object class and invoked to named instance.
 *	Each named instance must have a unique name (whatever it's class).
 *	Unlike NeL global commands, object commands are invoked in the context
 *	of the object instance.
 *
 *	In order to write an object that support commands, you must devive from
 *	template<class MyClass> CCommandsHandler. The class ICommandsHandler
 *	is used by the command registry to handle any type of object.
 *
 * \author Boris 'SoniX' Boucher
 * \author Nevrax France
 * \date 2005
 */
class ICommandsHandler
{
	/// Store the class name after handler registration
	const std::string		*_ClassName;
public:

	ICommandsHandler();

	/** The derived class call this method to register the instance in
	 *	the command registry.
	 *	Before calling this method, the object is not available for
	 *	commands invocation.
	 */
	void registerCommandsHandler();
	/** The derived class call this method to unregister the instance in
	 *	the command registry.
	 *	After this call, the object is no more liste in named object
	 *	list nor it can receive command invocation.
	 *	You can later re-register the object.
	 */
	void unregisterCommandsHandler();

	virtual const std::string &getCommandHandlerClassName() const =0;

	/** This methods implemented by CCommandHandler is used by the
	 *	command registry to retrieve the name of the object instance.
	 */
	virtual const std::string &getCommandHandlerName() const =0;

	/** This methods implemented by CCommandHandler is used by the
	 *	command registry to build the list of available commands
	 *	on the object class.
	 */
	virtual void fillCommandsHandlerList(TCommandHandlerClassInfo::TCommandsInfo &commandList) =0;

	/** Virtual destructor to unregister the object instance.
	 *	When all the instance of a given class are deleted,
	 *	the associated command an class information are
	 *	removed from the command registry.
	 */
	virtual ~ICommandsHandler();

	/** This methods implemented by CCommandHandler is used by the
	 *	command registry to start a command execution.
	 */
	virtual bool execute(const std::string &rawCommandString, const std::string &commandName, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human = true) =0;
};

template <class T>
struct TCommandHandler : public TCommandHandlerInfo
{
	typedef bool (T::*TCommand)(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human);

	TCommand	CommandHandler;
};


/** Template class used as base for derivation of object that support commands.
 *	To declare your object supporting commands, you must
 *	derive from this class with your class type as template parameter.
 *
 *	e.g :
 *	class CMyClass : public CCommandsHandler<CMyClass>
 *	{
 *	};
 *
 *	To easily generate the command table, NeL provide some macros :
 *
 *	class CMyClass : public CCommandsHandler<CMyClass>
 *	{
 *	public:
 *
 *		NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CMyClass)
 *			NLMISC_COMMAND_HANDLER_ADD(CMyClass, theCommand1, "help", "args")
 *			NLMISC_COMMAND_HANDLER_ADD(CMyClass, theCommand2, "other help", "other args")
 *		NLMISC_COMMAND_HANDLER_TABLE_END
 *
 *		NLMISC_CLASS_COMMAND_DECL(theCommand1)
 *		{
 *			// put yout code here
 *		}
 *
 *		NLMISC_CLASS_COMMAND_DECL(theCommand2)
 *		{
 *			// put yout code here
 *		}
 *	};
 *
 *	You can also derive a class and add some more commands in the
 *	derived class by using NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN:
 *
 *	class CMyDerivedClass : public CMyClass
 *	{
 *		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CMyClass)
 *			NLMISC_COMMAND_HANDLER_ADD(CMyClass, addedCommand, "help", "args")
 *		NLMISC_COMMAND_HANDLER_TABLE_END
 *
 *		NLMISC_CLASS_COMMAND_DECL(addedCommand)
 *		{
 *			// put yout code here
 *		}
 *	};
 *
 * \author Boris 'SoniX' Boucher
 * \author Nevrax France
 * \date 2005
 */
//template <class T>
//class CCommandsHandler : public ICommandsHandler
//{
//public:
//	/** Constructor, used to initialise the class name in the interface class */
////	CCommandsHandler()
////		: ICommandsHandler(T::getCommandHandlerClassName())
////	{
////	}
////
//	/** Virtual pure method used by execute to retrieve the method pointer.
//	 *	This method will be automatically implemented by the NLMISC_COMMAND_HANDLER_TABLE_BEGIN
//	 *	macro utility.
//	 */
////	virtual TCommand getCommandHandler(const std::string &commandName) =0;
//
//	/** Execute a command.
//	 *	Return false if no command of the name exist.
//	 */
////	bool execute(const std::string &rawCommandString, const std::string &commandName, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human = true)
////	{
////		TCommand cmd = getCommandHandler(commandName);
////		if (cmd != NULL)
////		{
////			T* tp = static_cast<T*>(this);
////			return (tp->*cmd)(rawCommandString, args, log, quiet, human);
////		}
////		else
////		{
////			log.displayNL("Command on object '%s' : unknow command '%s'",
////				getCommandHandlerName().c_str(),
////				commandName.c_str());
////			return false;
////		}
////	}
//};

/** Macro to start a command handler table.
 *	Use it inside the class declaration.
 */
#define NLMISC_COMMAND_HANDLER_TABLE_BEGIN(className)	\
	/* Typedef a method pointer on the template class.		*/	\
	/*	This type is the type of the command handler method.*/	\
	/*	It have the same signature as global NeL commands.	*/	\
	typedef bool (className::*TCommand)(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human);	\
	\
	/** Typedef for a container for command handler table. */	\
	typedef std::map<std::string, NLMISC::TCommandHandler<className> >	TCommandsTable;	\
	\
	virtual const std::string &getCommandHandlerClassName()	const\
	{	\
		static std::string className(#className);	\
		return className;	\
	}	\
	TCommand className##_getCommandHandler(const std::string &commandName)	\
	{	\
		TCommandsTable	commandTable = className##_getCommandsHandlerTable();	\
		TCommandsTable::iterator it = commandTable.find(commandName);	\
		\
		if (it != commandTable.end())	\
		{	\
			/** ok, we have the command handler*/	\
			return it->second.CommandHandler;	\
		}	\
		else	\
			return NULL;	\
	}	\
	virtual bool execute(const std::string &rawCommandString, const std::string &commandName, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human) \
	{ \
		TCommand cmd = className##_getCommandHandler(commandName); \
		if (cmd != NULL) \
		{ \
			if (!quiet)\
				log.displayNL("Execute command: %s", rawCommandString.c_str()); \
			return (this->*cmd)(rawCommandString, args, log, quiet, human); \
		} \
		else \
		{ \
			log.displayNL("Command on object '%s' : unknow command '%s'", \
				getCommandHandlerName().c_str(), \
				commandName.c_str()); \
			return false; \
		} \
	} \
	virtual void fillCommandsHandlerList(NLMISC::TCommandHandlerClassInfo::TCommandsInfo &commandList)	\
	{	\
		const TCommandsTable	&commandTable = className##_getCommandsHandlerTable();	\
		TCommandsTable::const_iterator first(commandTable.begin()), last(commandTable.end());	\
		for (; first != last; ++first)	\
		{	\
			commandList.insert(std::make_pair(first->first, first->second));	\
		}	\
	}	\
	\
	const TCommandsTable &className##_getCommandsHandlerTable()	\
	{	\
		static bool initialized = false;	\
		static TCommandsTable	commandsTable;	\
	\
		if (!initialized)	\
		{	\
			initialized = true; \


/** Macro to add a handler in the handler table.
 *	Use this macro between NLMISC_COMMAND_HANDLER_TABLE_BEGIN and
 *	NLMISC_COMMAND_HANDLER_TABLE_END macros.
 *
 *	The command name must match a method of the class with
 *	a command signature.
 *	You can easily declare command method using the macro
 *	NLMISC_CLASS_COMMAND_DECL
 *
 */
#define NLMISC_COMMAND_HANDLER_ADD(className, theCommandName, theCommandHelp, theCommandArgs) \
			{	\
				NLMISC::TCommandHandler<className> ch;	\
				ch.CommandArgs = theCommandArgs;	\
				ch.CommandHelp = theCommandHelp;	\
				ch.CommandHandler = &className::cmdHandler_##theCommandName; \
				commandsTable.insert(std::make_pair(std::string(#theCommandName), ch));	\
			}	\

/** Macro to end the command handler table.
 *	Must be put after the last command handler adding.
 */
#define NLMISC_COMMAND_HANDLER_TABLE_END	\
		}	\
		return commandsTable; \
	}	\

/** Macro to add commands in a class that derive from a class
 *	that already declare a command handler table.
 *	the most derivative class will override base class commands
 *	if they have the same name.
 */
#define NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(className, baseClassName)	\
	typedef bool (className::*TCommand)(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human);	\
	typedef std::map<std::string, NLMISC::TCommandHandler<className> >	TCommandsTable;	\
	virtual const std::string &getCommandHandlerClassName()	const\
	{	\
		static std::string className(#className);	\
		return className;	\
	}	\
	TCommand className##_getCommandHandler(const std::string &commandName)	\
	{	\
		TCommandsTable	commandTable = className##_getCommandsHandlerTable();	\
		TCommandsTable::iterator it = commandTable.find(commandName);	\
		\
		if (it != commandTable.end())	\
		{	\
			/* ok, we have the command handler*/	\
			return it->second.CommandHandler;	\
		}	\
		else	\
		{ \
			return NULL;\
		} \
	}	\
	virtual bool execute(const std::string &rawCommandString, const std::string &commandName, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human) \
	{ \
		TCommand cmd = className##_getCommandHandler(commandName); \
		if (cmd != NULL) \
		{ \
			return (this->*cmd)(rawCommandString, args, log, quiet, human); \
		} \
		else \
		{ \
			/* try with the base class */ \
			return baseClassName::execute(rawCommandString, commandName, args, log, quiet, human);\
		} \
	} \
	virtual void fillCommandsHandlerList(NLMISC::TCommandHandlerClassInfo::TCommandsInfo &commandList)	\
	{	\
		const TCommandsTable	&commandTable = className##_getCommandsHandlerTable();	\
		TCommandsTable::const_iterator first(commandTable.begin()), last(commandTable.end());	\
		for (; first != last; ++first)	\
		{	\
			commandList.insert(std::make_pair(first->first, first->second));	\
		}	\
		/* call base class to complete the command table */ \
		baseClassName::fillCommandsHandlerList(commandList); \
	}	\
	\
	const TCommandsTable &className##_getCommandsHandlerTable()	\
	{	\
		static bool initialized = false;	\
		static TCommandsTable	commandsTable;	\
	\
		if (!initialized)	\
		{	\
			initialized = true; \


// A macro to declare or implement inline the command method
#define NLMISC_CLASS_COMMAND_DECL(commandName) \
	bool cmdHandler_##commandName(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)

// A macro to implement the command method in a cpp (you still need to declare it in the class scope using the previous macro)
#define NLMISC_CLASS_COMMAND_IMPL(className, commandName) \
	bool className::cmdHandler_##commandName(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human)

// A macro to recall a base class command implementation
#define NLMISC_CLASS_COMMAND_CALL_BASE(baseClassName, commandName)\
	baseClassName::cmdHandler_##commandName(rawCommandString, args, log, quiet, human)

/** The command registry is a singleton that hold all available
 *	commands.
 *	It bridge the legacy global commands and variable with
 *	the object commands.
 *
 *	All the legacy static call made on ICommand are now forwarded
 *	directly to this class (e.g. IService::execute)
 *
 *	This class is modeled after the safe singlton pattern,
 *	so it is safe to use in a dynamicaly loaded library
 *	program.
 *
 * \author Boris 'SoniX' Boucher
 * \author Nevrax France
 * \date 2005
 */
class CCommandRegistry
{
	// this class is a safe singleton (dll friendly)
	NLMISC_SAFE_SINGLETON_DECL(CCommandRegistry);
	CCommandRegistry() {}

	NLMISC_CATEGORISED_COMMAND_FRIEND(nel, help);

	friend void cbVarChanged (CConfigFile::CVar &var);
	friend class ICommand;
	friend class ICommandsHandler;
	friend class INelContext;

	typedef std::map<std::string, ICommand *>	TCommand;
	typedef std::set<std::string>				TCategorySet;

	/// List of commands categories
	TCategorySet	_Categories;
	/// List of available command
	TCommand		_Commands;

	typedef CTwinMap<std::string, ICommandsHandler *>	TCommandsHandlers;
	/// Registry for commands handlers named instance
	TCommandsHandlers			_CommandsHandlers;

	typedef std::map<std::string, TCommandHandlerClassInfo >	TCommandsHandlersClass;
	/// Registry for commands name and handler class
	TCommandsHandlersClass		_CommandsHandlersClass;

	std::set<std::string>		_CommandsDisablingControlChar;

	/// Used by ICommand to register themselves
	void registerCommand(ICommand *command);
	/// Used by ICommand to unregister themselves
	void unregisterCommand(ICommand *command);


public:

	/// Called by command handlers to register themselves.
	void registerNamedCommandHandler(ICommandsHandler *handler, const std::string &className);
	/// Called by command handlers to unregister themselves.
	void unregisterNamedCommandHandler(ICommandsHandler *handler, const std::string &className);

	/// Executes the command and display output to the log
	/// \param quiet true if you don't want to display the "executing the command ..."
	bool execute (const std::string &commandWithArgs, NLMISC::CLog &log, bool quiet = false, bool human = true);

	/** Command name completion.
	 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
     * Completes commands used with prefixes (such as "help " for example) as well.
	 */
	void expand (std::string &commandName, NLMISC::CLog &log=*InfoLog);

	void serialCommands (IStream &f);

	/// returns true if the command exists
	bool exists (std::string const &commandName);

	/// Return true if a named command handler with that name is registered
	bool isNamedCommandHandler(const std::string &handlerName);

	/// if the string begin with an upper case, it s a variable, otherwise, it s a command
	bool isCommand (const std::string &str);

	/// Retrieve the interface over command object for the given command name.
	ICommand *getCommand(const std::string &commandName);

	/** declare a command to "enable control char". By default all commands "enable control char"
	 *
	 *	eg: if enableControlCharForCommand("region", false) is called, then the command:
	 *
	 *	region hello; "i am busy" \never disturb me please
	 *
	 *	won't treat '"', '\' and ';' as special control character.
	 *	Thus the final list of args will be (separated by '/' here for clarity):
	 *
	 *	hello;/"i/am/busy"/\never/disturb/me/please
	 */
	void	enableControlCharForCommand(const std::string &commandName, bool state);

	/// see enableControlCharForCommand()
	bool	isControlCharForCommandEnabled(const std::string &commandName);

	// initialisation for IVariable management (variable are an extension of commands)
	void initVariables(NLMISC::CConfigFile &configFile);

};

/** This class is only used to serialize easily a command for the admin service for example */
struct CSerialCommand
{
	CSerialCommand () : Name ("<Unknown>"), Type(ICommand::Unknown) { }
	CSerialCommand (std::string n, ICommand::TType t) : Name (n), Type(t) { }

	std::string Name;
	ICommand::TType Type;

	void serial (IStream &f)
	{
		f.serial (Name);
		f.serialEnum (Type);
	}
};


} // NLMISC


#endif // NL_COMMAND_H

/* End of command.h */
