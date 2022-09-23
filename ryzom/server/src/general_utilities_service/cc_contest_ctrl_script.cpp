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

#include "nel/misc/time_nl.h"
#include "nel/misc/i18n.h"

#include "game_share/utils.h"

#include "gus_module_factory.h"
#include "gus_module_manager.h"
#include "gus_net.h"
#include "gus_net_remote_module.h"
#include "cc_contest_ctrl_script.h"
#include "cc_module_messages.h"
#include "ce_module_messages.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// forward class declarations
//-----------------------------------------------------------------------------

class CContestCtrlScriptImplementation;
class CScriptLineAnswer;


//-----------------------------------------------------------------------------
// class IScriptLine
//-----------------------------------------------------------------------------

class IScriptLine: public CRefCount
{
public:
	virtual ~IScriptLine() {}
	virtual bool init(const CSString& rawArgs)=0;
	virtual bool executeLocalCode(CContestCtrlScriptImplementation* context)=0;
	virtual bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)=0;
	virtual CSString toString()=0;
};

typedef CSmartPtr<IScriptLine> IScriptLinePtr;


//-----------------------------------------------------------------------------
// class CContestCtrlScriptImplementation
//-----------------------------------------------------------------------------

class CContestCtrlScriptImplementation: public CContestCtrlScript, public IModule
{
public:
	// get hold of the singleton instance
	static CContestCtrlScriptImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CContestCtrlScriptImplementation();
	void clear();

public:
	// GUS::IModule methods
	bool initialiseModule(const NLMISC::CSString& rawArgs);
	void release();
	void serviceUpdate(NLMISC::TTime localTime);
	void receiveModuleMessage(GUSNET::CModuleMessage& msg);
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;
	void moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule);

public:
	// CContestCtrlScript methods
	void registerExecutor(uint32 executorId,const CSString& name);
	void recordWinner(uint32 executorId,const CSString& characterName);
	void load(const CSString& fileName);
	void start();
	void setDelay(uint32 duration);
	void stop();
	void list();
	void display();


public:
	// Methods used during script run time
	void waitAnswer(bool value);
	bool isAllWinnersFound();
	void treatWinners(uint32 executorId);

private:
	// flag set true when the CC module is active, otherwise false
	bool _IsActive;

	// The executor services who make it all happen on the remote shards (map of name to remote service ID)
	struct CExecutor { uint32 Id; std::vector<CSString> Winners; CExecutor(uint32 id=InvalidRemoteModuleId): Id(id) {} };
	typedef std::map<NLMISC::CSString,CExecutor> TExecutors;
	TExecutors _Executors;

	// the script lines
	typedef std::vector<IScriptLinePtr> TScript;
	TScript _Script;

	// the name of the currently loaded script file
	CSString _FileName;

	// the name of the contest that this module manages
	CSString _ContestName;

	// the number of prizes to be won (per shard)
	uint32 _NumPrizes;

	// instruction pointer
	uint32 _NextInstruction;

	// after 'delay' command - time to next instruction
	TTime _NextInstructionTime;

	// bools set when script is running, while waiting for answer and when all winners have been found
	bool _IsRunning;
	bool _WaitingForAnswer;
	bool _AllWinnersFound;
};


//-----------------------------------------------------------------------------
// CScriptLine specialisations
//-----------------------------------------------------------------------------

class CScriptLineTitle: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();

private:
	CSString _Lang;	// the language
	CSString _Txt;	// the text for the title
};

class CScriptLineSay: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();

private:
	CSString _Lang;	// the language
	CSString _Name;	// the name of the talker to display inthe chat window
	CSString _Txt;	// the text to say
};

class CScriptLineWait: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();

private:
	uint32 _Duration;
};

class CScriptLineAnswer: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();

private:
	CSString _Answer;
};

class CScriptLineWaitAnswer: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();

private:
	uint32 _Duration;
};

class CScriptLineDisplayWinners: public IScriptLine
{
public:
	bool init(const CSString& rawArgs);
	bool executeLocalCode(CContestCtrlScriptImplementation* context);
	bool executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context);
	CSString toString();
};


