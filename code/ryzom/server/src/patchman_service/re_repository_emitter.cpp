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
#include "nel/misc/md5.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"

// local
#include "administered_module.h"
#include "rr_module_itf.h"
#include "re_module_itf.h"
//#include "repository.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CRepositoryEmitter;


//-------------------------------------------------------------------------------------------------
// constants
//-------------------------------------------------------------------------------------------------

NLMISC::CVariable<uint32> REMaxMessageCount("re","REMaxMessageCount", "number of opponent one can handle without defense malus", 100, 0, true );
NLMISC::CVariable<uint32> REDataBlockSize("re","REDataBlockSize", "number of opponent one can handle without defense malus", 10480, 0, true );


//-----------------------------------------------------------------------------
// class CREConnectedReceiver
//-----------------------------------------------------------------------------

class CREConnectedReceiver: public NLMISC::CRefCount
{
	friend class CRepositoryEmitter;
public:
	// ctor
	CREConnectedReceiver(CRepositoryEmitter* parent, IModuleProxy *module);

	// add a file to the request list
	void addFile(const NLMISC::CSString& fileName);

	// update download a bit more file to the connected receiver....
	void update();

	// process an ack message from the receiver
	void processAck(const NLMISC::CSString& fileName,bool status);

	// return true if there is a file in the process of being downloaded or there are pending file requests
	bool isBusy();

	// simple read accessors
	uint32 getNumDataBlocks() const;
	uint32 getNumPendingFiles() const;
	const NLMISC::CSString& getCurrentFile() const;
	const NLMISC::CSString& getSendCounterLog() const;

private:
	void _readNextFile();

	typedef std::list<std::string> TDataBlocks;
	typedef std::list<std::string> TPendingFileNames;

	TModuleProxyPtr		_ModuleProxy;
	CRepositoryEmitter* _Parent;

	TPendingFileNames	_PendingFileNames;
	NLMISC::CSString	_CurrentFileName;
	uint32				_CurrentFileSize;
	TDataBlocks			_DataBlocks;

	uint32				_SendCount;
	uint32				_AckCount;

	NLMISC::CSString	_SendCounterLog;
};


//-----------------------------------------------------------------------------
// class CRepositoryEmitter
//-----------------------------------------------------------------------------

class CRepositoryEmitter : 
	public CAdministeredModuleBase,
	public CRepositoryEmitterSkel
{
public:
	// CModuleBase specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg);
	void onModuleUpdate();
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	std::string buildModuleManifest() const;
	void displayModule() const;

	static const std::string &getHelperString()
	{
		static string help = "name=<emitter name> path=<target directory>";
		return help;
	}

	virtual bool isImmediateDispatchingSupported() const { return false; }

public:
	// remaining public interface
	CRepositoryEmitter();
	const NLMISC::CSString& getTargetDirectory() const;

private:
	// pivate methods
	// 
	virtual void requestFile(NLNET::IModuleProxy *sender, const std::string &fileName);
	// 
	virtual void fileDataAck(NLNET::IModuleProxy *sender, const std::string &fileName, bool status);
	// 
	virtual void duplicateModuleError(NLNET::IModuleProxy *sender);

private:
	// private data
	NLMISC::CSString _Name;
	mutable NLMISC::CSString _Manifest;
	NLMISC::CSString _TargetDirectory;
	NLMISC::CSString _CurrentVersion;
	CRepository		_Repository;

	typedef NLMISC::CSmartPtr<CREConnectedReceiver> TConnectedReceiverPtr;
	typedef std::map<IModuleProxy*, TConnectedReceiverPtr> TConnectedReceivers;
	TConnectedReceivers _ConnectedReceivers;

	NLMISC::TTime _LastUpdateTime;

	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CRepositoryEmitter, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CRepositoryEmitter, dump, "Dump the current emitter status", "no args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
};


//-----------------------------------------------------------------------------
// methods CREConnectedReceiver
//-----------------------------------------------------------------------------

CREConnectedReceiver::CREConnectedReceiver(CRepositoryEmitter* parent, IModuleProxy *module)
{
	_Parent= parent;
	_ModuleProxy = module;
	_SendCount= 0;
	_AckCount= 0;
	_CurrentFileSize= 0;
	_SendCounterLog="::::::::::::::::::::";
}

void CREConnectedReceiver::addFile(const NLMISC::CSString& fileName)
{
	_PendingFileNames.push_back(fileName);
}

