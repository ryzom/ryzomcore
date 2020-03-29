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

// nel
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/net/service.h"

// stl
#include <ctime>

// game share
#include "game_share/utils.h"

// local
#include "gus_module_manager.h"
#include "gus_net_messages.h"
#include "gus_net_remote_module.h"

#include "ee_event_executor.h"
#include "ee_module_messages.h"
#include "em_module_messages.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

CVariable<string>	EventInstallDirectory("EventExecutor", "EventInstallDirectory", "Directory to install event files to", string("./events/install/"), 0, true);
CVariable<string>	ToolsArchiveDirectory("EventExecutor", "ToolsArchiveDirectory", "Directory where downloadable tools can be found", string("./events/tools_archive/"), 0, true);


//-----------------------------------------------------------------------------
// class CEventExecutorImplementation
//-----------------------------------------------------------------------------

class CEventExecutorImplementation: public CEventExecutor, public GUS::IModule
{
public:
	// get hold of the singleton instance
	static CEventExecutorImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CEventExecutorImplementation();
	void clear();

public:
	// GUS::IModule methods
	bool initialiseModule(const NLMISC::CSString& rawArgs);

	void receiveModuleMessage(GUSNET::CModuleMessage& msg);
	
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;


public:
	// CEventExecutor methods
	void cbLogin(uint32 senderId,const CMsgEELogin &msg);
	void cbUpload(uint32 senderId,const CMsgEEUpload &msg);
	void cbPeek(uint32 senderId,const CMsgEEPeek &msg) const;
	void cbEventStart(uint32 senderId,const CMsgEEEventStart &msg) const;
	void cbEventStop(uint32 senderId,const CMsgEEEventStop &msg) const;
	void cbToolsUpdateReq(uint32 senderId,const CMsgEEToolsUpdReq &msg);
	void cbToolsFileReq(uint32 senderId,const CMsgEEToolsFileReq &msg);

private:
	// private data
	// module control - module parameters and flag to say whether module is active
	bool _IsActive;

	NLMISC::CSString _Parameters;
	typedef map<CSString,CSString> TPasswords;
	TPasswords _Passwords;
};

//-----------------------------------------------------------------------------
// methods CEventExecutorImplementation / ctor
//-----------------------------------------------------------------------------

CEventExecutorImplementation* CEventExecutorImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CEventExecutorImplementation> ptr=NULL;
	if (ptr==NULL)
	{
		ptr= new CEventExecutorImplementation;
	}
	return ptr;
}

CEventExecutorImplementation::CEventExecutorImplementation()
{
	clear();
}

void CEventExecutorImplementation::clear()
{
	_Parameters.clear();
	_Passwords.clear();
	_IsActive= false;
}

//-----------------------------------------------------------------------------
// methods CEventExecutorImplementation / IModule
//-----------------------------------------------------------------------------

bool CEventExecutorImplementation::initialiseModule(const NLMISC::CSString& rawArgs)
{
	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be CE module activated at a time",return false);

	// syntax: <shard name> <list of specific names (like 'ai', 'egs', etc)>

	_Parameters= rawArgs;
	_Passwords.clear();

	// read the passwords from the cfg file
	bool errors= false;
	CConfigFile::CVar *var = IService::getInstance()->ConfigFile.getVarPtr ("Users");
	DROP_IF(var==NULL,"Cannot instantiate module due to missing 'Users' entry in cfg file",return false);

	for (sint i = 0; i < var->size(); i++)
	{
		CSString s= var->asString(i);
		if (s.strip().empty())
			continue;

		CSString user= s.firstWord(true).toLower();
		s= s.strip();

		if (s.empty())
		{
			nlwarning("Bad syntax: missing password (expecting <user> <password>): %s",var->asString(i).c_str());
			errors= true;
			continue;
		}

		if (_Passwords.find(user)!=_Passwords.end())
		{
			nlwarning("More than one entry for the same user: %s",var->asString(i).c_str());
			errors= true;
			continue;
		}

		_Passwords[user]= s;
	}


	bool _IsActive= (rawArgs.countWords()>=1 && !errors && !_Passwords.empty());
	return _IsActive;
}

