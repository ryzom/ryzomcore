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

#ifndef EM_EVENT_MANAGER_H
#define EM_EVENT_MANAGER_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"


//-----------------------------------------------------------------------------
// advanced class declarations
//-----------------------------------------------------------------------------

class CFileDescriptionContainer;


//-----------------------------------------------------------------------------
// class CEventManager
//-----------------------------------------------------------------------------

class CEventManager
{
public:
	// get hold of the singleton instance
	static CEventManager* getInstance();

public:
	// try to log in to a shard
	virtual void login(const NLMISC::CSString& shardName,const NLMISC::CSString& userId,const NLMISC::CSString& password)=0;

	// upload an event to a shard
	virtual void upload(const NLMISC::CSString& shardName,const NLMISC::CSString& eventName,const CFileDescriptionContainer& fdc,const NLMISC::CVectorSString& fileBodies)=0;

	// tell the shard to start the event running (if it's not already)
	virtual void startEvent(const NLMISC::CSString& shardName)=0;

	// tell the shard to stop the current event (if there is one running)
	virtual void stopEvent(const NLMISC::CSString& shardName)=0;

	// attempt to restart a shard (shut down and re-launch)
	virtual void updateTools()=0;

	// send a history request to a given shard
	virtual void peekInstalledEvent(const NLMISC::CSString& shardName) const=0;

	// get the set of shard names for the shards we're logged into
	virtual void getShards(NLMISC::CVectorSString& shardNames) const=0;
};


//-----------------------------------------------------------------------------
#endif
