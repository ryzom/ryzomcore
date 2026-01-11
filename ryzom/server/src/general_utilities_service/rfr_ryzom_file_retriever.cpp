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

// game share
#include "game_share/utils.h"

// local
#include "gus_module.h"
#include "gus_module_factory.h"
#include "gus_module_manager.h"
#include "rfr_ryzom_file_retriever.h"
#include "remote_saves_interface.h"
#include "rs_remote_saves.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;
using namespace SAVES;


//-----------------------------------------------------------------------------
// class CRFRShardInterface
//-----------------------------------------------------------------------------

class CRFRShardInterface: public NLMISC::CRefCount
{
public:
	CRFRShardInterface(const CSString& shardName);
	~CRFRShardInterface();

	CShardSavesInterface&				getShardSavesInterface();
	CMailSavesInterface&				getWwwSavesInterface();
	CIncrementalBackupSavesInterface&	getBakSavesInterface();

	void addWatch(const CSString& fileName);
	void removeWatch(const CSString& fileName);
	const std::set<CSString>& getWatches() const;

	void addCharacterIdNameMapping(const CSString& name,uint32& account,uint32& slot);
	bool getCharIdFromName(const CSString& name,uint32& account,uint32& slot);

private:
	// the set of saves interface objects
	CSmartPtr<CShardSavesInterface>				_ShardSavesInterface;
	CSmartPtr<CMailSavesInterface>				_WwwSavesInterface;
	CSmartPtr<CIncrementalBackupSavesInterface>	_BakSavesInterface;

	// pointer to the callback object used to watch for file changes in core files
	TSavesCallbackPtr					_FileListCallbackPtr;

	// set of names of files that are being watched (with paths)
	typedef std::set<CSString>			TWatches;
	TWatches							_Watches;

	// set of correspondances between character names and account ids
	typedef std::map<CSString,uint32>	TCharacters;
	TCharacters							_Characters;

};
typedef NLMISC::CSmartPtr<CRFRShardInterface> TCompleteShardFileInterfacePtr;


//-----------------------------------------------------------------------------
// class CRyzomFileRetrieverImplementation
//-----------------------------------------------------------------------------

class CRyzomFileRetrieverImplementation
:	public CRyzomFileRetriever, 
	public IModule
{
public:
	// get hold of the singleton instance
	static CRyzomFileRetrieverImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CRyzomFileRetrieverImplementation();
	void clear();

public:
	// GUS::IModule methods
	bool initialiseModule(const CSString& rawArgs);
	void release();
	CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

	// public interface exposed via CRyzomFileRetriever class
	void useShard(const CSString& shardName);
	void stopUsingShard(const CSString& shardName);
	CVectorSString getUsedShardList();
	CShardSavesInterface* getShardSavesInterface(const CSString& shardName);
	CMailSavesInterface* getWwwSavesInterface(const CSString& shardName);
	CIncrementalBackupSavesInterface* getBakSavesInterface(const CSString& shardName);
	CVectorSString getSavesModules();
	bool uploadFile(const CSString& shardName,const CSString& localFileName,const CSString& remoteFileName);
	bool downloadFile(const CSString& shardName,const CSString& remoteFileName,const CSString& localFileName);
	bool downloadBackupFiles(const CSString& shardName,const CSString& fileName,const CSString& localDirectory);
	bool getCharIdFromName(const CSString& shardName,const CSString& name,uint32& account,uint32& slot);
	bool getAccountIdFromName(const CSString& name,uint32& account);

	// public interface extensions
	void addAccountIdNameMapping(const CSString& name,uint32& account);

private:
	// private data
	// module control - module parameters and flag to say whether module is active
	bool _IsActive;

	// a map of shard name to shard management records
	typedef std::map<CSString,TCompleteShardFileInterfacePtr> TShards;
	TShards _Shards;

	// set of correspondances between character names and account ids
	typedef std::map<CSString,uint32>	TAccounts;
	TAccounts							_Accounts;
};


//-----------------------------------------------------------------------------
// class CRFRDownloadCallback
//-----------------------------------------------------------------------------

class CRFRDownloadCallback: public ISavesFileReceiveCallback
{
public:
	// ctor
	CRFRDownloadCallback(uint32 requestId,const CSString& destFileName);

