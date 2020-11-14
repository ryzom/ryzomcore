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
#include "nel/misc/debug.h"
#include "nel/misc/singleton.h"
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"

// local
#include "file_manager.h"
#include "file_repository.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-------------------------------------------------------------------------------------------------
// namespace PATCHMAN
//-------------------------------------------------------------------------------------------------

namespace PATCHMAN
{
		
	//-----------------------------------------------------------------------------
	// methods CFileRepository
	//-----------------------------------------------------------------------------

//	std::string CFileRepository::buildModuleManifest() const
//	{
//		// make sure we've been initialised
////		nlassert(_Parent!=NULL);
//
//		return "isFileRepository";
//	}
	CFileRepository::CFileRepository()
	{
		_FileRequestCount= 0;
		_FileInfoCount= 0;

		_MaxHistorySize= 10;
		_FileRequestHistorySize= 0;
		_FileInfoHistorySize= 0;
	}

	void CFileRepository::init(NLNET::IModule* parent,const NLMISC::CSString& rootDirectory)
	{
		CFileRepositorySkel::init(parent);
		_Interceptor.init(this, parent);
		_Parent= parent;
		_Directory= CFileManager::getInstance().getRepositoryDirectory(rootDirectory);
		_AdministeredModuleWrapper.init(dynamic_cast<CAdministeredModuleBase*>(parent));
	}