//-----------------------------------------------------------------------------
// templat routine buildScriptLine for building IScriptLine objects
//-----------------------------------------------------------------------------

template <class C> IScriptLinePtr buildScriptLine(const CSString& rawArgs,C*)
{
	// instantiate the new objects
	IScriptLinePtr ptr= new C;

	// call its init and make sur its happy
	bool isOK= ptr->init(rawArgs);

	// return the new object or NULL if the init failed
	return isOK? ptr: NULL;
}


//-----------------------------------------------------------------------------
// methods CScriptLineTitle
//-----------------------------------------------------------------------------

bool CScriptLineTitle::init(const CSString& rawArgs)
{
	CSString txtTail= rawArgs;
	CSString lang= txtTail.firstWordOrWords(true);

	if (!lang.isValidKeyword() || txtTail.empty())
	{
		nlwarning("Syntax error: TITLE <lang> <text>: title %s",rawArgs.c_str());
		return false;
	}

	if (lang!="de" && lang!="fr" && lang!="en")
	{
		nlwarning("Suspect language: %s",lang.c_str());
	}

	_Lang= lang;
	_Txt= txtTail;
	return true;
}

bool CScriptLineTitle::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: TITLE: %s: %s",_Lang.c_str(),_Txt.quote().c_str());
	return true;
}

bool CScriptLineTitle::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	sendModuleMessage(CMsgCCTitle(_Lang,_Txt),executorModuleId,context);
	TRemoteModulePtr executorModule= CGusNet::getInstance()->lookupRemoteModule(executorModuleId);
	nlassert(executorModule!=NULL);
	nldebug("CC Forwarding command to CE %s: Title: In Language '%s': %s",executorModule->getParameters().c_str(),_Lang.c_str(),_Txt.c_str());
	return true;
}

CSString CScriptLineTitle::toString()
{
	return "TITLE "+_Lang+" "+_Txt;
}


//-----------------------------------------------------------------------------
// methods CScriptLineSay
//-----------------------------------------------------------------------------

bool CScriptLineSay::init(const CSString& rawArgs)
{
	CSString txtTail= rawArgs;
	CSString lang= txtTail.firstWordOrWords(true);
	CSString name= txtTail.firstWordOrWords(true);

	if (!lang.isValidKeyword() 
//		|| !name.replace(" ","_").replace("'","_").replace("-","_").isValidKeyword() 
		|| txtTail.empty())
	{
		nlwarning("Syntax error: SAY <lang> <npc_name> <text>: say %s",rawArgs.c_str());
		return false;
	}

	if (lang!="de" && lang!="fr" && lang!="en")
	{
		nlwarning("Suspect language: %s",lang.c_str());
	}

	_Lang= lang;
	_Name= name;
	_Txt= txtTail;
	return true;
}

bool CScriptLineSay::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: SAY: %s: %s: %s",_Lang.c_str(),_Name.quote().c_str(),_Txt.quote().c_str());
	return true;
}

bool CScriptLineSay::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	sendModuleMessage(CMsgCCText(_Lang,_Name,_Txt),executorModuleId,context);
	TRemoteModulePtr executorModule= CGusNet::getInstance()->lookupRemoteModule(executorModuleId);
	nlassert(executorModule!=NULL);
	nldebug("CC Forwarding command to CE %s: Say: In Language '%s', Bot '%s': %s",executorModule->getParameters().c_str(),_Lang.c_str(),_Name.c_str(),_Txt.c_str());
	return true;
}

CSString CScriptLineSay::toString()
{
	return "SAY "+_Lang+" "+_Name.quoteIfNotAtomic()+" "+_Txt;
}


//-----------------------------------------------------------------------------
// methods CScriptLineWait
//-----------------------------------------------------------------------------

bool CScriptLineWait::init(const CSString& rawArgs)
{
	_Duration= 1000* rawArgs.atoi();
	return NLMISC::toString(_Duration/1000)==rawArgs;
}

bool CScriptLineWait::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: WAIT (%d)",_Duration/1000);
	context->setDelay(_Duration);
	return true;
}

bool CScriptLineWait::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	return true;
}

