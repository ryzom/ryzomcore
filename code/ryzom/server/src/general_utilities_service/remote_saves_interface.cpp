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
#include "remote_saves_interface.h"
#include "rs_remote_saves.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
//using namespace NLNET;
//using namespace GUS;


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// class CRemoteSavesManager
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	// methods CRemoteSavesInterface
	//-----------------------------------------------------------------------------

	CRemoteSavesInterface::CRemoteSavesInterface(const NLMISC::CSString& shardName,const NLMISC::CSString& type)
	{
		// set internal properties
		_ShardName=		shardName;
		_Type=			type;

		// register with the manager
		CRemoteSavesManager::getInstance()->registerSavesInterface(this);

		// provoke a connection attempt
		CRemoteSavesManager::getInstance()->getConnection(_ShardName,_Type);
	}

	CRemoteSavesInterface::~CRemoteSavesInterface()
	{
		// unregister from the manager
		CRemoteSavesManager::getInstance()->unregisterSavesInterface(this);
	}

	bool CRemoteSavesInterface::isReady() const
	{
		IRemoteSavesConnection* connection= CRemoteSavesManager::getInstance()->getConnection(_ShardName,_Type);
		return (connection!=NULL) && connection->isConnected();
	}

	const NLMISC::CSString& CRemoteSavesInterface::getShardName() const
	{
		return _ShardName;
	}

	const NLMISC::CSString& CRemoteSavesInterface::getType() const
	{
		return _Type;
	}

	void CRemoteSavesInterface::getCallbacks(CRemoteSavesInterface::TCallbackSet& result)
	{
		result= _Callbacks;
	}

	void CRemoteSavesInterface::addCallbackObject(const TSavesCallbackPtr& cb)
	{
		for (uint32 i=_Callbacks.size();i--;)
		{
			BOMB_IF(_Callbacks[i]==cb,"BUG: Attempting to add the same callback to an RS module more than once",return)
		}
		_Callbacks.push_back(cb);
		if (isReady())
		{
			CFileDescriptionContainer fdc;
			getFileList(fdc);
			cb->cbInit(fdc);
		}
	}

	void CRemoteSavesInterface::removeCallbackObject(const TSavesCallbackPtr& cb)
	{
		for (uint32 i=_Callbacks.size();i--;)
		{
			if (_Callbacks[i]==cb)
			{
				_Callbacks[i]=_Callbacks.back();
				_Callbacks.pop_back();
			}
		}
	}

	uint32 CRemoteSavesInterface::requestFile(const NLMISC::CSString& fileName)
	{
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());

		DROP_IF(connection==NULL,"Failed to send request for file: "+fileName+" because connection not found for SAVES module: "+getShardName()+" "+getType(),return ~0u);
		DROP_IF(!connection->isConnected(),"Failed to send request for file: "+fileName+" because SAVES module not currently connected: "+getShardName()+" "+getType(),return ~0u);

		return connection->requestFile(fileName,this);
	}

	uint32 CRemoteSavesInterface::uploadFile(const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody)
	{
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());

		DROP_IF(connection==NULL,"Failed to upload file: "+fileName+" because connection not found for SAVES module: "+getShardName()+" "+getType(),return ~0u);
		DROP_IF(!connection->isConnected(),"Failed to upload file: "+fileName+" because SAVES module not currently connected: "+getShardName()+" "+getType(),return ~0u);

		return connection->uploadFile(fileName,fileBody,this);
	}

	uint32 CRemoteSavesInterface::deleteFile(const NLMISC::CSString& fileName)
	{
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());

		DROP_IF(connection==NULL,"Failed to delete file: "+fileName+" because connection not found for SAVES module: "+getShardName()+" "+getType(),return ~0u);
		DROP_IF(!connection->isConnected(),"Failed to delete file "+fileName+" because SAVES module not currently connected: "+getShardName()+" "+getType(),return ~0u);

		return connection->deleteFile(fileName,this);
	}

	bool CRemoteSavesInterface::getFileList(CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());

		DROP_IF(connection==NULL,"Failed to get file list because connection not found for SAVES module: "+getShardName()+" "+getType(),return false);
		DROP_IF(!connection->isConnected(),"Failed to get file list because SAVES module not currently connected for: "+getShardName()+" "+getType(),return false);

		result= connection->getFileList();

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CShardSavesInterface
	//-----------------------------------------------------------------------------

	CShardSavesInterface::CShardSavesInterface(const NLMISC::CSString& shardName):
		CRemoteSavesInterface(shardName,"shard")
	{
	}

	const NLMISC::CSString& CShardSavesInterface::getAccountNamesFileName()
	{
		static NLMISC::CSString txt= "account_names.txt";
		return txt;
	}

	const NLMISC::CSString& CShardSavesInterface::getCharacterNamesFileName()
	{
		static NLMISC::CSString txt= "character_names.txt";
		return txt;
	}

	const NLMISC::CSString& CShardSavesInterface::getGameCycleFileName()
	{
		static NLMISC::CSString txt= "game_cycle.ticks";
		return txt;
	}

	const NLMISC::CSString& CShardSavesInterface::getGMPendingTPFileName()
	{
		static NLMISC::CSString txt= "gm_pending_tp.bin";
		return txt;
	}

	NLMISC::CSString CShardSavesInterface::getCharacterSaveFileName(uint32 account,uint32 slot)
	{
		return NLMISC::toString("characters/account_%u_%u_pdr.bin",account,slot);
	}


	bool CShardSavesInterface::getCharacterFileList(CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		const CFileDescriptionContainer& fdc= connection->getFileList();
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (fdc[i].FileName.left(19)=="characters/account_" && fdc[i].FileName.right(8)=="_pdr.bin")
				result.addFile(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
		}

		return true;
	}

	bool CShardSavesInterface::getGuildFileList(CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		const CFileDescriptionContainer& fdc= connection->getFileList();
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (fdc[i].FileName.left(13)=="guilds/guild_" && fdc[i].FileName.right(4)==".bin")
				result.addFile(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
		}

		return true;
	}

	bool CShardSavesInterface::getSaleStoreFileList(CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		const CFileDescriptionContainer& fdc= connection->getFileList();
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (fdc[i].FileName.left(22)=="sale_store/sale_store_" && 
				(fdc[i].FileName.right(8)=="_pdr.bin" || fdc[i].FileName=="sale_store/sale_store_version.bin"))
				result.addFile(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
		}

		return true;
	}

	bool CShardSavesInterface::getOfflineCharacterCommandsFileList(CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		const CFileDescriptionContainer& fdc= connection->getFileList();
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (fdc[i].FileName.left(36)=="characters_offline_commands/acocunt_" && fdc[i].FileName.right(8)==".offline_commands")
				result.addFile(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CMailSavesInterface
	//-----------------------------------------------------------------------------

	CMailSavesInterface::CMailSavesInterface(const NLMISC::CSString& shardName):
		CRemoteSavesInterface(shardName,"www")
	{
	}

	bool CMailSavesInterface::getEntityFileList(const NLMISC::CSString& entityName,CFileDescriptionContainer& result) const
	{
		result.clear();
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		const CFileDescriptionContainer& fdc= connection->getFileList();
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (fdc[i].FileName.left(entityName.size()+6)=="./"+entityName.left(2)+"/"+entityName+"/")
				result.addFile(fdc[i].FileName,fdc[i].FileTimeStamp,fdc[i].FileSize);
		}

		return true;
	}

	bool CMailSavesInterface::moveEntityFiles(const NLMISC::CSString& oldEntityName,const NLMISC::CSString& newEntityName,bool overwrite)
	{
		TRemoteSavesConnectionPtr connection= CRemoteSavesManager::getInstance()->getConnection(getShardName(),getType());
		if (connection==NULL)
			return false;

		// build the list of files to move
		CFileDescriptionContainer filesToMove;
		getEntityFileList(oldEntityName,filesToMove);

		// check whether any files already exist in target location
		CFileDescriptionContainer newLocationFiles;
		getEntityFileList(newEntityName,newLocationFiles);
		if (!newLocationFiles.empty())
		{
			// files exist so check whether we overwrite or not
			if (overwrite)
			{
				// overwriting - so start by deleting all of the old files
				for (uint32 i=0;i<newLocationFiles.size();++i)
				{
					connection->deleteFile(newLocationFiles[i].FileName,this);
				}
			}
			else
			{
				// not allowed to overwrite so give up
				nlwarning("Cannont move mail/forum files from %s to %s because %s already exists on this shard (%s)!",
					oldEntityName.c_str(),newEntityName.c_str(),newEntityName.c_str(),getShardName().c_str());
				return false;
			}
		}
		
		// do the file moving...
		NLMISC::CSString destination= "./"+newEntityName.left(2)+"/"+newEntityName+"/";
		for (uint32 i=0;i<filesToMove.size();++i)
		{
			connection->moveFile(filesToMove[i].FileName,destination,this);
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// methods CIncrementalBackupSavesInterface
	//-----------------------------------------------------------------------------

	CIncrementalBackupSavesInterface::CIncrementalBackupSavesInterface(const NLMISC::CSString& shardName):
		CRemoteSavesInterface(shardName,"bak")
	{
	}
}