	void CFileRepository::onModuleUp(IModuleProxy *module)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (NLMISC::CSString(module->getModuleManifest()).contains(ManifestEntryIsFileReceiver))
		{
			CFileReceiverProxy rr(module);
			rr.setupSubscriptions(_Parent);
			_AdministeredModuleWrapper.registerProgress("Receiver up: "+module->getModuleName());
		}
	}

	void CFileRepository::onModuleDown(IModuleProxy *module)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// if the module has subscribed to listen to stuff then end its subscriptions
		onFileRepositoryModuleDown(module);
	}

	void CFileRepository::onFileRepositoryModuleDown(IModuleProxy *module)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// if the module has subscribed to listen to stuff then end its subscriptions
		unsubscribeAll(module);
	}

	void CFileRepository::onModuleUpdate()
	{
		H_AUTO(CFileRepository_onModuleUpdate);

		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// update subscriptions
		_broadcastFileInfoChanges(_FileInfoChanges);

		// clear out the changes container (now that it's been treated)
		if (!_FileInfoChanges.empty())
		{
			_AdministeredModuleWrapper.registerProgress(NLMISC::toString("updated %d files",_FileInfoChanges.size()));
			_FileInfoChanges.clear();
		}
	}

	std::string CFileRepository::buildModuleManifest() const
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		return ManifestEntryIsFileRepository;
	}

	void CFileRepository::rescanFull()
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// delegate to the _Directory object
		_Directory->rescanFull(this);
	}

	void CFileRepository::rescanPartial()
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// delegate to the _Directory object
		_Directory->rescanPartial(this);
	}

	void CFileRepository::updateFile(const NLMISC::CSString& fileName)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// delegate to the _Directory object
		_Directory->updateFile(fileName,SFileInfo::RECALCULATE_IF_CHANGED,this);
	}

	void CFileRepository::getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,const NLNET::IModuleProxy *sender) const
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// delegate to the _Directory object
		_Directory->getFileInfo(fileSpec,result,const_cast<CFileRepository*>(this),sender);
	}

	TRepositoryDirectoryPtr CFileRepository::getRepositoryDirectory()
	{
		return _Directory;
	}

	void CFileRepository::getFile(const NLMISC::CSString& fileName,NLMISC::CSString& resultData,const NLNET::IModuleProxy* sender) const
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// clear out the result container before we begin...
		resultData.clear();

		// delegate to the _Directory object
		_Directory->getFile(fileName,resultData,const_cast<CFileRepository*>(this),sender);
	}

	void CFileRepository::dump(NLMISC::CLog& log)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		log.displayNL("-----------------------------------");
		log.displayNL("Recent Info requests (%u in all)",_FileInfoCount);
		log.displayNL("-----------------------------------");
		for (THistory::iterator it=_FileInfoHistory.begin(); it!=_FileInfoHistory.end();++it)
		{
			log.displayNL("  '%s'",it->c_str());
		}
		log.displayNL("-----------------------------------");
		log.displayNL("Recent Download requests (%u in all)",_FileRequestCount);
		log.displayNL("-----------------------------------");
		for (THistory::iterator it=_FileRequestHistory.begin(); it!=_FileRequestHistory.end();++it)
		{
			log.displayNL("  '%s'",it->c_str());
		}
		log.displayNL("-----------------------------------");
		log.displayNL("Active Subscriptions");
		log.displayNL("-----------------------------------");
		for (TSubscribers::iterator it=_Subscribers.begin(); it!=_Subscribers.end();++it)
		{
			log.displayNL("  '%s'",it->first.c_str());
		}
	}

	void CFileRepository::setMaxHistorySize(uint32 maxHistorySize)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		_MaxHistorySize= maxHistorySize;

		// if the new limit is bigger than the request history size then prune down the history list
		while (_FileRequestHistorySize > _MaxHistorySize)
		{
			_FileRequestHistory.pop_back();
			--_FileRequestHistorySize;
		}

		// if the new limit is bigger than the info history size then prune down the history list
		while (_FileInfoHistorySize > _MaxHistorySize)
		{
			_FileInfoHistory.pop_front();
			--_FileInfoHistorySize;
		}
	}

	uint32 CFileRepository::getMaxHistorySize() const
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		return _MaxHistorySize;
	}

	void CFileRepository::_broadcastFileInfoChanges(const TFileInfoVector& fileInfoChanges)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// update subscriptions
		for (TSubscribers::iterator sit=_Subscribers.begin(); sit!=_Subscribers.end();++sit)
		{
			// setup a filespec object for this subscriber
			CFileSpec fileSpec(sit->first.splitFrom('@').strip());

			// build a vector of files that matches the subscription's request
			TFileInfoVector infoVector;

			// run through the changes that we've accumulated since the last update...
			for (TFileInfoVector::const_iterator cit=fileInfoChanges.begin(); cit!=fileInfoChanges.end();++cit)
			{
				// if this change is for a file that matches the subscriber's filespec then addit to our result vector
				if (fileSpec.matches(cit->FileName) && cbValidateFileInfoRequest(sit->second,cit->FileName))
				{
					infoVector.push_back(*cit);
				}
			}

			// if we found changes that interest this subscriber then dispatch them
			if (!infoVector.empty())
			{
				CFileReceiverProxy client(sit->second);
				client.cbFileInfo(_Parent,infoVector);
			}
		}
	}

	void CFileRepository::requestFileInfo(NLNET::IModuleProxy *sender,const NLMISC::CSString& fileSpec)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// setup an object to receive the file info and fill it in
		TFileInfoVector result;
		getFileInfo(fileSpec,result,sender);

		// get hold of a proxy for the module who sent the request and return the file info to them
		CFileReceiverProxy fr(sender);
		fr.cbFileInfo(_Parent,result);

		// update our stats
		++_FileInfoCount;

		// add our new history record
		_FileInfoHistory.push_front((sender==NULL?"<local> ":"<"+sender->getModuleName()+"> ")+fileSpec);
		++_FileInfoHistorySize;

		// if we're grown too big then prune the oldest entry
		if (_FileInfoHistorySize>_MaxHistorySize)
		{
			_FileInfoHistory.pop_back();
			--_FileInfoHistorySize;
		}

		// deal with progress / state info
		_AdministeredModuleWrapper.registerProgress("File info request: "+sender->getModuleName()+" "+fileSpec);
		_AdministeredModuleWrapper.setStateVariable("InfoReq",NLMISC::toString(_FileInfoCount));
	}

	void CFileRepository::requestFileData(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// load the file chunk that's being requested
		CSString result;
		bool ok;

		// allow the overloadable validation callback a chance to prohibit read
		ok= cbValidateDownloadRequest(sender,fileName);

		// load the file (if validation was favorable)
		CSString fullFileName= _Directory->getRootDirectory()+fileName;
		ok&= CFileManager::getInstance().load(fullFileName,startOffset,numBytes,result);

		// get hold of a proxy for the module who sent the request and return the file info to them
		CFileReceiverProxy rr(sender);
		if (ok && !result.empty())
		{
			rr.cbFileData(_Parent,fileName,startOffset,NLNET::TBinBuffer((const uint8 *)&result[0],(uint32)result.size()));
		}
		else
		{
			rr.cbFileDataFailure(_Parent,fileName);
		}

		// update our stats
		++_FileRequestCount;

		// add our new history record
		_FileRequestHistory.push_front((sender==NULL?"<local> ":"<"+sender->getModuleName()+"> ")+fileName);
		++_FileRequestHistorySize;

		// if we're grown too big then prune the oldest entry
		if (_FileRequestHistorySize>_MaxHistorySize)
		{
			_FileRequestHistory.pop_back();
			--_FileRequestHistorySize;
		}

		// deal with progress / state info
		uint32 fileLen=CFileManager::getInstance().getFileSize(fullFileName);
		_AdministeredModuleWrapper.registerProgress("File data: "+sender->getModuleName()+" "+fileName+NLMISC::toString("(%d..%d/%d)",startOffset,startOffset+numBytes,fileLen));
		_AdministeredModuleWrapper.setStateVariable("DataReq",NLMISC::toString(_FileRequestCount));

		// set state variables to reflect 'end of file reached'
		if (fileLen<startOffset+numBytes)
		{
			_AdministeredModuleWrapper.registerError("Invalid request from: "+sender->getModuleName()+" for: '"+fileName+"'");
			_AdministeredModuleWrapper.registerProgress("Failed file: "+fileName);
		}
		else if (fileLen==startOffset+numBytes)
		{
			_AdministeredModuleWrapper.registerProgress("Finished file: "+fileName);
		}
	}

	void CFileRepository::subscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// build the string that we'll use to represent this subscription
		NLMISC::CSString subscriptionString= sender->getModuleName()+" @ "+fileSpec;

		// make sure the entry doesn't exist in our subscribers set
		DROP_IF(_Subscribers.find(subscriptionString)!=_Subscribers.end(),"Ignoring dumplicate request for the same subscription: '"+subscriptionString+'\'',return);

		// display a fancy message
		_AdministeredModuleWrapper.registerProgress("Subscribe '"+subscriptionString+"'");

		// get hold of the info on the requested files as it stands right now
		TFileInfoVector fileInfoVector;
		getFileInfo(fileSpec,fileInfoVector,sender);

		// dispatch the info to the sender
		CFileReceiverProxy client(sender);
		client.cbFileInfo(_Parent,fileInfoVector);

		// add this entry to our subscriptions set
		_Subscribers[subscriptionString]= sender;
	}

	void CFileRepository::unsubscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// build the string that we'll use to represent this subscription
		NLMISC::CSString subscriptionString= sender->getModuleName()+" @ "+fileSpec;

		// display a fancy message
		_AdministeredModuleWrapper.registerProgress("Unsubscribe '"+subscriptionString+"'");

		// make sure that this subscription really exists
		DROP_IF(_Subscribers.find(subscriptionString)==_Subscribers.end(),"Ignoring unsubscribe for unknown subscription: '"+subscriptionString+'\'',return);

		// remove the subscription
		_Subscribers.erase(subscriptionString);
	}

	void CFileRepository::unsubscribeAll(NLNET::IModuleProxy *sender)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// build the string that this module's subscriptions will contain
		NLMISC::CSString subscriptionString= sender->getModuleName()+" @ ";

		// iterate over our subscriptions looking for matches to erase
		TSubscribers::iterator last;
		TSubscribers::iterator it;
		for (it=_Subscribers.begin();it!=_Subscribers.end();)
		{
			// take a copy of the iterator and increment to point at the next element to be processed
			last=it;
			++it;

			// see if the last element needs to be erased
			if (last->second==sender)
			{
				// delegate to standard 'unsubscribe' to do the work
				unsubscribe(sender,last->first.splitFrom('@').strip());
			}
		}
	}

	void CFileRepository::getInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		_AdministeredModuleWrapper.registerProgress("getInfo "+sender->getModuleName()+" "+fileSpec);
		subscribe(sender,fileSpec);
		unsubscribe(sender,fileSpec);
	}

	void CFileRepository::cbFileInfoUpdate(const SFileInfo& fileInfo)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		_AdministeredModuleWrapper.registerProgress(NLMISC::toString("cbFileInfoUpdate: %s (%d bytes)",fileInfo.FileName.c_str(),fileInfo.FileSize));

		// add an entry to our changes list
		_FileInfoChanges.push_back(fileInfo);
	}

	void CFileRepository::cbFileInfoErased(const NLMISC::CSString& fileName)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		_AdministeredModuleWrapper.registerProgress("cbFileInfoErased: "+fileName);

		// add an entry to our changes list (this is an empty entry with just the filename set to represent a deleted file)
		SFileInfo fileInfo;
		fileInfo.FileName= fileName;
		_FileInfoChanges.push_back(fileInfo);
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, incRescan)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (args.size()!=0)
			return false;

		CFileRepository::rescanPartial();

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, fullRescan)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (args.size()!=0)
			return false;

		CFileRepository::rescanFull();

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, getFile)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (args.size()!=1)
			return true;

		NLMISC::CSString data;
		CFileRepository::getFile(args[0],data,NULL);
		log.displayNL("Retrieved %u bytes of data for file %s, starting: %s",data.size(), args[0].c_str(), data.left(20).quote().c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, getFileInfo)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// make sure a filespec was given
		if (args.size()!=1)
			return true;

		// lookup the set of files
		TFileInfoVector fileInfoVector;
		getFileInfo(args[0],fileInfoVector,NULL);

		// display a summary info message
		log.displayNL("Result of info request '%s': %d matches",args[0].c_str(),fileInfoVector.size());
		log.displayNL("- %-32s %10s %10s %s","checksum","time","size","name");

		// iterate over results, displaying the info
		for (TFileInfoVector::iterator it= fileInfoVector.begin(); it!= fileInfoVector.end(); ++it)
		{
			log.displayNL("- %-32s %10u %10u %s",it->Checksum.toString().c_str(),it->FileTime,it->FileSize,it->FileName.c_str());
		}
		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, updateFile)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (args.size()!=1)
			return true;

		CFileRepository::updateFile(args[0]);

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CFileRepository, MaxHistorySize)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		switch(args.size())
		{
		case 1:
			{
				uint32 newVal= NLMISC::CSString(args[0]).atoui();
				if (newVal==0 && args[0]!="0")
					break;

				CFileRepository::setMaxHistorySize(newVal);
			}
			// drop through...

		case 0:
			log.displayNL("MaxHistorySize %u",_MaxHistorySize);
			return true;
		}

		return false;
	}

} // end of namespace
