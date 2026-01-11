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
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"

// local
#include "gus_module_manager.h"
#include "gus_net.h"
#include "gus_net_messages.h"
#include "gus_net_remote_module.h"

#include "em_event_manager.h"
#include "em_module_messages.h"
#include "ee_module_messages.h"


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

extern NLMISC::CVariable<string> ToolsDirectory;
extern NLMISC::CVariable<string> ToolsShardName;


//-----------------------------------------------------------------------------
// class CEventManagerImplementation
//-----------------------------------------------------------------------------

class CEventManagerImplementation: public CEventManager, public GUS::IModule
{
public:
	// get hold of the singleton instance
	static CEventManagerImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CEventManagerImplementation();
	void clear();

public:
	// IModule methods
	bool initialiseModule(const NLMISC::CSString& rawArgs);

	void moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void receiveModuleMessage(GUSNET::CModuleMessage& msg);
	
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

public:
	// CEventManager methods
	void login(const NLMISC::CSString& shardName,const NLMISC::CSString& userId,const NLMISC::CSString& password);
	void upload(const NLMISC::CSString& shardName,const NLMISC::CSString& eventName,const CFileDescriptionContainer& fdc,const NLMISC::CVectorSString& fileBodies);
	void startEvent(const NLMISC::CSString& shardName);
	void stopEvent(const NLMISC::CSString& shardName);
	void updateTools();
	void peekInstalledEvent(const NLMISC::CSString& shardName) const;
	void getShards(NLMISC::CVectorSString& shardNames) const;

private:
	// the shard id type used as map key
	typedef NLMISC::CSString TShardId;

	// set of EE module Ids regrouped by shard id
	typedef std::set<uint32> TShardsMapEntry;
	typedef std::map<TShardId,TShardsMapEntry> TShards;
	TShards _Shards;

	// set of registered logins and passwords for different shards
	struct CLogin { NLMISC::CSString UserId, Password; uint32 ValidLoginCount; CLogin() {ValidLoginCount=0;} };
	typedef std::map<TShardId,CLogin> TLogins;
	TLogins _Logins;

	// set of tools files that need to be downloaded from the tools repository shard
	typedef std::vector<NLMISC::CSString> TToolsFilesToDownload;
	TToolsFilesToDownload _ToolsFilesToDownload;

	// module control - module parameters and flag to say whether module is active
	bool _IsActive;
};

//-----------------------------------------------------------------------------
// methods CEventManagerImplementation / ctor
//-----------------------------------------------------------------------------

CEventManagerImplementation* CEventManagerImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CEventManagerImplementation> ptr=NULL;
	if (ptr==NULL)
	{
		ptr= new CEventManagerImplementation;
	}
	return ptr;
}

CEventManagerImplementation::CEventManagerImplementation()
{
	clear();
}

void CEventManagerImplementation::clear()
{
	_IsActive= false;
}


//-----------------------------------------------------------------------------
// methods CEventManagerImplementation
//-----------------------------------------------------------------------------

bool CEventManagerImplementation::initialiseModule(const NLMISC::CSString& rawArgs)
{
	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be CE module activated at a time",return false);

	_IsActive= true;
	return true;
}

