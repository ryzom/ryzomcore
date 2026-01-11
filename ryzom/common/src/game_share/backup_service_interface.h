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


#ifndef BACKUP_SERVICE_INTERFACE_H
#define	BACKUP_SERVICE_INTERFACE_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/net/unified_network.h"
#include "nel/misc/variable.h"
#include "nel/misc/hierarchical_timer.h"
#include "file_description_container.h"


//-------------------------------------------------------------------------------------------------
// Configuration Variables
//-------------------------------------------------------------------------------------------------

extern NLMISC::CVariable<std::string>	BackupServiceIP;
extern NLMISC::CVariable<bool>    		UseBS;


//-------------------------------------------------------------------------------------------------
// Forward class declarations
//-------------------------------------------------------------------------------------------------

class CBackupServiceInterface;


//-------------------------------------------------------------------------------------------------
// class CBackupFileClass
// A File Class is a set of file pattern that are considered as equivalent (in term of content.)
// Say a XML file may have equivalent content as a BIN file, as thus may may appear in same class
// Only file modification date is used to differenciate a more consistent file.
//-------------------------------------------------------------------------------------------------

class CBackupFileClass
{
public:

	/// Equal patterns for this class
	std::vector<std::string>	Patterns;

	void	serial(NLMISC::IStream& s)
	{
		s.serialCont(Patterns);
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgSaveFile
// For sending only: first construct a CBackupMsgSaveFile object by name
// then call serial() on its DataMsg member for each data to pack.
// This class is designed to prevent from duplicating serialization before sending.
//
// *** Note ***
// The variable UseBS must be in the same state when creating an object CBackupMsgSaveFile
// and when passing it to CBackupServiceInterface::sendFile() or CBackupServiceInterface::append().
//-------------------------------------------------------------------------------------------------

struct CBackupMsgSaveFile
{
	// Type of message (the TypesStr table must be synchronized with it)
	enum TBackupMsgSaveFileType
	{
		SaveFile,			// Save file and create directory tree if not existing
		SaveFileCheck,		// Save file and create directory tree if not existing (same as above)
		AppendFile,			// Append file and create directory tree if not existing
		AppendFileCheck,	// Append file and create directory tree if not existing (same as above)

		NbTypes
	};

	// Constructor for sending
	CBackupMsgSaveFile( const std::string& filename, TBackupMsgSaveFileType msgType, const CBackupServiceInterface& itf );

	NLNET::CMessage DataMsg;
	std::string FileName; // the filename passed to the constructor

private:

	TBackupMsgSaveFileType _MsgType;
	friend class CBackupServiceInterface;

	static const char *_TypesStr [NbTypes];
};


//-------------------------------------------------------------------------------------------------
// class IBackupFileReceiveCallback
//-------------------------------------------------------------------------------------------------

class IBackupFileReceiveCallback: public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(IBackupFileReceiveCallback);
public:
	virtual ~IBackupFileReceiveCallback() {}
	// note: on entry to the callback the quantity of data in dataStream is guaranteed to be
	// the same as the value of fileDescription.FileSize
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)=0;
};


//-------------------------------------------------------------------------------------------------
// class IBackupFileClassReceiveCallback
//-------------------------------------------------------------------------------------------------

class IBackupFileClassReceiveCallback : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(IBackupFileClassReceiveCallback);
public:

	virtual ~IBackupFileClassReceiveCallback() {}
	virtual void callback(const CFileDescriptionContainer& fileList)=0;
};


//-------------------------------------------------------------------------------------------------
// class IBackupGenericAckCallback
//-------------------------------------------------------------------------------------------------

class IBackupGenericAckCallback: public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(IBackupGenericAckCallback);
public:
	virtual ~IBackupGenericAckCallback() {}
	virtual void callback(const std::string& fileName)=0;
};


//-------------------------------------------------------------------------------------------------
// class IBackupServiceConnection
//-------------------------------------------------------------------------------------------------

class IBackupServiceConnection
{
public:
	virtual void cbBSconnect(bool connecting)=0;
};


//-------------------------------------------------------------------------------------------------
// class CBackupServiceInterface
// Default values (after the singleton registry has been initialized):
// - The remote path is IService::getInstance()->SaveFilesDirectory.toString().
//   When UseBS=1, the BS will access files in its own SaveShardRoot + the provided remote path.
// - The local path is SaveShardRoot.get() + IService::getInstance()->SaveFilesDirectory.toString().
//   When UseBS=0, the files will be saved in the local path
//-------------------------------------------------------------------------------------------------

class CBackupServiceInterface: public NLMISC::CRefCount
{
public:

	// request that the backup service load a file
	void requestFile(const std::string& fileName, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb);

	// load a file synchronously
	void syncLoadFile(const std::string& fileName, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb);

	// load a set of file synchronously
	void syncLoadFiles(const std::vector<std::string>& fileNames, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb);

	// send a file to the backup service for saving. Use either the SaveFile or SaveFileCheck msg type.
	void sendFile(CBackupMsgSaveFile& msg, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb=NULL);

	// request BS sends me files matching the given file classes
	void requestFileClass(const std::string& directory, const std::vector<CBackupFileClass>& classes, NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> cb);

	// load a set of file synchronously
	void syncLoadFileClass(const std::string& directory, const std::vector<CBackupFileClass>& classes, NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> cb);


	// Append a line to a file
	void append(const std::string& filename, const std::string& line, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb=NULL);
	// Append a data stream to a file. Use either the AppendFile or the AppendFileCheck msg type.
	void append(CBackupMsgSaveFile& msg, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb=NULL);


	// request for a file to be deleted (WARNING: no archiving of the file)
	void deleteFile(const std::string& fileName, bool keepBackupOfFile = true, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb=NULL);


	// setup a callback to receive connection and disconnection events from the backup system
	void registerBSConnectionCallback(IBackupServiceConnection* cb);


	// Return the local path that is added before the provided filenames. Ex: "/home/nevrax/save_shard/s01/" added before "characters/account..."
	const std::string& getLocalPath() const { return _LocalPath; }

	// Return the remote path that is added between the BS root path and the provided filename. Ex: "s01/" added between root "/home/nevrax/save_shard/" and "characters/account..."
	const std::string& getRemotePath() const { return _RemotePath; }

	// Return the timestamp of the last packet ack (or ping pong) from the backup system
	NLMISC::TTime getLastAckTime() const;

	// Return the time between dispatch of last acked packet (or ping pong) and the associated ack
	NLMISC::TTime getLastAckDelay() const;

private:
	// Constructor. Called by the singleton registry init.
	CBackupServiceInterface() {}
	void init(const std::string& bsiname);
	friend class CBackupInterfaceSingleton;

	// Path modification. Called by the constructor or by callbacks from variables.
	void setRemotePath( const std::string& remotePath );
	void setLocalPath( const std::string& localPath );
	friend void onSaveShardRootModified(NLMISC::IVariable &var);

	std::string _LocalPath;
	std::string _RemotePath;
	std::string _Name;
};


//-------------------------------------------------------------------------------------------------
// globals - pre defined CBackupServiceInterface instances
//-------------------------------------------------------------------------------------------------

CBackupServiceInterface& getShardDependentBsi();
CBackupServiceInterface& getGlobalBsi();
//CBackupServiceInterface& getPDBsi();
#define Bsi		  getShardDependentBsi()
#define BsiGlobal getGlobalBsi()
//#define PDBsi	  getPDBsi()


//-------------------------------------------------------------------------------------------------
#endif
