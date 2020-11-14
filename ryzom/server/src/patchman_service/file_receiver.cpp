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
#include "nel/misc/mem_stream.h"
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"

// local
#include "file_receiver.h"
#include "module_admin_itf.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//-----------------------------------------------------------------------------
// some NLMISC Variable
//-----------------------------------------------------------------------------

NLMISC::CVariable<uint32> FileReceiverMaxMessageCount("patchman","FileReceiverMaxMessageCount", "number of packets we're allowed to send at a time", 100, 0, true );
NLMISC::CVariable<uint32> FileReceiverDataBlockSize("patchman","FileReceiverDataBlockSize", "maximum size of each data packet", 10480, 0, true );


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{		

	//-----------------------------------------------------------------------------
	// methods CFileReceiver - basics
	//-----------------------------------------------------------------------------

	CFileReceiver::CFileReceiver()
	{
		_Parent=NULL;
	}

	void CFileReceiver::init(NLNET::IModule* parent,const std::string& fileSpec)
	{
		CFileReceiverSkel::init(parent);
		_Parent= parent;
		_FileSpec= fileSpec;
		_AdministeredModuleWrapper.init(dynamic_cast<CAdministeredModuleBase*>(parent));
	}

	bool CFileReceiver::haveIdleProxies() const
	{
		// if we can find an idle proxy then return true
		for (TProxies::const_iterator pit= _Proxies.begin(); pit!=_Proxies.end();++pit)
		{
			if (pit->second.CurrentRequest==NULL)
				return true;
		}

		// no idle proxies found...
		return false;
	}

	void CFileReceiver::dump(NLMISC::CLog& log) const
	{
		log.displayNL("-----------------------------------");
		log.displayNL("File requests");
		log.displayNL("-----------------------------------");
		for (TFileRequests::const_iterator fit= _FileRequests.begin(); fit!= _FileRequests.end(); ++fit)
		{
			SFileRequest& request= *(*fit);
			log.displayNL("- File: '%s' %s (%d..%d/%d)",
				request.FileName.c_str(),
				(request.Emitter==NULL)? "No emitter": request.Emitter->getModuleName().c_str(),
				request.DataSoFar.size(),
				request.TotalDataRequested,
				request.ExpectedFileSize );
		}

		log.displayNL("-----------------------------------");
		log.displayNL("Connected proxies");
		log.displayNL("-----------------------------------");
		for (TProxies::const_iterator pit= _Proxies.begin(); pit!= _Proxies.end(); ++pit)
		{
			log.displayNL("- Repository  %s (%d files): Current Request: %s",
				pit->second.Proxy->getModuleName().c_str(),
				pit->second.FileInfo.size(),
				(pit->second.CurrentRequest==NULL)? "None": pit->second.CurrentRequest->FileName.c_str());
		}
		log.displayNL("-----------------------------------");
	}

	void CFileReceiver::dumpFileInfo(const std::string &fileSpec,NLMISC::CLog& log) const
	{
		// setup a vector to hold fileInfo results and call getFileInfo() to fill it in
		TFileInfoVector result;
		getFileInfo(fileSpec,result);

		// display a summary info message
		log.displayNL("Result of info request '%s': %d matches",fileSpec.c_str(),result.size());
		log.displayNL("- %-32s %10s %10s %s","checksum","time","size","name");

		// iterate over results, displaying the info
		for (TFileInfoVector::iterator it= result.begin(); it!=result.end(); ++it)
		{
			log.displayNL("- %-32s %10u %10u %s",it->Checksum.toString().c_str(),it->FileTime,it->FileSize,it->FileName.c_str());
		}
	}


	//-----------------------------------------------------------------------------
	// methods CFileReceiver - called from CModuleBase specialisations
	//-----------------------------------------------------------------------------

	void CFileReceiver::onModuleUp(NLNET::IModuleProxy *module)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (NLMISC::CSString(module->getModuleManifest()).contains(ManifestEntryIsFileRepository))
		{
			CFileRepositoryProxy spr(module);
			spr.subscribe(_Parent,_FileSpec);
			_log("Repository up: "+module->getModuleName());
			_Proxies[module].Proxy= module;
		}
	}

	void CFileReceiver::onModuleDown(NLNET::IModuleProxy *module)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		if (_Proxies.find(module)!=_Proxies.end())
		{
			_log("Repository down: "+module->getModuleName());

			// get a refference to the proxy
			SProxyInfo& theProxy= _Proxies[module];

			// signal the change of state of files that appear in the proxy file list
			for (TFileInfoMap::const_iterator fit= theProxy.FileInfo.begin(); fit!=theProxy.FileInfo.end();++fit)
			{
				// call a user callback (if there is one), signalling the change of info for the given file
				cbFileInfoChange(fit->second.FileName);
			}

			// grab the request that was running (if there was one)
			SFileRequest* theRequest= _Proxies[module].CurrentRequest;

			// remove the proxy from the proxies map
			_Proxies.erase(module);

			// try to reassign the request to someone else
			if (theRequest!=NULL)
			{
				// cleanup the broken file request and re-dispatch if possible
				_treatBrokenFileRequest(theRequest);
			}
		}
	}

	void CFileReceiver::onModuleUpdate()
	{
	}

	const std::string &CFileReceiver::getModuleManifest() const
	{
		static std::string manifest= ManifestEntryIsFileReceiver;
		return manifest;
	}


	//-----------------------------------------------------------------------------
	// methods CFileReceiver - main API
	//-----------------------------------------------------------------------------

	void CFileReceiver::requestFile(const std::string &fileName)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);
		_log("Registering request for: "+fileName);

		// create our new file request record
		TFileRequestPtr newRequest= new SFileRequest;
		newRequest->FileName= fileName;
		newRequest->ExpectedFileSize= ~0u;
		_FileRequests.push_back(newRequest);

		// try to dispatch the request to one of our proxies
		_requestFile(newRequest);
	}

	bool CFileReceiver::getSingleFileInfo(const std::string &fileName,SFileInfo& fileInfo) const
	{
		// run through the attached proxies to find a match for the file...
		for (TProxies::const_iterator pit= _Proxies.begin(); pit!=_Proxies.end();++pit)
		{
			// if there's a match for this proxy then it'll do
			TFileInfoMap::const_iterator fit= pit->second.FileInfo.find(fileName);
			if (fit!=pit->second.FileInfo.end())
			{
				fileInfo= fit->second;
				return true;
			}
		}

		// failed to find a match so clear out the file info record and return false
		fileInfo.clear();
		return false;
	}

	void CFileReceiver::getFileInfo(const std::string &fileSpec,TFileInfoVector& result) const
	{
		// setup an object for testing matches for a given filespec
		CFileSpec pattern(fileSpec);

		// run through the attached proxies to find a match for the file...
		for (TProxies::const_iterator pit= _Proxies.begin(); pit!=_Proxies.end();++pit)
		{
			// if this is a request for a particular file then just do a lookup
			if (!pattern.isWild())
			{
				// if there's a match for this proxy then it'll do
				TFileInfoMap::const_iterator fit= pit->second.FileInfo.find(fileSpec);
				if (fit!=pit->second.FileInfo.end())
				{
					result.push_back(fit->second);
				}
				continue;
			}

			// iterate over all of the entries for the proxy
			for (TFileInfoMap::const_iterator fit=pit->second.FileInfo.begin(); fit!= pit->second.FileInfo.end(); ++fit)
			{
				// if the pattern is set to 'all' then iterate
				if (pattern.matches(fit->second.FileName))
				{
					result.push_back(fit->second);
				}
			}
		}
	}


	//-----------------------------------------------------------------------------
	// methods CFileReceiver - message callbacks
	//-----------------------------------------------------------------------------

	void CFileReceiver::setupSubscriptions(NLNET::IModuleProxy *sender)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// make sure the proxy that sent this list exist
		DROP_IF(_Proxies.find(sender)==_Proxies.end(),"Ignoring unexpected SetupSubscriptions from module "+sender->getModuleName(),return);

		// send the subscription request
		CFileRepositoryProxy spr(sender);
		spr.subscribe(_Parent,_FileSpec);
		_log(NLMISC::toString("setupSubscriptions from: %s",sender->getModuleName().c_str()));
	}

	void CFileReceiver::cbFileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &files)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);
		_log(NLMISC::toString("List (%d files) from: %s",files.size(),sender->getModuleName().c_str()));

		// make sure the proxy that sent this list exist
		DROP_IF(_Proxies.find(sender)==_Proxies.end(),"Ignoring unexpected file list from module "+sender->getModuleName(),return);

		// get a refference to the proxy
		SProxyInfo& theProxy= _Proxies[sender];

		// store away the proxy's file list
		for (TFileInfoVector::const_iterator fit= files.begin(); fit!=files.end();++fit)
		{
			if (fit->FileTime!=0)
			{
				theProxy.FileInfo[fit->FileName]= *fit;
			}
			else
			{
				theProxy.FileInfo.erase(fit->FileName);
			}

			// call a user callback (if there is one), signalling the change of info for the given file
			cbFileInfoChange(fit->FileName);
		}

		// if the proxy was idle then look for something to do
		if (theProxy.CurrentRequest==NULL)
		{
			_lookForNewJob(theProxy);
		}
	}

	void CFileReceiver::cbFileData(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);

		// look for the request that this data block corresponds to
		DROP_IF(_Proxies.find(sender)==_Proxies.end(),"Ignoring unexpected file data for file '"+fileName+"' from module "+sender->getModuleName(),return);

		SProxyInfo& theSender= _Proxies[sender];
		TFileRequestPtr theRequest= theSender.CurrentRequest;
		DROP_IF(theRequest==NULL,"Ignoring unexpected file data for file '"+fileName+"' from module "+sender->getModuleName(),return);
		BOMB_IF(theRequest->Emitter!=theSender.Proxy,"Ignoring file data for file '"+fileName+"' from broken module "+sender->getModuleName(),return);
		DROP_IF(theRequest->FileName!=fileName,"Ignoring unexpected file data for file '"+fileName+"' when expecting file '"+theRequest->FileName+"' from module "+sender->getModuleName(),return);

		// clear the download log for this file in case we've finished ... we'll set it again at the end of the routine
		_clearDownloadLog(fileName);

		// did we just deal the whole file in a single block?
		if (data.getBufferSize()>=theRequest->ExpectedFileSize)
		{
			// we've reached the end of file
			_dealWithReceivedFile(sender,theRequest,data);
			return;
		}

		// add the data to the file
		CSString& theBuffer= theRequest->DataSoFar;
		uint32 oldSize= (uint32)theBuffer.size();
		theBuffer.resize(oldSize+data.getBufferSize());
		memcpy(&(theBuffer[oldSize]), data.getBuffer(), data.getBufferSize());

		// have we reached the end of file?
		if (theRequest->DataSoFar.size()>=theRequest->ExpectedFileSize)
		{
			// we've reached the end of file
			_dealWithReceivedFile(sender,theRequest,NLNET::TBinBuffer((const uint8 *)&theRequest->DataSoFar[0],(uint32)theRequest->DataSoFar.size()));
			return;
		}

		// we're not at the end of file so think about adding a request for another data block
		if (theRequest->TotalDataRequested< theRequest->ExpectedFileSize)
		{
			// work out size of block to request
			uint32 requestSize= min( uint32(theRequest->ExpectedFileSize- theRequest->TotalDataRequested), uint32(FileReceiverDataBlockSize) );

			// send the request
			CFileRepositoryProxy fr(sender);
			fr.requestFileData(_Parent,fileName,theRequest->TotalDataRequested,requestSize);

			// register info about the request we sent
			theRequest->TotalDataRequested+= requestSize;
		}

		// log our progress
		_downloadLog(fileName,(uint32)theRequest->DataSoFar.size(),theRequest->ExpectedFileSize);
	}

	void CFileReceiver::cbFileDataFailure(NLNET::IModuleProxy *sender, const std::string &fileName)
	{
		// make sure we've been initialised
		nlassert(_Parent!=NULL);
		_logError("Read Failure "+sender->getModuleName()+": "+fileName);

		// clear the download log for this file as it's all broken
		_clearDownloadLog(fileName);

		// look for the request that this data block corresponds to
		DROP_IF(_Proxies.find(sender)==_Proxies.end(),"Ignoring unexpected file read failure for file '"+fileName+"' from module "+sender->getModuleName(),return);

		SProxyInfo& theSender= _Proxies[sender];
		TFileRequestPtr theRequest= theSender.CurrentRequest;
		
		DROP_IF(theRequest==NULL,"Ignoring unexpected file data for file '"+fileName+"' from module "+sender->getModuleName(),return);
		DROP_IF(theRequest->Emitter==NULL,"Ignoring already treaded file error for '"+fileName+"' from module "+sender->getModuleName(),return);
		BOMB_IF(theRequest->Emitter!=theSender.Proxy,"Ignoring file read failure for file '"+fileName+"' from broken module "+sender->getModuleName(),return);
		DROP_IF(theRequest->FileName!=fileName,"Ignoring unexpected file read failure for file '"+fileName+"' when expecting file '"+theRequest->FileName+"' from module "+sender->getModuleName(),return);

		// cleanup the broken file request and re-dispatch if possible
		_treatBrokenFileRequest(theRequest);
	}


	//-----------------------------------------------------------------------------
	// methods CFileReceiver - private methods
	//-----------------------------------------------------------------------------

	void CFileReceiver::_log(const NLMISC::CSString& msg) const
	{
		const CAdministeredModuleBase* adminModule= dynamic_cast<const CAdministeredModuleBase*>(_Parent);
		if (adminModule!=NULL)
		{
			adminModule->registerProgress(msg);
		}
		else
		{
			nldebug("CFileReceiver_%s",msg.c_str());
		}
	}

	void CFileReceiver::_logError(const NLMISC::CSString& msg) const
	{
		const CAdministeredModuleBase* adminModule= dynamic_cast<const CAdministeredModuleBase*>(_Parent);
		if (adminModule!=NULL)
		{
			adminModule->registerError(msg);
		}
		else
		{
			nlwarning("CFileReceiver: %s",msg.c_str());
		}
	}

	void CFileReceiver::_logState(const NLMISC::CSString& state) const
	{
		const CAdministeredModuleBase* adminModule= dynamic_cast<const CAdministeredModuleBase*>(_Parent);
		if (adminModule!=NULL)
		{
			adminModule->setStateVariable("state",state);
		}
	}

	void CFileReceiver::_downloadLog(const NLMISC::CSString& fileName,uint32 bytesSoFar, uint32 bytesExpected) const
	{
		const CAdministeredModuleBase* adminModule= dynamic_cast<const CAdministeredModuleBase*>(_Parent);
		if (adminModule!=NULL)
		{
			adminModule->setStateVariable(fileName,NLMISC::toString("%d/%d",bytesSoFar,bytesExpected));
		}
		else
		{
			nldebug("CFileReceiver_Download: %s: %d/%d",fileName.c_str(),bytesSoFar,bytesExpected);
		}
	}

	void CFileReceiver::_clearDownloadLog(const NLMISC::CSString& fileName) const
	{
		const CAdministeredModuleBase* adminModule= dynamic_cast<const CAdministeredModuleBase*>(_Parent);
		if (adminModule!=NULL)
		{
			adminModule->clearStateVariable(fileName);
		}
	}

	void CFileReceiver::_requestFile(TFileRequestPtr theRequest)
	{
		// if all of the proxies are busy then giveup
		if (!haveIdleProxies())
			return;

		// use a little set for the proxies capable of responding to my request
		TFileRequestMatches requestMatches;

		// give each of our connected proxys a chance to take on this new file
		for (TProxies::iterator pit= _Proxies.begin(); pit!=_Proxies.end();++pit)
		{
			// see whether the proxy has a record for the file that we're after
			TFileInfoMap::iterator fit= pit->second.FileInfo.find(theRequest->FileName);
			if (fit!=pit->second.FileInfo.end())
			{
				requestMatches[pit->first]=fit->second;
			}
		}

		// if we found no candidates for our file then throw a warning and give up
		DROP_IF(requestMatches.empty(),"No connected emitters found for requested file: "+theRequest->FileName,return);

		// validate the set of request matches
		// this routine will strip out any requests that the derived class doesn't like
		// the set may be empty on return
		cbValidateRequestMatches(requestMatches);

		// we're good to go if we can find a proxy who's not busy already...
		for (TFileRequestMatches::iterator rit= requestMatches.begin(); rit!=requestMatches.end(); ++rit)
		{
			// if the proxy doesn't exist then skip it
			BOMB_IF(_Proxies.find(rit->first)==_Proxies.end(),"ERROR: Ignoring bad proxy value in _requestFile()",continue);
			SProxyInfo& theProxy= _Proxies[rit->first];

			// if the proxy is busy then just skip on forwards...
			if (theProxy.CurrentRequest!=NULL)
				continue;

			// we've found a proxy who's not busy so we can dispatch messages...
			CFileRepositoryProxy fr(_Proxies[rit->first].Proxy);

			// set the job that the proxy is working on
			theProxy.CurrentRequest= theRequest;
			theRequest->Emitter= theProxy.Proxy;
			theRequest->ExpectedFileSize= theProxy.FileInfo[theRequest->FileName].FileSize;
			theRequest->DataSoFar.reserve(theRequest->ExpectedFileSize);

			// dispatch as many packets as we're allowed to...
			for (uint32 i=0;i<FileReceiverMaxMessageCount;++i)
			{
				// work out size of block to request
				uint32 requestSize= min((uint32)FileReceiverDataBlockSize,theRequest->ExpectedFileSize-theRequest->TotalDataRequested);

				// dispatch the request
				fr.requestFileData(_Parent,theRequest->FileName,theRequest->TotalDataRequested,requestSize);

				// update the count of data requested
				theRequest->TotalDataRequested+= requestSize;

				// make sure we don't try to send more info than the file contains
				nlassert(theRequest->ExpectedFileSize>=theRequest->TotalDataRequested);

				// if we've requested all of the data that there is in the file then break out now
				if (theRequest->ExpectedFileSize==theRequest->TotalDataRequested)
					break;
			}
		}
		_downloadLog(theRequest->FileName,0,theRequest->ExpectedFileSize);
	}

	void CFileReceiver::_dealWithReceivedFile(TProxyPtr sender,TFileRequestPtr theRequest,const NLNET::TBinBuffer& data)
	{
		// log progress..
		_clearDownloadLog(theRequest->FileName);
		_log("receivedFile: "+theRequest->FileName+NLMISC::toString("(%d bytes)",data.getBufferSize()));

		// call the user callback for the file data
		NLMISC::CMemStream memStream;
		memStream.fill(data.getBuffer(),data.getBufferSize());
		memStream.invert();
		cbFileDownloadSuccess(theRequest->FileName,memStream);

		// look for the proxy record for the emitter
		TProxies::iterator pit= _Proxies.find(sender);
		BOMB_IF(pit==_Proxies.end(),"ERROR: Failed to identify the sender for the received file: "+theRequest->FileName,return);

		// liberate this request
		for (TFileRequests::iterator fit=_FileRequests.begin(); fit!=_FileRequests.end();++fit)
		{
			if (*fit==theRequest)
			{
				_FileRequests.erase(fit);
				break;
			}
		}

		// cleanup the emitter
		pit->second.CurrentRequest= NULL;

		// look for a new job for the sender
		_lookForNewJob(pit->second);
	}

	void CFileReceiver::_treatBrokenFileRequest(TFileRequestPtr theRequest)
	{
		// log progress..
		_clearDownloadLog(theRequest->FileName);
		_logError("treatBrokenFile: "+theRequest->FileName);

		// call the user callback
		cbRetryAfterFileDownloadFailure(theRequest->FileName);

		// remove the file entry from the map of the sender to ensure that we don't lock up
		if (_Proxies.find(theRequest->Emitter)!= _Proxies.end())
		{
			_Proxies[theRequest->Emitter].FileInfo.erase(theRequest->FileName);
		}

		// cleanup proxy usage
		if (_Proxies.find(theRequest->Emitter) != _Proxies.end())
		{
			_Proxies[theRequest->Emitter].CurrentRequest = NULL;
		}

		// cleanup the request in order to be able to reassign it
		theRequest->Emitter= NULL;
		theRequest->ExpectedFileSize= 0;
		theRequest->DataSoFar.clear();
		theRequest->TotalDataRequested = 0;

		// try to reassign the request...
		_requestFile(theRequest);
	}

	void CFileReceiver::_lookForNewJob(SProxyInfo& theProxy)
	{
		// run through the request set looking for requests that haven't yet been handled...
		for (TFileRequests::iterator fit=_FileRequests.begin(); fit!=_FileRequests.end() && theProxy.CurrentRequest==NULL;++fit)
		{
			if ((*fit)->Emitter==NULL)
			{
				_requestFile(*fit);
			}
		}

		_logState(theProxy.CurrentRequest==NULL?"Idle":"Busy");
	}

} // end of namespace