CSString CScriptLineWait::toString()
{
	return NLMISC::toString("WAIT %u",_Duration/1000);
}


//-----------------------------------------------------------------------------
// methods CScriptLineAnswer
//-----------------------------------------------------------------------------

bool CScriptLineAnswer::init(const CSString& rawArgs)
{
	_Answer= rawArgs;
	return true;
}

bool CScriptLineAnswer::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: ANSWER <hidden>: %d words",_Answer.countWords());
	return true;
}

bool CScriptLineAnswer::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	sendModuleMessage(CMsgCCAnswer(_Answer),executorModuleId,context);
	return true;
}

CSString CScriptLineAnswer::toString()
{
	return NLMISC::toString("ANSWER <hidden>");
}


//-----------------------------------------------------------------------------
// methods CScriptLineWaitAnswer
//-----------------------------------------------------------------------------

bool CScriptLineWaitAnswer::init(const CSString& rawArgs)
{
	_Duration= 1000* rawArgs.atoi();
	return NLMISC::toString(_Duration/1000)==rawArgs;
}

bool CScriptLineWaitAnswer::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: WAIT_ANSWER (%d)",_Duration/1000);
	DROP_IF(context->isAllWinnersFound(),"Ignoring 'wait_answer' because all winners already found",return true); 
	context->setDelay(_Duration);
	context->waitAnswer(true);
	return true;
}

bool CScriptLineWaitAnswer::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	return true;
}

CSString CScriptLineWaitAnswer::toString()
{
	return NLMISC::toString("WAIT_ANSWER %u",_Duration/1000);
}


//-----------------------------------------------------------------------------
// methods CScriptLineDisplayWinners
//-----------------------------------------------------------------------------

bool CScriptLineDisplayWinners::init(const CSString& rawArgs)
{
	return true;
}

bool CScriptLineDisplayWinners::executeLocalCode(CContestCtrlScriptImplementation* context)
{
	DebugLog->displayNL("CC command: DISPLAY_WINNERS");
	return true;
}

bool CScriptLineDisplayWinners::executeRemoteCode(uint32 executorModuleId,CContestCtrlScriptImplementation* context)
{
	context->treatWinners(executorModuleId);

	return true;
}

CSString CScriptLineDisplayWinners::toString()
{
	return "DISPLAY_WINNERS";
}


//-----------------------------------------------------------------------------
// methods CContestCtrlScriptImplementation ctor
//-----------------------------------------------------------------------------

CContestCtrlScriptImplementation* CContestCtrlScriptImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CContestCtrlScriptImplementation> ptr=NULL;
	if (ptr==NULL)
	{
		ptr= new CContestCtrlScriptImplementation;
	}
	return ptr;
}

CContestCtrlScriptImplementation::CContestCtrlScriptImplementation()
{
	clear();
}

void CContestCtrlScriptImplementation::clear()
{
	_IsActive= false;
	_Executors.clear();
	_Script.clear();
	_FileName.clear();
	_ContestName.clear();
	_NumPrizes=0;
	_NextInstruction=~0u;
	_NextInstructionTime=~0u;
	_IsRunning=false;
	_WaitingForAnswer=false;
	_AllWinnersFound=false;
}


//-----------------------------------------------------------------------------
// methods CContestCtrlScriptImplementation / CContestCtrlScript
//-----------------------------------------------------------------------------

bool CContestCtrlScriptImplementation::initialiseModule(const NLMISC::CSString& rawArgs)
{
	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be one CC module activated at a time",return false);

	// process the command line arguments
	_NumPrizes= rawArgs.wordOrWords(1).strip().atoi();
	DROP_IF(rawArgs.countWordOrWords()!=2||_NumPrizes==0,"Invalid arguments for CC module - expected <name> <num_prizes> found: "+rawArgs,return false);
	_ContestName= rawArgs.wordOrWords(0).strip();

	// done (success)
	_IsActive= true;
	return true;
}

void CContestCtrlScriptImplementation::release()
{
	// mark the module as inactive so that it can be re-instantiated later
	clear();
}

