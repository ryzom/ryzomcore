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
#include "nel/misc/time_nl.h"
#include "nel/misc/variable.h"
#include "nel/net/service.h"
#include "nel/misc/i18n.h"

// game share
#include "game_share/utils.h"

// gus
#include "gus_net_remote_module.h"
#include "gus_module_factory.h"
#include "gus_net_messages.h"
#include "gus_net.h"
#include "gus_chat.h"
#include "gus_client_manager.h"
#include "gus_mirror.h"

// local
#include "ce_contest_executor.h"
#include "cc_module_messages.h"
#include "ce_module_messages.h"
#include "cl_module_messages.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------------

CVariable<uint32> ReplyPauseTime("ContestExecutor", "CE_ReplyPauseTime", "Contest Executor module minimum delay between player answers", 60*1000, 0, true);


//-----------------------------------------------------------------------------
// constants - named strings used as identifiers
//-----------------------------------------------------------------------------

const char* promptSubmitAnswers=	"promptSubmitAnswers";
const char* errorNoQuestion=		"errorNoQuestion";
const char* errorTooLate=			"errorTooLate";
const char* treatingPendingAnswer=	"treatingPendingAnswer";
const char* announceWinners=		"announceWinners";
const char* treatedAnswer0=			"treatedAnswer0";
const char* treatedAnswer1=			"treatedAnswer1";
const char* treatedAnswer2=			"treatedAnswer2";
const char* treatedAnswer3=			"treatedAnswer3";
const char* queuedAnswer0=			"queuedAnswer0";
const char* queuedAnswer1=			"queuedAnswer1";
const char* queuedAnswer2=			"queuedAnswer2";
const char* queuedAnswer2b=         "queuedAnswer2b";

const char* systemName=				"systemName";


//-----------------------------------------------------------------------------
// forward class declarations
//-----------------------------------------------------------------------------

class CContestExecutorImplementation;
class CScriptLineAnswer;

//-----------------------------------------------------------------------------
// class CContestExecutorImplementation
//-----------------------------------------------------------------------------

class CContestExecutorImplementation
:	public CContestExecutor, 
	public IModule,
	public IChatCallback
{
public:
	// get hold of the singleton instance
	static CContestExecutorImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CContestExecutorImplementation();
	void clear();

public:
	// GUS::IModule methods
	bool initialiseModule(const NLMISC::CSString& rawArgs);
	void release();
	void tickUpdate(NLMISC::TGameCycle tickNumber);
	void moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void receiveModuleMessage(GUSNET::CModuleMessage& msg);
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

	// IChatCallback
	void receiveMessage(GUS::TClientId clientId,const ucstring& txt);
	void clientReadyInChannel(CChatChannel *chatChannel, GUS::TClientId clientId) ;
	bool isClientAllowedInChatChannel(GUS::TClientId clientId, CChatChannel *chatChannel);


public:
	// CContestExecutor methods
	void beginContest(uint32 ctrlModuleId, const NLMISC::CSString& name);
	void setTitle(const NLMISC::CSString& txt);
	void addText(const NLMISC::CSString& speakerName,const NLMISC::CSString& txt);
	void addAnswer(const NLMISC::CSString& answer);
	void submitAnswer(TClientId clientId,const NLMISC::CSString& characterName,const ucstring& answer);
	void acknowledgeWinners(const vector<CSString>& winners);
	void endContest();
	void display();

	// deal with system messages
	void initChatTexts();
	bool readChatTextFile(const NLMISC::CSString& fileName);
	const ucstring& getChatText(const CSString& msgName);

	// send a log message to the loggers
	void log(const IMsgCL& msg);


private:
	// utility methods
	void processAnswer(const NLMISC::CSString& characterName, const ucstring &answer);

private:
	// module control - module parameters and flag to say whether module is active
	bool _IsActive;
	NLMISC::CSString _Language;
	NLMISC::CSString _Name;

	// boolean that's used to mark whether a contest is running
	bool _IsRunning;
	bool _FinishedAnswers;
	TTime _StartTime;
	uint32 _CtrlModuleId;

	// the chart channel used to display stuff on the client
	TChatChannelPtr _ChatChannel;

	// title for this language
	CSString _Title;

	// text so far
	struct CTextEntry{ CSString SpeakerName, Txt; };
	typedef std::vector<CTextEntry> TTextSoFar;
	TTextSoFar _TextSoFar;

	// valid answers
	typedef CVectorSString TAnAnswer;
	typedef std::vector<TAnAnswer> TValidAnswers;
	TValidAnswers _ValidAnswers;
	uint32 _MinAnswerLength, _MaxAnswerLength, _MinAnswerWordCount, _MaxAnswerWordCount;

	// pending answers
	struct CCharacterRecord { uint64 PauseEndTime; ucstring NextAnswer; CEntityId Id; CCharacterRecord() { PauseEndTime=CTime::getLocalTime(); } };
	typedef std::map<CSString,CCharacterRecord> TCharacterRecords;
	TCharacterRecords _CharacterRecords;

	// winners
	struct CWinnerRecord { uint64 Time; ucstring Answer; CSString Name; };
	typedef std::vector<CWinnerRecord> TWinnerRecords;
	TWinnerRecords _WinnerRecords;

	// system messages
	typedef std::map<CSString,ucstring> TChatTexts;
	TChatTexts _ChatTexts;

	// log channels
	typedef std::set<uint32> TLoggers;
	TLoggers _Loggers;
};


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation ctor
//-----------------------------------------------------------------------------