void CREConnectedReceiver::update()
{
	CRepositoryReceiverProxy rr(_ModuleProxy);

	// if there's nothing to do then skip this update
	if (_PendingFileNames.empty() && _DataBlocks.empty())
	{
		_SendCounterLog= _SendCounterLog.splitFrom(':')+':';
		return;
	}

	// work out how many data messages we should send this update
	uint32 numMessagesToSend= REMaxMessageCount- _SendCount+ _AckCount;

	// add a value to the logger
	_SendCounterLog= _SendCounterLog.splitFrom(':')+NLMISC::toString("%d:",numMessagesToSend);

	while (numMessagesToSend>0)
	{
		// if it's time to start work on a new file then do it...
		if (_DataBlocks.empty())
		{
			// if there are no more files to process then we're all done
			if (_PendingFileNames.empty())
				return;
			// read the next file into the data blocks vector
			_readNextFile();

			// send a 'start of file' message
			rr.beginFile(_Parent, _CurrentFileName, _CurrentFileSize);
		}

		// if the current file isn't empty then send the next data block...
		if (!_DataBlocks.empty())
		{
			// send a 'file data' message
			rr.fileData(_Parent, _CurrentFileName, _DataBlocks.front());
			_DataBlocks.pop_front();
			++_SendCount;
			--numMessagesToSend;
		}

		// if we've just dealt with the last data block for this file then dispatch an end of file message
		if (_DataBlocks.empty())
		{
			// send an 'end of file' message
			rr.fileEnd(_Parent, _CurrentFileName);
			_CurrentFileName.clear();
			_CurrentFileSize=0;
		}
	}
}

void CREConnectedReceiver::processAck(const NLMISC::CSString& fileName,bool status)
{
	++_AckCount;

	// if there was an error with the current file then abort sending of the rest of its data
	if (status==false && fileName==_CurrentFileName)
	{
		// forget any remaining unsent data blocks
		_DataBlocks.clear();

		// send an 'end of file' message
		CRepositoryReceiverProxy rr(_ModuleProxy);
		rr.fileEnd(_Parent, _CurrentFileName);
		_CurrentFileName.clear();
		_CurrentFileSize=0;
	}
}

bool CREConnectedReceiver::isBusy()
{
	return !_PendingFileNames.empty() || !_DataBlocks.empty();
}

uint32 CREConnectedReceiver::getNumDataBlocks() const
{
	return _DataBlocks.size();
}

uint32 CREConnectedReceiver::getNumPendingFiles() const
{
	return _PendingFileNames.size();
}

const NLMISC::CSString& CREConnectedReceiver::getCurrentFile() const
{
	return _CurrentFileName;
}

const NLMISC::CSString& CREConnectedReceiver::getSendCounterLog() const
{
	return _SendCounterLog;
}

void CREConnectedReceiver::_readNextFile()
{
	// if there's nothing to do then do nothing!
	if (_PendingFileNames.empty())
		return;

	// make sure there isn't already data being sent...
	BOMB_IF(!_DataBlocks.empty(),"Cannot read next file because there are already data blocks from a previous file in the send queue...",return);

	// extract the next entry from the pending file list
	_CurrentFileName= _PendingFileNames.front();
	_PendingFileNames.pop_front();

	// build the file name
	CSString fileName= _Parent->getTargetDirectory()+ _CurrentFileName;

	// display a little message to say what we're doing
	nldebug("VERBOSE_RE Treating request for file: %s",_CurrentFileName.c_str());

	// read the file from disk
	NLMISC::CSString fileData;
	bool readOk= fileData.readFromFile(fileName);
	DROP_IF(!readOk,"Failed to read requested file: "+fileName,return);
	_CurrentFileSize= fileData.size();

	// chunk the file data into blocks
	uint32 numBlocks= (_CurrentFileSize+ REDataBlockSize-1)/REDataBlockSize;
	for (uint32 i=0;i<numBlocks;++i)
	{
		_DataBlocks.push_back(fileData.substr(i*REDataBlockSize,REDataBlockSize));
	}
}


//-----------------------------------------------------------------------------
// methods CRepositoryEmitter
//-----------------------------------------------------------------------------

CRepositoryEmitter::CRepositoryEmitter()
{
	_LastUpdateTime=0;
}

const NLMISC::CSString& CRepositoryEmitter::getTargetDirectory() const
{
	return _TargetDirectory;
}