void CEventManagerImplementation::moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	// we're only interested in EE modules
	if (remoteModule->getName()!="EE")
		return;

	// extract the shard name from the remote module's parameters
	NLMISC::CSString shardName= remoteModule->getParameters().firstWordConst().strip();

	// add this module id to the shard entry that it belongs to in our shards map
	_Shards[shardName].insert(remoteModule->getUniqueId());

	// if we are logged into this shard then login to this module too
	if (_Logins.find(shardName)!=_Logins.end())
	{
		nlinfo("New connection to event executor detected: %d: %s %s",remoteModule->getUniqueId(),remoteModule->getName().c_str(),remoteModule->getParameters().c_str());
		nlinfo("Sending login request to event executor: %d: %s %s",remoteModule->getUniqueId(),remoteModule->getName().c_str(),remoteModule->getParameters().c_str());
		CMsgEELogin loginMsg(_Logins[shardName].UserId,_Logins[shardName].Password);
		sendModuleMessage(loginMsg,remoteModule->getUniqueId(),this);

		// special case for treating the fact that one of the remote modules is responsible for tools repository
		if (shardName==ToolsShardName.get() && !_ToolsFilesToDownload.empty())
		{
			CMsgEEToolsFileReq fileReqMsg(_Logins[shardName].UserId,_Logins[shardName].Password,_ToolsFilesToDownload.back());
			sendModuleMessage(fileReqMsg,remoteModule->getUniqueId(),this);
		}
	}
}

void CEventManagerImplementation::moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	// we're only interested in EE modules
	if (remoteModule->getName()!="EE")
		return;

	// extract the shard name from the remote module's parameters
	NLMISC::CSString shardName= remoteModule->getParameters().firstWordConst().strip();

	// remove the entry from the connected executors map
	_Shards[shardName].erase(remoteModule->getUniqueId());

	// if the shard is one that we're logged onto then display a message
	if (_Logins.find(shardName)!=_Logins.end())
	{
		nlinfo("Lost connection to event executor: %d: %s %s",remoteModule->getUniqueId(),remoteModule->getName().c_str(),remoteModule->getParameters().c_str());
		--_Logins[shardName].ValidLoginCount;
	}
}