void CEventExecutorImplementation::receiveModuleMessage(GUSNET::CModuleMessage& msg)
{
	if (msg.getMessageName()==CMsgEELogin().getName())
	{
		// 'login'
		CMsgEELogin loginMsg(msg.getMsgBody());
		cbLogin(msg.getSenderId(),loginMsg);
	}
	else if (msg.getMessageName()==CMsgEEUpload().getName())
	{
		// 'upload'
		CMsgEEUpload uploadMsg(msg.getMsgBody());
		cbUpload(msg.getSenderId(),uploadMsg);
	}
	else if (msg.getMessageName()==CMsgEEPeek().getName())
	{
		// 'peek'
		CMsgEEPeek peekMsg(msg.getMsgBody());
		cbPeek(msg.getSenderId(),peekMsg);
	}
	else if (msg.getMessageName()==CMsgEEEventStart().getName())
	{
		// 'event start'
		CMsgEEEventStart startMsg(msg.getMsgBody());
		cbEventStart(msg.getSenderId(),startMsg);
	}
	else if (msg.getMessageName()==CMsgEEEventStop().getName())
	{
		// 'event stop'
		CMsgEEEventStop stopMsg(msg.getMsgBody());
		cbEventStop(msg.getSenderId(),stopMsg);
	}
	else if (msg.getMessageName()==CMsgEEToolsUpdReq().getName())
	{
		// 'tools_update_request'
		CMsgEEToolsUpdReq updMsg(msg.getMsgBody());
		cbToolsUpdateReq(msg.getSenderId(),updMsg);
	}
	else if (msg.getMessageName()==CMsgEEToolsFileReq().getName())
	{
		// 'tools_file_request'
		CMsgEEToolsFileReq fileMsg(msg.getMsgBody());
		cbToolsFileReq(msg.getSenderId(),fileMsg);
	}
}
	
NLMISC::CSString CEventExecutorImplementation::getState() const
{
	return getName()+" "+getParameters();
}

NLMISC::CSString CEventExecutorImplementation::getName() const
{
	return "EE";
}

NLMISC::CSString CEventExecutorImplementation::getParameters() const
{
	return _Parameters;
}

void CEventExecutorImplementation::displayModule() const
{
	NLMISC::InfoLog->displayNL("%s",getState().c_str());
}


//-----------------------------------------------------------------------------
// methods CEventExecutorImplementation / CEventExecutor
//-----------------------------------------------------------------------------

void CEventExecutorImplementation::cbLogin(uint32 remoteModuleId, const CMsgEELogin& msg)
{
	CSString user= msg.getUser().toLower();
	if(_Passwords.find(user)==_Passwords.end())
	{
		nlwarning("Ignoring login request from unknown user: %s",user.c_str());
		CMsgEMLoginReply replyMsg(false,"Login refused");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}
	CSString password= _Passwords[user];
	if(!msg.testPassword(password))
	{
		nlwarning("Ignoring login attempt due to bad password for user: %s",user.c_str());
		CMsgEMLoginReply replyMsg(false,"Login refused");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}

	nlinfo("EE: Login received OK");
	CMsgEMLoginReply replyMsg(true,"Login accepted");
	sendModuleMessage(replyMsg,remoteModuleId,this);
}

