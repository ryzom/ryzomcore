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

#ifndef RS_REMOTE_SAVES_H
#define RS_REMOTE_SAVES_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/file_description_container.h"

// local
#include "gus_module.h"


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// forward class decalarations
	//-----------------------------------------------------------------------------

	class CRemoteSavesInterface;
	typedef NLMISC::CSmartPtr<CRemoteSavesInterface> TRemoteSavesInterfacePtr;


	//-----------------------------------------------------------------------------
	// class IRemoteSavesConnection
	//-----------------------------------------------------------------------------

	class IRemoteSavesConnection: public GUS::IModule
	{
	public:
		virtual const CFileDescriptionContainer& getFileList() const =0;

		virtual uint32 requestFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor) =0;
		virtual uint32 uploadFile(const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody,CRemoteSavesInterface* requestor) =0;
		virtual uint32 deleteFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor) =0;
		virtual uint32 moveFile(const NLMISC::CSString& fileName,const NLMISC::CSString& destination,CRemoteSavesInterface* requestor) =0;

		virtual bool isConnected() const =0;
	};
	typedef NLMISC::CSmartPtr<IRemoteSavesConnection> TRemoteSavesConnectionPtr;


	//-----------------------------------------------------------------------------
	// class CRemoteSavesManager
	//-----------------------------------------------------------------------------

	class CRemoteSavesManager: public NLMISC::CRefCount
	{
	public:
		virtual ~CRemoteSavesManager() {}

		static CRemoteSavesManager* getInstance();

		// interface used by CRemoteSavesInterface objects in their ctor to declare themselves
		virtual void registerSavesInterface(TRemoteSavesInterfacePtr si)=0;
		// interface used by CRemoteSavesInterface objects in their dtor to undeclare themselves
		virtual void unregisterSavesInterface(CRemoteSavesInterface* si)=0;

		// interface used by IRemoteSavesConnection objects in their ctor to declare themselves
		virtual void registerRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection)=0;
		// interface used by IRemoteSavesConnection objects in their dtor to undeclare themselves
		virtual void unregisterRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection)=0;

		// interface used to retrieve a pointer to a connection object for a given shard
		// a new one is instantiated if an appropriate module doesn't already exist
		virtual IRemoteSavesConnection* getConnection(const NLMISC::CSString& shardName,const NLMISC::CSString& type) const=0;
	};
}


//-----------------------------------------------------------------------------
#endif
