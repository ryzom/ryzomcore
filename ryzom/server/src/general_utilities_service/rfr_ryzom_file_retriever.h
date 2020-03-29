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

#ifndef RFR_RYZOM_FILE_RETRIEVER_H
#define RFR_RYZOM_FILE_RETRIEVER_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"


//-----------------------------------------------------------------------------
// forward class definitions
//-----------------------------------------------------------------------------

namespace SAVES
{
	class CShardSavesInterface;
	class CMailSavesInterface;
	class CIncrementalBackupSavesInterface;
}


//-----------------------------------------------------------------------------
// class CRyzomFileRetriever
//-----------------------------------------------------------------------------

class CRyzomFileRetriever
{
public:
	// get hold of the singleton instance
	static CRyzomFileRetriever* getInstance();

public:
	// manage the list of used shards
	virtual void useShard(const NLMISC::CSString& shardName)=0;
	virtual void stopUsingShard(const NLMISC::CSString& shardName)=0;
	virtual NLMISC::CVectorSString getUsedShardList()=0;

	// accessors for the save interface objects for the used shards
	virtual SAVES::CShardSavesInterface*				getShardSavesInterface(const NLMISC::CSString& shardName)=0;
	virtual SAVES::CMailSavesInterface*					getWwwSavesInterface(const NLMISC::CSString& shardName)=0;
	virtual SAVES::CIncrementalBackupSavesInterface*	getBakSavesInterface(const NLMISC::CSString& shardName)=0;

	// accessors for the set of remote gus modules that have been detected that declare themselves as saves managers
	virtual NLMISC::CVectorSString getSavesModules()=0;

	// accessor for uploading / downloading files from a shard
	virtual bool uploadFile(const NLMISC::CSString& shardName,const NLMISC::CSString& localFileName,const NLMISC::CSString& remoteFileName)=0;
	virtual bool downloadFile(const NLMISC::CSString& shardName,const NLMISC::CSString& remoteFileName,const NLMISC::CSString& localFileName)=0;
	virtual bool downloadBackupFiles(const NLMISC::CSString& shardName,const NLMISC::CSString& fileName,const NLMISC::CSString& localDirectory)=0;

	// accessors for looking up character and account names to retrieve numeric ids
	// returrn true if ids found otherwise false
	virtual bool getCharIdFromName(const NLMISC::CSString& shardName,const NLMISC::CSString& name,uint32& account,uint32& slot)=0;
	virtual bool getAccountIdFromName(const NLMISC::CSString& name,uint32& account)=0;
};


//-----------------------------------------------------------------------------
#endif