void CEventExecutorImplementation::cbUpload(uint32 remoteModuleId, const CMsgEEUpload& msg)
{
	CSString user= msg.getUser().toLower();
	if(_Passwords.find(user)==_Passwords.end())
	{
		nlwarning("Ignoring login request from unknown user: %s",user.c_str());
		CMsgEMLoginReply replyMsg(false,"Upload refused");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}
	CSString password= _Passwords[user];
	if(!msg.testPassword(password))
	{
		nlwarning("Ignoring login attempt due to bad password for user: %s",user.c_str());
		CMsgEMLoginReply replyMsg(false,"Upload refused");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}

	nlinfo("EE: Upload received - beginning work");

	// ensure that the received message is valid looking
	if (msg.getFdc().size()!=msg.getFileBodies().size())
	{
		nlwarning("There's something fishy about the uploaded file set - ignoring it");
		CMsgEMUploadReply replyMsg(false,"Invalid header data in upload message");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}

	// create the install directory (do nothing if already exists)
	NLMISC::CFile::createDirectoryTree(EventInstallDirectory);

	// delete all of the files in the directory
	CVectorSString wildcards;
	CFileDescriptionContainer fdc;
	wildcards.push_back("*.primitive");
	wildcards.push_back("*.txt");
	wildcards.push_back("*.uxt");
	wildcards.push_back("*.gss");
	fdc.addFiles(EventInstallDirectory.get()+"/", reinterpret_cast<vector<string> &>(wildcards));
	for(uint32 i=0;i<fdc.size();++i)
	{
		nlinfo("EE: - deleting file: %s",fdc[i].FileName.c_str());
		NLMISC::CFile::deleteFile(fdc[i].FileName);
	}

	// ensure that all of the files in the directory have been deleted
	fdc.clear();
	fdc.addFiles(EventInstallDirectory.get()+"/", reinterpret_cast<vector<string> &>(wildcards));
	if (!fdc.empty())
	{
		nlwarning("Failed to delete all files in the event install directory");
		CMsgEMUploadReply replyMsg(false,"Failed to delete all of the files inthe event install directory");
		sendModuleMessage(replyMsg,remoteModuleId,this);
		return;
	}

	// write the uploaded files to the directory
	for (uint32 i=0;i<msg.getFdc().size();++i)
	{
		CSString filename= EventInstallDirectory.get()+"/"+NLMISC::CFile::getFilename(msg.getFdc()[i].FileName);
		nlinfo("EE: - writing file: %s",filename.c_str());
		msg.getFileBodies()[i].writeToFile(filename);		
		if (!NLMISC::CFile::fileExists(filename) || NLMISC::CFile::getFileSize(filename)!=msg.getFdc()[i].FileSize)
		{
			nlwarning("Failed to write file correctly: %s",filename.c_str());
			CMsgEMUploadReply replyMsg(false,"Failed to write file correctly: "+filename);
			sendModuleMessage(replyMsg,remoteModuleId,this);
			return;
		}
	}

	// lookup the current time
	time_t dateTime;
	time(&dateTime);
	CSString dateTimeStr= IDisplayer::dateToHumanString(dateTime);

	// create the event description and write it to a disk file
	CSString eventDescription;
	eventDescription= msg.getEventName()+", User: "+user+", Date: "+dateTimeStr;

	// append the event description to the history log file
	CSString eventHistory;
	eventHistory.readFromFile(EventInstallDirectory.get()+"/event_description.log");
	eventHistory+="\n"+eventDescription;
	eventHistory.writeToFile(EventInstallDirectory.get()+"/event_description.log");

	// send a success message to the EM module
	nlinfo("EE: Upload received - finished");
	CMsgEMUploadReply replyMsg(true,"Installed Event: "+eventDescription);
	sendModuleMessage(replyMsg,remoteModuleId,this);

	// use the state system to load the new event onto the shard
	NLMISC::ICommand::execute("ssActiveStatesEnd eventRunning",*NLMISC::InfoLog);
	NLMISC::CSString scriptFileName= EventInstallDirectory.get()+"/event_load_script.gss";
	if (NLMISC::CFile::fileExists(scriptFileName))
	{
		NLMISC::ICommand::execute(("ssScrLoad "+scriptFileName).c_str(),*NLMISC::InfoLog);
		NLMISC::ICommand::execute("ssActiveStatesBegin eventRunning",*NLMISC::InfoLog);
	}
}

void CEventExecutorImplementation::cbPeek(uint32 remoteModuleId, const CMsgEEPeek &msg) const
{
	nlinfo("EE: Peek received - sending back file list");

	// build the installed file list
	CVectorSString wildcards;
	CFileDescriptionContainer fdc;
	wildcards.push_back("*.primitive");
	wildcards.push_back("*.txt");
	wildcards.push_back("*.uxt");
	wildcards.push_back("*.gss");
	fdc.addFiles(EventInstallDirectory.get()+"/", reinterpret_cast<vector<string> &>(wildcards));

	// lookup the event description
	CSString eventDescription;
	eventDescription.readFromFile(EventInstallDirectory.get()+"/event_description.log");

	// send a reply message back to the EM module
	CMsgEMPeekReply replyMsg("Installed event: "+eventDescription,fdc);
	sendModuleMessage(replyMsg,remoteModuleId,this);
}

