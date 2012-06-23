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



#ifndef RYAI_AIS_MESSAGES_H
#define RYAI_AIS_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"

#include "nel/net/transport_class.h"


//----------------------------------------------------------------
// AIDS -> AIS: Upload a manager definition script

class CMsgAIUploadActions : public NLNET::CTransportClass
{
public:
	std::string Data;

	CMsgAIUploadActions()
	{
	}
	
	CMsgAIUploadActions(std::string data) 
	{
		Data=data;
	}

	virtual void description ()
	{
		className ("CMsgAIUploadActions");
		property (std::string("data"), PropString, std::string(), Data);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// AIDS -> AIS: Start Managers

class CMsgAISpawnMgrs : public NLNET::CTransportClass
{
public:
	std::vector <uint16> MgrId;

	CMsgAISpawnMgrs()
	{
	}
	
	CMsgAISpawnMgrs(uint16 mgrId,const std::string &name) 
	{
		MgrId.push_back(mgrId);
	}

	virtual void description ()
	{
		className ("CMsgAISpawnMgrs");
		propertyCont ("mgrId", PropUInt16, MgrId);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// AIDS -> AIS: Save, stop and unload Managers

class CMsgAIDespawnMgrs : public NLNET::CTransportClass
{
public:
	std::vector <uint16> MgrId;

	CMsgAIDespawnMgrs()
	{
	}
	
	CMsgAIDespawnMgrs(uint16 mgrId) 
	{
		MgrId.push_back(mgrId);
	}

	virtual void description ()
	{
		className ("CMsgAIDespawnMgrs");
		propertyCont ("mgrId", PropUInt16, MgrId);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// AIDS -> AIS: Save managers' backups

class CMsgAIBackupMgrs : public NLNET::CTransportClass
{
public:
	std::vector <uint16> MgrId;

	CMsgAIBackupMgrs()
	{
	}
	
	CMsgAIBackupMgrs(uint16 mgrId) 
	{
		MgrId.push_back(mgrId);
	}

	virtual void description ()
	{
		className ("CMsgAIBackupMgrs");
		propertyCont ("mgrId", PropUInt16, MgrId);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// AIDS -> AIS: Close managers and unload from RAM

class CMsgAICloseMgrs : public NLNET::CTransportClass
{
public:
	std::vector <uint16> MgrId;

	CMsgAICloseMgrs()
	{
	}
	
	CMsgAICloseMgrs(uint16 mgrId) 
	{
		MgrId.push_back(mgrId);
	}

	virtual void description ()
	{
		className ("CMsgAICloseMgrs");
		propertyCont ("mgrId", PropUInt16, MgrId);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


#endif

