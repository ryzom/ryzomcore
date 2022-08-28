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
#include "administered_module.h"
#include "file_repository.h"
#include "file_receiver.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// class CServerPatchBridge
//-----------------------------------------------------------------------------

class CServerPatchBridge: 
	public CAdministeredModuleBase,
	public CFileRepository,
	public CFileReceiver
{
public:
	// ctor
	CServerPatchBridge();

	// CModuleBase specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg);
	void onModuleUpdate();
	std::string buildModuleManifest() const;

	// prevent modules from misbehaving when they run on the same service together
	bool isImmediateDispatchingSupported() const { return false; }

protected:
	// specialisations of CFileRepositorySkel methods (overloading default CFileRepository behaviour)
	void requestFileData(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes);

	// specialisations of overloadable callback methods
	void cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data);
	void cbRetryAfterFileDownloadFailure(const NLMISC::CSString& fileName);
	void cbFileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &changes);

	// specialisation of overloadable methods from CFileRepository
	void getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,const NLNET::IModuleProxy *sender) const;
	void onFileRepositoryModuleDown(NLNET::IModuleProxy *module);


private:
	// private data
	NLMISC::CSString _Manifest;

	struct SDelayedFileRequest
	{
		NLMISC::CSString FileName;
		TProxyPtr Requestor;
		uint32 StartOffset;
		uint32 NumBytes;

		SDelayedFileRequest(): Requestor(NULL), StartOffset(0), NumBytes(0) {}
	};
	typedef vector<SDelayedFileRequest> TDelayedFileRequestVector;			// vector of requests pertaining to a single file
	typedef map<CSString,TDelayedFileRequestVector> TDelayedFileRequests;	// map of file name to request vectors
	TDelayedFileRequests _DelayedFileRequests;

protected:
	// NLMISC_COMMANDs for this module
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchBridge, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CServerPatchBridge, dump, "Dump the current status", "no args")
		NLMISC_COMMAND_HANDLER_ADDS_FOR_FILE_REPOSITORY(CServerPatchBridge)
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
};


//-----------------------------------------------------------------------------
// methods CServerPatchBridge
//-----------------------------------------------------------------------------

CServerPatchBridge::CServerPatchBridge()
{
}

bool CServerPatchBridge::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	NLMISC::CSString logMsg;

	// setup the root directory
	NLMISC::CSString rootDirectory;
	const TParsedCommandLine *targetDir = initInfo.getParam("path");
	if (targetDir != NULL)
	{
		rootDirectory= targetDir->ParamValue;
	}
	else
	{
		rootDirectory= NLMISC::CPath::getCurrentPath();
	}
	logMsg+=" Path: "+rootDirectory;

	// initialise the module base classes...
	logMsg+= CAdministeredModuleBase::init(initInfo);
	CFileRepository::init(this,rootDirectory);
	CFileReceiver::init(this,"*/*");

	// now  that the base classes have been initialised, we can cumulate the module manifests
	_Manifest= (CFileRepository::buildModuleManifest()+" "+CFileReceiver::getModuleManifest()+" "+_Manifest);
	_Manifest = _Manifest.strip();

	// scan our local file cache to buildup our starting file base
	setStateVariable("State","Scanning");
	broadcastStateInfo();
	CFileRepository::rescanFull();

	// we're all done so let the world know
	registerProgress("SPB Initialised: "+logMsg+" "+_Manifest);
	setStateVariable("State","Initialised");
	broadcastStateInfo();

	return true;
}

void CServerPatchBridge::onModuleUp(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUp(module);
	CFileRepository::onModuleUp(module);
	CFileReceiver::onModuleUp(module);

	if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
	{
		// register with the administrator
		CServerPatchManagerProxy manager(module);
		manager.registerAdministeredModule(this,false,false,false,true);
		registerProgress("registering with manager: "+module->getModuleName());
	}
}

void CServerPatchBridge::onModuleDown(IModuleProxy *module)
{
	// allow base classes to do their stuff
	CAdministeredModuleBase::onModuleDown(module);
	CFileRepository::onModuleDown(module);
	CFileReceiver::onModuleDown(module);
}