	// specialisation of methods from class ISavesFileReceiveCallback
	void cbFileReceived(uint32 requestId,const CSString& fileName,const CSString& fileBody);
	void cbGenericReply(uint32 requestId,bool successFlag,const CSString& explanation);

private:
	uint32				_RequestId;
	CSString			_DestFileName;

	// a smart ptr to self to control auto deletion
	TSavesCallbackPtr	_SelfPtr;
};


//-----------------------------------------------------------------------------
// class CRFRUploadCallback
//-----------------------------------------------------------------------------

class CRFRUploadCallback: public ISavesFileReceiveCallback
{
public:
	// ctor
	CRFRUploadCallback(uint32 requestId,const CSString& srcFileName);

	// specialisation of methods from class ISavesFileReceiveCallback
	void cbFileReceived(uint32 requestId,const CSString& fileName,const CSString& fileBody);
	void cbGenericReply(uint32 requestId,bool successFlag,const CSString& explanation);

private:
	uint32		_RequestId;
	CSString	_SrcFileName;

	// a smart ptr to self to control auto deletion
	TSavesCallbackPtr	_SelfPtr;
};


//-----------------------------------------------------------------------------
// class CRFRFileListCallback
//-----------------------------------------------------------------------------

class CRFRFileListCallback: public ISavesCallback
{
public:
	// ctor
	CRFRFileListCallback(CRFRShardInterface* shardFileInterface);

	// specialisation of methods from class ISavesFileListCallback
	void cbInit(const CFileDescriptionContainer& fdc);
	void cbFileListChanged(	const CFileDescriptionContainer& newFiles,
							const CFileDescriptionContainer& modifiedFiles,
							const NLMISC::CVectorSString&	 deletedFile,
							const CFileDescriptionContainer& oldFileList,
							const CFileDescriptionContainer& newFileList);
	void cbFileReceived(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody);
	void cbGenericReply(uint32 requestId,bool successFlag,const NLMISC::CSString& explanation);

private:
	NLMISC::CRefPtr<CRFRShardInterface> _ShardFileInterface;

	uint32 _AccountFileRequest;
	uint32 _CharacterFileRequest;
};


//-----------------------------------------------------------------------------
// methods CRFRShardInterface
//-----------------------------------------------------------------------------

CRFRShardInterface::CRFRShardInterface(const CSString& shardName)
{
	// setup the set of interface objects
	_ShardSavesInterface=	new CShardSavesInterface				(shardName);
	_WwwSavesInterface=		new CMailSavesInterface					(shardName);
	_BakSavesInterface=		new CIncrementalBackupSavesInterface	(shardName);

	// put together a new callback object to watch for updates to the central files such as name list files etc
	_FileListCallbackPtr= new CRFRFileListCallback(this);

	// register the callback object with the saves shard interface in question
	_ShardSavesInterface->addCallbackObject(_FileListCallbackPtr);
}

CRFRShardInterface::~CRFRShardInterface()
{
	_ShardSavesInterface->removeCallbackObject(_FileListCallbackPtr);

	// unregister save interfaces from the manager (to make sure everything is disconnected cleanly)
	CRemoteSavesManager::getInstance()->unregisterSavesInterface(_ShardSavesInterface);
	CRemoteSavesManager::getInstance()->unregisterSavesInterface(_WwwSavesInterface);
	CRemoteSavesManager::getInstance()->unregisterSavesInterface(_BakSavesInterface);
}

CShardSavesInterface& CRFRShardInterface::getShardSavesInterface()
{
	return *_ShardSavesInterface;
}

CMailSavesInterface& CRFRShardInterface::getWwwSavesInterface()
{
	return *_WwwSavesInterface;
}

CIncrementalBackupSavesInterface& CRFRShardInterface::getBakSavesInterface()
{
	return *_BakSavesInterface;
}

void CRFRShardInterface::addWatch(const CSString& fileName)
{
	_Watches.insert(fileName);
}

void CRFRShardInterface::removeWatch(const CSString& fileName)
{
	_Watches.erase(fileName);
}