bool CRepositoryEmitter::initModule(const TParsedCommandLine &initInfo)
{
	bool ret = CModuleBase::initModule(initInfo);

	const TParsedCommandLine *targetDir = initInfo.getParam("path");
//	DROP_IF(targetDir == NULL,"path() parameter not found in command line", IModuleManager::getInstance().deleteModule(this); return;);
	BOMB_IF(targetDir == NULL,"path() parameter not found in command line - this is a BOMB because I can't return without leaking!", return false;);
	_TargetDirectory = CPath::standardizePath(targetDir->ParamValue, true);
	
	const TParsedCommandLine *name = initInfo.getParam("fileSet");
//	DROP_IF(name == NULL,"fileSet() parameter not found in command line", IModuleManager::getInstance().deleteModule(this); return;);
	BOMB_IF(name == NULL,"fileSet() parameter not found in command line - this is a BOMB because I can't return without leaking!", return false;);
	_Name= name->ParamValue;

	nlinfo("RE %s: Initialising with target directory: %s",_Name.c_str(),_TargetDirectory.c_str());
	ret &= _Repository.init(_Name,_TargetDirectory);

	// initialise the state variables
	setStateVariable("State","Initialising");
	setStateVariable("Connections","0");

	return false;
}

void CRepositoryEmitter::onModuleUp(IModuleProxy *module)
{
	// allow the base class a chance to do it's stuff
	CAdministeredModuleBase::onModuleUp(module);

	TParsedCommandLine pci;
	pci.parseParamList(module->getModuleManifest());

	if (module->getModuleClassName()=="RepositoryReceiver" 
		&& pci.getParam("name") != NULL
		&& pci.getParam("name")->ParamValue == _Name)
	{
		nlinfo("RE %s: Detecting connection of new RR module - sending off a copy of our file list...",_Name.c_str());
		CRepositoryReceiverProxy rr(module);
		
		std::vector<PATCHMAN::TFileRecord>	files;
		_Repository.fillShortList(files);

		rr.fileList(this, _Repository.getVersion(), files);

		BOMB_IF(_ConnectedReceivers.find(module) != _ConnectedReceivers.end(), "Trying to insert an already known receiver", return);
		_ConnectedReceivers[module]= new CREConnectedReceiver(this,module);

		setStateVariable("Connections",NLMISC::toString(_ConnectedReceivers.size()));
	}

	if (module->getModuleClassName()=="ServerPatchApplier")
	{
		nlinfo("RE %s: Detecting connection of new SPA module...",_Name.c_str());
		
		BOMB_IF(_ConnectedReceivers.find(module) != _ConnectedReceivers.end(), "Trying to insert an already known receiver", return);
		_ConnectedReceivers[module]= new CREConnectedReceiver(this,module);

		setStateVariable("Connections",NLMISC::toString(_ConnectedReceivers.size()));
	}
}

void CRepositoryEmitter::onModuleDown(IModuleProxy *module)
{
	// allow the base class a chance to do it's stuff
	CAdministeredModuleBase::onModuleDown(module);

	if (_ConnectedReceivers.find(module) != _ConnectedReceivers.end())
	{
		nlinfo("RE %s: Detecting deconnection of RR module '%s'",_Name.c_str(), module->getModuleName().c_str());

		_ConnectedReceivers.erase(module);

		setStateVariable("Connections",NLMISC::toString(_ConnectedReceivers.size()));
	}
}

//void CRepositoryEmitter::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CRepositoryEmitterSkel::onDispatchMessage(sender, msg))
//		return;
//
//	if (CAdministeredModuleBase::onDispatchMessage(sender, msg))
//		return;
//
//	// unhandled message....
//
//	BOMB("CRepositoryEmitter::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'", return);
//
//}
	