void CEventManagerImplementation::receiveModuleMessage(GUSNET::CModuleMessage& msg)
{
	// work out the shard name
	CSString shardName;
	for (TShards::iterator it=_Shards.begin(); shardName.empty() && it!=_Shards.end(); ++it)
	{
		for (TShardsMapEntry::iterator it2= (*it).second.begin(); shardName.empty() && it2!=(*it).second.end(); ++it2)
		{
			if ((*it2)==msg.getSenderId())
				shardName= (*it).first;
		}
	}

	if (msg.getMessageName()==CMsgEMLoginReply().getName())
	{
		// 'login'
		CMsgEMLoginReply loginMsg(msg.getMsgBody());
		if (loginMsg.isAccepted())
		{
			NLMISC::InfoLog->displayNL("Login accepted [%s:%d]: %s",shardName.c_str(),msg.getSenderId(),loginMsg.getResultTxt().c_str());
			++_Logins[shardName].ValidLoginCount;
		}
		else
		{
			NLMISC::WarningLog->displayNL("Login rejected [%s:%d]: %s",shardName.c_str(),msg.getSenderId(),loginMsg.getResultTxt().c_str());
		}
	}
	else if (msg.getMessageName()==CMsgEMUploadReply().getName())
	{
		// 'upload'
		CMsgEMUploadReply uploadMsg(msg.getMsgBody());
		NLMISC::InfoLog->displayNL("Event upload received reply [%s:%d]: %s",shardName.c_str(),msg.getSenderId(),uploadMsg.getResultTxt().c_str());
	}
	else if (msg.getMessageName()==CMsgEMToolsUpdReply().getName())
	{
		// 'tools_update'
		CMsgEMToolsUpdReply updMsg(msg.getMsgBody());
		NLMISC::InfoLog->displayNL("Tools update request received reply");

		// build a list of files in the tools directory right now
		std::map<NLMISC::CSString,NLMISC::CHashKeyMD5> files;
		CFileDescriptionContainer fdc;
		fdc.addFileSpec(ToolsDirectory.get()+"/*",true);
		for (uint32 i=0;i<fdc.size();++i)
		{
			CSString fileName= fdc[i].FileName.leftCrop(ToolsDirectory.get().size());
			while (fileName.left(1)=="/")
				fileName= fileName.leftCrop(1);
			nldebug("Calculating checksum for file: %s",fileName.c_str());
			files[fileName]= NLMISC::getMD5(fdc[i].FileName);
		}

		// run through the files in the input message looking for files that aren't up to date
		uint32 missingFiles=0;
		uint32 matchingFiles=0;
		uint32 nonMatchingFiles=0;
		uint32 downloadSize=0;
		for (uint32 i=updMsg.getNumFiles();i--;)
		{
			if (files.find(updMsg.getFileName(i))==files.end())
			{
				++missingFiles;
				downloadSize+= updMsg.getFileSize(i);
				_ToolsFilesToDownload.push_back(updMsg.getFileName(i));
			}
			else
			{
				if (files[updMsg.getFileName(i)]!=updMsg.getCheckSum(i))
				{
					++nonMatchingFiles;
					downloadSize+= updMsg.getFileSize(i);
					_ToolsFilesToDownload.push_back(updMsg.getFileName(i));
				}
				else
				{
					++matchingFiles;
				}
				files.erase(updMsg.getFileName(i));
			}
		}
		NLMISC::InfoLog->displayNL(
			"Matching Files: %d, Non-Matching Files: %d, Missing Files: %d, Download Size: %d",
			matchingFiles,nonMatchingFiles,missingFiles,downloadSize);

		DROP_IF(_ToolsFilesToDownload.empty(),"All files are up to date - nothing to do",return);
		DROP_IF(_Logins.find(ToolsShardName.get())==_Logins.end(),"You must be logged into the "+ToolsShardName.get()+" shard in order to update your tools",return);

		// send up a request for the first file...
		CMsgEEToolsFileReq fileReqMsg(_Logins[ToolsShardName.get()].UserId,_Logins[ToolsShardName.get()].Password,_ToolsFilesToDownload.back());
		sendModuleMessage(fileReqMsg,msg.getSenderId(),this);
	}
	else if (msg.getMessageName()==CMsgEMToolsFileReply().getName())
	{
		// 'tools_file'
		CMsgEMToolsFileReply fileMsg(msg.getMsgBody());
		NLMISC::InfoLog->displayNL("Tools file request received reply");

		// save the received file to disk
		nlinfo("Received tools file: %s (%d to go)",fileMsg.getFileName().c_str(),_ToolsFilesToDownload.size()-1);
		fileMsg.getFileData().writeToFile(ToolsDirectory.get()+"/"+fileMsg.getFileName());

		// make sure the code is running as intended (if not we have a bug!)...
		BOMB_IF(_ToolsFilesToDownload.back()!=fileMsg.getFileName(),
			"ERROR: Tools update aborted because: Expecting file '"+_ToolsFilesToDownload.back()+"' but received file '"+fileMsg.getFileName()+"'",
			_ToolsFilesToDownload.clear();return);

		// send request for the next file...
		_ToolsFilesToDownload.pop_back();
		DROP_IF(_Logins.find(ToolsShardName.get())==_Logins.end(),"You must be logged into the "+ToolsShardName.get()+" shard in order to update your tools",return);
		if (!_ToolsFilesToDownload.empty())
		{
			CMsgEEToolsFileReq fileReqMsg(_Logins[ToolsShardName.get()].UserId,_Logins[ToolsShardName.get()].Password,_ToolsFilesToDownload.back());
			sendModuleMessage(fileReqMsg,msg.getSenderId(),this);
		}
	}
	else if (msg.getMessageName()==CMsgEMPeekReply().getName())
	{
		// 'history'
		CMsgEMPeekReply peekMsg(msg.getMsgBody());
		NLMISC::InfoLog->displayNL("Peek installed event received reply [%s:%d]",shardName.c_str(),msg.getSenderId());
		NLMISC::CVectorSString lines;
		peekMsg.getResultTxt().splitLines(lines);
		NLMISC::WarningLog->displayNL("History:");
		for (uint32 i=0;i<lines.size();++i)
		{
			NLMISC::InfoLog->displayNL("%4d: %s",i,lines[i].c_str());
		}
		NLMISC::WarningLog->displayNL("Files:");
		for (uint32 i=0;i<peekMsg.getFdc().size();++i)
		{
			NLMISC::InfoLog->displayNL("%4d: %-30s %10d %s",i,peekMsg.getFdc()[i].FileName.c_str(),peekMsg.getFdc()[i].FileSize,IDisplayer::dateToHumanString(peekMsg.getFdc()[i].FileTimeStamp));
		}
	}
}
	