CContestExecutorImplementation* CContestExecutorImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CContestExecutorImplementation> ptr=NULL;
	if (ptr==NULL)
	{
		ptr= new CContestExecutorImplementation;
	}
	return ptr;
}

CContestExecutorImplementation::CContestExecutorImplementation()
{
	clear();
	initChatTexts();
}

void CContestExecutorImplementation::clear()
{
	_IsActive= false;
	_Language.clear();
	_Name.clear();

	_IsRunning= false;
	_FinishedAnswers= false;
	_StartTime=0;
	_CtrlModuleId=InvalidRemoteModuleId;
	_MinAnswerLength= ~0u;
	_MaxAnswerLength= 0;
	_MinAnswerWordCount= ~0u;
	_MaxAnswerWordCount= 0;

	_Title.clear();
	_TextSoFar.clear();
	_ValidAnswers.clear();
	_CharacterRecords.clear();
	_WinnerRecords.clear();
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / IModule
//-----------------------------------------------------------------------------

bool CContestExecutorImplementation::initialiseModule(const NLMISC::CSString& rawArgs)
{
	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be CE module activated at a time",return false);
	DROP_IF(rawArgs.countWordOrWords()!=2,"error expected: \"<language> <name>\" but found: \""+rawArgs+"\"",return false);

	// process the command line arguments
	_Language= rawArgs.wordOrWords(0).strip();
	_Name= rawArgs.wordOrWords(1).strip();
	nlinfo("Initialising contest executor  Language= %s  Contest Name= %s",_Language.c_str(),_Name.c_str());

	// done (success)
	_IsActive= true;
	return true;
}

void CContestExecutorImplementation::release()
{
	if (_ChatChannel != NULL)
	{
		// close and destroy the channel
		_ChatChannel->closeChannel();
		CChatManager::getInstance()->removeChannel(_ChatChannel);
		_ChatChannel = NULL;
	}
	// mark the module as inactive so that it can be re-instantiated later
	clear();
}

void CContestExecutorImplementation::tickUpdate(NLMISC::TGameCycle tickNumber)
{
	// by definition the module shouldn't receive messages if it isn't active!
	nlassert(_IsActive);

	// run through the pending answers looking for one that needs executing
	for (TCharacterRecords::iterator it=_CharacterRecords.begin();it!=_CharacterRecords.end();++it)
	{
		const CSString& playerName= (*it).first;
		CCharacterRecord& player= (*it).second;
		TTime localTime= CTime::getLocalTime();
		if (player.NextAnswer.empty())
			continue;
		if ((sint32)(localTime-player.PauseEndTime)<0)
			continue;
		TClientId cid= CGusMirror::getInstance()->getDataSet("fe_temp")->getDataSetRow(player.Id);
		DROP_IF(cid==INVALID_DATASET_ROW,"Can't send message to deconnected player: "+playerName,continue);
		_ChatChannel->sendMessage(cid,getChatText(systemName),getChatText(treatingPendingAnswer)+": "+player.NextAnswer);
		_ChatChannel->sendMessage(cid,getChatText(systemName),getChatText(treatedAnswer3));
		processAnswer(playerName,player.NextAnswer);
	}
}

void CContestExecutorImplementation::moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	if (remoteModule->getName()=="CL")
		_Loggers.insert(remoteModule->getUniqueId());
}

