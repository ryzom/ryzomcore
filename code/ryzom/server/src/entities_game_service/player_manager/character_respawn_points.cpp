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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "game_share/msg_client_server.h"
#include "game_share/fame.h"
#include "game_share/continent.h"

//#include "backward_compatibility/spawn_zones_back_compat.h"
#include "player_manager/character_respawn_points.h"
#include "player_manager/character.h"
#include "pvp_manager/pvp_manager_2.h"
#include "modules/shard_unifier_client.h"

#include "zone_manager.h"




//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CCharacterRespawnPoints);

extern CGenericXmlMsgHeaderManager	GenericMsgManager;


//-----------------------------------------------------------------------------
// methods CCharacterRespawnPoints
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CCharacterRespawnPoints::CCharacterRespawnPoints(CCharacter &c) : _Char(c)
{
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::addRespawnPoint(TRespawnPoint respawnPoint)
{
	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
	if (zone == NULL)
	{
		DEBUG_STOP;
		return;
	}

	// check if respawn point was already added
	if (std::find(_RegularRespawnPoints.begin(), _RegularRespawnPoints.end(), respawnPoint) != _RegularRespawnPoints.end())
	{
		return;
	}

	// add the respawn point
	_RegularRespawnPoints.push_back(respawnPoint);

	// add the point to client database only if needed
	const CMissionRespawnPoints * missionRespawnPoints = getMissionRespawnPoints( zone->getContinent() );
	if (missionRespawnPoints == NULL || !missionRespawnPoints->getHideOthers())
	{
		if (isUsableRegularRespawnPoint(_Char.getCurrentContinent(), respawnPoint))
		{
			addRespawnPointToUserDb(respawnPoint);
		}
	}

	// update the ring database
	if (IShardUnifierEvent::getInstance() != NULL)
	{
		IShardUnifierEvent::getInstance()->onUpdateCharRespawnPoints(_Char.getId(), buildRingPoints());
	}
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::addDefaultRespawnPoint(CONTINENT::TContinent continent)
{
	// WARNING: hardcoded values
	string defaultPlaceName;
	switch (continent)
	{
	case CONTINENT::FYROS:
		defaultPlaceName = "place_pyr";
		break;
	case CONTINENT::ZORAI:
		defaultPlaceName = "place_zora";
		break;
	case CONTINENT::TRYKER:
		defaultPlaceName = "place_fairhaven";
		break;
	case CONTINENT::MATIS:
		defaultPlaceName = "place_yrkanis";
		break;
	case CONTINENT::BAGNE:
		defaultPlaceName = "place_arrival_from_matis_bagne";
		break;
	case CONTINENT::NEXUS:
		defaultPlaceName = "place_arrival_from_bagne_nexus";
		break;
	case CONTINENT::ROUTE_GOUFFRE:
		defaultPlaceName = "place_arrival_from_fyros_route_gouffre";
		break;
	case CONTINENT::SOURCES:
		defaultPlaceName = "place_arrival_from_fyros_sources";
		break;
	case CONTINENT::TERRE:
		defaultPlaceName = "place_arrival_from_zorai_terre";
		break;
	case CONTINENT::FYROS_ISLAND:
		defaultPlaceName = "tp_fyros_island_ep2";
		break;
	case CONTINENT::FYROS_NEWBIE:
		defaultPlaceName = "place_aegus";
		break;
	case CONTINENT::TRYKER_NEWBIE:
		defaultPlaceName = "place_aubermouth";
		break;
	case CONTINENT::TRYKER_ISLAND:
		defaultPlaceName = "tp_tryker_island_ep 7";
		break;
	case CONTINENT::MATIS_ISLAND:
		defaultPlaceName = "tp_matis_island_ep 2";
		break;
	case CONTINENT::ZORAI_NEWBIE:
		defaultPlaceName = "place_qai_lo";
		break;
	case CONTINENT::MATIS_NEWBIE:
		defaultPlaceName = "place_stalli";
		break;
	case CONTINENT::CORRUPTED_MOOR:
		defaultPlaceName = "place_tcm_karavan_camp";
		break;
	case CONTINENT::KITINIERE:
		defaultPlaceName = "kitiniere_entrance";
		break;
	default:
		defaultPlaceName = "newbie_start_point";
	}

	CPlace * defaultPlace = CZoneManager::getInstance().getPlaceFromName(defaultPlaceName);
	if (defaultPlace == NULL)
	{
		nlwarning("<RESPAWN_POINT> %s : no default respawn point because '%s' in continent '%s' is unknown",
			_Char.getId().toString().c_str(),
			defaultPlaceName.c_str(),
			CONTINENT::toString(continent).c_str()
			);
		return;
	}

	if (defaultPlace->getRespawnPoints().empty())
	{
		nlwarning("<RESPAWN_POINT> %s : no default respawn point because '%s' in continent '%s' has no spawn zone",
			_Char.getId().toString().c_str(),
			defaultPlaceName.c_str(),
			CONTINENT::toString(continent).c_str()
			);
		return;
	}

	// add the default respawn point
	uint i;
	for( i=0; i<defaultPlace->getRespawnPoints().size(); ++i )
	{
		addRespawnPoint(defaultPlace->getRespawnPoints()[i]);
	}
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::setMissionRespawnPoints(CONTINENT::TContinent continent, const std::vector<TRespawnPoint> & respawnPoints, bool hideOthers)
{
	if (respawnPoints.empty())
		return;

	CMissionRespawnPoints & missionRespawnPoints = _MissionRespawnPointsByContinent[continent];
	static_cast<std::vector<TRespawnPoint> &>(missionRespawnPoints) = respawnPoints;
	missionRespawnPoints.setHideOthers(hideOthers);

	if (_Char.getCurrentContinent() == continent)
		resetUserDb();
}

//-----------------------------------------------------------------------------
const CCharacterRespawnPoints::CMissionRespawnPoints * CCharacterRespawnPoints::getMissionRespawnPoints(CONTINENT::TContinent continent) const
{
	map<sint32, CMissionRespawnPoints>::const_iterator it = _MissionRespawnPointsByContinent.find(continent);
	if (it != _MissionRespawnPointsByContinent.end())
	{
		return &(*it).second;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
bool CCharacterRespawnPoints::setRingAdventureRespawnpoint(const CFarPosition &farPos)
{
	if( farPos == _RingRespawnPoint )
		return false;

	_RingRespawnPoint = farPos;
	resetUserDb();
	return true;
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::clearRingRespawnpoint()
{
	_RingRespawnPoint.SessionId = TSessionId(0);
	resetUserDb();
}

//-----------------------------------------------------------------------------
bool CCharacterRespawnPoints::getRingAdventuresRespawnPoint( sint32 &x, sint32 &y ) const
{
	if( _RingRespawnPoint.SessionId.asInt() != 0 )
	{
		x = _RingRespawnPoint.PosState.X;
		y = _RingRespawnPoint.PosState.Y;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::getUsableRespawnPoints(CONTINENT::TContinent continent, std::vector<TRespawnPoint> & respawnPoints) const
{
	respawnPoints.clear();

	// ring re-spawn point override mission and regular re-spawn point
	if( _RingRespawnPoint.SessionId.asInt() != 0 )
	{
		respawnPoints.push_back(0); //only one ring re-spawn point
		return;
	}

	// mission respawn points
	const CMissionRespawnPoints * missionRespawnPoints = getMissionRespawnPoints(continent);
	if (missionRespawnPoints != NULL)
	{
		nlassert(!missionRespawnPoints->empty());
		respawnPoints = *missionRespawnPoints;

		// if they hide other respawn points, just return
		if (missionRespawnPoints->getHideOthers())
		{
			return;
		}
	}

	// for regular respawn points, check that player can use them (kami/karavan constraint)
	for (uint i = 0; i < _RegularRespawnPoints.size(); i++)
	{
		TRespawnPoint respawnPoint = _RegularRespawnPoints[i];
		if (isUsableRegularRespawnPoint(continent, respawnPoint))
		{
			// if the respawn point already was added from mission respawn points, skip it
			if (	missionRespawnPoints != NULL
				&&	std::find(missionRespawnPoints->begin(), missionRespawnPoints->end(), respawnPoint) != missionRespawnPoints->end())
			{
				continue;
			}

			respawnPoints.push_back(respawnPoint);
		}
	}
}

//-----------------------------------------------------------------------------
CONTINENT::TRespawnPointCounters CCharacterRespawnPoints::buildRingPoints() const
{
	CONTINENT::TRespawnPointCounters ret;

	CONTINENT::TRespawnPointCounters capital;

	for (uint i=0; i<_RegularRespawnPoints.size(); ++i)
	{
		const TRespawnPoint &rp = _RegularRespawnPoints[i];

		const CTpSpawnZone *tsz = CZoneManager::getInstance().getTpSpawnZone(rp);
		if (tsz != NULL 
			/*&& (tsz->getType() == RESPAWN_POINT::KAMI || tsz->getType() == RESPAWN_POINT::KARAVAN)*/)
		{
			ret[tsz->getContinent()]++;
			if(tsz->getPlaceType() == PLACE_TYPE::Capital)
			{
				capital[tsz->getContinent()]++;
			}
		}
	}

	// we must only add one point per re spawn points linked in capital
	for( CONTINENT::TRespawnPointCounters::iterator it = capital.begin(); it != capital.end(); ++it )
	{
		ret[(*it).first] -= (*it).second - 1;
	}

	return ret;
}


//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::clearRespawnPoints()
{
	_RegularRespawnPoints.clear();
	_MissionRespawnPointsByContinent.clear();
	resetUserDb();
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::cbClientReady()
{
	resetUserDb();
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::cbContinentChanged(CONTINENT::TContinent previousContinent)
{
	nlassert(previousContinent != _Char.getCurrentContinent());

	// clear the mission respawn points when the user leaves the associated continent
	std::map<sint32, CMissionRespawnPoints>::iterator it = _MissionRespawnPointsByContinent.find(previousContinent);
	if (it != _MissionRespawnPointsByContinent.end())
	{
		_MissionRespawnPointsByContinent.erase(it);
	}

	resetUserDb();
}

//-----------------------------------------------------------------------------
bool CCharacterRespawnPoints::isUsableRegularRespawnPoint(CONTINENT::TContinent continent, TRespawnPoint respawnPoint) const
{
	// no usable respawn point on an unknown continent!
	if (continent == CONTINENT::UNKNOWN)
		return false;

	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
	if (zone == NULL)
		return false;

	// check if the respawn point is in the continent
	if (zone->getContinent() != continent)
		return false;

	// check if the player is kami or karavan 	 
	static const uint32 kamiFaction = CStaticFames::getInstance().getFactionIndex("kami"); 	 
	static const uint32 karavanFaction = CStaticFames::getInstance().getFactionIndex("karavan"); 	 
	sint32 kamiFame = CFameInterface::getInstance().getFameIndexed(_Char.getId(), kamiFaction); 	 
	sint32 karavanFame = CFameInterface::getInstance().getFameIndexed(_Char.getId(), karavanFaction); 	 
	bool isKami = (kamiFame >= karavanFame); 	 
  	 
	if (zone->getType() == RESPAWN_POINT::KAMI && !isKami) 	 
	{
		return false; 	 
	}
	else if (zone->getType() == RESPAWN_POINT::KARAVAN && isKami) 	 
	{
		return false; 	 
    }
	return CPVPManager2::getInstance()->isRespawnValid( &_Char, respawnPoint );
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::resetUserDb() const
{
	if ( !_Char.getEnterFlag() )
		return;

	NLMISC::CEntityId id = _Char.getId();
	CMessage msgout("IMPULSION_ID");
	msgout.serial( id );
	CBitMemStream bms;
	nlverify( GenericMsgManager.pushNameToStream("DEATH:RESPAWN_POINT", bms) );

	CRespawnPointsMsg respawnPointMsg;

	if( _RingRespawnPoint.SessionId.asInt() != 0 )
	{
		CRespawnPointsMsg::SRespawnPoint rs;
		rs.x = _RingRespawnPoint.PosState.X;
		rs.y = _RingRespawnPoint.PosState.Y;
		respawnPointMsg.RespawnPoints.push_back(rs);
	}
	else
	{
		vector<TRespawnPoint> respawnPoints;
		getUsableRespawnPoints(_Char.getCurrentContinent(), respawnPoints);

		for (uint i = 0; i < respawnPoints.size(); i++)
		{
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoints[i]);
			if (zone == NULL)
			{
				nlwarning("<RESPAWN_POINT> %s : invalid zone id %u", id.toString().c_str(), respawnPoints[i]);
				continue;
			}

			CRespawnPointsMsg::SRespawnPoint rs;
			zone->getCenter(rs.x, rs.y);
			respawnPointMsg.RespawnPoints.push_back(rs);
		}
	}
	respawnPointMsg.NeedToReset = true;
	bms.serial( respawnPointMsg );
	msgout.serialMemStream( bms );
	CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(id.getDynamicId()), msgout);
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::addRespawnPointToUserDb(TRespawnPoint respawnPoint) const
{
	NLMISC::CEntityId id = _Char.getId();
	CMessage msgout("IMPULSION_ID");
	msgout.serial( id );
	CBitMemStream bms;
	nlverify( GenericMsgManager.pushNameToStream("DEATH:RESPAWN_POINT", bms) );

	CRespawnPointsMsg respawnPointMsg;
	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
	if (zone == NULL)
	{
		nlwarning("<RESPAWN_POINT> %s : invalid zone id %u", id.toString().c_str(), respawnPoint);
		return;
	}

	CRespawnPointsMsg::SRespawnPoint rs;
	zone->getCenter(rs.x, rs.y);
	respawnPointMsg.RespawnPoints.push_back(rs);
	respawnPointMsg.NeedToReset = false;
	bms.serial( respawnPointMsg );
	msgout.serialMemStream( bms );
	CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(id.getDynamicId()), msgout);
}

//-----------------------------------------------------------------------------
void CCharacterRespawnPoints::dumpRespawnPoints(NLMISC::CLog & log) const
{
	log.displayNL("Regular respawn points of player %s", _Char.getId().toString().c_str());
	for (uint i = 0; i < _RegularRespawnPoints.size(); i++)
	{
		const TRespawnPoint & respawnPoint = _RegularRespawnPoints[i];
		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
		if (zone == NULL)
		{
			log.displayNL("    %u : invalid spawn zone", respawnPoint);
			continue;
		}

		log.displayNL("    %s : id=%u, zone type='%s', continent='%s', usable=%s",
			zone->getName().c_str(),
			respawnPoint,
			RESPAWN_POINT::toString(zone->getType()).c_str(),
			CONTINENT::toString(zone->getContinent()).c_str(),
			isUsableRegularRespawnPoint(zone->getContinent(), respawnPoint) ? "true" : "false"
			);
	}

	log.displayNL("Mission respawn points of player %s", _Char.getId().toString().c_str());
	map<sint32, CMissionRespawnPoints>::const_iterator it;
	for (it = _MissionRespawnPointsByContinent.begin(); it != _MissionRespawnPointsByContinent.end(); ++it)
	{
		const CMissionRespawnPoints & missionRespawnPoints = (*it).second;

		log.displayNL("Continent '%s' (hideOthers=%s):",
			CONTINENT::toString(CONTINENT::TContinent((*it).first)).c_str(),
			missionRespawnPoints.getHideOthers()?"true":"false"
			);
		for (uint i = 0; i < missionRespawnPoints.size(); i++)
		{
			const TRespawnPoint & respawnPoint = missionRespawnPoints[i];
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
			if (zone == NULL)
			{
				log.displayNL("    %u : invalid spawn zone", respawnPoint);
				continue;
			}

			log.displayNL("    %s : id=%u, zone type='%s', continent='%s'",
				zone->getName().c_str(),
				respawnPoint,
				RESPAWN_POINT::toString(zone->getType()).c_str(),
				CONTINENT::toString(zone->getContinent()).c_str()
				);
		}
	}

	log.displayNL("Usable respawn points of player %s in the current continent '%s'",
		_Char.getId().toString().c_str(),
		CONTINENT::toString(_Char.getCurrentContinent()).c_str()
		);
	vector<TRespawnPoint> usableRespawnPoints;
	getUsableRespawnPoints(_Char.getCurrentContinent(), usableRespawnPoints);
	for (uint i = 0; i < usableRespawnPoints.size(); i++)
	{
		const TRespawnPoint & respawnPoint = usableRespawnPoints[i];
		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
		if (zone == NULL)
		{
			log.displayNL("    %u : invalid spawn zone", respawnPoint);
			continue;
		}

		log.displayNL("    %s : id=%u, zone type='%s', continent='%s'",
			zone->getName().c_str(),
			respawnPoint,
			RESPAWN_POINT::toString(zone->getType()).c_str(),
			CONTINENT::toString(zone->getContinent()).c_str()
			);
	}
}

//-----------------------------------------------------------------------------
//void CCharacterRespawnPoints::legacyLoad(NLMISC::IStream & f)
//{
//	if (f.isReading())
//	{
//		vector<uint16> oldSpawnZoneIds;
//		f.serialCont( oldSpawnZoneIds );
//
//		// convert old zone ids to new
//		_RegularRespawnPoints.clear();
//		for (uint i = 0; i < oldSpawnZoneIds.size(); i++)
//		{
//			const TRespawnPoint newId = BACK_COMPAT::oldToNewSpawnZoneId( oldSpawnZoneIds[i] );
//			if (newId != InvalidSpawnZoneId)
//			{
//				_RegularRespawnPoints.push_back( newId );
//			}
//		}
//	}
//	else
//	{
//		// ensure we won't try to save in old format anymore
//		nlassertex(false, ("<RESPAWN_POINT> you should not save in old format anymore!!!") );
//	}	
//}