void CContestCtrlScriptImplementation::serviceUpdate(NLMISC::TTime localTime)
{
	if (!_IsRunning)
		return;

	// see whether all of the winners have been found (or whether we've run out of script to run)
	if (_WaitingForAnswer)
	{
		_AllWinnersFound= true;

		// run through the executors to see if any still need more winners
		for (TExecutors::iterator it=_Executors.begin();it!=_Executors.end();++it)
		{
			if ((*it).second.Winners.size()<_NumPrizes && ((_NextInstruction<_Script.size())||(localTime-_NextInstructionTime<0)))
			{
				_AllWinnersFound= false;
				break;
			}
		}
		if (_AllWinnersFound)
		{
			_WaitingForAnswer= false;
			_NextInstructionTime= 0;
		}
	}

	// run any remaining script
	while (_NextInstruction<_Script.size())
	{
		// deal with delay to next instruction (this may be a timeout if '_WaitingForAnswer' is true)
		if (localTime-_NextInstructionTime<0)
			return;

		// deal with timeout while waiting for the correct answer
		_WaitingForAnswer= false;

		// execute the next instruction
		_Script[_NextInstruction]->executeLocalCode(this);
		for (TExecutors::iterator it= _Executors.begin(); it!=_Executors.end();++it)
		{
			_Script[_NextInstruction]->executeRemoteCode((*it).second.Id,this);
		}

		++_NextInstruction;
	}

	// if we're at the end then terminate...
	if(localTime-_NextInstructionTime>=0)
	{
		stop();
	}
}

void CContestCtrlScriptImplementation::receiveModuleMessage(GUSNET::CModuleMessage& msg)
{
	// by definition the module shouldn't receive messages if it isn't active!
	nlassert(_IsActive);

	CSString msgName= msg.getMessageName();
	nlinfo("CC - Received message: %s",msgName.c_str());

	if (msgName==CMsgCERegister::getName())	{ registerExecutor(msg.getSenderId(),CMsgCERegister(msg.getMsgBody()).getTxt()); }
	else if (msgName==CMsgCEWinner::getName())	{ recordWinner(msg.getSenderId(),CMsgCEWinner(msg.getMsgBody()).getWinnerName()); }
}

NLMISC::CSString CContestCtrlScriptImplementation::getState() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	CSString result;
	result+= getName();
	result+= ' ';
	result+= getParameters();
	result+= ':';
	result+= (_FileName.empty())? " No script file loaded": (" ScriptFile: "+_FileName);
	result+= (!_IsRunning)?" - Not running":NLMISC::toString(" - Next Instruction: %d/%d  Time: %d",
			_NextInstruction,_Script.size(),uint32(_NextInstructionTime-CTime::getLocalTime())/1000);
	result+= _WaitingForAnswer?" - Waiting for answer":"";

	return result;
}

NLMISC::CSString CContestCtrlScriptImplementation::getName() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return "CC";
}

NLMISC::CSString CContestCtrlScriptImplementation::getParameters() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return NLMISC::toString("%s %d",_ContestName.quoteIfNotAtomic().c_str(),_NumPrizes);
}

void CContestCtrlScriptImplementation::displayModule() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	// if not running then just display the file name
	if (!_IsRunning)
	{
		InfoLog->displayNL("MODULE CC: Contest Script: %s Not running",_FileName.empty()?"\"\"":_FileName.quoteIfNotAtomic().c_str());
	}
	else
	{
		// calculate delay to next instruction
		sint64 delta= (sint64)(_NextInstructionTime-CTime::getLocalTime());
		if (delta<0)
			delta=0;

		// display info
		InfoLog->displayNL("MODULE CC: Script: %s  Prizes: %d, Line: %u, Waiting: %u %s",
			_FileName.empty()?"\"\"":_FileName.quoteIfNotAtomic().c_str(), _NumPrizes,
			_NextInstruction,(sint32)delta/1000,_WaitingForAnswer?"Waiting for answer":"");
	}

	// display the executors
	nlinfo("There are %u connected executor :", _Executors.size());
	for (TExecutors::const_iterator it= _Executors.begin(); it!=_Executors.end();++it)
	{
		nlinfo("- Executor: %d: Lang:%s, Correct Answers: %d",
			it->second.Id,
			it->first.c_str(),
			it->second.Winners.size());
	}
}