void CContestExecutorImplementation::moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	if (remoteModule->getName()=="CL" && _Loggers.find(remoteModule->getUniqueId())!=_Loggers.end())
		_Loggers.erase(remoteModule->getUniqueId());
}

void CContestExecutorImplementation::receiveModuleMessage(GUSNET::CModuleMessage& msg)
{
	// by definition the module shouldn't receive messages if it isn't active!
	nlassert(_IsActive);

	CSString msgName= msg.getMessageName();
	nlinfo("CE - Received message: %s",msgName.c_str());

	// check that this module has been received from the CC module who's in charge of this contest
	DROP_IF(_IsRunning && msg.getSenderId()!=_CtrlModuleId && msgName!=CMsgCCBegin::getName(),
		"Ignoring message from wrong CC module",return);

	if (msgName==CMsgCCBegin::getName())		{ beginContest(msg.getSenderId(),CMsgCCBegin(msg.getMsgBody()).getContestName()); }
	if (msgName==CMsgCCTitle::getName())		
	{ 
		_FinishedAnswers= false;
		_ValidAnswers.clear();
		CMsgCCTitle title(msg.getMsgBody()); 
		if (title.getLang()==_Language) 
			setTitle(title.getTxt()); 
	}
	if (msgName==CMsgCCText::getName())			{ CMsgCCText text(msg.getMsgBody()); if (text.getLang()==_Language) addText(text.getSpeaker(),text.getTxt()); }
	if (msgName==CMsgCCAnswer::getName())		{ addAnswer(CMsgCCAnswer(msg.getMsgBody()).getTxt()); }
	if (msgName==CMsgCCAckWinners::getName())	{ acknowledgeWinners(CMsgCCAckWinners(msg.getMsgBody()).getWinners()); }
	if (msgName==CMsgCCEndAnswers::getName())	
	{ 
		_FinishedAnswers= true; 
	}
	if (msgName==CMsgCCEnd::getName())			{ endContest(); }
}

NLMISC::CSString CContestExecutorImplementation::getState() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	CSString result;
	result+= getName();
	result+= ' ';
	result+= getParameters();
	result+= ':';
	result+= NLMISC::toString("  Title: %s  Text Lines: %d  Valid Answers: %d  Winners So Far: %d  Recent Answers: %d",
			_Title.quote().c_str(),_TextSoFar.size(),_ValidAnswers.size(),_WinnerRecords.size(),_CharacterRecords.size());

	return result;
}

NLMISC::CSString CContestExecutorImplementation::getName() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return "CE";
}

NLMISC::CSString CContestExecutorImplementation::getParameters() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return _Language+" "+_Name.quoteIfNotAtomic();
}

void CContestExecutorImplementation::displayModule() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	// if not running then just display the file name
	if (!_IsRunning)
	{
		InfoLog->displayNL("MODULE CE: Contest Script Executor: Not running");
		return;
	}

	// display info
	InfoLog->displayNL("MODULE %s %s",getName().c_str(),getParameters().c_str());
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / IChatCallback
//-----------------------------------------------------------------------------

void CContestExecutorImplementation::receiveMessage(GUS::TClientId clientId,const ucstring& txt)
{
	const CSString &charName = CClientManager::getInstance()->getCharacterName(clientId);
	submitAnswer(clientId, charName.splitTo('$'), txt);
}

