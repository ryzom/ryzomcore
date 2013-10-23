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
#ifndef TXT_COMMAND_H
#define TXT_COMMAND_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/sstring.h"


//-------------------------------------------------------------------------------------------------
// MACRO TXT_COMMAND_SET
//-------------------------------------------------------------------------------------------------

#define TXT_COMMAND_SET(setName,CONTEXT_CLASS)\
class __CTxtCommandSet_##setName: public ITxtCommandSet<CONTEXT_CLASS>\
{\
public:\
	static __CTxtCommandSet_##setName* getInstance()\
	{\
		static __CTxtCommandSet_##setName *p=NULL;\
		if (p==NULL) p= new __CTxtCommandSet_##setName;\
		return p;\
	}\
};\
static CTxtCommandSetPtr<__CTxtCommandSet_##setName> setName;


//-------------------------------------------------------------------------------------------------
// MACRO TXT_COMMAND
//-------------------------------------------------------------------------------------------------

#define TXT_COMMAND(cmdName,setName,CONTEXT_CLASS)\
struct __CTxtCommand_##cmdName##CONTEXT_CLASS: public ITxtCommand<CONTEXT_CLASS>\
{\
	static __CTxtCommand_##cmdName##CONTEXT_CLASS* getInstance()\
	{\
		static __CTxtCommand_##cmdName##CONTEXT_CLASS *p=NULL;\
		if (p==NULL) p= new __CTxtCommand_##cmdName##CONTEXT_CLASS;\
		return p;\
	}\
	virtual const char* getName() const {return #cmdName;}\
	virtual CTxtCommandResult execute(CONTEXT_CLASS& context,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& fullCmdLine);\
private:\
	__CTxtCommand_##cmdName##CONTEXT_CLASS() {}\
};\
static ITxtCommandRegisterer<__CTxtCommand_##cmdName##CONTEXT_CLASS,__CTxtCommandSet_##setName> __CTxtCommand_##cmdName##CONTEXT_CLASS##_Registerer;\
CTxtCommandResult __CTxtCommand_##cmdName##CONTEXT_CLASS::execute(CONTEXT_CLASS& context,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& fullCmdLine)


//-------------------------------------------------------------------------------------------------
// class CTxtCommandResult
//-------------------------------------------------------------------------------------------------

class CTxtCommandResult
{
public:
	enum TType
	{ 
		SUCCESS,			// command execution was successful
		SYNTAX_ERROR,		// there was a syntax error in the command line
		BAD_PERMISSION,		// the user doesn't have the right to run the given command
		UNKNOWN_COMMAND,	// behave as if the command was not recognised
		EXECUTION_ERROR		// there was an error during execution of the command
	};
	CTxtCommandResult(const bool& success): _Type(success?SUCCESS:SYNTAX_ERROR) {}
	CTxtCommandResult(const TType& type): _Type(type) {}
	CTxtCommandResult(const TType& type,const NLMISC::CSString& reason): _Type(type), _Reason(reason) {}
	TType getType() const { return _Type; }
	const NLMISC::CSString& getReason() const { return _Reason; }
private:
	TType _Type;
	NLMISC::CSString _Reason;
};


//-------------------------------------------------------------------------------------------------
// class ITxtCommand
//-------------------------------------------------------------------------------------------------

template <class CONTEXT_CLASS> class ITxtCommand
{
public:
	virtual const char* getName() const =0;
	virtual CTxtCommandResult execute(CONTEXT_CLASS& context,const NLMISC::CVectorSString& args,const NLMISC::CSString& rawArgs,const NLMISC::CSString& fullCmdLine) =0;
};


//-------------------------------------------------------------------------------------------------
// class ITxtCommandRegisterer
//-------------------------------------------------------------------------------------------------

template <class CMD,class SET> struct ITxtCommandRegisterer
{
	ITxtCommandRegisterer()
	{
		SET::getInstance()->registerTxtCommand(CMD::getInstance());
	}
};


//-------------------------------------------------------------------------------------------------
// class ITxtCommandSet
//-------------------------------------------------------------------------------------------------

template <class CONTEXT_CLASS> class ITxtCommandSet
{
public:
	void registerTxtCommand(ITxtCommand<CONTEXT_CLASS>* txtCommand)
	{
		nlassert(_TxtCommands.find(txtCommand->getName())==_TxtCommands.end());
		_TxtCommands[txtCommand->getName()]= txtCommand;
	}
	CTxtCommandResult execute(CONTEXT_CLASS& context,const NLMISC::CSString& cmdLine)
	{
		NLMISC::CSString cmdTail=cmdLine;
		NLMISC::CSString keyword=cmdTail.firstWord(true);
		typename TTxtCommands::iterator it= _TxtCommands.find(keyword);
		if (it==_TxtCommands.end()) return CTxtCommandResult::UNKNOWN_COMMAND;
		NLMISC::CVectorSString args;
		cmdTail.splitWords(args);
		return it->second->execute(context,args,cmdTail,cmdLine);
	}
private:
	typedef ITxtCommand<CONTEXT_CLASS> TTxtCommand;
	typedef std::map<NLMISC::CSString,TTxtCommand*> TTxtCommands;
	TTxtCommands _TxtCommands;
};


//-------------------------------------------------------------------------------------------------
// class ITxtCommandRegisterer
//-------------------------------------------------------------------------------------------------

template <class SET> struct CTxtCommandSetPtr
{
	CTxtCommandSetPtr()
	{
		SET::getInstance();
	}

	SET& operator*()
	{
		return *SET::getInstance();
	}

	SET* operator->()
	{
		return SET::getInstance();
	}
};


//-------------------------------------------------------------------------------------------------
#endif
