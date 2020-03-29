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

/*

  Script format:
	The script is line based
	Empty lines are allowed
	each script line begins with a keyword. The supported keywords are:
		'include' <file_name>							// include another script file here
		'sub' <script_name>								// begin a script that is not executed automatically
		'state' <state_name>							// begin a new state
		'begin'											// begin a script to be executed on 'start of state' for current state
		'end'											// begin a script to be executed on 'end of state' for current state
		'serviceUp' <service_name>						// begin a script to be executed on 'serviceUp' for named service and current state
		'serviceDown' <service_name>					// begin a script to be executed on 'serviceDown' for named service and current state
		'cmd' <command_line>							// add a command to the current script
		'cmd' <key_var>#<key_val>.<target_var>=<val>	// add a 'setVar' command to the current script
		'runScript' <script_name>						// add a 'runScript' command to the current script
		'//' <comment_text>
*/

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"

// game share
#include "game_share/utils.h"

// local
#include "ss_state_manager.h"
#include "ss_script_manager.h"
#include "ss_command_executor.h"


//-------------------------------------------------------------------------------------------------
// methods CScriptManager
//-------------------------------------------------------------------------------------------------

CScriptManager::CScriptManager() 
{
}

CScriptManager* CScriptManager::getInstance()
{
	static CScriptManager* ptr=NULL;
	if (ptr==NULL)
		ptr= new CScriptManager;

	return ptr;
}

// clear the set of loaded scripts from RAM
void CScriptManager::clear()
{
	_Scripts.clear();
}

// read the named script file
bool CScriptManager::readScriptFile(const std::string& fileName)
{
	// a handy container for tracking included files and avoiding recursive includes
	std::set<NLMISC::CSString> scriptFilesRead;
	scriptFilesRead.insert(fileName);

	// read the file from disk
	NLMISC::CSString s;
	s.readFromFile(fileName);
	if (s.empty())
	{
		nlwarning("File not found or file empty: %s",fileName.c_str());
		return false;
	}

	// split the text into lines
	NLMISC::CVectorSString lines;
	s.splitLines(lines);

	// pre-process to deal with comments and 'include's
	for (uint32 i=0;i<lines.size();++i)
	{
		// strip comments and leading and trailing blanks
		lines[i]= lines[i].strip();
		if (lines[i].left(2)=="//")
			lines[i].clear();

		// deal with 'include' lines
		if (lines[i].firstWord()=="include")
		{
			// get hold of the file name and make sure it hasn't been read already
			NLMISC::CSString fn= lines[i].tailFromFirstWord().strip();
			if (scriptFilesRead.find(fn)!=scriptFilesRead.end())
			{
				nlwarning("attempt to include file more than once: %s",fn.c_str());
				return false;
			}
			scriptFilesRead.insert(fn);

			// read the new file
			NLMISC::CSString body;
			body.readFromFile(fn);
			if (body.empty())
			{
				nlwarning("File not found or file empty: %s",fileName.c_str());
				return false;
			}
			// split the new file into lines
			NLMISC::CVectorSString newLines;
			body.splitLines(newLines);
			// remove the 'insert' line and substitute in the new text from the new file
			lines.erase(lines.begin()+i);
			lines.insert(lines.begin()+i,newLines.begin(),newLines.end());
		}
	}

	// check syntax
	for (uint32 i=0;i<lines.size();++i)
	{
		// skip blank lines
		if (lines[i].empty())
			continue;

		// extract the first word and the word count
		NLMISC::CSString keyword= lines[i].firstWord();
		uint32 wordCount= lines[i].countWords();

		// depending on the first word - check the number of paramateres
		if (keyword=="state")				{ BOMB_IF(wordCount!=2,"Syntax error: "+lines[i],return false); }
		else if (keyword=="begin")			{ BOMB_IF(wordCount!=1,"Syntax error: "+lines[i],return false); }
		else if (keyword=="end")			{ BOMB_IF(wordCount!=1,"Syntax error: "+lines[i],return false); }
		else if (keyword=="serviceUp")		{ BOMB_IF(wordCount!=2,"Syntax error: "+lines[i],return false); }
		else if (keyword=="serviceDown")	{ BOMB_IF(wordCount!=2,"Syntax error: "+lines[i],return false); }
		else if (keyword=="sub")			{ BOMB_IF(wordCount!=2,"Syntax error: "+lines[i],return false); }
		else if (keyword=="cmd")			{ BOMB_IF(wordCount<2,"Syntax error: "+lines[i],return false); }
		else if (keyword=="runScript")		{ BOMB_IF(wordCount!=2 && wordCount!=3,"Syntax error: "+lines[i],return false); }
		else								{ BOMB("Syntax error - unrecognised keyword: "+lines[i],return false); }
	}

	// keep track of the active state
	NLMISC::CSString theState;
	CStateManager::getInstance()->clearValidStateList();

	// add entries to scripts
	TScriptLines* theScript=NULL;
	for (uint32 i=0;i<lines.size();++i)
	{
		NLMISC::CSString line= lines[i].strip();
		NLMISC::CSString keyword= line.firstWord();

		// skip blank lines
		if (line.empty())
			continue;

		// see whether we have a script command or a new script name
		if (keyword=="cmd" || keyword=="runScript")
		{
			// make sure there's an active script to add the command to
			BOMB_IF(theScript==NULL,"script commands found but no active script",return false);
			(*theScript).push_back(lines[i].strip());
		}
		else if (keyword=="state")
		{
			theState=lines[i].word(1).strip();
			CStateManager::getInstance()->addValidState(theState);
		}
		else if (keyword=="sub")
		{
			theScript=&_Scripts[lines[i].tailFromFirstWord().strip()];
		}
		else
		{
			// make sure there's an active state to add the command to
			BOMB_IF(theState.empty(),"new script name found but no active state",return false);
			NLMISC::CVectorSString words;
			lines[i].splitWords(words);
			words.insert(words.begin()+1,theState);
			NLMISC::CSString scriptName;
			scriptName.join(words,'_');
			theScript=&_Scripts[scriptName];
		}
	}

	// success!!
	return true;
}