void CContestCtrlScriptImplementation::moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	if (remoteModule->getName()=="CE")
	{
		// inform all 'CE's of existence of this competition
		sendModuleMessage(CMsgCCBegin(_ContestName),remoteModule->getUniqueId(),this);

		nldebug("CC - Informing competition executor of our existence: CE %s",remoteModule->getParameters().c_str());
	}
}

void CContestCtrlScriptImplementation::moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	for (TExecutors::iterator it= _Executors.begin(); it!=_Executors.end();++it)
	{
		if ((*it).second.Id==remoteModule->getUniqueId())
		{
			nlinfo("CC - Removing executor: %d: CE %s",remoteModule->getUniqueId(),remoteModule->getParameters().c_str());
			_Executors.erase(it);
			return;
		}
	}
}


//-----------------------------------------------------------------------------
// methods CContestCtrlScriptImplementation / CContestCtrlScript
//-----------------------------------------------------------------------------

void CContestCtrlScriptImplementation::registerExecutor(uint32 executorId,const CSString& name)
{
	WARN_IF(_Executors.find(name)!=_Executors.end(),"Replacing previous CE mapping for '"+name+"' with: "+toString(executorId));
	_Executors[name]= CExecutor(executorId);
	nlinfo("Registering CE: %s: %d",name.c_str(),executorId);

	if (_IsRunning && _NextInstruction>0)
	{
		nlinfo("Running script lines 0 to %d for CE: %s:",_NextInstruction,name.c_str());
		for (uint32 i=0;i<_NextInstruction;++i)
		{
			_Script[i]->executeRemoteCode(executorId,this);
		}
	}
}

void CContestCtrlScriptImplementation::recordWinner(uint32 executorId,const CSString& characterName)
{
	// locate the executor...
	for (TExecutors::iterator it=_Executors.begin();it!=_Executors.end();++it)
	{
		// if the executor's id matches the id on the message we received...
		if ((*it).second.Id==executorId)
		{
			// add the winner
			(*it).second.Winners.push_back(characterName);

			if ((*it).second.Winners.size()==_NumPrizes)
				sendModuleMessage(CMsgCCEndAnswers(),executorId,this);

			nlinfo("adding winner for CE '%s': %s",(*it).first.c_str(),characterName.c_str());
			return;
		}
	}
	STOP("Received 'winner' callback from unrecognised Executor service");
}

void CContestCtrlScriptImplementation::load(const CSString& fileName)
{
	nldebug("CC Loading contest script file: %s",fileName.c_str());

	// clear out the current script
	_Script.clear();
	_FileName.clear();

	// load the file
	ucstring ucFileBody;
	CI18N::readTextFile(fileName,ucFileBody);
	CSString fileBody=	ucFileBody.toUtf8();
	if (fileBody.empty())
		return;

	// split into lines
	CVectorSString lines;
	fileBody.splitLines(lines);

	// iterate over lines
	bool allOK=true;
	for (uint32 i=0;i<lines.size();++i)
	{
		// ignore blank lines and comments
		CSString line=lines[i].strip();
		if (line.empty() || line.left(2)=="//")
			continue;

		// extract the keyword
		CSString keyword= line.firstWord(true);
		IScriptLinePtr scriptLine;

		// look through valid keywords for a match...
		if (keyword=="TITLE")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineTitle*)NULL);
		}
		else if (keyword=="SAY")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineSay*)NULL);
		}
		else if (keyword=="WAIT")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineWait*)NULL);
		}
		else if (keyword=="ANSWER")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineAnswer*)NULL);
		}
		else if (keyword=="WAIT_ANSWER")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineWaitAnswer*)NULL);
		}
		else if (keyword=="DISPLAY_WINNERS")
		{
			scriptLine= buildScriptLine(line.strip(),(CScriptLineDisplayWinners*)NULL);
		}

		// make sure the script line was built ok
		if (scriptLine==NULL)
		{
			allOK=false;
			nlwarning("Failed to parse script line: %s%s",keyword.c_str(),line.c_str());
			continue;
		}

		// add the new line to the script
		_Script.push_back(scriptLine);
	}
	// if there was an error then clear out the script
	if (!allOK)
	{
		nlwarning("Failed to load context script file: %s",fileName.c_str());
		_Script.clear();
	}

	// record the file name and proclaim success
	_FileName=fileName;
	nldebug("CC Script read success: %s",_FileName.c_str());
}

