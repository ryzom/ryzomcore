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
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/singleton.h"
#include "nel/misc/command.h"
#include "nel/misc/file.h"

// game share
#include "game_share/utils.h"

// local
#include "patchman_tester.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{
	//-----------------------------------------------------------------------------
	// class CVariableSet
	//-----------------------------------------------------------------------------

	class CVariableSet
	{
	public:
		// clear out the set of variables
		void clear();

		// set the value of a variable
		void set(const CSString& varName,const CSString& value);

		// get the value of a variable
		const CSString& get(const CSString& varName) const;

		// expand the variable names in a string out to their values (may do simple expression eval later too)
		// variable names are assumed to be of the form: '$'<var_name>  eg: "$MYVAR"
		CSString expand(CSString inputString) const;

		// dump the variable set to a given log
		void dump(CLog& log) const;

	private:
		typedef map<CSString,CSString> TVariables;
		TVariables _Variables;
	};


	//-----------------------------------------------------------------------------
	// class CScriptLine
	//-----------------------------------------------------------------------------

	class CScriptLine
	{
	public:
		// ctors
		CScriptLine();
		CScriptLine(const CSString& context, const CSString& keyword, const CSString& args);

		// accessors
		bool isValid() const;
		CSString toString() const;

		// execution - returns false if script should abort, otherwise true
		// if there is an error then the error message will be not empty on return
		bool execute(CVariableSet& vars, CSString& errorMsg) const;

	private:
		enum TActions
		{
			// the invalid operation 'no op'
			ACT_NOP,
			// conditional - stop execution of this routine with 'success' return value if condition fails
			ACT_ONLYIF, ACT_ONLYIF_NOT,
			// assert
			ACT_ASSERT, ACT_ASSERT_NOT,
			// set, inc or dec a variable
			ACT_SET, ACT_INC, ACT_DEC,
			// display a debug, info or warning
			ACT_DEBUG, ACT_INFO, ACT_WARN,
			// execute an NLMISC command
			ACT_CMD,
		};

		CSString _RawCmdLine;
		CSString _Context;

		TActions _Action;
		CVectorSString _Args;
	};


	//-----------------------------------------------------------------------------
	// class CScriptRoutine
	//-----------------------------------------------------------------------------

	class CScriptRoutine
	{
	public:
		// ctor
		CScriptRoutine(const CSString& name=CSString());

		// building up the script routine from input...
		// returns true if line is valid, otherwise false
		bool addLine(const CSString& context, const CSString& keyword,const CSString& args);

		// execute the script
		// return true if execution went OK, false if errors encountered / script asserts triggered
		bool execute(CVariableSet& vars) const;

		// accessors
		bool isEmpty() const;
		CSString getName() const;


	private:
		// the script name (the name of the event that triggers it)
		CSString _Name;

		// the script lines
		typedef vector<CScriptLine> TLines;
		TLines _Lines;
	};


	//-----------------------------------------------------------------------------
	// class CPatchmanTesterImplementation
	//-----------------------------------------------------------------------------

	class CPatchmanTesterImplementation: public CPatchmanTester, public CRefCount
	{
	public:
		// clear everything
		void clear();

		// clear loaded script(s) but leave state intact
		void clearScript();

		// clear state variables but leave scripts intact
		void clearState();

		// load a script file
		void loadScript(const NLMISC::CSString& fileName);

		// set a state variable
		void set(const NLMISC::CSString& variableName,const NLMISC::CSString& value);
		void set(const NLMISC::CSString& variableName,sint32 value);

		// trigger an event
		void trigger(const NLMISC::CSString& eventName);

		// debug routine - display internal state info
		void dump(NLMISC::CLog& log);

		// debugging routine - displays the syntax help for the patchtest script files
		void help(NLMISC::CLog& log);

	private:
		// private methods
		typedef std::set<NLMISC::CSString> TFileNameSet;
		void _readScriptFile(const NLMISC::CSString& fileName,uint32& errors,TFileNameSet& fileNameSet);

	private:
		// state variables (map of var name to value)
		CVariableSet _VariableSet;

		// scripts (vector of routines)
		typedef vector<CScriptRoutine> TScripts;
		TScripts _Scripts;

	};

	//-----------------------------------------------------------------------------
	// methods CVariableSet
	//-----------------------------------------------------------------------------

	void CVariableSet::clear()
	{
		_Variables.clear();
	}

	void CVariableSet::set(const CSString& varName,const CSString& value)
	{
		if (value.empty())
		{
			_Variables.erase(varName);
			return;
		}

		_Variables[varName]= value;
	}

	const CSString& CVariableSet::get(const CSString& varName) const
	{
		// setup an empty string to use if the requestsed variable doesn't exist
		static CSString emptyString;

		// lookup the requested variable in the variable map
		TVariables::const_iterator it= _Variables.find(varName);

		// if the variable was found then return its value else return the empty string
		return (it== _Variables.end())? emptyString: it->second;
	}

	CSString CVariableSet::expand(CSString inputString) const
	{
		// setup a result buffer
		CSString result;

		while (!inputString.empty())
		{
			// add all text up to the next variable name to our result buffer
			result+= inputString.splitTo('$',true,false);

			// if we've just absorbed the entire remainder of the input then drop out of the while loop
			if (inputString.empty()) break;

			// skip the '$' sign
			inputString= inputString.leftCrop(1);

			// extract the variable name
			CSString varName= inputString.splitToOneOfSeparators(" \t:=$\"\'`&|~^+-*/()<>[]{};,.?!",true,false,false,false,false);

			// if a variable name was found...
			if (!varName.empty())
			{
				// add the value of the variable to the result
				result+= get(varName);
			}
			else
			{
				// no variable name found so keep the '$' intact in the result expression (to help debugging)
				result+='$';
			}
		}

		return result;
	}

	void CVariableSet::dump(CLog& log) const
	{

		// iterate over the variables to determine the length of the longest variable name
		uint32 maxlen=0;
		for (TVariables::const_iterator it= _Variables.begin(); it!=_Variables.end(); ++it)
		{
			maxlen=max(maxlen,(uint32)it->first.size());
		}

		// display the variables
		for (TVariables::const_iterator it= _Variables.begin(); it!=_Variables.end(); ++it)
		{
			log.displayNL("%*s = %s",maxlen,it->first.c_str(),it->second.c_str());
		}
	}


	//-----------------------------------------------------------------------------
	// methods CScriptLine
	//-----------------------------------------------------------------------------

	CScriptLine::CScriptLine(): _Action(ACT_NOP)
	{
	}

	CScriptLine::CScriptLine(const CSString& context, const CSString& keyword, const CSString& args): _Action(ACT_NOP), _Context(context), _RawCmdLine(keyword+" "+args)
	{
		if		(keyword=="assert")
		{
			CSString argTxt= args;
			CSString varName= argTxt.firstWord(true).strip();
			CSString op= argTxt.firstWord(true).strip();	// should be a '!' or a '='
					 op+= argTxt.firstWord(true).strip();	// should be a '='
			DROP_IF(op!="==" && op!="!=",_Context+": Missing '==' or '!=' in line: "+keyword+" "+args,return);
			_Args.push_back(varName);
			_Args.push_back(argTxt.strip());
			_Action= (op=="==")? ACT_ASSERT: ACT_ASSERT_NOT;
		}
		else if (keyword=="onlyif")
		{
			CSString argTxt= args;
			CSString varName= argTxt.firstWord(true).strip();
			CSString op= argTxt.firstWord(true).strip();	// should be a '!' or a '='
					 op+= argTxt.firstWord(true).strip();	// should be a '='
			DROP_IF(op!="==" && op!="!=",_Context+": Missing '==' or '!=' in line: "+keyword+" "+args,return);
			_Args.push_back(varName);
			_Args.push_back(argTxt.strip());
			_Action= (op=="==")? ACT_ONLYIF: ACT_ONLYIF_NOT;
		}
		else if (keyword=="set")
		{
			CSString argTxt= args;
			CSString varName= argTxt.firstWord(true).strip();
			CSString op= argTxt.firstWord(true).strip();
			DROP_IF(op!="=",_Context+": Missing '=' in line: "+keyword+" "+args,return);
			_Args.push_back(varName);
			_Args.push_back(argTxt.strip());
			_Action= ACT_SET;
		}
		else if (keyword=="inc")		{	_Args.push_back(args);	_Action= ACT_INC;	}
		else if (keyword=="dec")		{	_Args.push_back(args);	_Action= ACT_DEC;	}
		else if (keyword=="debug")		{	_Args.push_back(args);	_Action= ACT_DEBUG;	}
		else if (keyword=="info")		{	_Args.push_back(args);	_Action= ACT_INFO;	}
		else if (keyword=="warn")		{	_Args.push_back(args);	_Action= ACT_WARN;	}
		else if (keyword=="cmd")		{	_Args.push_back(args);	_Action= ACT_CMD;	}
		else
		{
			DROP(context+": Unable to build script line from input: "+keyword+" "+args,return);
		}
	}

	bool CScriptLine::isValid() const
	{
		return (_Action!=ACT_NOP);
	}

	CSString CScriptLine::toString() const
	{
		return _RawCmdLine;
	}

	bool CScriptLine::execute(CVariableSet& vars, CSString& errorMsg) const
	{
		// start by clearing out the error message, to avoid confusion
		errorMsg.clear();

		switch (_Action)
		{
		case ACT_NOP:
			BOMB("Trying to execute bad script command: "+_RawCmdLine,return false);

		case ACT_ONLYIF:
			nlassert(_Args.size()==2);
			if (vars.get(_Args[0]) != _Args[1])
			{
				return false;
			}
			break;

		case ACT_ONLYIF_NOT:
			nlassert(_Args.size()==2);
			if (vars.get(_Args[0]) == _Args[1])
			{
				return false;
			}
			break;

		case ACT_ASSERT:
			nlassert(_Args.size()==2);
			if (vars.get(_Args[0]) != _Args[1])
			{
				errorMsg= _Context+": Assert Failed: "+_RawCmdLine;
				return false;
			}
			break;

		case ACT_ASSERT_NOT:
			nlassert(_Args.size()==2);
			if (vars.get(_Args[0]) == _Args[1])
			{
				errorMsg= _Context+": Assert Failed: "+_RawCmdLine;
				return false;
			}
			break;

		case ACT_SET:
			nlassert(_Args.size()==2);
			vars.set(_Args[0], vars.expand(_Args[1]));
			break;

		case ACT_INC:
			nlassert(_Args.size()==1);
			vars.set(_Args[0], NLMISC::toString("%i",vars.get(_Args[0]).atosi()+1));
			break;

		case ACT_DEC:
			nlassert(_Args.size()==1);
			vars.set(_Args[0], NLMISC::toString("%i",vars.get(_Args[0]).atosi()-1));
			break;

		case ACT_DEBUG:
			nlassert(_Args.size()==1);
			nldebug("%s",vars.expand(_Args[0]).c_str());
			break;

		case ACT_INFO:
			nlassert(_Args.size()==1);
			nlinfo("%s",vars.expand(_Args[0]).c_str());
			break;

		case ACT_WARN:
			nlassert(_Args.size()==1);
			nlwarning("%s",vars.expand(_Args[0]).c_str());
			break;

		case ACT_CMD:
			nlassert(_Args.size()==1);
			NLMISC::ICommand::execute(vars.expand(_Args[0]),*NLMISC::InfoLog);
			break;
		}

		// all went well so return true ... the script can continue
		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CScriptRoutine
	//-----------------------------------------------------------------------------

	CScriptRoutine::CScriptRoutine(const CSString& name): _Name(name)
	{
	}

	bool CScriptRoutine::addLine(const CSString& context, const CSString& keyword,const CSString& args)
	{
		// try to build a new script line from the supplied text...
		CScriptLine theNewLine(context,keyword,args);

		// if we failed to create a valid line from the given text then just return false
		// there should already have been an explication of the problem in the CScriptLine ctor
		if (!theNewLine.isValid())
		{
			return false;
		}

		// the line is valid so append it to our _Lines vector
		_Lines.push_back(theNewLine);
		return true;
	}

	bool CScriptRoutine::execute(CVariableSet& vars) const
	{
		for (uint32 i=0; i<_Lines.size(); ++i)
		{
			// sertup a variable to hold an error message in case of failed assert, etc
			CSString errorMsg;

			// execute the line
			bool ok= _Lines[i].execute(vars,errorMsg);

			// if the execution returned true then continue to the next line
			if (ok) continue;

			// the execute returned false so see if we have an error...
			if (!errorMsg.empty())
			{
				nlwarning("Test script failed: %s",errorMsg.c_str());
			}

			// return success or fail depending on whether we have an error message...
			return errorMsg.empty();
		}

		// the script executed successfully through to the end
		return true;
	}

	bool CScriptRoutine::isEmpty() const
	{
		return _Lines.empty();
	}

	CSString CScriptRoutine::getName() const
	{
		return _Name;
	}


	//-----------------------------------------------------------------------------
	// methods CPatchmanTesterImplementation
	//-----------------------------------------------------------------------------

	void CPatchmanTesterImplementation::clear()
	{
		clearScript();
		clearState();
	}

	void CPatchmanTesterImplementation::clearScript()
	{
		_Scripts.clear();
	}

	void CPatchmanTesterImplementation::clearState()
	{
		_VariableSet.clear();
	}

	void CPatchmanTesterImplementation::loadScript(const NLMISC::CSString& fileName)
	{
		// setup variables used by recursive script file reader
		uint32 errors=0;
		TFileNameSet fileNameSet;

		// try reading ht e script file
		_readScriptFile(fileName,errors,fileNameSet);

		// if there were errors then abort the read...
		if (errors!=0)
		{
			nlwarning("Script parse failed: %u errors encountered",errors);
			clear();
		}
	}

	void CPatchmanTesterImplementation::_readScriptFile(const NLMISC::CSString& fileName,uint32& errors,CPatchmanTesterImplementation::TFileNameSet& fileNameSet)
	{
		// read in the src file
		NLMISC::CSString fileContents;
		fileContents.readFromFile(fileName);
		DROP_IF(fileContents.empty(),"File not found: "+fileName, ++errors;return);

		// split the file into lines
		NLMISC::CVectorSString lines;
		fileContents.splitLines(lines);

		// process the lines one by one
		for (uint32 i=0;i<lines.size();++i)
		{
			// setup a context string to pre-pend to error messages
			NLMISC::CSString context= NLMISC::toString("%s:%u: ",fileName.c_str(),i+1);

			// remove comments and encapsulating blanks
			NLMISC::CSString line= lines[i].splitTo("//").strip();
			if (line.empty()) continue;

			// split the line into keyword and args
			NLMISC::CSString args= line;
			NLMISC::CSString keyword= args.strtok(" \t");

			// try to treat the keyword
			if (keyword=="include")
			{
				DROP_IF(args.empty(),context+"No file name found following 'include': "+line, ++errors;continue);
				DROP_IF(fileNameSet.find(args)!=fileNameSet.end(),context+"Warning: Duplicate 'include' block ignored: "+line, continue);
				fileNameSet.insert(args);
				_readScriptFile(args.unquoteIfQuoted(),errors,fileNameSet);
			}
			else if (keyword=="on")
			{
				DROP_IF(args.empty(),context+"No event name found following 'on': "+line, ++errors;continue);
				DROP_IF(args.countWords()!=1,context+"Invalid event name found following 'on': "+line, ++errors;continue);
				// create a new script and append it to our vector
				_Scripts.push_back(CScriptRoutine(args));
			}
			else
			{
				DROP_IF(_Scripts.empty(),context+"Expecting 'on <event_name>' but found: "+line, ++errors;continue);
				bool ok= _Scripts.back().addLine(context,keyword,args);
				if (!ok) ++errors;
			}
		}
	}

	void CPatchmanTesterImplementation::set(const NLMISC::CSString& variableName,const NLMISC::CSString& value)
	{
		_VariableSet.set(variableName,value);
		trigger(variableName);
	}

	void CPatchmanTesterImplementation::set(const NLMISC::CSString& variableName,sint32 value)
	{
		set(variableName,NLMISC::toString("%i",value));
	}

	void CPatchmanTesterImplementation::trigger(const NLMISC::CSString& eventName)
	{
		for (uint32 i=0;i <_Scripts.size(); ++i)
		{
			if (_Scripts[i].getName()==eventName)
			{
				_Scripts[i].execute(_VariableSet);
			}
		}
	}

	void CPatchmanTesterImplementation::dump(NLMISC::CLog& log)
	{
		log.displayNL("---------------------------------------------");
		log.displayNL("dump of patchman's patchtest state variables");
		log.displayNL("---------------------------------------------");
		_VariableSet.dump(log);
		log.displayNL("---------------------------------------------");
		log.displayNL("list of triggerable scripts");
		log.displayNL("---------------------------------------------");

		// build a set of script triggers to eliminate duplicates
		typedef std::set<CSString> TTheSet;
		TTheSet theSet;
		for (uint32 i=0;i<_Scripts.size();++i)
		{
			theSet.insert(_Scripts[i].getName());
		}

		// run through the set we just built, displaying scrip names
		for (TTheSet::iterator it= theSet.begin(); it!=theSet.end(); ++it)
		{
			log.displayNL("on %s",it->c_str());
		}

		log.displayNL("---------------------------------------------");
	}

	void CPatchmanTesterImplementation::help(NLMISC::CLog& log)
	{
		log.displayNL("---------------------------------------------");
		log.displayNL("patchman's patchtest script");
		log.displayNL("---------------------------------------------");
		log.displayNL("include <file name>");
		log.displayNL("on <event name>|<variable name>");
		log.displayNL("    cmd <nlmisc command line>");
		log.displayNL("    assert <variable> '=='|'!=' <value>|<txt>");
		log.displayNL("    onlyif <variable> '=='|'!=' <value>|<txt>");
		log.displayNL("    set <variable> '=' <value>|<txt>");
		log.displayNL("    inc <variable>");
		log.displayNL("    dec <variable>");
		log.displayNL("    warn <variable>");
		log.displayNL("    info <variable>");
		log.displayNL("    debug <variable>");
		log.displayNL("---------------------------------------------");
		log.displayNL("Note: variable name substitution example:");
		log.displayNL("    set toto abc d");
		log.displayNL("    info hello $toto$toto $toto");
		log.displayNL("=>  \"hello abc dabc d abc d\"");
		log.displayNL("---------------------------------------------");
	}


	//-----------------------------------------------------------------------------
	// methods CPatchmanTester
	//-----------------------------------------------------------------------------

	CPatchmanTester& CPatchmanTester::getInstance()
	{
		static CSmartPtr<CPatchmanTesterImplementation> theInstance;
		if (theInstance==NULL)
		{
			theInstance= new CPatchmanTesterImplementation;
		}

		return *theInstance;
	}

} // end of namespace


NLMISC_CATEGORISED_COMMAND(patchman,patchtestClear,"clear out the patchtest singleton","")
{
	if (args.size()!=0)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().clear();
	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,patchtestDump,"dump the state of the patchtest singleton","")
{
	if (args.size()!=0)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().dump(log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,patchtestLoad,"load a patchtest script","<file name>")
{
	if (args.size()!=1)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().loadScript(args[0]);
	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,patchtestHelp,"display a list of keywords for the patch script","")
{
	if (args.size()!=0)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().help(log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,patchtestSet,"set a patchtest variable","<var name> <value>")
{
	if (args.size()!=2)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().set(args[0],args[1]);
	return true;
}

NLMISC_CATEGORISED_COMMAND(patchman,patchtestTrigger,"trigger a patchtest event","<event name>")
{
	if (args.size()!=1)
		return false;

	PATCHMAN::CPatchmanTester::getInstance().trigger(args[0]);
	return true;
}