// display the contents of the script manager
void CScriptManager::display() const
{
	for (TScripts::const_iterator script=_Scripts.begin();script!=_Scripts.end();++script)
	{
		nldebug("SS script: %s",(*script).first.c_str());
		for (uint32 i=0;i<(*script).second.size();++i)
		{
			nldebug("SS \t%s",(*script).second[i].c_str());
		}
	}
}

// add a line to a named script
void CScriptManager::addScriptLine(const std::string& scriptName,const std::string& line)
{
	_Scripts[scriptName].push_back(line);
}

// run the named script (if it exists)
void CScriptManager::runScript(const std::string& scriptName,uint32 delay) const
{
//	nldebug("SS RunScript %s, %d",scriptName.c_str(),delay);
	TScripts::const_iterator script= _Scripts.find(scriptName);

	if (script==_Scripts.end())
		return;

	for (uint32 i=0;i<(*script).second.size();++i)
	{
		uint32 thisDelay=delay;
		NLMISC::CSString keyword=(*script).second[i].firstWordConst().strip();
		NLMISC::CSString cmdLine=(*script).second[i].tailFromFirstWord().strip();
		if (cmdLine.firstWord().atoi()!=0 || cmdLine.firstWord().strip()=="0")
		{
			thisDelay+= cmdLine.firstWord().atoi();
			cmdLine= cmdLine.tailFromFirstWord().strip();
		}
		if (keyword=="runScript")
		{
			runScript(cmdLine.c_str(),thisDelay);
		}
		else
		{
			CCommandExecutor::getInstance()->addCommand(NLMISC::toString("%d ",thisDelay)+cmdLine);
//			nldebug("SS Script execute: %s",(NLMISC::toString("%d ",thisDelay)+cmdLine).c_str());
		}
	}
}