void CServerPatchBridge::onFileRepositoryModuleDown(IModuleProxy *module)
{
	// BEFORE CALLING CFileReceiver::onModuleDown - prepare to cleanup our file info lists...
	// build a set of all of the files that I can access (including those via the proxy who's going away)
	TFileInfoVector pre;
	getFileInfo("*/*",pre,NULL);
	std::sort(pre.begin(),pre.end());

	// allow base classes to do their stuff
	CFileRepository::onFileRepositoryModuleDown(module);

	// AFTER CALLING CFileReceiver::onModuleDown - build a cleaned file list...
	TFileInfoVector post;
	getFileInfo("*/*",post,NULL);
	std::sort(post.begin(),post.end());

	// build a vector of the entries in 'post' that are not identical to their conterparts in 'pre'
	TFileInfoVector lostFiles;
	TFileInfoVector::iterator preit= pre.begin();
	TFileInfoVector::iterator postit= post.begin();
	while(preit!=pre.end() && postit!=post.end())
	{
		// if the two entries match then move forwards together...
		if (preit->FileName==postit->FileName)
		{
			++preit;
			++postit;
			continue;
		}
		// the two don't match so just move one of them forwards...
		BOMB_IF(postit->FileName<preit->FileName,"BUG: Extra files have appeared in the module down process!!",++postit;continue);

		// setup an empty file info record and add it to the lostFiles vector...
		lostFiles.push_back(SFileInfo());
		lostFiles.back().FileName= preit->FileName;
		++preit;
	}
	// if there's anything left in 'pre' after the end of 'post' then just add it to the lostFiles container
	for (;preit!=pre.end();++preit)
	{
		lostFiles.push_back(SFileInfo());
		lostFiles.back().FileName= preit->FileName;
	}

	// send the lost files vector to all of our subscribers
	_broadcastFileInfoChanges(lostFiles);
	registerProgress(NLMISC::toString("Module down %s: Access lost to %d files",module->getModuleName().c_str(),lostFiles.size()));

	// if we have any pending requests for this module then erase them
	for (TDelayedFileRequests::iterator fit=_DelayedFileRequests.begin(); fit!=_DelayedFileRequests.end();)
	{
		uint32 goodRecordsCount= 0;
		for (uint32 i=0; i< fit->second.size(); ++i)
		{
			// if the requestor is NOT the module that's goign down then keep the record...
			if (fit->second[i].Requestor!=module)
			{
				// if some of the records in this vector have been deleted then copy this entry down into the nextfree slot
				if (i!=goodRecordsCount) fit->second[goodRecordsCount]= fit->second[i];
				// increment the count of good records found so far
				++goodRecordsCount;
			}
		}

		// if we removed requests here...
		if (fit->second.size() != goodRecordsCount)
		{
			// scale down the vector as required...
			fit->second.resize(goodRecordsCount);

			// if all the requests here were for the same requestor then delete the request all together...
			if (goodRecordsCount==0)
			{
				registerProgress("Dropped pending request for "+fit->first+" for lost module: "+module->getModuleName());
				TDelayedFileRequests::iterator deadIt= fit;
				++fit;
				_DelayedFileRequests.erase(deadIt);
				continue;
			}
			registerProgress("Removed lost module "+module->getModuleName()+" from pending requests for: "+fit->first);
		}

		// iterate...
		++fit;
	}
}

//void CServerPatchBridge::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CAdministeredModuleBase::onDispatchMessage(sender, msg))
//		return;
//
//	if (CFileRepository::onDispatchMessage(sender, msg))
//		return;
//
//	if (CFileReceiver::onDispatchMessage(sender, msg))
//		return;
//
//	registerError("ignoring MSG: "+msg.getName());
//	STOP("CServerPatchBridge::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'");
//}
	
void CServerPatchBridge::onModuleUpdate()
{
	H_AUTO(CServerPatchBridge_onModuleUpdate);

	// allow the base classes a chance to do their stuff
	CAdministeredModuleBase::onModuleUpdate();
	CFileRepository::onModuleUpdate();
	CFileReceiver::onModuleUpdate();

	setStateVariable("State",NLMISC::toString("Running"));
}

