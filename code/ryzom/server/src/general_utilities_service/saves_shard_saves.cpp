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
#include "saves_module_messages.h"
#include "rs_module_messages.h"
#include "saves_unit.h"
#include "gus_utils.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// class CShardSaves
	//-----------------------------------------------------------------------------

	class CShardSaves: public GUS::IModule
	{
	public:
		// IModule specialisation implementation
		bool initialiseModule(const NLMISC::CSString& rawArgs);
		void serviceUpdate(NLMISC::TTime localTime);
		void receiveModuleMessage(GUSNET::CModuleMessage& msg);

		NLMISC::CSString getState() const;
		NLMISC::CSString getName() const;
		NLMISC::CSString getParameters() const;
		void displayModule() const;

	public:

		// remaining public interface
		void processNextMessage();
		void processMsgRegister		(uint32 sender,CMsgSavesRegister&	msgBody);
		void processMsgUnregister	(uint32 sender,CMsgSavesUnregister& msgBody);
		void processMsgFileRequest	(uint32 sender,CMsgSavesFileRequest& msgBody);
		void processMsgUpload		(uint32 sender,CMsgSavesUpload&		msgBody);
		void processMsgDelete		(uint32 sender,CMsgSavesDelete&		msgBody);
		void processMsgMove			(uint32 sender,CMsgSavesMove&		msgBody);

	private:
		CSString	_ShardName;
		CSString	_Path;
		CSString	_Type;
		CSavesUnit	_Unit;
		uint32		_UpdateCounter;

		typedef vector<TModuleMessagePtr> TMessages;
		TMessages _Messages;

		TModuleIdVector _Subscribers;
	};


	//-----------------------------------------------------------------------------
	// methods CShardSaves - remaining public interface
	//-----------------------------------------------------------------------------

	void CShardSaves::processNextMessage()
	{
		// if there's nothing to do then just return
		if (_Messages.empty())
			return;

		// pop the front element of the pending messages queue
		TModuleMessagePtr msg=_Messages.front();
		_Messages.erase(_Messages.begin());

		// treat the message...
		if (msg->getMessageName()==CMsgSavesRegister().getName())
		{
			CMsgSavesRegister msgBody(msg->getMsgBody());
			processMsgRegister(msg->getSenderId(),msgBody);
		}
		else if (msg->getMessageName()==CMsgSavesUnregister().getName())
		{
			CMsgSavesUnregister msgBody(msg->getMsgBody());
			processMsgUnregister(msg->getSenderId(),msgBody);
		}
		else if (msg->getMessageName()==CMsgSavesFileRequest().getName())
		{
			CMsgSavesFileRequest msgBody(msg->getMsgBody());
			processMsgFileRequest(msg->getSenderId(),msgBody);
		}
		else if (msg->getMessageName()==CMsgSavesUpload().getName())
		{
			CMsgSavesUpload msgBody(msg->getMsgBody());
			processMsgUpload(msg->getSenderId(),msgBody);
		}
		else if (msg->getMessageName()==CMsgSavesDelete().getName())
		{
			CMsgSavesDelete msgBody(msg->getMsgBody());
			processMsgDelete(msg->getSenderId(),msgBody);
		}
		else if (msg->getMessageName()==CMsgSavesMove().getName())
		{
			CMsgSavesMove msgBody(msg->getMsgBody());
			processMsgMove(msg->getSenderId(),msgBody);
		}
	}

	void CShardSaves::processMsgRegister(uint32 sender,CMsgSavesRegister& msgBody)
	{
		nlinfo("SAVES module %s %s: Received Subscribe from module: %d",getParameters().word(0).c_str(),getParameters().word(1).c_str(),sender);

		// make sure the sender isn't already in the subscribers list
		for (uint32 i=0;i<_Subscribers.size();++i)
		{
			if (_Subscribers[i]==sender)
			{
				nlwarning("Ignoring attempt to register same remote module more than once");
				CMsgRSGenericReply msg(~0u,false,"Unable to treat 'register' message because module is already registered");
				sendModuleMessage(msg,sender,this);
				return;
			}
		}

		// add the sender to the subscribers list
		_Subscribers.push_back(sender);

		// build a new file description container
		CFileDescriptionContainer fdc;
		_Unit.getFileList(fdc);

		// send messages round to the subscribers...
		CMsgRSInit msg(fdc);
		sendModuleMessage(msg,sender,this);
	}

	void CShardSaves::processMsgUnregister(uint32 sender,CMsgSavesUnregister& msgBody)
	{
		nlinfo("Received Unsubscribe from module: %d",sender);
		for (uint32 i=_Subscribers.size();i--;)
		{
			if (_Subscribers[i]==sender)
			{
				_Subscribers[i]=_Subscribers.back();
				_Subscribers.pop_back();
				nlinfo("- Unsubscribing module: %d",sender);
			}
		}

	}

	void CShardSaves::processMsgFileRequest(uint32 sender,CMsgSavesFileRequest& msgBody)
	{
		// *** todo ***
		// add log info on who (user) what (action) when where (any info on ip address, etc)

		nlinfo("Received Request from module: %d for file download: %s with id %d",sender,msgBody.getFileName().c_str(),msgBody.getRequestId());
		CSString filePath= cleanPath(msgBody.getFileName(),false);

		// make sure we find the file in our directory tree
		if (filePath.left(1)=="." || filePath.left(1)=="/")
		{
			nlwarning("Request to download file refused because file not in directory tree: %s",msgBody.getFileName().c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to get file due to illegal path: "+msgBody.getFileName());
			sendModuleMessage(msg,sender,this);
			return;
		}

		// read the file
		CSString fileBody;
		fileBody.readFromFile(_Path+filePath);

		// compose a message and send it back to the remote module
		CMsgRSDownload msg(msgBody.getRequestId(),filePath,fileBody);
		sendModuleMessage(msg,sender,this);
		nlinfo("sent download file to module %d: %s (%d bytes) with id: %d",sender,filePath.c_str(),fileBody.size(),msgBody.getRequestId());
	}

	void CShardSaves::processMsgUpload(uint32 sender,CMsgSavesUpload& msgBody)
	{
		// *** todo ***
		// add log info on who (user) what (action) when where (any info on ip address, etc)

		nlinfo("Received Request from module: %d for file upload: %s (%d bytes)",
			sender,msgBody.getFileName().c_str(),msgBody.getFileBody().size());
		CSString filePath= cleanPath(msgBody.getFileName(),false);

		// make sure we find the file in our directory tree
		if (filePath.left(1)=="." || filePath.left(1)=="/")
		{
			nlwarning("Request to upload file refused because file not in directory tree: %s",filePath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to upload file due to illegal path: "+filePath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// write the message content to a file
		msgBody.getFileBody().writeToFile(_Path+filePath);

		// send a confirmation message back to the requestor
		CMsgRSGenericReply msg(msgBody.getRequestId(),true,"Upload file done: "+filePath);
		sendModuleMessage(msg,sender,this);
	}

	void CShardSaves::processMsgDelete(uint32 sender,CMsgSavesDelete& msgBody)
	{
		// *** todo ***
		// add log info on who (user) what (action) when where (any info on ip address, etc)

		nlinfo("Received Request from module: %d for file delete: %s",sender,msgBody.getFileName().c_str());
		CSString filePath= cleanPath(msgBody.getFileName(),false);

		// make sure we find the file in our directory tree
		if (filePath.left(1)=="." || filePath.left(1)=="/")
		{
			nlwarning("Request to delete file refused because file not in directory tree: %s",filePath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to delete file due to illegal path: "+filePath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// make sure the file exists
		if (!CFile::fileExists(_Path+filePath))
		{
			nlwarning("Request to delete file failed because file not found: %s",filePath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to delete file because file not found: "+filePath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// do the deleting
		nlinfo("Treating request to delete file: %s",filePath.c_str());
		CFile::deleteFile(_Path+filePath);

		// make sure the file no longer exists
		if (CFile::fileExists(_Path+filePath))
		{
			nlwarning("Request to delete file failed for unknown reason: %s",filePath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to delete file for unknown reason: "+filePath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// send a confirmation message back to the requestor
		CMsgRSGenericReply msg(msgBody.getRequestId(),true,"Delete file done: "+filePath);
		sendModuleMessage(msg,sender,this);
	}

	void CShardSaves::processMsgMove(uint32 sender,CMsgSavesMove& msgBody)
	{
		// *** todo ***
		// add log info on who (user) what (action) when where (any info on ip address, etc)

		nlinfo("Received Request from module: %d for file move: %s to: %s",
			sender,msgBody.getFileName().c_str(),msgBody.getDestination().c_str());
		CSString srcPath= cleanPath(msgBody.getFileName(),false);
		CSString dstPath= cleanPath(msgBody.getDestination(),false);

		// make sure we find the file in our directory tree
		if (srcPath.left(1)=="." || srcPath.left(1)=="/" || dstPath.left(1)=="." || dstPath.left(1)=="/")
		{
			nlwarning("Request to move file refused because src or dst paths not in directory tree: %s, %s",srcPath.c_str(),dstPath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to move file due to illegal path: "+srcPath+", "+dstPath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// make sure the src file exists
		if (!CFile::fileExists(_Path+srcPath))
		{
			nlwarning("Request to move file failed because file not found: %s",srcPath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to delete file because file not found: "+srcPath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// make sure the dst file doesn't exist
		if (CFile::fileExists(_Path+dstPath))
		{
			nlwarning("Request to move file failed because files exists with destination name: %s",dstPath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to move file because files exists with destination name: "+dstPath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// do the moving
		nlinfo("Treating request to move file: %s => %s",srcPath.c_str(),dstPath.c_str());
		CFile::moveFile((_Path+dstPath).c_str(),(_Path+srcPath).c_str());

		// make sure the src file no longer exists and that a destination file now exists
		if (CFile::fileExists(_Path+srcPath) && !CFile::fileExists(_Path+dstPath))
		{
			nlwarning("Request to move file failed for unknown reason: %s, %s",srcPath.c_str(),dstPath.c_str());
			CMsgRSGenericReply msg(msgBody.getRequestId(),false,"Failed to move file for unknown reason: "+srcPath+", "+dstPath);
			sendModuleMessage(msg,sender,this);
			return;
		}

		// send a confirmation message back to the requestor
		CMsgRSGenericReply msg(msgBody.getRequestId(),true,"Move file done: "+srcPath+", "+dstPath);
		sendModuleMessage(msg,sender,this);
	}


	//-----------------------------------------------------------------------------
	// methods CShardSaves - IModule implementation
	//-----------------------------------------------------------------------------

	bool CShardSaves::initialiseModule(const NLMISC::CSString& rawArgs)
	{
		// decorticate command line args
		NLMISC::CSString args= rawArgs;
		_ShardName= args.firstWord(true).strip();
		_Type= args.firstWord(true).strip();
		_Path= cleanPath(args.strip().unquoteIfQuoted().strip(),true);
		nlinfo("initialising SAVES module => Directory (%s) => (%s) => (%s)",args.c_str(),args.strip().unquoteIfQuoted().c_str(),_Path.c_str());

		// check for syntax errors in args
		DROP_IF(args.strip().empty(),"syntax error. Syntax: modulesAdd saves <shard name> shard|www|bak <path>",return false);
		if (_Type=="shard" )	_Unit.init(_Path,CSavesUnit::SHARD);
		else if (_Type=="bak" )	_Unit.init(_Path,CSavesUnit::BAK);
		else if (_Type=="www" )	_Unit.init(_Path,CSavesUnit::WWW);
		else					DROP("syntax error. Syntax: modulesAdd saves <shard name> shard|www|bak <path>",return false);

		return true;
	}

	void CShardSaves::serviceUpdate(NLMISC::TTime localTime)
	{

		// have the unit perform its own update once every 2 ticks
		if(_UpdateCounter++%2==0)
			_Unit.update();

		// see whether we have anything important to send to our subscribers
		TMsgRSUpdatePtr msgPtr= _Unit.popNextChangeSet();
		if (!msgPtr->empty())
		{
			sendModuleMessage(*msgPtr,_Subscribers,this);
		}

		// process the next waiting message in the waiting message queue
		processNextMessage();
	}

	void CShardSaves::receiveModuleMessage(GUSNET::CModuleMessage& msg)
	{
		_Messages.push_back(&msg);
	}

	NLMISC::CSString CShardSaves::getState() const
	{
		return getName()+" "+getParameters();
	}

	NLMISC::CSString CShardSaves::getName() const
	{
		return "SAVES";
	}

	NLMISC::CSString CShardSaves::getParameters() const
	{
		return _ShardName+" "+_Type+" "+_Path.quoteIfNotAtomic(false);
	}

	void CShardSaves::displayModule() const
	{
		// *** todo ***
	}


	//-----------------------------------------------------------------------------
	// CShardSaves registration
	//-----------------------------------------------------------------------------

	REGISTER_GUS_MODULE(CShardSaves,"SAVES","<shard name> shard|www|bak <path>","Save files server (operates with RS modules)")
}