void CContestExecutorImplementation::clientReadyInChannel(CChatChannel *chatChannel, GUS::TClientId clientId)
{
	if (chatChannel == _ChatChannel)
	{
		// when a new client connects, send him all messages so far
		for (uint i = 0; i < _TextSoFar.size(); i++)
		{
			_ChatChannel->sendMessage(clientId, _TextSoFar[i].SpeakerName, _TextSoFar[i].Txt);
		}
	}
}

bool CContestExecutorImplementation::isClientAllowedInChatChannel(GUS::TClientId clientId, CChatChannel *chatChannel)
{
	return true;
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / CContestExecutor: ctrl i/f
//-----------------------------------------------------------------------------

void CContestExecutorImplementation::beginContest(uint32 ctrlModuleId, const NLMISC::CSString& name)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
//	DROP_IF(_IsRunning,"You cannot start contest '"+name+"' because another one is already running ('"+_Name+"')",return);
	DROP_IF(_Name!=name,"Ignoring contest '"+name+"' because we are configured for contest: '"+_Name+"'",return);

	nlinfo("Begin contest: %s",name.c_str());
	_StartTime= CTime::getLocalTime();
	_CtrlModuleId= ctrlModuleId;
	_ChatChannel= CChatManager::getInstance()->getChatChannel("ContestChatChannel");
	if (_ChatChannel == NULL)
		_ChatChannel = CChatManager::getInstance()->createChatChannel("ContestChatChannel");
	DROP_IF(_ChatChannel==NULL,"beginContest() FAILED because failed to create chat channel",return);
	_IsRunning= true;
	_FinishedAnswers= false;
	_ValidAnswers.clear();
	// Register for callback
	_ChatChannel->setChatCallback(this);

	sendModuleMessage(CMsgCERegister(_Language),_CtrlModuleId,this);
}

void CContestExecutorImplementation::setTitle(const NLMISC::CSString& title)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);

	nlinfo("Setting title: %s",title.c_str());
	_Title= title;

	// the name of the chat channel for translation purposes
	CSString chatChannelName= "ccChat";

	// send a message to the IOS to set the name of the chat window
//
//	CIOSMsgSetPhrase(chatChannelName,title).send();
//
//	NLNET::CMessage msg("SET_PHRASE");
//	msg.serial(chatChannelName);
//	ucstring ucTitle;
//	ucTitle.fromUtf8(title);
//	ucTitle=ucstring(chatChannelName+"(){[")+ucTitle+ucstring("]}");
//	msg.serial(ucTitle);
//	CUnifiedNetwork::getInstance()->send("IOS",msg);

	// open the chat channel
	_ChatChannel->openChannel(chatChannelName, 1000, true, true, false);
	_ChatChannel->setChannelTitle(title);
}

void CContestExecutorImplementation::addText(const NLMISC::CSString& speakerName,const NLMISC::CSString& txt)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);

	nlinfo("Adding text: Speaker: %s  Txt: %s",speakerName.c_str(),txt.c_str());
	CTextEntry txtEntry;
	txtEntry.SpeakerName=speakerName;
	txtEntry.Txt=txt;
	_TextSoFar.push_back(txtEntry);
	_ChatChannel->broadcastMessage(speakerName,txt);

	// send the new text to the log
	log(CMsgCLText(_Name+"_"+_Language,speakerName,txt));
}

void CContestExecutorImplementation::addAnswer(const NLMISC::CSString& answer)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);
	nlinfo("Adding a valid answer (now %d valid answers)",_ValidAnswers.size()+1);

	// split the answer into words
	CVectorSString words;
	CSString s=answer;
	uint32 totalSize=0;
	while (!s.empty())
	{
		CSString word= s.strtok(" \t,.;");
		if (!word.empty())
			words.push_back(word);
		totalSize+= word.size();
	}
	_ValidAnswers.push_back(words);

	// set the min or max word count
	if (words.size()<_MinAnswerWordCount) _MinAnswerWordCount= words.size();
	if (words.size()>_MaxAnswerWordCount) _MaxAnswerWordCount= words.size();

	// calculate the min and max answer length values
	if (totalSize<_MinAnswerLength) _MinAnswerLength= totalSize;
	if (totalSize+2*words.size()>_MaxAnswerLength) _MaxAnswerLength= totalSize+2*words.size();

	// if this is the first answer to be added then inform the clients that they can now start answering...
	if (_ValidAnswers.size()==1)
	{
		_ChatChannel->broadcastMessage(getChatText(systemName), getChatText(promptSubmitAnswers));
	}
}