const std::set<CSString>& CRFRShardInterface::getWatches() const
{
//	CVectorSString result;
//
//	// copy strings in _Watches set into result vector
//	for (TWatches::iterator it=_Watches.begin();it!=_Watches.end();++it)
//	{
//		result.push_back(*it);
//	}
//
//	return result;
	return _Watches;
}

void CRFRShardInterface::addCharacterIdNameMapping(const CSString& name,uint32& account,uint32& slot)
{
	DROP_IF(name.empty(),NLMISC::toString("Ignoring name mapping for account %u slot %u because name is empty",account,slot),return);
	DROP_IF(account>=(1<<28), NLMISC::toString("Ignoring name mapping for '%s' due to invalid account number: %u",name.c_str(),account),return);
	DROP_IF(slot>=(1<<4), NLMISC::toString("Ignoring name mapping for '%s' due to invalid slot number: %u",name.c_str(),slot),return);

	uint32 accountId= account*16+slot;

	// lookup the character name in the characters map
	TCharacters::iterator it= _Characters.find(name);

	// if found then check for a character move or other such name change
	if (it!=_Characters.end())
	{
		// if the character mapping hasn't changed then skip it
		if (it->second== accountId)
			return;

		// we have a mapping change so wap out a log
		nlinfo("Character name assignment move from account %d slot %d to account %d slot %d for name: %s",accountId/16,accountId%16,account,slot,name.c_str());
	}

	_Characters[name]= accountId;
}

bool CRFRShardInterface::getCharIdFromName(const CSString& name,uint32& account,uint32& slot)
{
	// lookup the character name in the characters map
	TCharacters::iterator it= _Characters.find(name);

	// if not found return false...
	if (it==_Characters.end())
	{
		account=~0u;
		slot=~0u;
		return false;
	}

	// setup result value and return true
	account= it->second/16;
	slot= it->second%16;
	return true;
}


//-----------------------------------------------------------------------------
// methods CRyzomFileRetriever - remaining public interface
//-----------------------------------------------------------------------------

CRyzomFileRetrieverImplementation* CRyzomFileRetrieverImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CRyzomFileRetrieverImplementation> ptr=NULL;
	if (ptr==NULL)
	{
		ptr= new CRyzomFileRetrieverImplementation;
	}
	return ptr;
}

CRyzomFileRetrieverImplementation::CRyzomFileRetrieverImplementation()
{
	clear();
}

void CRyzomFileRetrieverImplementation::clear()
{
	_IsActive= false;
	_Shards.clear();
	_Accounts.clear();
}

void CRyzomFileRetrieverImplementation::useShard(const CSString& shardName)
{
	if (_Shards.find(shardName)==_Shards.end())
	{
		_Shards[shardName]= new CRFRShardInterface(shardName);
	}
}

void CRyzomFileRetrieverImplementation::stopUsingShard(const CSString& shardName)
{
	if (_Shards.find(shardName)!=_Shards.end())
	{
		_Shards.erase(shardName);
	}
}

CVectorSString CRyzomFileRetrieverImplementation::getUsedShardList()
{
	CVectorSString result;
	for (TShards::iterator it= _Shards.begin(); it!=_Shards.end();++it)
	{
		result.push_back((*it).first);
	}
	return result;
}

CShardSavesInterface* CRyzomFileRetrieverImplementation::getShardSavesInterface(const CSString& shardName)
{
	TShards::iterator it= _Shards.find(shardName);
	return (it==_Shards.end())? NULL: &it->second->getShardSavesInterface();
}

CMailSavesInterface* CRyzomFileRetrieverImplementation::getWwwSavesInterface(const CSString& shardName)
{
	TShards::iterator it= _Shards.find(shardName);
	return (it==_Shards.end())? NULL: &it->second->getWwwSavesInterface();
}

CIncrementalBackupSavesInterface* CRyzomFileRetrieverImplementation::getBakSavesInterface(const CSString& shardName)
{
	TShards::iterator it= _Shards.find(shardName);
	return (it==_Shards.end())? NULL: &it->second->getBakSavesInterface();
}

CVectorSString CRyzomFileRetrieverImplementation::getSavesModules()
{
	CVectorSString result;

	// get the complete modules list
	CModuleManager::TModuleVector modules;
	CModuleManager::getInstance()->getModules(modules);

	// iterate over the list looking for SAVES modules
	for (uint32 i=0;i<modules.size();++i)
	{
		if (modules[i]->getName()=="SAVES")
		{
			result.push_back(modules[i]->getParameters());
		}
	}
	return result;
}

