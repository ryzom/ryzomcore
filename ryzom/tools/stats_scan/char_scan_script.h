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

#ifndef CHAR_SCAN_SCRIPT_H
#define CHAR_SCAN_SCRIPT_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "character_scan_job.h"


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CCharScanScript;					// an object that represents a script (one or more cumulated files)
class CCharScanScriptFile;				// an object that represents a script file
class ICharScanScriptCommand;			// virtual base class for objects that represents commands that can be used in script files
class CCharScanScriptCommandRegistry;	// singleton registry for ICharScanScriptCommand objects
template <class C> class CCharScanScriptCommandRegisterer; // template class used to register script commands in the registry


//-------------------------------------------------------------------------------------------------
// class CCharScanScript
//-------------------------------------------------------------------------------------------------

class CCharScanScript
{
public:
	// public interface
	bool addScriptFile(const std::string& fileName);
	void applyToJob(CCharacterScanJob& job);

private:
	// prohibit implicit copy
	CCharScanScript(const CCharScanScript&);

private:
	// private data
	std::vector<CCharScanScriptFile> _ScriptFiles;
};


//-------------------------------------------------------------------------------------------------
// class CCharScanScriptFile
//-------------------------------------------------------------------------------------------------

class CCharScanScriptFile
{
public:
	// public interface
	bool parseFile(const std::string& fileName,	CCharScanScript* container=NULL);
	bool applyToJob(CCharacterScanJob& job);

	const std::string& getFileName() const;
	const std::string& getDescription() const;

public:
	// interface for script commands
	bool setDescription(const std::string& description);
	bool setOutputPath(const std::string& path);
	bool addFilter(const std::string& rawArgs);
	bool addInfoExtractor(const std::string& rawArgs);
	bool addInputFiles(const std::string& rawArgs);

private:
	// private data
	std::string _FileName;
	std::string _Description;
	std::string _OutputPath;
	std::vector<std::string> _Filters;
	std::vector<std::string> _InfoExtractors;
	std::vector<std::string> _InputFiles;
};


//-------------------------------------------------------------------------------------------------
// class ICharScanScriptCommand
//-------------------------------------------------------------------------------------------------

class ICharScanScriptCommand: public NLMISC::CRefCount
{
public:
	virtual ~ICharScanScriptCommand() {}
	virtual const char* getName()=0;
	virtual const char* getSyntax()=0;
	virtual const char* getDescription()=0;
	virtual bool		execute(CCharScanScriptFile& scriptFile,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& rawCmdLine,CCharScanScript* container)=0;
};


//-------------------------------------------------------------------------------------------------
// class CCharScanScriptCommandRegistry
//-------------------------------------------------------------------------------------------------

class CCharScanScriptCommandRegistry
{
public:
	// accessor for the singleton instance
	static CCharScanScriptCommandRegistry* getInstance();

public:
	// register a script command
	void registerScriptCommand(NLMISC::CSmartPtr<ICharScanScriptCommand> scriptCommand);

	// display the set of script commands
	void displayScriptCommands(NLMISC::CLog* log=NLMISC::InfoLog);

	// execute a script command for a given script file object
	bool execute(CCharScanScriptFile& scriptFile,const NLMISC::CSString& commandLine,CCharScanScript* container);
				 
private:
	// this is a singleton so ctor is private
	CCharScanScriptCommandRegistry() {}

private:
	// singleton data
	typedef std::vector<NLMISC::CSmartPtr<ICharScanScriptCommand> > TScriptCommands;
	TScriptCommands _ScriptCommands;
};


//-------------------------------------------------------------------------------------------------
// class CCharScanScriptCommandRegisterer
//-------------------------------------------------------------------------------------------------

template <class C>
class CCharScanScriptCommandRegisterer
{
public:
	CCharScanScriptCommandRegisterer()
	{
		CCharScanScriptCommandRegistry::getInstance()->registerScriptCommand(new C);
	}
};


//-------------------------------------------------------------------------------------------------
// MACRO CHAR_SCAN_SCRIPT_COMMAND()
//-------------------------------------------------------------------------------------------------

#define CHAR_SCAN_SCRIPT_COMMAND(name,syntax,description)\
	class CCharScriptCommand_##name: public ICharScanScriptCommand\
{\
public:\
	virtual const char* getName()			{return #name;}\
	virtual const char* getSyntax()			{return syntax;}\
	virtual const char* getDescription()	{return description;}\
\
	virtual bool		execute(CCharScanScriptFile& scriptFile,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& rawCmdLine,CCharScanScript* container);\
};\
CCharScanScriptCommandRegisterer<CCharScriptCommand_##name> __Registerer_CCharScriptCommand_##name;\
bool CCharScriptCommand_##name::execute(CCharScanScriptFile& scriptFile,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& rawCmdLine,CCharScanScript* container)


//-------------------------------------------------------------------------------------------------
#endif
