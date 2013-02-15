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

#ifndef GUS_NET_H
#define GUS_NET_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/mem_stream.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"

#include "gus_module.h"
#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// class CModuleMessage
	//-----------------------------------------------------------------------------

	class CModuleMessage: public NLMISC::CRefCount
	{
	public:
		// ctor
		CModuleMessage();

		// read accessors
		const NLMISC::CSString& getMessageName() const;
		uint32					getSenderId() const;
		TRawMsgBodyPtr			getMsgBody() const;
		const TModuleIdVector&	getDestinationModuleIds() const;

		// write accessors
		void setMessageName(const NLMISC::CSString& name);
		void setSenderId(uint32 id);
		void setMsgBody(TRawMsgBodyPtr body);
		void setDestinationModuleIds(const TModuleIdVector& remoteIds);
		void setDestinationModuleId(uint32 remoteId);

		// serialise
		void serial(NLMISC::IStream& stream);

	private:
		// private data
		NLMISC::CSString	_MsgName;
		uint32				_SenderId;
		TRawMsgBodyPtr		_Body;
		TModuleIdVector		_DestinationModuleIds;
	};
	typedef NLMISC::CSmartPtr<CModuleMessage> TModuleMessagePtr;


	//-----------------------------------------------------------------------------
	// class CGusNet
	//-----------------------------------------------------------------------------

	class CGusNet
	{
	public:
		// get hold of the singleton instance
		static CGusNet* getInstance();

	public:
		// CGusNet interface

		// register / unregister a local module when it is instantiated (in CModuleManager)
		virtual void registerModule(GUS::TModulePtr module)=0;
		virtual void unregisterModule(GUS::TModulePtr module)=0;

		// lookup a remote module record from 
		virtual TRemoteModulePtr lookupRemoteModule(uint32 uniqueId)=0;

		// send the message to a single remote modules
		virtual void sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 destinationModuleId, uint32 senderModuleId)=0;

		// send the message to several remote modules
		virtual void sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,const TModuleIdVector& destinationModuleIds, uint32 senderModuleId)=0;

		// broadcast the message to all remote modules
		virtual void broadcastMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody, uint32 senderModuleId)=0;

		// display info on the net singleton
		virtual void display() const=0;
	};


	//-----------------------------------------------------------------------------
	// wrappers for CGusNet():: sendModuleMessage()
	//-----------------------------------------------------------------------------

	// send a message with a single template object attached
 	template<class T> void sendModuleMessage(const NLMISC::CSString& msgName, const T& object, uint32 destinationModuleId, const GUS::IModule* senderModule);
	template<class T> void sendModuleMessage(const NLMISC::CSString& msgName, const T& object, const TModuleIdVector& destinationModuleIds, const GUS::IModule* senderModule);
}

//-----------------------------------------------------------------------------
// includes for inclines
//-----------------------------------------------------------------------------

#include "gus_module_manager.h"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// wrappers for CGusNet():: sendModuleMessage()
	//-----------------------------------------------------------------------------

	// send a message with a single template object attached to a specific module
 	template<class T> void sendModuleMessage(const T& object, uint32 destinationModuleId, const GUS::IModule* senderModule)
	{
		TRawMsgBodyPtr msgBody= new CRawMsgBody;
		msgBody->serial(const_cast<T&>(object));
		CGusNet::getInstance()->sendMessage(object.getName(),msgBody,destinationModuleId,GUS::CModuleManager::getInstance()->getModuleId(senderModule));
	}

	// send a message with a single template object attached to several specific modules
	template<class T> void sendModuleMessage(const T& object, const TModuleIdVector& destinationModuleIds, const GUS::IModule* senderModule)
	{
		TRawMsgBodyPtr msgBody= new CRawMsgBody;
		msgBody->serial(const_cast<T&>(object));
		CGusNet::getInstance()->sendMessage(object.getName(),msgBody,destinationModuleIds,GUS::CModuleManager::getInstance()->getModuleId(senderModule));
	}
}


//-----------------------------------------------------------------------------
#endif