bool CRyzomFileRetrieverImplementation::uploadFile(const CSString& shardName,const CSString& localFileName,const CSString& remoteFileName)
{
	// get a pointer to the interface for shard save files and make sure it exists...
	CShardSavesInterface* shardSaves= getShardSavesInterface(shardName);
	DROP_IF(shardSaves==NULL,"No shard saves module found matching the shard: "+shardName,return false);
	DROP_IF(!shardSaves->isReady(),"Remote saves manager module is not ready for shard: "+shardName,return true);

	// put in a request for the file to be retrieved
	uint32 requestId= shardSaves->requestFile(remoteFileName);

	// put together a new callback object to handle the new file request
	TSavesCallbackPtr callbackPtr=	new CRFRUploadCallback(requestId,localFileName);

	// register the callback object with the saves shard interface in question
	shardSaves->addCallbackObject(callbackPtr);

	return true;
}

bool CRyzomFileRetrieverImplementation::downloadFile(const CSString& shardName,const CSString& remoteFileName,const CSString& localFileName)
{
	// get a pointer to the interface for shard save files and make sure it exists...
	CShardSavesInterface* shardSaves= getShardSavesInterface(shardName);
	DROP_IF(shardSaves==NULL,"No shard saves module found matching the shard: "+shardName,return false);
	DROP_IF(!shardSaves->isReady(),"Remote saves manager module is not ready for shard: "+shardName,return true);

	// put in a request for the file to be retrieved
	uint32 requestId= shardSaves->requestFile(remoteFileName);

	// put together a new callback object to handle the new file request
	TSavesCallbackPtr callbackPtr=	new CRFRDownloadCallback(requestId,localFileName);

	// register the callback object with the saves shard interface in question
	shardSaves->addCallbackObject(callbackPtr);

	return true;
}

bool CRyzomFileRetrieverImplementation::downloadBackupFiles(const CSString& shardName,const CSString& fileName,const CSString& localDirectory)
{
	// get a pointer to the interface for incremental backup files and make sure it exists...
	CIncrementalBackupSavesInterface* bakSaves= getBakSavesInterface(shardName);
	DROP_IF(bakSaves==NULL,"No shard saves module found matching the shard: "+shardName,return false);
	DROP_IF(!bakSaves->isReady(),"Remote saves manager module is not ready for shard: "+shardName,return true);

	// get hold of the list of files...
	CFileDescriptionContainer fdc;
	bakSaves->getFileList(fdc);

	// iterate over the file list...
	for (uint32 i=0;i<fdc.size();++i)
	{
		// skip entries that don't interest us...
		if (fdc[i].FileName.right(fileName.size())!=fileName)
			continue;

		// generate a local filename for the downloaded file
		CSString fdcFileNameUnderscore = fdc[i].FileName;
		fdcFileNameUnderscore = fdcFileNameUnderscore.replace("/","_");
		CSString localFileNameWithoutPath = fileName + "_" + fdcFileNameUnderscore;
		localFileNameWithoutPath = localFileNameWithoutPath.replace(".","_");
		CSString localFileName = NLMISC::CPath::standardizePath(localDirectory) + localFileNameWithoutPath;
		nlinfo("Requesting file download: REMOTE:%s => LOCAL:%s",fdc[i].FileName.c_str(),localFileName.c_str());

		// put in a request for the file to be retrieved
		uint32 requestId= bakSaves->requestFile(fdc[i].FileName);

		// put together a new callback object to handle the new file request
		TSavesCallbackPtr callbackPtr=	new CRFRDownloadCallback(requestId,localFileName);

		// register the callback object with the saves shard interface in question
		bakSaves->addCallbackObject(callbackPtr);
	}

	return true;
}