void CRepositoryEmitter::onModuleUpdate()
{
	H_AUTO(CRepositoryEmitter_onModuleUpdate);

	// set a little state variable and broadcast the update to all listening SPMs
	setStateVariable("State","Idle");

	// allow the base class a chance to do it's stuff
	CAdministeredModuleBase::onModuleUpdate();

	NLMISC::TTime localTime = NLMISC::CTime::getLocalTime();
	// iterate over the connected receivers calling their updates
	bool isConnectedReceiverBusy=false;
	TConnectedReceivers::iterator it= _ConnectedReceivers.begin();
	TConnectedReceivers::iterator itEnd= _ConnectedReceivers.end();
	uint32 busyCount=0;
	CSString sendLog;
	for (;it!=itEnd;++it)
	{
		it->second->update();
		sendLog= (sendLog.empty()? "": (sendLog+" - ")) + it->second->getSendCounterLog();
		isConnectedReceiverBusy|= it->second->isBusy();
		busyCount += (it->second->isBusy()?1:0);
	}
	setStateVariable("SendLog",sendLog);
	setStateVariable("BusyConnections",NLMISC::toString(busyCount));

	// check whether it's time to rescan repository for changes
	// note: if any of the connected receivers are still processing messages then we use a much slower rescan frequency
	uint32 rescanPeriod= isConnectedReceiverBusy? 60*1000: 3*1000;
	if (localTime-_LastUpdateTime<rescanPeriod)
		return;

	// set a little state variable and broadcast the update to all listening SPMs
	setStateVariable("State","Rescanning repository");
	broadcastStateInfo();

//	nlinfo("RE Checking for updates (%dms since last update finished)...",localTime-lastTime);
	// check for changes in the directory that we watch over
	uint32 updates= _Repository.update();
	setStateVariable("RepositoryVersion",NLMISC::toString(_Repository.getVersion()));
	setStateVariable("RepositorySize",NLMISC::toString(_Repository.size()));

	// set a little state variable and broadcast the update to all listening SPMs
	setStateVariable("State","Processing repository update");
	broadcastStateInfo();

	// if updates were found then dispatch them to any and all known RR modules
	if (updates>0)
	{
		nlinfo("Sending updated file list out to %d connected repository receivers",_ConnectedReceivers.size());
		std::vector<PATCHMAN::TFileRecord>	files;
		_Repository.fillShortList(files);

		TConnectedReceivers::iterator first(_ConnectedReceivers.begin()), last(_ConnectedReceivers.end());
		for (; first != last; ++first)
		{
			TConnectedReceiverPtr &cr = first->second;

			CRepositoryReceiverProxy rr(cr->_ModuleProxy);
			rr.fileList(this, _Repository.getVersion(), files);
		}
	}

	_LastUpdateTime=localTime;
}

NLMISC::CSString CRepositoryEmitter::getState() const
{
	uint32 receiverCount=0;
	uint32 busyCount=0;
	TConnectedReceivers::const_iterator it= _ConnectedReceivers.begin();
	TConnectedReceivers::const_iterator itEnd= _ConnectedReceivers.end();
	for (;it!=itEnd;++it)
	{
		++receiverCount;
		if (it->second->isBusy())
			++busyCount;
	}

	return getName()+" "+_Name+": "+NLMISC::toString("Busy connections: %u/%u",busyCount,receiverCount);
}

NLMISC::CSString CRepositoryEmitter::getName() const
{
	return "RE";
}

std::string CRepositoryEmitter::buildModuleManifest() const
{
	_Manifest = string("path=")+_TargetDirectory+" name="+_Name;
	return _Manifest;
}

NLMISC_CLASS_COMMAND_IMPL(CRepositoryEmitter, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	log.displayNL("");
	log.displayNL("----------------------------------");
	log.displayNL("Dumping Repository state:");
	log.displayNL("----------------------------------");
	_Repository.display(log);
	log.displayNL("");
	log.displayNL("----------------------------------");
	log.displayNL("Dumping Connected Receiver states:");
	log.displayNL("----------------------------------");

	log.displayNL("List of %u conneted receivers:", _ConnectedReceivers.size());
	TConnectedReceivers::const_iterator it= _ConnectedReceivers.begin();
	TConnectedReceivers::const_iterator itEnd= _ConnectedReceivers.end();
	for (;it!=itEnd;++it)
	{
		nlinfo("Connection %d: Current File: \"%s\"  Pending Blocks: %d  Pending Files: %d",it->first,it->second->getCurrentFile().c_str(),it->second->getNumDataBlocks(),it->second->getNumPendingFiles());
	}
	log.displayNL("----------------------------------");
	return true;
}

void CRepositoryEmitter::requestFile(NLNET::IModuleProxy *sender, const std::string &fileName)
{
	nldebug("VERBOSE_RE Treating incoming file request: %s", fileName.c_str());

	// add the file name to the pending files list...
	BOMB_IF(_ConnectedReceivers.find(sender) == _ConnectedReceivers.end(), "Recevie file request from unregistered rr '"<<sender->getModuleName()<<"'", return);
	_ConnectedReceivers[sender]->addFile(fileName);
}

void CRepositoryEmitter::fileDataAck(NLNET::IModuleProxy *sender, const std::string &fileName, bool status)
{
	// have the appropriate 'connected receiver' object treat the ack
	BOMB_IF(_ConnectedReceivers.find(sender) == _ConnectedReceivers.end(), "Recevie file data ack from unregistered rr '"<<sender->getModuleName()<<"'", return);
	_ConnectedReceivers[sender]->processAck(fileName, status);
}

void CRepositoryEmitter::duplicateModuleError(NLNET::IModuleProxy *sender)
{
	BOMB("This is a duplicate module - it is time for it to commit suicide... just hasn't quite been implemented yet!",return);
}


//-----------------------------------------------------------------------------
// CRepositoryEmitter registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CRepositoryEmitter, "RepositoryEmitter");
