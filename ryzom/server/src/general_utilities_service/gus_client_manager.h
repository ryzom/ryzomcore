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

#ifndef GUS_CLIENT_MANAGER_H
#define GUS_CLIENT_MANAGER_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/smart_ptr.h"

#include "game_share/base_types.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// public typedefs & associated constants
	//-----------------------------------------------------------------------------

	typedef TDataSetRow TClientId;
	static const TClientId BadTClientId= TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW);


	//-----------------------------------------------------------------------------
	// class CCharacterId
	//-----------------------------------------------------------------------------
	// 

	class CCharacterId
	{
	public:
		// ctors
		CCharacterId()										{_Data=NLMISC::CEntityId::Unknown.getShortId();}
		CCharacterId(const CCharacterId& other)				{_Data=other._Data;}
		CCharacterId(const NLMISC::CEntityId& id)			{_Data=id.getShortId();}
		CCharacterId(uint32 accountId,uint32 slot)			{_Data=(uint64(accountId)<<4)+slot; nlassert(slot<16);}
		CCharacterId(sint64 data)							{_Data=data;}

		// compareson operators
		bool operator==(const CCharacterId& other) const	{return _Data==other._Data;}
		bool operator!=(const CCharacterId& other) const	{return _Data!=other._Data;}
		bool operator<=(const CCharacterId& other) const	{return _Data<=other._Data;}
		bool operator>=(const CCharacterId& other) const	{return _Data>=other._Data;}
		bool operator<(const CCharacterId& other) const		{return _Data<other._Data;}
		bool operator>(const CCharacterId& other) const		{return _Data>other._Data;}

		// implicit cast operator
		operator sint64()									{return _Data;}

		// decortication into account id and slot
		uint32 getAccountId() const							{return (uint32)(_Data>>4);}
		uint32 getSlot() const								{return (uint32)(_Data&15);}

	private:
		// private data
		sint64 _Data;
	};


	//-----------------------------------------------------------------------------
	// class CClientManager
	//-----------------------------------------------------------------------------

	class CClientManager
	{
	public:
		//-----------------------------------------------------------------------------
		// Callback classes
		//-----------------------------------------------------------------------------

		// connect
		class IConnectionHandler: public NLMISC::CRefCount
		{
		public:
			virtual ~IConnectionHandler() {}
			virtual void connect(TClientId)=0;
			virtual void disconnect(TClientId)=0;
		};
		typedef NLMISC::CSmartPtr<IConnectionHandler> TConnectionHandlerPtr;


		//-----------------------------------------------------------------------------
		// Public interface
		//-----------------------------------------------------------------------------

		// get the singleton instance
		static CClientManager* getInstance();

		// setup a callback object to handle player connection & disconnection events
		// the callback should be called immediately for all existing players
		virtual void setConnectionCallback(TConnectionHandlerPtr callback)=0;

		// get the character name for a connected player
		virtual const NLMISC::CSString& getCharacterName(TClientId clientId) const=0;

		// get the character entity id for a connected player
		virtual const NLMISC::CEntityId& getEntityId(TClientId clientId) const=0;

		// get the client id for a connected player from their character name
		virtual TClientId getClientId(const NLMISC::CSString& name) const=0;

		// get the client id for a connected player from their entity id
		virtual TClientId getClientId(const CCharacterId& id) const=0;

		// get the client id for a connected player from their account number
		virtual TClientId getClientIdFromAccount(uint32 accountId) const=0;
	};
}

//-----------------------------------------------------------------------------
#endif
