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



#include "stdpch.h"
#include "aids_interface.h"
#include "ai_share/aids_messages.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

//---------------------------------------------------------------------------------
// service up and service down callbacks

static void cbServiceUp( const std::string& serviceName, NLNET::TServiceId serviceId, void * )
{
	if (serviceName=="AIDS")
	{
		// send a message to the service to say 'I exist'
		CMsgAIServiceUp().send(serviceId);
		// send messages to say which managers are up and running
	}
}

static void cbServiceDown( const std::string& serviceName, NLNET::TServiceId serviceId, void * )
{
}


//---------------------------------------------------------------------------------
// classic init() and release()
void CAIDSInterface::init()
{
	// register the service up and service down callbacks
	CUnifiedNetwork::getInstance()->setServiceUpCallback("AIDS", cbServiceUp, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "AIDS", cbServiceDown, 0);
}

void CAIDSInterface::release()
{
}

//---------------------------------------------------------------------------------
// send text messages back to the AIDS
void CAIDSInterface::info(const std::string &s)
{
	string str = string("AIS: %3d: INF: ")+s;
	CMsgAIFeedback(str).send("AIDS");
}

void CAIDSInterface::debug(const std::string &s)
{
	string str = string("AIS: %3d: DBG: ")+s;
	CMsgAIFeedback(str).send("AIDS");
}

void CAIDSInterface::warning(const std::string &s)
{
	string str = string("AIS: %3d: WRN: ")+s;
	CMsgAIFeedback(str).send("AIDS");
}

