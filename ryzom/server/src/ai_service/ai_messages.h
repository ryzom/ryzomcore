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



#ifndef RYAI_MESSAGES_H
#define RYAI_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"

#include "nel/net/transport_class.h"


//----------------------------------------------------------------
// Message sent by AIS to AIDS in reaction to AIDS's service up 

class CMsgAIServiceUp : public NLNET::CTransportClass
{
public:
	CMsgAIServiceUp(float processorAllocation,float ramAllocation) 
	{
		_processorAllocation=processorAllocation;
		_ramAllocation=ramAllocation;
	}

	virtual void description ()
	{
		className ("CMsgAIServiceUp");
		property ("_processorAllocation", PropFloat, 0.0f, _processorAllocation);
		property ("_ramAllocation", PropFloat, 0.0f, _ramAllocation);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId  id);

protected:
	float _processorAllocation;
	float _ramAllocation;
};

//----------------------------------------------------------------
// AIDS -> AIS: Load & start Managers

class CMsgAIOpenMgrs : public NLNET::CTransportClass
{
public:
	CMsgAIOpenMgrs(uint16 idx,const std::string &name) 
	{
		_idx.push_back(idx);
		_name.push_back(name);
	}

	void push_back(uint16 idx,constr std::string &name)
	{
		_idx.push_back(idx);
		_name.push_back(name);
	}

	virtual void description ()
	{
		className ("CMsgAIOpenMgrs");
		propertyCont ("idx", PropUInt16, _idx);
		propertyCont ("name", PropString, _name);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId  id);

protected:
	std::vector <uint16> _idx;
	std::vector <std::string> _name;
};


//----------------------------------------------------------------
// AIDS -> AIS: Reload Managers

class CMsgAIReloadMgrs : public NLNET::CTransportClass
{
public:
	CMsgAIReloadMgrs(uint16 idx) 
	{
		_idx.push_back(idx);
	}

	void push_back(uint16 idx)
	{
		_idx.push_back(idx);
	}

	virtual void description ()
	{
		className ("CMsgAIReloadMgrs");
		propertyCont ("idx", PropUInt16, _idx);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);

protected:
	std::vector <uint16> _idx;
};


//----------------------------------------------------------------
// AIDS -> AIS: Save, stop and unload Managers

class CMsgAICloseMgrs : public NLNET::CTransportClass
{
public:
	CMsgAICloseMgrs(uint16 idx) 
	{
		_idx.push_back(idx);
	}

	void push_back(uint16 idx)
	{
		_idx.push_back(idx);
	}

	virtual void description ()
	{
		className ("CMsgAICloseMgrs");
		propertyCont ("idx", PropUInt16, _idx);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);

protected:
	std::vector <uint16> _idx;
};


//----------------------------------------------------------------
// AIDS -> AIS: Save managers' backups

class CMsgAIBackupMgrs : public NLNET::CTransportClass
{
public:
	CMsgAIBackupMgrs(uint16 idx) 
	{
		_idx.push_back(idx);
	}

	void push_back(uint16 idx)
	{
		_idx.push_back(idx);
	}

	virtual void description ()
	{
		className ("CMsgAIBackupMgrs");
		propertyCont ("idx", PropUInt16, _idx);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);

protected:
	std::vector <uint16> _idx;
};


#endif

