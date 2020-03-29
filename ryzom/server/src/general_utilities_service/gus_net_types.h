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

#ifndef GUS_NET_TYPES_H
#define GUS_NET_TYPES_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/mem_stream.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// forward class declarations
	//-----------------------------------------------------------------------------

	class IModule;
	class CModuleManager;
}


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// forward class declarations
	//-----------------------------------------------------------------------------

	class CConnectionModule;
	class CHubModule;

	class IRemoteModule;
	class CRemoteModuleOnHub;
	class CRemoteModuleViaConnection;

	// messages
	class CModuleMessage;
	class CMsgRegisterModule;


	//-----------------------------------------------------------------------------
	// class CRawMsgBody
	//-----------------------------------------------------------------------------

	class CRawMsgBody: public NLMISC::CRefCount, public NLMISC::CMemStream
	{
	};
	typedef NLMISC::CSmartPtr<CRawMsgBody> TRawMsgBodyPtr;


	//-----------------------------------------------------------------------------
	// global typedefs
	//-----------------------------------------------------------------------------

	typedef std::vector<uint32> TModuleIdVector;
	typedef uint32 TRemoteModuleId;
	const TRemoteModuleId InvalidRemoteModuleId=~0u;


	//-----------------------------------------------------------------------------
	// public pointer type definitions
	//-----------------------------------------------------------------------------

	typedef NLMISC::CSmartPtr<CHubModule>			THubModulePtr;
	typedef NLMISC::CSmartPtr<CConnectionModule>	TConnectionModulePtr;

	typedef NLMISC::CSmartPtr<IRemoteModule> TRemoteModulePtr;
	typedef NLMISC::CSmartPtr<CRemoteModuleOnHub> TRemoteModuleOnHubPtr;
	typedef NLMISC::CSmartPtr<CRemoteModuleViaConnection> TRemoteModuleViaConnectionPtr;
}

//-----------------------------------------------------------------------------
#endif
