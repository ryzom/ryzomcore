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



#ifndef NL_COMMAND_EVENT_MANAGER_H
#define NL_COMMAND_EVENT_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/eval_num_expr.h"
#include "nel/net/unified_network.h"

#include <map>
#include <vector>
#include <string>


/**
 * A Manager that triggers events and is receptive to events
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CCommandEventManager
{
	friend void	cbProcessScriptEvent( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );

public:
	/// A type of variable, basically consisting in a name and a value (as a string)
	typedef std::pair<std::string, std::string>				TVar;

	/// The variable environment of a script
	class CEnvironment : public NLMISC::CEvalNumExpr
	{
	private:
		/// The map type of vars
		typedef	std::map<std::string, std::string>	TVarMap;

		/// The map of vars itself
		TVarMap		_Map;

		/// The empty string
		std::string	_Empty;

	public:
		CEnvironment() : _Empty("") {}

		/// Clear the whole environment
		void				clear() { _Map.clear(); }
		/// Checks if var exists
		bool				exists(const std::string &var) { return _Map.find(var) != _Map.end(); }
		/// Get the value of a var
		const std::string	&get(const std::string &var) { TVarMap::iterator it=_Map.find(var); return (it!=_Map.end()) ? (*it).second : _Empty; }
		/// Set the value of a var
		const std::string	&set(const std::string &var, const std::string &value)
		{
			std::pair<TVarMap::iterator, bool>	res = _Map.insert(std::make_pair<std::string, std::string>(var, value));
			if (!res.second)
				(*(res.first)).second = value;
			return (*(res.first)).second;
		}
		/// Erases a var
		void				erase(const std::string &var) { _Map.erase(var); }
		/// Replaces text by value in string (variable must have the form $(variable) to be replaced by their value)
		std::string			replace(const std::string &text);

	protected:
		virtual NLMISC::CEvalNumExpr::TReturnState	evalValue(const char *value, double &result)
		{
			TVarMap::iterator	it;
			if (*value == '?')
			{
				it = _Map.find(value+1);
				if (it == _Map.end())
					result = 0.0;
				else
					result = 1.0;
				return NLMISC::CEvalNumExpr::NoError;
			}
			else
			{
				it = _Map.find(value);
				if (it == _Map.end())
					return NLMISC::CEvalNumExpr::UnkownValue;
				result = atof((*it).second.c_str());
				return NLMISC::CEvalNumExpr::NoError;
			}
		}
	};

	class CScript;

	/// A script node
	class CScriptNode
	{
	public:
		enum EState
		{
			Ok,
			Error,
			Halt,
		};

		/// The line in the script for this node
		uint		Line;

		/// Current running script
		CScriptNode	*CurrentRun;

		/// The root script
		CScript		*Root;

	public:
		/// default constructor
		CScriptNode() : Line(0), CurrentRun(NULL) {}

		/// virtual destructor
		virtual ~CScriptNode() {}

		/// Execute the script node, returns the next node to be executed, or NULL if script finished
		virtual EState	execute(CEnvironment &env) = 0;

	protected:
		/// Execute current node (previous run returned halt)
		EState			goOnCurrentRun(CEnvironment &env)
		{
			if (!CurrentRun)
				return Error;

			EState	state = CurrentRun->execute(env);
			if (state == Ok)
				CurrentRun = NULL;
			return state;
		}

		/// Execute child
		EState			executeChild(CScriptNode* child, CEnvironment &env)
		{
			EState	state = child->execute(env);
			if (state == Halt || state == Error)
				CurrentRun = child;
			return state;
		}
	};

	/// The script class that receives events, generates events, and triggers commands
	class CScript
	{
	public:
		/// The name of the script
		std::string					Name;
		/// The script environment
		CEnvironment				Environment;
		/// The script text
		CScriptNode					*Root;
		/// Verbosity
		bool						Verbose;

		/// Constructor.
		CScript(const std::string &name) : Name(name), Verbose(false) {}

		///
		bool	parse(const char *text);

		/// Inits script before execution with no default parameters
		void	init()				{ init(std::vector<TVar>()); }

		/// Inits script before execution, using default parameters
		void	init(const std::vector<TVar> &initVars)
		{
			uint	i;
			for (i=0; i<initVars.size(); ++i)
				Environment.set(initVars[i].first, initVars[i].second);
		}

		/// Runs script till it is finished or it waits for an event
		bool	run();
	};

private:
	typedef std::map<std::string, CScript*>	TScriptMap;
	static TScriptMap						_RunningScripts;
	static std::string						_Empty;

public:
	static void	init();
	static void	release();

	static bool	update();

	static bool					runScript(const std::string &scriptName, const std::string &scriptFile, bool verbose=false);
	static bool					stopScript(const std::string &scriptName);
	static const std::string	&getVarInScript(const std::string &scriptName, const std::string &varName);
	static void					setVarInScript(const std::string &scriptName, const std::string &varName, const std::string &value);
	static void					eraseVarInScript(const std::string &scriptName, const std::string &varName);

	static void					sendEvent(const std::string &var, const std::string &value);

	static void					setScriptVerbose(const std::string &scriptName, bool verbosity=true);

private:

	/// Constructor
	CCommandEventManager();

protected:
	//
	static void					processEvent(const std::string &var, const std::string &value);
};


#endif // NL_COMMAND_EVENT_MANAGER_H

/* End of command_event_manager.h */