std::string CServerPatchBridge::buildModuleManifest() const
{
	return _Manifest;
}

void CServerPatchBridge::cbFileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &changes)
{
	// start by letting our base classes do their stuff...
	CFileReceiver::cbFileInfo(sender, changes);

	// update subscriptions
	CFileRepository::_broadcastFileInfoChanges(changes);

	// report on progress
	registerProgress(NLMISC::toString("Forwarded %d info changes to subscribers",changes.size()));
}

void CServerPatchBridge::getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,const NLNET::IModuleProxy *sender) const
{
	// start by clearing out the result vector...
	result.clear();

	// get info from our different connected emitters
	CFileReceiver::getFileInfo(fileSpec,result);

	// build a set of the file names of the found files for quick access
	std::set<CSString> foundFiles;
	for (TFileInfoVector::iterator it=result.begin(); it!=result.end();++it)
	{
		foundFiles.insert(it->FileName);
	}

	// add the info on any files that I have locally but that the connected emitters don't have
	TFileInfoVector localFiles;
	CFileRepository::getFileInfo(fileSpec,localFiles,sender);
	for (TFileInfoVector::iterator it=localFiles.begin(); it!=localFiles.end();++it)
	{
		if (foundFiles.find(it->FileName)==foundFiles.end())
		{
			result.push_back(*it);
		}
	}
}

void CServerPatchBridge::requestFileData(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes)
{
	// if this file is already being uploaded to satisfy another request then just queue us up
	if (_DelayedFileRequests.find(fileName)!=_DelayedFileRequests.end())
	{
		SDelayedFileRequest& theRequest= vectAppend(_DelayedFileRequests.find(fileName)->second);
		theRequest.Requestor=	sender;
		theRequest.FileName=	fileName;
		theRequest.StartOffset=	startOffset;
		theRequest.NumBytes=	numBytes;
		registerProgress("Queued additional request for file: "+fileName+" for "+sender->getModuleName());
		return;
	}

	// get local info (for the file in the local directory)
	TFileInfoVector infoVect;
	CFileRepository::getFileInfo(fileName,infoVect,sender);

	// lookup our connected emitters to get the most up to date file info
	SFileInfo fileInfo;
	CFileReceiver::getSingleFileInfo(fileName,fileInfo);

	// determine whether our own version is up to date
	bool upToDate= !infoVect.empty();
	if (upToDate)
	{
		upToDate =	(fileInfo.FileTime==0) || 
					( (fileInfo.FileSize==infoVect[0].FileSize) && 
					  (fileInfo.Checksum==infoVect[0].Checksum) );
	}

	// if we're not up to date then look to see if we have another file locally with the same properties (an identical file)
	if (!upToDate && fileInfo.FileTime!=0)
	{
		// iterate over a copy of the complete vector of file info looking for a match
		TFileInfoVector allFileInfo;
		CFileRepository::getFileInfo("*/*",allFileInfo,sender);
		for (TFileInfoVector::iterator it= allFileInfo.begin(); it!=allFileInfo.end(); ++it)
		{
			// test file size and checksum of the files
			const SFileInfo& itInfo= *it;
			if ( (itInfo.FileSize==fileInfo.FileSize) && (itInfo.Checksum==fileInfo.Checksum) )
			{
				// we've found a match so copy the file and mark us as up to date

				// start by broadcasting what we're up to so that spa modules can keep track
				registerProgress("Copying: '"+itInfo.FileName+" to '"+fileInfo.FileName+"'");
				broadcastStateInfo();

				// lookup the root directory and copy the file (making sure the target directory exists before we begin)
				CSString rootDirectory= getRepositoryDirectory()->getRootDirectory();
				CSString fullFileName= rootDirectory+fileInfo.FileName;
				NLMISC::CFile::createDirectoryTree(NLMISC::CFile::getPath(fullFileName));
				bool ok= NLMISC::CFile::copyFile(fullFileName,rootDirectory+itInfo.FileName);
				WARN_IF(!ok,"Failed to copy file: '"+itInfo.FileName+" to '"+fileInfo.FileName+"'");

				// force our repository to update it's info concerning the file that we just copied and rebuild our info vector
				CFileRepository::updateFile(fileName);
				infoVect.clear();
				CFileRepository::getFileInfo(fileName,infoVect,sender);

				// see if we have succeeded in creating a match
				upToDate= ( (fileInfo.FileSize==infoVect[0].FileSize) && (fileInfo.Checksum==infoVect[0].Checksum) );
				if (upToDate) break;
			}
		}
	}

	// if we have the file locally then delegate to CFileRepository
	if (upToDate)
	{
		registerProgress("Dispatching local copy of file: "+fileName+" to "+sender->getModuleName());
		CFileRepository::requestFileData(sender,fileName,startOffset,numBytes);
		return;
	}

	// if we can't see an emitter to ask for the file then give up
	if (fileInfo.FileTime==0)
	{
		registerError("Failed to treat request for file: "+fileName+" from "+sender->getModuleName());
		CFileReceiverProxy fr(sender);
		fr.cbFileDataFailure(this,fileName);
		return;
	}

	// send a file request to our emitters
	CFileReceiver::requestFile(fileName);

	// add an entry to the delayed requests map
	SDelayedFileRequest& theRequest= vectAppend(_DelayedFileRequests[fileName]);
	theRequest.Requestor=	sender;
	theRequest.FileName=	fileName;
	theRequest.StartOffset=	startOffset;
	theRequest.NumBytes=	numBytes;
	registerProgress("Queued request for file: "+fileName+" for "+sender->getModuleName());
}