void CContestExecutorImplementation::acknowledgeWinners(const vector<CSString>& winners)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);

	// we can't handle more answer
	_FinishedAnswers= true; 

	// display a heading line
	_ChatChannel->broadcastMessage(getChatText(systemName), getChatText(announceWinners));
	_ChatChannel->broadcastMessage(getChatText(systemName), getChatText(treatedAnswer1));

	nlinfo("The following are winners:");
	// display the winner list
	for (uint32 i=0;i<_WinnerRecords.size();++i)
	{
		uint32 j;
		for (j=0;j<winners.size();++j)
		{
			if (winners[j]==_WinnerRecords[i].Name)
				break;
		}
		uint32 displayTime = uint32(_WinnerRecords[i].Time-_StartTime)/1000;
		if (j<winners.size())
		{
//			_ChatChannel->broadcastMessage(getChatText(systemName), ucstring("* "+_WinnerRecords[i].Name));
			nlinfo("- %5u: %s: %s",
				displayTime ,
				_WinnerRecords[i].Name.c_str(),
				_WinnerRecords[i].Answer.toString().c_str());
		}
		else
		{
			nlinfo("x %5u: %s: %s => Correct answer but too late",
				displayTime,
				_WinnerRecords[i].Name.c_str(),
				_WinnerRecords[i].Answer.toString().c_str());
		}

		// send the declared winner to the log
		log(CMsgCLWinner(_WinnerRecords[i].Name));
	}
}

void CContestExecutorImplementation::endContest()
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);

	_ChatChannel->closeChannel();

//	CChatManager::getInstance()->removeChannel(_ChatChannel);
//	_ChatChannel = NULL;
	
	_MinAnswerLength= ~0u;
	_MaxAnswerLength= 0;
	_MinAnswerWordCount= ~0u;
	_MaxAnswerWordCount= 0;
	
	_Title.clear();
	_TextSoFar.clear();
	_ValidAnswers.clear();
	_CharacterRecords.clear();
	_WinnerRecords.clear();

	_FinishedAnswers = true;
	
	nlinfo("End of context...");
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / CContestExecutor: client i/f
//-----------------------------------------------------------------------------

class CCEChatCallback: public IChatCallback
{
public:
	void receiveMessage(TClientId clientId,const ucstring& txt);
};

void CCEChatCallback::receiveMessage(TClientId clientId,const ucstring& txt)
{
	// get the character name from the client id
	const CSString& characterName= CClientManager::getInstance()->getCharacterName(clientId);
	BOMB_IF(characterName.empty(),"CClientManager failed to identify character name",return);

	// submit the answer
	CContestExecutorImplementation::getInstance()->submitAnswer(clientId, characterName, txt);
}