void CRyzomFileRetrieverImplementation::addAccountIdNameMapping(const CSString& name,uint32& account)
{
	DROP_IF(name.empty(),NLMISC::toString("Ignoring name mapping for account %u because name is empty",account),return);
	DROP_IF(account>=(1<<28), NLMISC::toString("Ignoring name mapping for '%s' due to invalid account number: %u",name.c_str(),account),return);

	// lookup the account name in the accounts map
	TAccounts::iterator it= _Accounts.find(name);

	// make sure no previous mapping existed for this account name
	if (it!=_Accounts.end())
	{
		uint32 oldAccount= _Accounts[name];

		// a mapping existed so if it's the same as the one we have here then we just need to ignore it
		if (oldAccount==account)
			return;

		// the mapping has changed so pull the alarm bell
		nlwarning("Remapping account name '%s' from id %u to id %u",name.c_str(),oldAccount,account);
	}

	// assign the new mapping
	_Accounts[name]=account;
}

bool CRyzomFileRetrieverImplementation::getAccountIdFromName(const CSString& name,uint32& account)
{
	// lookup the account name in the accounts map
	TAccounts::iterator it= _Accounts.find(name);

	// if not found return false...
	if (it==_Accounts.end())
	{
		account=~0u;
		return false;
	}

	// setup result value and return true
	account= it->second;
	return true;
}

bool CRyzomFileRetrieverImplementation::getCharIdFromName(const CSString& shardName,const CSString& name,uint32& account,uint32& slot)
{
	// lookup the shard name in the shards map
	TShards::iterator it= _Shards.find(shardName);

	// if not found return false...
	if (it==_Shards.end())
	{
		account=~0u;
		slot=~0u;
		return false;
	}

	return it->second->getCharIdFromName(name,account,slot);
}


//-----------------------------------------------------------------------------
// methods CRyzomFileRetrieverImplementation / IModule
//-----------------------------------------------------------------------------

bool CRyzomFileRetrieverImplementation::initialiseModule(const CSString& rawArgs)
{
	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be one SS module activated at a time",return false);
	DROP_IF(rawArgs.countWordOrWords()!=0,"error expected no parameters but found: \""+rawArgs+"\"",return false);

	// done (success)
	_IsActive= true;
	return true;
}

void CRyzomFileRetrieverImplementation::release()
{
	_IsActive= false;
	clear();
}

CSString CRyzomFileRetrieverImplementation::getState() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	CSString result;
	result+= getName();
	result+= ' ';
	result+= getParameters();
	result+= ':';

	return result;
}

NLMISC::CSString CRyzomFileRetrieverImplementation::getName() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return "RFR";
}

NLMISC::CSString CRyzomFileRetrieverImplementation::getParameters() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	return "";
}

void CRyzomFileRetrieverImplementation::displayModule() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	// display info
	InfoLog->displayNL("MODULE %s %s",getName().c_str(),getParameters().c_str());
}


//-----------------------------------------------------------------------------
// methods CRFRDownloadCallback
//-----------------------------------------------------------------------------

CRFRDownloadCallback::CRFRDownloadCallback(uint32 requestId,const CSString& destFileName)
{
	_RequestId=		requestId;
	_DestFileName=	destFileName;
	_SelfPtr=		this;
}

void CRFRDownloadCallback::cbFileReceived(uint32 requestId,const CSString& fileName,const CSString& fileBody)
{
	if (requestId!=_RequestId)
		return;

	// self destruct at the end of the routine
	TSavesCallbackPtr selfPtr= _SelfPtr;
	_SelfPtr= NULL;

	// display a quick info...
	nlinfo("Receiving file download for file: %s (from remote file: %s)",_DestFileName.c_str(),fileName.c_str());

	// if the destination directory doesn't exist then create it
	CSString path= NLMISC::CPath::standardizePath(NLMISC::CFile::getPath(_DestFileName));
	if (!NLMISC::CFile::isDirectory(path))
		NLMISC::CFile::createDirectoryTree(path);

	// write the file to the destination directory
	fileBody.writeToFile(_DestFileName);
}

void CRFRDownloadCallback::cbGenericReply(uint32 requestId,bool successFlag,const CSString& explanation)
{
	if (requestId!=_RequestId)
		return;

	// self destruct at the end of the routine
	TSavesCallbackPtr selfPtr= _SelfPtr;
	_SelfPtr= NULL;

	if (successFlag)
	{
		nlinfo("Received acknowledge for file download: %s",_DestFileName.c_str());
		STOP("Requested a download but instead of receiving the file data I have received a successful generic reply");
	}
	else
	{
		nlwarning("File download failed: %s: explanation: %s",_DestFileName.c_str(),explanation.c_str());
	}
}


