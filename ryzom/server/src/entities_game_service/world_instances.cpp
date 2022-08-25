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
#include "server_share/used_continent.h"
#include "world_instances.h"
#include "egs_variables.h"

using namespace std;
using namespace NLMISC;


CWorldInstances	*CWorldInstances::_Instance = NULL;

CWorldInstances &CWorldInstances::instance()
{
	if (_Instance == NULL)
		_Instance = new CWorldInstances;

	return *_Instance;
}

void CWorldInstances::reportAICollisionAvailable (const std::string &name, NLNET::TServiceId id, CReportAICollisionAvailableMsgImp &msg)
{
	TAISInfoCont::iterator it(_AISInfos.find(id));
	if (it != _AISInfos.end())
	{
		// there is already info for this continent, remove it (new one orverwrite the old)
		if (VerboseWorldInstance)
			nldebug("CWorldInstances::reportAICollisionAvailable: removing old AIS collision info for AIS %u", id.get());
		_AISInfos.erase(it);
	}

	TAISInfo	aisinfo;

	aisinfo.AISId = id;
	aisinfo.AvailableCollision = msg.ContinentsCollision;

	if (VerboseWorldInstance)
	{
		nldebug("CWorldInstances::reportAICollisionAvailable: adding %u collision available for AIS %u:", aisinfo.AvailableCollision.size(), id.get());
		for (uint i=0; i<aisinfo.AvailableCollision.size(); ++i)
		{
			nldebug("	Collision for continent '%s'", aisinfo.AvailableCollision[i].c_str());
		}
	}

	_AISInfos[id] = aisinfo;
}

void CWorldInstances::reportStaticAIInstance (const std::string &name, NLNET::TServiceId id, CReportStaticAIInstanceMsgImp &msg)
{
	if (VerboseWorldInstance)
		nldebug("CWorldInstances::reportStaticAIInstance: AIS %u report new instance #%u with continent '%s':",
			id.get(), 
			msg.InstanceNumber, 
			msg.InstanceContinent.c_str());

	TInstanceInfoCont::iterator it(_InstanceInfos.find(msg.InstanceNumber));
	if (it != _InstanceInfos.end())
	{
		// check if there are conflict
		if (it->second.AISId != id)
		{
			// arg, this instance is already running on another AIS
			nlwarning("CWorldInstances::reportStaticAIInstance: AIS %u report static instance %u (for continent '%s') but this instance is already spawned by AIS %u for continent '%s'.",
				id.get(),
				msg.InstanceNumber,
				msg.InstanceContinent.c_str(),
				it->second.AISId.get(),
				it->second.ContinentName.c_str());
			nlwarning("	EGS ask this AIS to despawn the bad instance.");

			// send the despawn request.
			CWarnBadInstanceMsg	despawn;
			despawn.InstanceNumber = it->first;
			despawn.send(id);

			// no more to do
/////////////////////////////////////
			return;
/////////////////////////////////////
		}
	}

	// Check if this instance does not break static instance rules
	CUsedContinent uc = CUsedContinent::instance();
	uint32 in = uc.getInstanceForContinent(msg.InstanceContinent);
	if (in != INVALID_AI_INSTANCE && in != msg.InstanceNumber)
	{
		// bad instance !
		nlwarning("CWorldInstance::reportAIInstance: AIS %u report static instance %u (continent '%s'). This instance is reserved for continent '%s'. Requesting despawn.",
			id.get(),
			msg.InstanceNumber,
			msg.InstanceContinent.c_str(),
			uc.getContinentForInstance(msg.InstanceNumber).c_str());
		nlwarning("	EGS ask this AIS to despawn the bad instance.");

		// send the despawn request.
		CWarnBadInstanceMsg	despawn;
		despawn.InstanceNumber = it->first;
		despawn.send(id);

		// no more to do
/////////////////////////////////////
		return;
/////////////////////////////////////
	}

	// Store the instance information
	TInstanceInfo ii;
	ii.InstanceNumber = msg.InstanceNumber;
	ii.ContinentName = msg.InstanceContinent;
	ii.AISId = id;

	if (VerboseWorldInstance)
		nldebug("CWorldInstance::reportStaticAIInstance: AIS %u report static instance %u (continent '%s')",
			ii.AISId.get(), 
			ii.InstanceNumber,
			ii.ContinentName.c_str());
	_InstanceInfos[ii.InstanceNumber] = ii;

	// report new AI instance ready
	if (_AIReadyCallback != NULL)
		_AIReadyCallback->onAiInstanceReady(msg);

}

void CWorldInstances::reportAIInstanceDespawn (const std::string &name, NLNET::TServiceId id, CReportAIInstanceDespawnMsgImp &msg)
{
	for (uint i=0; i<msg.InstanceNumbers.size(); ++i)
	{
		uint32 in = msg.InstanceNumbers[i];
		TInstanceInfoCont::iterator it(_InstanceInfos.find(in));
		if (it == _InstanceInfos.end())
		{
			nlwarning("CWorldInstance::reportAIInstanceDespawn: AIS %u report despawn for instance %u. This instance is not known here !",
				id.get(), in);
			// skip to next one
			continue;
		}

		// report AI instance down
		CReportStaticAIInstanceMsg msg;
		msg.InstanceNumber = (*it).second.InstanceNumber;
		msg.InstanceContinent = (*it).second.ContinentName;
		if (_AIReadyCallback != NULL)
			_AIReadyCallback->onAiInstanceDown(msg);

		// ok, the instance is know, remove it from the container.
		if (VerboseWorldInstance)
			nldebug("CWorldInstance::reportAIInstanceDespawn: AIS %u report depawn for instance %u (continent '%s')",
				id.get(), in, it->second.ContinentName.c_str());
		_InstanceInfos.erase(it);

		// TODO : if the instance is a dynamic instance, warn the player mananger
		// to remove any player that are still in this instance.
	}
}


void CReportAICollisionAvailableMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CWorldInstances::instance().reportAICollisionAvailable(name, id, *this);
}

void CReportStaticAIInstanceMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CWorldInstances::instance().reportStaticAIInstance(name, id, *this);
}

void CReportAIInstanceDespawnMsgImp ::callback (const std::string &name, NLNET::TServiceId id)
{
	CWorldInstances::instance().reportAIInstanceDespawn(name, id, *this);
}


CWorldInstances::CWorldInstances()
: _AIReadyCallback(NULL)
{
}

void CWorldInstances::registerAiInstanceReadyCallback(IAIInstanceReady *callback)
{
	_AIReadyCallback = callback;
}

/// This method sends a transport class message
void CWorldInstances::msgToAIInstance(uint32 instanceNumber, CMirrorTransportClass &msg)
{
	if( CUsedContinent::instance().getContinentForInstance( instanceNumber ) == CONTINENT::toString(CONTINENT::INDOORS ) )
		return;

	// lookup in the registered instance to find the corresponding AIS.
	TInstanceInfoCont::iterator it(_InstanceInfos.find(instanceNumber));

	if( it == _InstanceInfos.end())
	{
		// oups !
		nlwarning("CWorldInstances::msgToAIInstance: Can't send the transport class '%s' to AIInstance #%u, not created.", 
			typeid(msg).name(), 
			instanceNumber);
		return;
	}

	// send the message to the convenient AIS.
	msg.send(it->second.AISId);
}

/// This method sends a CMessage
void CWorldInstances::msgToAIInstance2(uint32 instanceNumber, NLNET::CMessage &msg)
{
	if( CUsedContinent::instance().getContinentForInstance( instanceNumber ) == CONTINENT::toString(CONTINENT::INDOORS ) )
		return;
	
	// lookup in the registered instance to find the corresponding AIS.
	TInstanceInfoCont::iterator it(_InstanceInfos.find(instanceNumber));
	
	if( it == _InstanceInfos.end())
	{
		// oups !
		nlwarning("CWorldInstances::msgToAIInstance: Can't send the message '%s' to AIInstance #%u, not created.", 
			msg.getName().c_str(), 
			instanceNumber);
		return;
	}
	
	// send the message to the convenient AIS.
//	msg.send(it->second.AISId);
	NLNET::CUnifiedNetwork::getInstance()->send( it->second.AISId, msg );
}

/*
 * Return the AIS Id or 0 if no AIS is currently online (no warning if not found)
 */
NLNET::TServiceId CWorldInstances::getAISId(uint32 instanceNumber ) const
{
	// lookup in the registered instance to find the corresponding AIS.
	TInstanceInfoCont::const_iterator it(_InstanceInfos.find(instanceNumber));
	if( it == _InstanceInfos.end())
		return NLNET::TServiceId(0);
	else
		return it->second.AISId;
}


void CWorldInstances::aiDisconnection(NLNET::TServiceId aisId)
{

	// cleanup instance info
	vector<uint32>	instanceToRemove;
	TInstanceInfoCont::iterator first(_InstanceInfos.begin()), last(_InstanceInfos.end());
	for (;first != last; ++first)
	{
		TInstanceInfo &ii = first->second;
		if (ii.AISId == aisId)
		{
			// report AI instance down
			CReportStaticAIInstanceMsg msg;
			msg.InstanceNumber = ii.InstanceNumber;
			msg.InstanceContinent = ii.ContinentName;
			if (_AIReadyCallback != NULL)
				_AIReadyCallback->onAiInstanceDown(msg);

			instanceToRemove.push_back(first->first);
		}
	}

	while (!instanceToRemove.empty())
	{
		_InstanceInfos.erase(instanceToRemove.back());
		instanceToRemove.pop_back();
	}

	// cleanup collision info
	TAISInfoCont::iterator it(_AISInfos.find(aisId));
	if (it != _AISInfos.end())
	{
		_AISInfos.erase(it);
	}
	
}

bool CWorldInstances::getAIServiceIdForInstance(uint32 instanceNumber, NLNET::TServiceId &AISId)
{
	if (_InstanceInfos.find(instanceNumber) != _InstanceInfos.end())
	{
		AISId = _InstanceInfos[instanceNumber].AISId;
		return true;
	}
	return false;
}


bool CWorldInstances::getAIInstanceNameFromeServiceId(NLNET::TServiceId AISID, std::string & name)
{
	for ( TInstanceInfoCont::iterator it = _InstanceInfos.begin(); it != _InstanceInfos.end(); ++it )
	{
		if ( (*it).second.AISId == AISID )
		{
			name = (*it).second.ContinentName;
			return true;
		}
	}
	return false;
}