void CContestCtrlScriptImplementation::start()
{
	_IsRunning=true;
	_NextInstruction=0;
	_NextInstructionTime=0;
	_WaitingForAnswer=false;
	_AllWinnersFound=false;
	for (TExecutors::iterator it=_Executors.begin();it!=_Executors.end();++it)
	{
		(*it).second.Winners.clear();
	}

	nlinfo("CC Start of contest: %s (%d registered executors)",_ContestName.quoteIfNotAtomic().c_str(),_Executors.size());
	WARN_IF(_Executors.empty(),"There are currently no registered executors for this contest");
}

void CContestCtrlScriptImplementation::stop()
{
	_IsRunning=false;
	_NextInstruction=~0u;
	_NextInstructionTime=~0u;
	_WaitingForAnswer=false;

	// inform all of the executors that the contest is over
	for (TExecutors::iterator it=_Executors.begin();it!=_Executors.end();++it)
	{
		// send a message to the executor to tell it to end the contest
		sendModuleMessage(CMsgCCEnd(),(*it).second.Id,this);
	}

	nlinfo("CC End of contest: %s",_ContestName.quoteIfNotAtomic().c_str());
}

void CContestCtrlScriptImplementation::list()
{
	if (_Script.empty())
	{
		nldebug("CC No script currently loaded ... nothing to list");
		return;
	}

	nldebug("CC Listing script: %s",_FileName.c_str());
	for (uint32 i=0;i<_Script.size();++i)
		InfoLog->displayNL("%3d %s",i,_Script[i]->toString().c_str());
}

void CContestCtrlScriptImplementation::display()
{
	if (_IsActive)
	{
		displayModule();
	}
	else
	{
		nlinfo("No CC module instantiated - use addModule CE to instantiate now");
	}
}


//-----------------------------------------------------------------------------
// methods CContestCtrlScriptImplementation / Scrript run time
//-----------------------------------------------------------------------------

void CContestCtrlScriptImplementation::setDelay(uint32 duration)
{
	_NextInstructionTime= CTime::getLocalTime()+ duration;
}

void CContestCtrlScriptImplementation::waitAnswer(bool value)
{
	_WaitingForAnswer= value;
}

bool CContestCtrlScriptImplementation::isAllWinnersFound()
{
	return _AllWinnersFound;
}

void CContestCtrlScriptImplementation::treatWinners(uint32 executorId)
{
	// log the list of winners
	for (TExecutors::iterator it=_Executors.begin();it!=_Executors.end();++it)
	{
		if (executorId!=(*it).second.Id)
			continue;

		nlinfo("Winners for CE '%s' (%d):",(*it).first.c_str(),(*it).second.Winners.size());
		for (uint32 i=0;i<(*it).second.Winners.size() && i<_NumPrizes;++i)
		{
			nlinfo("- %s",(*it).second.Winners[i].c_str());
		}
		// send a message to each of the CEs to have them broadcast the winners to their players
		std::vector<CSString> winners= (*it).second.Winners;
		if (winners.size()>_NumPrizes)
			winners.resize(_NumPrizes);
		sendModuleMessage(CMsgCCAckWinners(winners),(*it).second.Id,this);
	}
}


//-----------------------------------------------------------------------------
// methods CContestCtrlScript
//-----------------------------------------------------------------------------

CContestCtrlScript* CContestCtrlScript::getInstance()
{
	return CContestCtrlScriptImplementation::getInstance();
}


//-----------------------------------------------------------------------------
// Register the module
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CContestCtrlScriptImplementation,"CC","<contest name>","Contest control")


//-----------------------------------------------------------------------------