//-----------------------------------------------------------------------------
// methods CRFRUploadCallback
//-----------------------------------------------------------------------------

CRFRUploadCallback::CRFRUploadCallback(uint32 requestId,const CSString& srcFileName)
{
	_RequestId=		requestId;
	_SrcFileName=	srcFileName;
	_SelfPtr=		this;
}

void CRFRUploadCallback::cbFileReceived(uint32 requestId,const CSString& fileName,const CSString& fileBody)
{
	if (requestId!=_RequestId)
		return;

	// self destruct at the end of the routine
	TSavesCallbackPtr selfPtr= _SelfPtr;
	_SelfPtr= NULL;

	STOP("We requested an upload and have received a download!");
}

void CRFRUploadCallback::cbGenericReply(uint32 requestId,bool successFlag,const CSString& explanation)
{
	if (requestId!=_RequestId)
		return;

	// self destruct at the end of the routine
	TSavesCallbackPtr selfPtr= _SelfPtr;
	_SelfPtr= NULL;

	if (successFlag)
	{
		nlinfo("Received acknowledge for file upload: %s",_SrcFileName.c_str());
	}
	else
	{
		nlwarning("File upload failed: %s: explanation: %s",_SrcFileName.c_str(),explanation.c_str());
	}
}


//-----------------------------------------------------------------------------
// methods CRFRFileListCallback
//-----------------------------------------------------------------------------

CRFRFileListCallback::CRFRFileListCallback(CRFRShardInterface* shardFileInterface)
{
	_ShardFileInterface= shardFileInterface;
}

void CRFRFileListCallback::cbInit(const CFileDescriptionContainer& fdc)
{
	// make sure our parent still exists
	if (_ShardFileInterface==NULL)
		return;

	// make sure the parent's shard save interface is still intact
	CShardSavesInterface& shardSaves= _ShardFileInterface->getShardSavesInterface();

	// put in immediate requests for download of account and character name files
	_AccountFileRequest= shardSaves.requestFile("account_names.txt");
	_CharacterFileRequest= shardSaves.requestFile("character_names.txt");
}

void CRFRFileListCallback::cbFileListChanged(	const CFileDescriptionContainer& newFiles,
												const CFileDescriptionContainer& modifiedFiles,
												const NLMISC::CVectorSString&	 deletedFile,
												const CFileDescriptionContainer& oldFileList,
												const CFileDescriptionContainer& newFileList)
{
	if (_ShardFileInterface==NULL)
		return;

	std::set<CSString> watches= _ShardFileInterface->getWatches();

	// iterate over new files
	for (uint32 i=0;i<newFiles.size();++i)
	{
		// check for matches with watched file names
		if (watches.find(newFiles[i].FileName)!=watches.end())
		{
			nlinfo("Watched file: new: %s",newFiles[i].toString().c_str());
		}

		// check for changes to accounts and character names files
		if (newFiles[i].FileName=="account_names.txt")
			_AccountFileRequest= _ShardFileInterface->getShardSavesInterface().requestFile("account_names.txt");

		if (newFiles[i].FileName=="character_names.txt")
			_CharacterFileRequest= _ShardFileInterface->getShardSavesInterface().requestFile("character_names.txt");
	}

	for (uint32 i=0;i<modifiedFiles.size();++i)
	{
		// check for matches with watched file names
		if (watches.find(modifiedFiles[i].FileName)!=watches.end())
		{
			nlinfo("Watched file: chg: %s",modifiedFiles[i].toString().c_str());
		}

		// check for changes to accounts and character names files
		if (modifiedFiles[i].FileName=="account_names.txt")
			_AccountFileRequest= _ShardFileInterface->getShardSavesInterface().requestFile("account_names.txt");

		if (modifiedFiles[i].FileName=="character_names.txt")
			_CharacterFileRequest= _ShardFileInterface->getShardSavesInterface().requestFile("character_names.txt");
	}

	for (uint32 i=0;i<deletedFile.size();++i)
	{
		// check for matches with watched file names
		if (watches.find(deletedFile[i])!=watches.end())
		{
			nlinfo("Watched file: del: %s",deletedFile[i].c_str());
		}

		// check for changes to accounts and character names files
		if (deletedFile[i]=="account_names.txt")
			nlwarning("account_names.txt has just been deleted!!");

		if (deletedFile[i]=="character_names.txt")
			nlwarning("character_names.txt has just been deleted!!");
	}
}