NLMISC::CSString CEventManagerImplementation::getState() const
{
	return getName()+" "+getParameters();
}

NLMISC::CSString CEventManagerImplementation::getName() const
{
	return "EM";
}

NLMISC::CSString CEventManagerImplementation::getParameters() const
{
	return "";
}

void CEventManagerImplementation::displayModule() const
{
	// display contents of _Shards
	NLMISC::InfoLog->displayNL("Detected Shards:");
	for (TShards::const_iterator it=_Shards.begin(); it!=_Shards.end(); ++it)
	{
		CSString s= (*it).first+ ": ";
		for (TShardsMapEntry::const_iterator it2= (*it).second.begin(); it2!=(*it).second.end(); ++it2)
		{
			s+= NLMISC::toString((*it2))+" ";
		}
		NLMISC::InfoLog->displayNL("- %s",s.c_str());
	}

	// display _Logins
	NLMISC::InfoLog->displayNL("Logged in shards:");
	for (TLogins::const_iterator it=_Logins.begin(); it!=_Logins.end(); ++it)
	{
		NLMISC::InfoLog->displayNL("- shard: %s  user: %s  accepted logins: %d",(*it).first.c_str(),(*it).second.UserId.c_str(),(*it).second.ValidLoginCount);
	}
}


//-----------------------------------------------------------------------------
// methods CEventManagerImplementation / CEventManager
//-----------------------------------------------------------------------------

void CEventManagerImplementation::login(const NLMISC::CSString& shardName,const NLMISC::CSString& userId,const NLMISC::CSString& password)
{
	// make sure we don't try to change login for a shard that's already correctly logged
	DROP_IF(_Logins.find(shardName)!=_Logins.end() && _Logins[shardName].ValidLoginCount!=0,"You may not change a login once you are already logged in",return);

	// store the user id and password for later use
	_Logins[shardName].UserId= userId;
	_Logins[shardName].Password= password;
	_Logins[shardName].ValidLoginCount= 0;

	// see whether there are any connected executors for the given shard name
	if (_Shards.find(shardName)!=_Shards.end())
	{
		// build vector of destinations to send the module message to
		TModuleIdVector moduleIds;
		for (TShardsMapEntry::const_iterator it= _Shards[shardName].begin(); it!= _Shards[shardName].end(); ++it)
		{
			nlinfo("Sending login request to event executor: %d",(*it));
			moduleIds.push_back(*it);
		}
		// dispatch a unique message and let gusnet system route it to the ee modules as required
		CMsgEELogin loginMsg(_Logins[shardName].UserId,_Logins[shardName].Password);
		sendModuleMessage(loginMsg,moduleIds,this);
	}
	else
	{
		// display a quick warning incase user miss-typed the shard name at login time
		NLMISC::WarningLog->displayNL("No event executor modules are currently connected for id: %s",shardName.c_str());
	}
}

