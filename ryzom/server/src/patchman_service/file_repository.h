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

/** 
 * ***********************************************************************
 * ****************************** IMPORTANT ******************************
 * ***********************************************************************
 *
 * The following set of routines all need to be called by the derived
 * class for the repositry to function correctly:
 *	- init()
 *	- onModuleUp()
 *	- onModuleDown()
 *	- onModuleUpdate()
 *	- onDispatchMessage()
 *	- getModuleManifest()
 *
 * ***********************************************************************
 *
 */

#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/md5.h"
#include "nel/net/module_builder_parts.h"

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
	// class CFileRepository
	//-----------------------------------------------------------------------------

	class CFileRepository:
		public CFileRepositorySkel,
		public IFileRequestValidator,
		public IFileInfoUpdateListener
	{
	public:
		// get the module manifest component required for CFileReceiver modules to recognise us
		std::string buildModuleManifest() const;

	public:
		// ctor and init
		CFileRepository();

		// methods for use in derived classes
		void init(NLNET::IModule* parent,const NLMISC::CSString& rootDirectory);
		void onModuleUp(NLNET::IModuleProxy *module);
		void onModuleDown(NLNET::IModuleProxy *module);
		void onModuleUpdate();

		// update methods
		void rescanFull();
		void rescanPartial();
		void updateFile(const NLMISC::CSString& fileName);

		// overloadable method for getting the file list for subscribers
		virtual void getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,const NLNET::IModuleProxy* sender) const;

		// query methods
		TRepositoryDirectoryPtr getRepositoryDirectory();
		void getFile(const NLMISC::CSString& fileName,NLMISC::CSString& resultData,const NLNET::IModuleProxy* sender) const;
		void dump(NLMISC::CLog& log);

		// stats system accessors
		void setMaxHistorySize(uint32 maxHistorySize);
		uint32 getMaxHistorySize() const;

	protected:
		// treatment of incoming file requests
		void requestFileInfo(NLNET::IModuleProxy *sender,const NLMISC::CSString& fileSpec);
		void requestFileData(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes);

		// treatment of subscriptions and unsubscriptions
		void subscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec);
		void unsubscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec);
		void unsubscribeAll(NLNET::IModuleProxy *sender);
		void getInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec);

		// IFileInfoUpdateListener specialisation implementation
		void cbFileInfoUpdate(const SFileInfo& fileInfo);
		void cbFileInfoErased(const NLMISC::CSString& fileName);

		// broadcast a set of file info changes to all subscribers
		void _broadcastFileInfoChanges(const TFileInfoVector& fileInfoChanges);

		// overloadable method for treating my module down
		virtual void onFileRepositoryModuleDown(NLNET::IModuleProxy *module);

	private:
		// private data

		typedef NLNET::CInterceptorForwarder<CFileRepository> TInterceptor;
		/// module interceptor forwarder
		TInterceptor	_Interceptor;

		friend class NLNET::CInterceptorForwarder<CFileRepository>;

		// the module object that this object is part of
		NLNET::IModule* _Parent;

		// a wrapper round '_Parent' to allow us to log to the parent's CAdministeredModuleBase interface
		CAdministeredModuleWrapper _AdministeredModuleWrapper;

		// the set of subscribers
		typedef std::map<NLMISC::CSString,NLNET::IModuleProxy*> TSubscribers;
		TSubscribers _Subscribers;

		// the recent info changes that the subscribers may be interested in
		TFileInfoVector _FileInfoChanges;

		// system for history of recent info requests and data requests
		uint32 _MaxHistorySize;
		typedef std::list<NLMISC::CSString> THistory;

		// the history for info requests
		THistory _FileInfoHistory;
		uint32 _FileInfoHistorySize;
		uint32 _FileInfoCount;

		// the history for file data requests
		THistory _FileRequestHistory;
		uint32 _FileRequestHistorySize;
		uint32 _FileRequestCount;

	protected:
		// data that may be accessed directlry by derived objects

		// a smart pointers to our repository directory
		TRepositoryDirectoryPtr _Directory;


	protected:
		// declaration of NLMISC_COMMANDS implemented by this class
		NLMISC_CLASS_COMMAND_DECL(dump);
		NLMISC_CLASS_COMMAND_DECL(incRescan);
		NLMISC_CLASS_COMMAND_DECL(fullRescan);
		NLMISC_CLASS_COMMAND_DECL(getFile);
		NLMISC_CLASS_COMMAND_DECL(getFileInfo);
		NLMISC_CLASS_COMMAND_DECL(updateFile);
		NLMISC_CLASS_COMMAND_DECL(MaxHistorySize);

		// macro for adding the NLMISC_COMMANDS provided here to the derived class's command table
		#define NLMISC_COMMAND_HANDLER_ADDS_FOR_FILE_REPOSITORY(className) \
			NLMISC_COMMAND_HANDLER_ADD(className, incRescan, "Perform next increment of incremental rescan", "no args")\
			NLMISC_COMMAND_HANDLER_ADD(className, fullRescan, "Perform a full rescan", "no args")\
			NLMISC_COMMAND_HANDLER_ADD(className, getFile, "get a file", "<file name>")\
			NLMISC_COMMAND_HANDLER_ADD(className, getFileInfo, "get info on a set of files", "<file spec>")\
			NLMISC_COMMAND_HANDLER_ADD(className, updateFile, "force a rescan of a given file", "<file name>")\
			NLMISC_COMMAND_HANDLER_ADD(className, MaxHistorySize, "the maximum size allowed for the history buffers", "[<num entries>]")
	};

} // end of namespace

#endif
