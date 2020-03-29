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

//nel
#include "nel/misc/command.h"

// game share
#include "game_share/utils.h"

// local
#include "ss_state_manager.h"
#include "ss_script_manager.h"
#include "ss_command_executor.h"
#include "ss_service_comms_manager.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// CStateManager Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ShardScript,ssActiveStatesResetAll,"reset the active states","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CStateManager::getInstance()->resetAll();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssActiveStatesBegin,"activate a state","<state_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CStateManager::getInstance()->beginState(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssActiveStatesEnd,"deactivate an active state","<state_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CStateManager::getInstance()->endState(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssActiveStatesDisplay,"display the set of active states","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CStateManager::getInstance()->display();

	return true;
}


//-----------------------------------------------------------------------------
// CScriptManager Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ShardScript,ssScrLoad,"clear current script from ram and read a new script file","<file_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CScriptManager::getInstance()->clear();
	return CScriptManager::getInstance()->readScriptFile(args[0]);
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssScrRoutineRun,"run a routine from the currently loaded script file","<routine_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CScriptManager::getInstance()->runScript(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssScrDisplay,"display the currently loaded script routines","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CScriptManager::getInstance()->display();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssScrAddLine,"display the currently loaded script routines","<script_name><line>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	CScriptManager::getInstance()->addScriptLine(args[0],args[1]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssScrHelp,"display some info on script syntax","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	log.displayNL("%s",	"Script format (for *.shard_script):");
	log.displayNL("\t%s",		"The script is line based");
	log.displayNL("\t%s",		"Empty lines are allowed");
	log.displayNL("\t%s",		"Each script line begins with a keyword. The supported keywords are:");
	log.displayNL("\t\t%-50s%s",	"'include' <file_name>","// include another script file here");
	log.displayNL("\t\t%-50s%s",	"'sub' <script_name>","// begin a script that is not executed automatically");
	log.displayNL("\t\t%-50s%s",	"'state' <state_name>","// begin a new state");
	log.displayNL("\t\t%-50s%s",	"'begin'","// begin a script to be executed on 'start of state' for current state");
	log.displayNL("\t\t%-50s%s",	"'end'","// begin a script to be executed on 'end of state' for current state");
	log.displayNL("\t\t%-50s%s",	"'serviceUp' <service_name>","// begin a script to be executed on 'serviceUp' for named service and current state");
	log.displayNL("\t\t%-50s%s",	"'serviceDown' <service_name>","// begin a script to be executed on 'serviceDown' for named service and current state");
	log.displayNL("\t\t%-50s%s",	"'cmd' <command_line>","// add a command to the current script");
	log.displayNL("\t\t%-50s%s",	"'cmd' <key_var>#<key_val>.<target_var>=<val>","// add a 'setVar' command to the current script");
	log.displayNL("\t\t%-50s%s",	"'runScript' <script_name>","// add a 'runScript' command to the current script");
	log.displayNL("\t\t%-50s%s",	"'//' <comment_text>","// add a comment to the current script");

	return true;
}


//-----------------------------------------------------------------------------
// CCommandExecutor Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ShardScript,ssCmdExecutorDisplay,"list pending commands with remaining times","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CCommandExecutor::getInstance()->display();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssCmdExecutorAdd,"add a command","<ticks_before_execution> <service_name> <command>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	NLMISC::CVectorSString argVector;
	argVector= reinterpret_cast<const vector<CSString> &>(args);
	NLMISC::CSString cmdLine;
	cmdLine.join(argVector,' ');

	CCommandExecutor::getInstance()->addCommand(cmdLine);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardScript,ssCmdExecutorSimTick,"simulate the tick update","[<count=1>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0 && args.size()!=1)
		return false;

	uint32 count=1;
	if (!args.empty())
	{
		count= atoi(args[0].c_str());
		if (NLMISC::toString("%u",count)!=args[0])
		{
			nlwarning("Bad numeric argument for 'count': %s",args[0].c_str());
			return false;
		}
	}

	for (uint32 i=0;i<count;++i)
		CCommandExecutor::getInstance()->tickUpdate();

	return true;
}


//-----------------------------------------------------------------------------
// CCommandExecutor Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ShardScript,ssServiceCommsManagerExecute,"execute a command on a remote service","<service_wildcards><cmdLine>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	CServiceCommsManager::getInstance()->execute(args[0],args[1]);

	return true;
}


NLMISC_CATEGORISED_COMMAND(ShardScript,ssServiceCommsManagerDisplay,"display the state of the comms manager (connected services, etc)","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CServiceCommsManager::getInstance()->display();

	return true;
}


//-----------------------------------------------------------------------------