void CContestExecutorImplementation::submitAnswer(TClientId clientId, const CSString& characterName, const ucstring& answer)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);
	nldebug("Treating answer: From player: '%s'  Answer: '%s'",characterName.c_str(),answer.toString().c_str());

	// make sure that players are allowed to submit answers
	if (_ValidAnswers.empty())
	{
		_ChatChannel->sendMessage(clientId, getChatText(systemName), getChatText(errorNoQuestion));
		return;
	}

	if (_FinishedAnswers)
	{
		_ChatChannel->sendMessage(clientId, getChatText(systemName), getChatText(errorTooLate));
		return;
	}

	// get hold of the player
	TTime localTime= CTime::getLocalTime();
	if (_CharacterRecords.find(characterName)==_CharacterRecords.end())
	{
		_CharacterRecords[characterName].Id= CGusMirror::getInstance()->getDataSet("fe_temp")->getEntityId(clientId);
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(treatedAnswer0)+" "+answer);
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(treatedAnswer1));
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(treatedAnswer2));
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(treatedAnswer3));
	}
	else 
	{
		if (! (sint32(_CharacterRecords[characterName].PauseEndTime-localTime)>0 || !_CharacterRecords[characterName].NextAnswer.empty()) )
		{
			_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(queuedAnswer0)+" "+answer);
			_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(treatedAnswer3));
		}
	}

	// get hold of the record for this player - if none exist a new one is created with PauseEndTime set to now
	CCharacterRecord& thePlayer= _CharacterRecords[characterName];

	// check whether we are in the pause case - ie there has been a previous answer submitted shortly ago
	if (sint32(thePlayer.PauseEndTime-localTime)>0 || !thePlayer.NextAnswer.empty())
	{
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(queuedAnswer0)+" "+answer);
		_ChatChannel->sendMessage(clientId,getChatText(systemName),getChatText(queuedAnswer1));
		_ChatChannel->sendMessage(clientId,getChatText(systemName),
								  getChatText(queuedAnswer2)+" "+
								  ucstring(NLMISC::toString("%d",sint32(thePlayer.PauseEndTime-localTime)/1000))+" "+
								  getChatText(queuedAnswer2b));
		thePlayer.NextAnswer = answer;
		return;
	}

	// check the answer right away
	processAnswer(characterName,answer);
}

void CContestExecutorImplementation::processAnswer(const NLMISC::CSString& characterName, const ucstring &answer)
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	DROP_IF(!_IsRunning,"Operation not permitted when there is no contest running",return);

	nlinfo("Testing answer validity : player '%s', answer '%s'", characterName.c_str(), answer.toString().c_str());

	// prepare the answer attempt
	ucstring answerRemainder = answer;
	CSString attempt=answer.toUtf8();

	// update the pause timer 
	_CharacterRecords[characterName].PauseEndTime= CTime::getLocalTime()+ReplyPauseTime;
	_CharacterRecords[characterName].NextAnswer.clear();

	// perform a quick size test...
	if (attempt.size()<_MinAnswerLength || attempt.size()>_MaxAnswerLength)
		return;

	// split into words, stripping out common punctuation
	CVectorSString words;
	while(!attempt.empty())
	{
		vectAppend(words)=attempt.strtok(" \t,;.");
	}
	if (words.size()<_MinAnswerWordCount || words.size()>_MaxAnswerWordCount)
		return;

	uint32 i;
	for (i=0;i<_ValidAnswers.size();++i)
	{
		uint32 j;
		for (j=0;j<_ValidAnswers[i].size();++j)
		{
			uint32 k;
			for (k=0;k<words.size();++k)
			{
				// if words match
				if (_ValidAnswers[i][j]==words[k])
					break;
			}
			// if no match found
			if (k==words.size())
				break;
		}
		// if a complete phrase matched all th way through
		if (j==_ValidAnswers[i].size())
			break;
	}

	// if a match was found
	if (i<_ValidAnswers.size())
	{
		// make sure the player isn't already in the winner list
		for (uint32 i=0;i<_WinnerRecords.size();++i)
		{
			if (_WinnerRecords[i].Name==characterName)
				return;
		}

		// make a record of the winner for later use
		nlinfo("New winner found");
		CWinnerRecord& winnerRecord= vectAppend(_WinnerRecords);
		winnerRecord.Name=characterName;
		winnerRecord.Answer=answerRemainder;
		winnerRecord.Time=CTime::getLocalTime();

		// create a log in our log file
		InfoLog->displayNL("%u: Winner: '%s' - Answer: '%s'",
			uint32(winnerRecord.Time/1000),
			characterName.c_str(),
			answerRemainder.toString().c_str());

		// send the network message to the controler to infor of the winner
		sendModuleMessage(CMsgCEWinner(characterName),_CtrlModuleId,this);
	}
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / CContestExecutor: misc
//-----------------------------------------------------------------------------

