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



#ifndef RYAI_AIDS_MESSAGES_H
#define RYAI_AIDS_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"

#include "nel/net/transport_class.h"


//----------------------------------------------------------------
// Message sent by AIS to AIDS in reaction to AIDS's service up 

class CMsgAIServiceUp : public NLNET::CTransportClass
{
public:
	float ProcessorAllocation;
	float RamAllocation;
	std::vector<uint16> ManagersRunning;

	CMsgAIServiceUp() 
	{
		ProcessorAllocation=1.0f;
		RamAllocation=1.0f;
	}
	
	CMsgAIServiceUp(float processorAllocation,float ramAllocation) 
	{
		ProcessorAllocation=processorAllocation;
		RamAllocation=ramAllocation;
	}

	virtual void description ()
	{
		className ("CMsgAIServiceUp");
		property ("ProcessorAllocation", PropFloat, 1.0f, ProcessorAllocation);
		property ("RamAllocation", PropFloat, 1.0f, RamAllocation);
		propertyCont ("ManagersRunning", PropUInt16, ManagersRunning);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// Message sent by AIS to AIDS each time new manager comes up

class CMsgAIManagerUp : public NLNET::CTransportClass
{
public:
	uint16 MgrId;
	std::string Name;

	CMsgAIManagerUp() 
	{
	}
	
	CMsgAIManagerUp(uint mgrId,std::string Name) 
	{
		MgrId=mgrId;
	}

	virtual void description ()
	{
		className ("CMsgAIManagerUp");
		property ("MgrId", PropUInt16, uint16(0), MgrId);
		property ("Name", PropString, std::string(), Name);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


//----------------------------------------------------------------
// Message sent by AIS to AIDS any time there's an important info to display

class CMsgAIFeedback : public NLNET::CTransportClass
{
public:
	std::string Message;

	CMsgAIFeedback() 
	{
	}
	
	CMsgAIFeedback(std::string message) 
	{
		Message=message;
	}

	CMsgAIFeedback(char *msgStr) 
	{
		Message=std::string(msgStr);
	}

	virtual void description ()
	{
		className ("CMsgAIFeedback");
		property ("Msg", PropString, std::string(), Message);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


#endif

