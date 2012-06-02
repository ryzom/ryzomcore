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

#ifndef FILE_RECEIVER_H
#define FILE_RECEIVER_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
//#include "game_share/deployment_configuration.h"

// local
#include "module_admin_itf.h"
#include "administered_module.h"


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{		

	//-----------------------------------------------------------------------------
	// class CFileReceiver
	//-----------------------------------------------------------------------------

	class CFileReceiver:
		public PATCHMAN::CFileReceiverSkel
	{
	public:
		// stuff for the derived class to hook up ---------------------------------

		// ctor & init
		CFileReceiver();
		void init(NLNET::IModule* parent,const std::string& fileSpec="*/*");

		// CModuleBase specialisation implementation
		void onModuleUp(NLNET::IModuleProxy *module);
		void onModuleDown(NLNET::IModuleProxy *module);
		void onModuleUpdate();
		const std::string &getModuleManifest() const;

	public:
		// handy types ------------------------------------------------------------
		typedef NLNET::IModuleProxy* TProxyPtr;
		typedef std::map<TProxyPtr,SFileInfo> TFileRequestMatches;


		// remaining public interface ---------------------------------------------

		// send off a request for a file... the request will come back via the callbacks below
		void requestFile(const std::string &fileName);

		// lookup info on a particular file
		bool getSingleFileInfo(const std::string &fileName,SFileInfo& fileInfo) const;

		// lookup info on all files matching a given file spec
		void getFileInfo(const std::string &fileSpec,TFileInfoVector& result) const;

		// status info accessors
		bool haveIdleProxies() const;

		// a handy status dump method
		void dump(NLMISC::CLog& log) const;

		// a handy method to dump the set of files matching a given spec to the given log
		void dumpFileInfo(const std::string &fileSpec,NLMISC::CLog& log) const;


		// overloadable callback methods ------------------------------------------

		// The following method is called to allow user to invalidate matches for
		// a file request that they have made
		// entries can simply be erased from the requestMatches container
		// it is valid for the container to be empty on exit
		virtual void cbValidateRequestMatches(TFileRequestMatches& requestMatches) {}

		// the following callback is called after a file has been successfully downloaded
		virtual void cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data) {}

		// the following callback is called after a file download has failed
		// note that the system will not try re-downloading from the same emitter
		// unless the emitter sends us a file update at a later date re-iterating
		// the valisdity of their file (we assume the file has been deleted)
		// if another emitter claims to be able to deliver the file then the
		// system will retry automatically (no need for an extra retry request
		// to be generated here)
		virtual void cbRetryAfterFileDownloadFailure(const NLMISC::CSString& fileName) {}

		// the following callback is called after a new information record is recieved for a file
		// the information change may be either addition, deletion or info change for the given file from a given proxy
		// this can be the addition of an extra proxy providing the same info as other existing proxies
		virtual void cbFileInfoChange(const NLMISC::CSString& fileName) {}


	protected:
		// protected methods - for treating incoming messages ---------------------
		void setupSubscriptions(NLNET::IModuleProxy *sender);
		void cbFileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &files);
		void cbFileData(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data);
		void cbFileDataFailure(NLNET::IModuleProxy *sender, const std::string &fileName);

	private:
		// private data -----------------------------------------------------------
		NLNET::IModule* _Parent;
		NLMISC::CSString _FileSpec;
		CAdministeredModuleWrapper _AdministeredModuleWrapper;

		// some handy data types
		struct SFileRequest: public NLMISC::CRefCount
		{
			NLMISC::CSString FileName;
			NLMISC::CSString DataSoFar;
			TProxyPtr Emitter;
			uint32 ExpectedFileSize;
			uint32 TotalDataRequested;

			SFileRequest(): Emitter(NULL), ExpectedFileSize(0), TotalDataRequested(0) {}
		};
		typedef NLMISC::CSmartPtr<SFileRequest> TFileRequestPtr;
		typedef std::list<TFileRequestPtr> TFileRequests;

		// queued file requests
		TFileRequests _FileRequests;			// vector of files that need to be downloaded from RE module

		// connected proxies
		struct SProxyInfo
		{
			TProxyPtr			Proxy;			// the proxy id
			TFileInfoMap		FileInfo;		// the set of files reported by the proxy
			TFileRequestPtr		CurrentRequest;	// the request that the proxy is treating right now
		};
		typedef std::map<TProxyPtr,SProxyInfo> TProxies;
		TProxies _Proxies;

		// private methods --------------------------------------------------------
		void _log(const NLMISC::CSString& msg) const;
		void _logError(const NLMISC::CSString& msg) const;
		void _logState(const NLMISC::CSString& state) const;
		void _downloadLog(const NLMISC::CSString& fileName,uint32 bytesSoFar, uint32 bytesExpected) const;
		void _clearDownloadLog(const NLMISC::CSString& fileName) const;

		void _requestFile(TFileRequestPtr theRequest);
		void _treatBrokenFileRequest(TFileRequestPtr theRequest);
		void _dealWithReceivedFile(TProxyPtr sender,TFileRequestPtr theRequest,const NLNET::TBinBuffer& data);
		void _lookForNewJob(SProxyInfo& theProxy);
	};

} // end of namespace

//-----------------------------------------------------------------------------
#endif