void CContestExecutorImplementation::display()
{
	DROP_IF(!_IsActive,"The CE module is not instantiated",return);
	displayModule();
}


//-----------------------------------------------------------------------------
// methods CContestExecutorImplementation / Localised system messages
//-----------------------------------------------------------------------------

void CContestExecutorImplementation::initChatTexts()
{
	ucstring ucs;
	_ChatTexts[promptSubmitAnswers]= ucs;	// "You may now submit answers"
	_ChatTexts[errorNoQuestion]= ucs;		// "Your answer has been ignored because no question has been asked"
	_ChatTexts[errorTooLate]= ucs;			// "Your answer comes too late - all of the winners have now been chosen"
	_ChatTexts[treatingPendingAnswer]= ucs;	// "Your last answer is being processed: "
	_ChatTexts[announceWinners]= ucs;		// "The contest is over. The winners are:"
	_ChatTexts[treatedAnswer0]= ucs;		// "You have submitted an answer: <answer>"
	_ChatTexts[treatedAnswer1]= ucs;		// "The winners will be announced within the next few minutes"
	_ChatTexts[treatedAnswer2]= ucs;		// "Please stay on line"
	_ChatTexts[treatedAnswer3]= ucs;		// "You will not be allowed to submit another answer for the next 60 seconds"
	_ChatTexts[queuedAnswer0]= ucs;			// "You have submitted another answer: "
	_ChatTexts[queuedAnswer1]= ucs;			// "You are only allowed to submit one answer every 60 seconds."
	_ChatTexts[queuedAnswer2]= ucs;			// "This answer will be treated in."
	_ChatTexts[queuedAnswer2b]= ucs;		// "seconds time."

	_ChatTexts[systemName]= ucs;			// The name of the "speaken" in the chat for sys info type messages
}

bool CContestExecutorImplementation::readChatTextFile(const CSString& fileName)
{
	// load the file
	ucstring ucFileBody;
	CI18N::readTextFile(fileName,ucFileBody);
	CSString fileBody=	ucFileBody.toUtf8();
	if (fileBody.empty())
		return false;

	// empty out the previous system messages (if any)
	for (TChatTexts::iterator it=_ChatTexts.begin();it!=_ChatTexts.end();++it)
		(*it).second.clear();

	// parse the file
	CVectorSString lines;
	fileBody.splitLines(lines);
	bool ok=true;
	for (uint32 i=0;i<lines.size();++i)
	{
		CSString line= lines[i].strip();
		if (line.empty())
			continue;
		if (line.left(2)=="//")
			continue;
		CSString keyword= line.firstWord(true);
		DROP_IF (line.empty(),"Invalid line: "+lines[i],ok=false;continue);
		DROP_IF (_ChatTexts.find(keyword)==_ChatTexts.end(),"Skipping unknown keyword: "+lines[i],continue);
		_ChatTexts[keyword].fromUtf8(line.leftStrip());
	}

	// ensure that all of the keywords have been dealt with
	for (TChatTexts::iterator it=_ChatTexts.begin();it!=_ChatTexts.end();++it)
	{
		if ((*it).second.empty())
		{
			nlwarning("Failed to find translation for keyword: %s",(*it).first.c_str());
			ok=false;
		}
	}
	return ok;
}

const ucstring& CContestExecutorImplementation::getChatText(const CSString& msgName)
{
	nlassert(_ChatTexts.find(msgName)!=_ChatTexts.end());
	return _ChatTexts[msgName];
}

void CContestExecutorImplementation::log(const IMsgCL& msg)
{
	for (TLoggers::iterator it= _Loggers.begin(); it!=_Loggers.end();++it)
	{
		sendModuleMessage(msg,(*it),this);
	}
}

//-----------------------------------------------------------------------------
// methods CContestExecutor
//-----------------------------------------------------------------------------

CContestExecutor* CContestExecutor::getInstance()
{
	return CContestExecutorImplementation::getInstance();
}


//-----------------------------------------------------------------------------
// Register the module
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CContestExecutorImplementation,"CE","<language> <contest name>","Contest executor")


//-----------------------------------------------------------------------------
