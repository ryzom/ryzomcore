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

#ifndef REMOTE_SAVES_INTERFACE_H
#define REMOTE_SAVES_INTERFACE_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nem
#include "nel/misc/smart_ptr.h"

// game share
#include "game_share/file_description_container.h"


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// Quick notes on ISavesCallback classes:
	//
	// ISavesCallback				- an interface for classes that want to handle all callbacks
	// ISavesFileListCallback		- an interface for classes that only want to for handle file list callbacks
	// ISavesFileReceiveCallback	- an interface for classes that only want to for handle file receive callbacks


	//-----------------------------------------------------------------------------
	// class ISavesCallback
	//-----------------------------------------------------------------------------

	class ISavesCallback: public NLMISC::CRefCount
	{
	public:
		// virtual dtor
		virtual ~ISavesCallback() {}

		// callback when the file list is first received - this happens only once at the start
		virtual void cbInit(const CFileDescriptionContainer& fdc) =0;

		// callback whenever a file change list is received
		virtual void cbFileListChanged(	const CFileDescriptionContainer& newFiles,
										const CFileDescriptionContainer& modifiedFiles,
										const NLMISC::CVectorSString&	 deletedFile,
										const CFileDescriptionContainer& oldFileList,
										const CFileDescriptionContainer& newFileList) =0;

		// callback when the body of a file is received
		virtual void cbFileReceived(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody) =0;

		// callback that receives generic replies for diferent messages that are sent to SAVES module
		virtual void cbGenericReply(uint32 requestId,bool successFlag,const NLMISC::CSString& explanation) =0;
	};
	typedef NLMISC::CSmartPtr<ISavesCallback> TSavesCallbackPtr;


	//-----------------------------------------------------------------------------
	// class ISavesFileListCallback
	//-----------------------------------------------------------------------------

	class ISavesFileListCallback: public ISavesCallback
	{
	public:
		// virtual dtor
		virtual ~ISavesFileListCallback() {}

		// callback when the file list is first received - this happens only once at the start
		virtual void cbInit(const CFileDescriptionContainer& fdc) =0;

		// callback whenever a file change list is received
		virtual void cbFileListChanged(	const CFileDescriptionContainer& newFiles,
										const CFileDescriptionContainer& modifiedFiles,
										const NLMISC::CVectorSString&	 deletedFile,
										const CFileDescriptionContainer& oldFileList,
										const CFileDescriptionContainer& newFileList) =0;

		// empty implementations for unwanted callbacks
		virtual void cbFileReceived(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody) {}
		virtual void cbGenericReply(uint32 requestId,bool successFlag,const NLMISC::CSString& explanation) {}
	};


	//-----------------------------------------------------------------------------
	// class ISavesFileReceiveCallback
	//-----------------------------------------------------------------------------

	class ISavesFileReceiveCallback: public ISavesCallback
	{
	public:
		// virtual dtor
		virtual ~ISavesFileReceiveCallback() {}

		// empty implementations for unwanted callbacks
		void cbInit(const CFileDescriptionContainer& fdc) {}
		void cbFileListChanged(	const CFileDescriptionContainer& newFiles,
								const CFileDescriptionContainer& modifiedFiles,
								const NLMISC::CVectorSString&	 deletedFile,
								const CFileDescriptionContainer& oldFileList,
								const CFileDescriptionContainer& newFileList) {}

		// callback when the body of a file is received
		virtual void cbFileReceived(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody) =0;

		// callback that receives generic replies for diferent messages that are sent to SAVES module
		virtual void cbGenericReply(uint32 requestId,bool successFlag,const NLMISC::CSString& explanation) =0;
	};


	//-----------------------------------------------------------------------------
	// class CRemoteSavesInterface
	//-----------------------------------------------------------------------------

	class CRemoteSavesInterface: public NLMISC::CRefCount
	{
	public:
		// ctor - sets subscription destination and mode
		CRemoteSavesInterface(const NLMISC::CSString& shardName,const NLMISC::CSString& type);
		// dtor
		virtual ~CRemoteSavesInterface();
		// flag to say whether the interface is connected or not - will be set to true just before first
		// call to cbInit() for registered callbacks
		bool isReady() const;

		// get shard name as declared at module instantiation
		const NLMISC::CSString& getShardName() const;
		// get shard type as declared at module instantiation
		const NLMISC::CSString& getType() const;

		// define the data type for the container used to retrieve the callback set
		typedef std::vector<TSavesCallbackPtr> TCallbackSet;
		// get the set of callback objects
		void getCallbacks(TCallbackSet& result);
		// add a callback object to receive file list changes and file request replies
		void addCallbackObject(const TSavesCallbackPtr& cb);
		// remove a callback object
		void removeCallbackObject(const TSavesCallbackPtr& cb);

		// download a file from the server and call the registered callback(s)
		// returns the unique request id that will be supplied to the callbacks when a reply is received
		// returns ~0u if the file is not found in the currently active FDC
		uint32 requestFile(const NLMISC::CSString& fileName);
		// upload a file to the server
		// returns the unique request id that will be supplied to the callbacks when a reply is received
		// returns ~0u if the file isn't in the directory tree represented by the SAVES module
		uint32 uploadFile(const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody);
		// delete a file on the server
		// returns the unique request id that will be supplied to the callbacks when a reply is received
		// returns ~0u if the file is not found in the currently active FDC
		uint32 deleteFile(const NLMISC::CSString& fileName);

		// get the FDC container for the file list as it currently stands
		// returns false and clears result if not isReady()
		bool getFileList(CFileDescriptionContainer& result) const;

	private:
		NLMISC::CSString			_ShardName;
		NLMISC::CSString			_Type;
		TCallbackSet				_Callbacks;
	};
	typedef NLMISC::CSmartPtr<CRemoteSavesInterface> TRemoteSavesInterfacePtr;


	//-----------------------------------------------------------------------------
	// class CShardSavesInterface
	//-----------------------------------------------------------------------------

	class CShardSavesInterface: public CRemoteSavesInterface
	{
	public:
		// ctor - sets subscription destination and mode
		CShardSavesInterface(const NLMISC::CSString& shardName);
		// get full name of account_names.txt file
		static const NLMISC::CSString& getAccountNamesFileName();
		// get full name of character_names.txt file
		static const NLMISC::CSString& getCharacterNamesFileName();
		// get full name of the game_cycle.ticks file
		static const NLMISC::CSString& getGameCycleFileName();
		// get full name of the gm_pending_tp.bin file
		static const NLMISC::CSString& getGMPendingTPFileName();
		// get full name and path of character save file
		static NLMISC::CSString getCharacterSaveFileName(uint32 account,uint32 slot);
		// get the list of character save files - return false if not isReady()
		bool getCharacterFileList(CFileDescriptionContainer& result) const;
		// get the list of guild save files - return false if not isReady()
		bool getGuildFileList(CFileDescriptionContainer& result) const;
		// get the list of files for items for sale in the sale store - return false if not isReady()
		bool getSaleStoreFileList(CFileDescriptionContainer& result) const;
		// get the list of files for offline character commands - return false if not isReady()
		bool getOfflineCharacterCommandsFileList(CFileDescriptionContainer& result) const;
	};


	//-----------------------------------------------------------------------------
	// class CMailSavesInterface
	//-----------------------------------------------------------------------------

	class CMailSavesInterface: public CRemoteSavesInterface
	{
	public:
		// ctor - sets subscription destination and mode
		CMailSavesInterface(const NLMISC::CSString& shardName);
		// get the set of files for a given entity - return false if not isReady()
		bool getEntityFileList(const NLMISC::CSString& entityName,CFileDescriptionContainer& result) const;
		// move the files for an entity from one folder to another - return false if not isReady()
		bool moveEntityFiles(const NLMISC::CSString& oldEntityName,const NLMISC::CSString& newEntityName,bool overwrite=false);
	};


	//-----------------------------------------------------------------------------
	// methods CIncrementalBackupSavesInterface
	//-----------------------------------------------------------------------------

	class CIncrementalBackupSavesInterface: public CRemoteSavesInterface
	{
	public:
		// ctor - sets subscription destination and mode
		CIncrementalBackupSavesInterface(const NLMISC::CSString& shardName);

		// *** todo *** - add sensible interface for incremental backup files here
	};
}


//-----------------------------------------------------------------------------
#endif