void CEventManagerImplementation::upload(const NLMISC::CSString& shardName,const NLMISC::CSString& eventName,const CFileDescriptionContainer& fdc,const NLMISC::CVectorSString& fileBodies)
{
	// make sure that there are connected executors for the given shard name
	if (_Logins.find(shardName)==_Logins.end() || _Logins[shardName].ValidLoginCount != _Shards[shardName].size())
	{
		NLMISC::WarningLog->displayNL("Cannot upload data as login has not been completed for all connected executors: %s",shardName.c_str());
		return;
	}

	if (_Shards.find(shardName)==_Shards.end())
	{
		NLMISC::WarningLog->displayNL("Cannot upload data as no connections found to shard: %s",shardName.c_str());
		return;
	}

	// build vector of destinations to send the module message to
	TModuleIdVector moduleIds;
	for (TShardsMapEntry::iterator it= _Shards[shardName].begin(); it!= _Shards[shardName].end(); ++it)
	{
		nlinfo("Uploading event data to event executor: %d",(*it));
		moduleIds.push_back(*it);
	}

	// dispatch a unique message and let gusnet system route it to the ee modules as required
	CMsgEEUpload uploadMsg(_Logins[shardName].UserId,_Logins[shardName].Password,eventName,fdc,fileBodies);
	sendModuleMessage(uploadMsg,moduleIds,this);
}

//void CEventManagerImplementation::restartShard(const NLMISC::CSString& shardName)
//{
//	// make sure that there are connected executors for the given shard name
//	if (_Logins.find(shardName)==_Logins.end() || _Logins[shardName].ValidLoginCount != _Shards[shardName].size())
//	{
//		NLMISC::WarningLog->displayNL("Cannot restart shard as login has not been completed for all connected executors: %s",shardName.c_str());
//		return;
//	}
//
//	if (_Shards.find(shardName)==_Shards.end())
//	{
//		NLMISC::WarningLog->displayNL("Cannot restart shard as no connections found: %s",shardName.c_str());
//		return;
//	}
//
//	// build vector of destinations to send the module message to
//	TModuleIdVector moduleIds;
//	for (TShardsMapEntry::iterator it= _Shards[shardName].begin(); it!= _Shards[shardName].end(); ++it)
//	{
//		nlinfo("Sending shard restart request to event executor: %d",(*it));
//		moduleIds.push_back(*it);
//	}
//
//	// dispatch a unique message and let gusnet system route it to the ee modules as required
//	CMsgEERestartShard restartMsg(_Logins[shardName].UserId,_Logins[shardName].Password);
//	sendModuleMessage(restartMsg,moduleIds,this);
//}

void CEventManagerImplementation::startEvent(const NLMISC::CSString& shardName)
{
	// lookup the logins and shards map entries
	TLogins::const_iterator login= _Logins.find(shardName);
	TShards::const_iterator shard= _Shards.find(shardName);

	// make sure that there are connected executors for the given shard name
	DROP_IF(shard==_Shards.end(),"Cannot request event start as no connections found to shard: "+shardName,return);
	DROP_IF(login==_Logins.end() || (*login).second.ValidLoginCount!=(*shard).second.size(), 
		"Cannot request event start as login has not been completed for all connected executors: "+shardName,return);

	// build vector of destinations to send the module message to
	TModuleIdVector moduleIds;
	for (TShardsMapEntry::const_iterator it= (*shard).second.begin(); it!= (*shard).second.end(); ++it)
	{
		nlinfo("Sending event start request to event executor: %d",(*it));
		moduleIds.push_back(*it);
	}

	// dispatch a unique message and let gusnet system route it to the ee modules as required
	CMsgEEEventStart startShardMsg((*login).second.UserId,(*login).second.Password);
	sendModuleMessage(startShardMsg,moduleIds,this);
}

void CEventManagerImplementation::stopEvent(const NLMISC::CSString& shardName)
{
	// lookup the logins and shards map entries
	TLogins::const_iterator login= _Logins.find(shardName);
	TShards::const_iterator shard= _Shards.find(shardName);

	// make sure that there are connected executors for the given shard name
	DROP_IF(shard==_Shards.end(),"Cannot request event stop as no connections found to shard: "+shardName,return);
	DROP_IF(login==_Logins.end() || (*login).second.ValidLoginCount!=(*shard).second.size(), 
		"Cannot request event stop as login has not been completed for all connected executors: "+shardName,return);

	// build vector of destinations to send the module message to
	TModuleIdVector moduleIds;
	for (TShardsMapEntry::const_iterator it= (*shard).second.begin(); it!= (*shard).second.end(); ++it)
	{
		nlinfo("Sending event stop request to event executor: %d",(*it));
		moduleIds.push_back(*it);
	}

	// dispatch a unique message and let gusnet system route it to the ee modules as required
	CMsgEEEventStop stopShardMsg((*login).second.UserId,(*login).second.Password);
	sendModuleMessage(stopShardMsg,moduleIds,this);
}