void CEventExecutorImplementation::cbEventStart(uint32 senderId,const CMsgEEEventStart &msg) const
{
	NLMISC::ICommand::execute("ssActiveStatesBegin eventRunning",*NLMISC::InfoLog);

	// build the installed file list
	CFileDescriptionContainer fdc;

	// lookup the event description
	CSString eventDescription;
	eventDescription.readFromFile(EventInstallDirectory.get()+"/event_description.log");
	CVectorSString lines;
	eventDescription.splitLines(lines);
	while (!lines.empty() && lines.back().strip().empty())
		lines.pop_back();
	CSString theEventName;
	if (!lines.empty())
		theEventName= lines.back();

	// send a reply message back to the EM module
	CMsgEMPeekReply replyMsg("Starting event: "+theEventName,fdc);
	sendModuleMessage(replyMsg,senderId,this);
}

void CEventExecutorImplementation::cbEventStop(uint32 senderId,const CMsgEEEventStop &msg) const
{
	NLMISC::ICommand::execute("ssActiveStatesEnd eventRunning",*NLMISC::InfoLog);

	// build the installed file list
	CFileDescriptionContainer fdc;

	// lookup the event description
	CSString eventDescription;
	eventDescription.readFromFile(EventInstallDirectory.get()+"/event_description.log");
	CVectorSString lines;
	eventDescription.splitLines(lines);
	while (!lines.empty() && lines.back().strip().empty())
		lines.pop_back();
	CSString theEventName;
	if (!lines.empty())
		theEventName= lines.back();

	// send a reply message back to the EM module
	CMsgEMPeekReply replyMsg("Stopping event: "+theEventName,fdc);
	sendModuleMessage(replyMsg,senderId,this);
}

void CEventExecutorImplementation::cbToolsUpdateReq(uint32 remoteModuleId, const CMsgEEToolsUpdReq& msg)
{
	// scan the tools archive directory for files
	CFileDescriptionContainer fdc;
	fdc.addFileSpec(ToolsArchiveDirectory.get()+"/*",true);

	// setup the reply message
	CMsgEMToolsUpdReply replyMsg(NLMISC::toString("Tools archive contains %d files",fdc.size()));

	// fill out the reply message
	for (uint32 i=0;i<fdc.size();++i)
	{
		CSString fileName= fdc[i].FileName.leftCrop(ToolsArchiveDirectory.get().size());
		while (fileName.left(1)=="/")
			fileName= fileName.leftCrop(1);
		uint32 fileSize= fdc[i].FileSize;
		NLMISC::CHashKeyMD5 checkSum= NLMISC::getMD5(fdc[i].FileName);
		replyMsg.addFile(fileName,checkSum,fileSize);
	}

	// send the reply message back to the requestor
	sendModuleMessage(replyMsg,remoteModuleId,this);
}

void CEventExecutorImplementation::cbToolsFileReq(uint32 remoteModuleId, const CMsgEEToolsFileReq& msg)
{
	// look for the requested file, rad its data and calculate the checksum
	NLMISC::CSString fileName= ToolsArchiveDirectory.get()+"/"+msg.getFileName();
	NLMISC::CSString fileData;
	fileData.readFromFile(fileName);
	NLMISC::CHashKeyMD5 checkSum= NLMISC::getMD5((uint8*)&fileData[0],fileData.size());

	// setup the reply message
	CMsgEMToolsFileReply replyMsg(
		NLMISC::toString("File: %s (%d bytes): checksum %s",fileName.c_str(),fileData.size(),checkSum.toString().c_str()).c_str(),
		fileName,fileData);

	// send the reply message back to the requestor
	sendModuleMessage(replyMsg,remoteModuleId,this);
}


//-----------------------------------------------------------------------------
// methods CEventExecutor
//-----------------------------------------------------------------------------

CEventExecutor* CEventExecutor::getInstance()
{
	return CEventExecutorImplementation::getInstance();
}


//-----------------------------------------------------------------------------
// Register the module
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CEventExecutorImplementation,CEventExecutorImplementation::getInstance()->getName(),"<shard name>","Event executor")


//-----------------------------------------------------------------------------