void CRFRFileListCallback::cbFileReceived(uint32 requestId,const CSString& fileName,const CSString& fileBody)
{
	if (_ShardFileInterface==NULL)
		nlinfo("Callback object ignoring message because parent is NULL");

	// if our parent has been destroyed we have nothing to do
	if (_ShardFileInterface==NULL)
		return;

//	nlinfo("Module %s receiving message: ...",_ShardFileInterface->getShardSavesInterface().getShardName().c_str());

	// we check the filenames and not the request IDs here as it allows us to react to the most recently recieved changes of
	// the names files as they arrive whether they correspond to the most recent requests that we have made or not

	if (fileName=="account_names.txt")
	{
		nlinfo("Scanning account_names.txt file from shard: %s",_ShardFileInterface->getShardSavesInterface().getShardName().c_str());

		CVectorSString lines;
		fileBody.splitLines(lines);
		uint32 emptyLines=0;
		uint32 invalidLines=0;
		for (uint32 i=0;i<lines.size();++i)
		{
			CSString line= lines[i].strip();
			if (line.empty())
			{
				++emptyLines;
				continue;
			}
			CSString name= line.strtok(" \t");
			uint32 account= line.strip().atoui();
			if (account==0 && line.strip()!="0")
			{
				++invalidLines;
				continue;
			}
			CRyzomFileRetrieverImplementation::getInstance()->addAccountIdNameMapping(name,account);
		}
		nlinfo("Finished scanning account_names.txt file - %d lines treated, %d blank lines skipped, %d invalid lines ignored",lines.size()-emptyLines-invalidLines,emptyLines,invalidLines);
	}

	if (fileName=="character_names.txt")
	{
		nlinfo("Scanning character_name.txt file from shard: %s",_ShardFileInterface->getShardSavesInterface().getShardName().c_str());

		CVectorSString lines;
		fileBody.splitLines(lines);
		uint32 emptyLines=0;
		uint32 invalidLines=0;
		for (uint32 i=0;i<lines.size();++i)
		{
			CSString line= lines[i].strip();
			if (line.empty())
			{
				++emptyLines;
				continue;
			}
			CSString name= line.strtok(" \t");
			uint32 account= line.strtok(" \t").atoui();
			uint32 slot= line.strip().atoui();
			if ((slot==0 && line.strip()!="0") || (account==0 && lines[i].word(1).strip()!="0"))
			{
				++invalidLines;
				continue;
			}
			_ShardFileInterface->addCharacterIdNameMapping(name,account,slot);
		}
		nlinfo("Finished scanning character_names.txt file - %d lines treated, %d blank lines skipped, %d invalid lines ignored",lines.size()-emptyLines-invalidLines,emptyLines,invalidLines);
	}
}

void CRFRFileListCallback::cbGenericReply(uint32 requestId,bool successFlag,const CSString& explanation)
{
	// if this is not a reply saying that our last request for an update failed then we just ignore it
	if (requestId!=_AccountFileRequest && requestId!=_CharacterFileRequest)
		return;

	const char* fileName= (requestId==_AccountFileRequest)? "account_names.txt": "character_names.txt";

	if (successFlag)
	{
		nlinfo("Received acknowledge for file download: %s",fileName);
		STOP("Requested a download but instead of receiving the file data I have received a successful generic reply");
	}
	else
	{
		nlwarning("File download failed: %s: explanation: %s",fileName,explanation.c_str());
	}
}


//-----------------------------------------------------------------------------
// methods CRyzomFileRetriever
//-----------------------------------------------------------------------------

CRyzomFileRetriever* CRyzomFileRetriever::getInstance()
{
	return CRyzomFileRetrieverImplementation::getInstance();
}


//-----------------------------------------------------------------------------
// CRyzomFileRetriever registration
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CRyzomFileRetrieverImplementation,"RFR","","Interface used by NLMISC_COMMAND commands to access remote saves")