void CEventManagerImplementation::updateTools()
{
	NLMISC::CSString shardName= ToolsShardName.get();

	// make sure that there are connected executors for the given shard name
	if (_Logins.find(shardName)==_Logins.end() || _Logins[shardName].ValidLoginCount != _Shards[shardName].size())
	{
		NLMISC::WarningLog->displayNL("Cannot restart shard as login has not been completed for all connected executors: %s",shardName.c_str());
		return;
	}

	if (_Shards.find(shardName)==_Shards.end())
	{
		NLMISC::WarningLog->displayNL("Cannot restart shard as no connections found: %s",shardName.c_str());
		return;
	}

	TModuleIdVector moduleIds;
	for (TShardsMapEntry::iterator it= _Shards[shardName].begin(); it!= _Shards[shardName].end(); ++it)
	{
		moduleIds.push_back(*it);
	}

	switch (moduleIds.size())
	{
	case 0:
		nlwarning("Failed to launch tools update as there are no connected modules that claim to manage the tools repository");
		return;

	case 1:
		nlinfo("Sending tools update request to event executor: %d",moduleIds[0]);
		break;

	default:
		nlwarning("Failed to launch tools update as there are more than 1 connected modules that claim to manage the tools repository!");
		return;
	}

	// dispatch a message and let gusnet system route it to the ee module as required
	CMsgEEToolsUpdReq updateReqMsg(_Logins[shardName].UserId,_Logins[shardName].Password);
	sendModuleMessage(updateReqMsg,moduleIds,this);
}

void CEventManagerImplementation::peekInstalledEvent(const NLMISC::CSString& shardName) const
{
	// lookup the logins and shards map entries
	TLogins::const_iterator login= _Logins.find(shardName);
	TShards::const_iterator shard= _Shards.find(shardName);

	// make sure that there are connected executors for the given shard name
	DROP_IF(shard==_Shards.end(),"Cannot request history as no connections found to shard: "+shardName,return);
	DROP_IF(login==_Logins.end() || (*login).second.ValidLoginCount!=(*shard).second.size(), 
		"Cannot request history as login has not been completed for all connected executors: "+shardName,return);

	// build vector of destinations to send the module message to
	TModuleIdVector moduleIds;
	for (TShardsMapEntry::const_iterator it= (*shard).second.begin(); it!= (*shard).second.end(); ++it)
	{
		nlinfo("Sending history request to event executor: %d",(*it));
		moduleIds.push_back(*it);
	}

	// dispatch a unique message and let gusnet system route it to the ee modules as required
	CMsgEEPeek historyMsg((*login).second.UserId,(*login).second.Password);
	sendModuleMessage(historyMsg,moduleIds,this);
}

void CEventManagerImplementation::getShards(NLMISC::CVectorSString& shardNames) const
{
	shardNames.clear();
	for (TLogins::const_iterator it=_Logins.begin(); it!=_Logins.end(); ++it)
	{
		shardNames.push_back((*it).first);
	}
}


//-----------------------------------------------------------------------------
// methods CEventManager
//-----------------------------------------------------------------------------

CEventManager* CEventManager::getInstance()
{
	return CEventManagerImplementation::getInstance();
}


//-----------------------------------------------------------------------------
// Register the module
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CEventManagerImplementation,CEventManagerImplementation::getInstance()->getName(),"","Event manager")



//-----------------------------------------------------------------------------