void CServerPatchBridge::cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data)
{
//	registerProgress("Received file: "+fileName);

	// if the latest info for the file (eg size) doesn't match with the data that we've received then consider that there's a problem and go round again
	SFileInfo fileInfo;
	CFileReceiver::getSingleFileInfo(fileName,fileInfo);
	if (fileInfo.FileSize!=data.size())
	{
		registerError(NLMISC::toString("File received but data size (%d) doesn't correspond to latest info size (%d) so throwing out data and trying again: %s",data.size(),fileInfo.FileSize,fileName.c_str()));
		CFileReceiver::requestFile(fileName);
		return;
	}

	// save the file away to disk
	CFileManager::getInstance().save(_Directory->getRootDirectory()+fileName,data);

	// force our repository to update it's info concerning the file that we just saved
	CFileRepository::updateFile(fileName);

	// see if we have any queued requests for this file and treat them if we do
	if (_DelayedFileRequests.find(fileName)!=_DelayedFileRequests.end())
	{
		// get hold of a ref to the vector of requests concerning this file...
		TDelayedFileRequestVector& vect= _DelayedFileRequests.find(fileName)->second;
		registerProgress(NLMISC::toString("Treating %d delayed requests relating to received file: %s",vect.size(),fileName.c_str()));

		// treat each request in the vector
		for (TDelayedFileRequestVector::iterator it= vect.begin(); it!= vect.end(); ++it)
		{
			CFileRepository::requestFileData(it->Requestor,it->FileName,it->StartOffset,it->NumBytes);
		}

		// we've finished treating the requests so ditch them
		_DelayedFileRequests.erase(fileName);
	}
}

void CServerPatchBridge::cbRetryAfterFileDownloadFailure(const NLMISC::CSString& fileName)
{
	registerError("Download failed for file: "+fileName);
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchBridge, dump)
{
	if (args.size()!=0)
		return false;

	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	log.displayNL("");
	CFileRepository::dump(log);

	log.displayNL("");
	log.displayNL("-----------------------------------");
	log.displayNL("Delayed Requests (awaiting upload)");
	log.displayNL("-----------------------------------");
	for (TDelayedFileRequests::iterator it=_DelayedFileRequests.begin(); it!=_DelayedFileRequests.end(); ++it)
	{
		log.displayNL("  '%s' (%d requests)",it->first.c_str(),it->second.size());
	}
	log.displayNL("-----------------------------------");
	CFileReceiver::dump(log);

	return true;
}


//-----------------------------------------------------------------------------
// CServerPatchBridge registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchBridge, "ServerPatchBridge");

