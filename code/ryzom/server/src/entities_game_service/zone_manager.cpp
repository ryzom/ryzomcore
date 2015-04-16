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

#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/config_file.h"
#include "nel/net/service.h"
#include "nel/ligo/primitive_utils.h"

#include "game_share/time_weather_season/time_and_season.h"

#include "zone_manager.h"
#include "mission_manager/mission_manager.h"
#include "primitives_parser.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "deposit.h"
#include "egs_globals.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/character_respawn_points.h"
#include "stables/stable.h"
#include "chat_groups_ids.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp_safe_zone.h"

//#include "backward_compatibility/spawn_zones_back_compat.h"
#include "backward_compatibility/places_back_compat.h"

#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"

#include "outpost_manager/outpost_manager.h"
#include "modules/shard_unifier_client.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

// The ecotype zones
CEcotypeZones CZoneManager::_EcotypeZones;
/*
const std::string CContinent::ContNames [] = 
{
	"fyros",
	"tryker"
	"zorai",
	"matis",
};
*/
/*
arg0: is the zone name id

arg1:
if zone is not pvp
	arg1 is interpreted as a boolean (0 - inactive, 1 - active)
if zone is a pvp zone
	arg1 is interpreted as 
		0 - inactive
		1 - active with faction point rewards
		2 - active without faction point rewards
*/
void cbSetZoneState( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	string sZoneName;
	uint32 nState;
	
	msgin.serial(sZoneName);
	msgin.serial(nState);
	
	CZoneManager *pZM = &CZoneManager::getInstance();

	// get the places
	CPlace *place = pZM->getPlaceFromName(sZoneName);
	if (place != NULL)
		if (place->isGooPath())
			place->setGooActive(nState != 0);



	// get the deposit zone (linear search)
	const vector<CDeposit*> &rDeps = pZM->getDeposits();
	for (uint32 i = 0; i < rDeps.size(); ++i)
	{
		CDeposit *pd = rDeps[i];
		if (pd->getName() == sZoneName)
		{
			pd->enable(nState!=0);
			return;
		}
	}

	// get the pvp zone
	CPVPManager *pPVPM = CPVPManager::getInstance();
	IPVPZone *pPVPZ = pPVPM->getPVPZone(sZoneName);
	if (pPVPZ != NULL)
	{
		pPVPZ->setActive(nState!=0);
		
		CPVPVersusZone *pPVPVZ = dynamic_cast<CPVPVersusZone*>(pPVPZ);
		if (pPVPVZ != NULL)
			pPVPVZ->giveFactionPoints(nState == 1);

		return;
	}

	// TODO : treat other zones !
}


//-----------------------------------------------
// CGooPath build
//-----------------------------------------------
/*bool CGooPath::build(const NLLIGO::CPrimPath * path,uint16 id )
{
	_Id = id;
	*( (NLLIGO::CPrimPath*)this ) = *path;
	
	if ( !zone->getPropertyByName("name",_Name) )
	{
		nlwarning("<CGooPath build> : no name in goo place %u", id);
		return false;
	}
	
	if ( VPoints.empty() )
	{
		nlwarning("<CGooPath build> :no points in goo place %u/%s", id, _Name.c_str());
		return false;
	}
	// get the bounding box
	float minX = VPoints[0].x;
	float minY = VPoints[0].y;
	float maxX = VPoints[0].x;
	float maxY = VPoints[0].y;
	for ( uint i = 1; i < VPoints.size(); i++)
	{
		if ( VPoints[i].x < minX )minX = VPoints[i].x;
		if ( VPoints[i].y < minY )minY = VPoints[i].y;
		if ( VPoints[i].x > maxX )maxX = VPoints[i].x;
		if ( VPoints[i].y > maxY )maxY = VPoints[i].y;
	}
	// get the center of the Box
	_CenterX = sint32 ( ( minX + maxX ) *1000.0f / 2.0f );
	_CenterY = sint32 ( ( minY + maxY ) *1000.0f / 2.0f );
	return true;
}// CGooPath build
*/

//-----------------------------------------------
// Tp Dest zone build
//-----------------------------------------------
bool CTpSpawnZone::build(const NLLIGO::CPrimPoint * point)
{
	*( (NLLIGO::CPrimPoint*)this ) = *point;
	_Continent = CONTINENT::UNKNOWN;
	string value;
	if ( !point->getPropertyByName("name",_Name) )
	{
		nlwarning("<CTpSpawnZone build> : no name in CTpSpawnZone" );
		return false;
	}
	if ( !point->getPropertyByName("radius",value) )
	{
		nlwarning("<CTpSpawnZone build> : no radius in CTpSpawnZone '%s'",_Name.c_str() );
		return false;
	}

	// get radius in mm
	_Radius = uint16( atof( value.c_str() ) * 1000.0f );
	// get the z value if necessary
	if ( !point->getPropertyByName("use_z", value) )
	{
		nlwarning("<CTpSpawnZone build> : no use_z in CTpSpawnZone '%s'",_Name.c_str() );
		return false;
	}

	if ( value == "true" )
	{
		if ( !point->getPropertyByName("z", value) )
		{
			nlwarning("<CTpSpawnZone build> : no z in CTpSpawnZone '%s'",_Name.c_str() );
			return false;
		}
		NLMISC::fromString(value, Point.z);
		Point.z = float( sint32 (1000.0f* Point.z) );
	}
	else
		Point.z = 0.0f;
	


	// convert coords in mm
	Point.x = float( sint32 (1000.0f* Point.x) );
	Point.y = float( sint32 (1000.0f* Point.y) );


	if ( !point->getPropertyByName("type", value)  )
	{
		nlwarning("<CTpSpawnZone build> : no type in CTpSpawnZone '%s'",_Name.c_str() );
		return false;
	}
	_Type = RESPAWN_POINT::toRespawnPointType( value );
	if ( _Type == RESPAWN_POINT::UNKNOWN )
	{
		nlwarning("<CTpSpawnZone build> : invalid type '%s' in CTpSpawnZone '%s'",value.c_str(), _Name.c_str() );
		return false;
	}
	return true;
}

//-----------------------------------------------
// CPlace build
//-----------------------------------------------
bool CPlace::build(const NLLIGO::CPrimPath * path, uint16 id)
{
	_Id = id;
	if (!path->VPoints.empty())
		*( (NLLIGO::CPrimZone*)this ) = *(NLLIGO::CPrimZone*) path;
	
	if ( !path->getPropertyByName("name",_Name) )
	{
		nlwarning("<CPlace build> : no name for GooPath %u", id);
		return false;
	}

	if ( VPoints.empty() )
	{
		nlwarning("<CPlace build> :no points in GooPath %u/%s", id, _Name.c_str());
		return false;
	}

	string val;
	if (path->getPropertyByName("main place", val))
		_MainPlace = val == "true";
	else
		_MainPlace = false;

	nlverify (CPrimitivesParser::getAlias(path, _Alias));
//	_Alias = NLMISC::fromString( val.c_str() );
	nlassert( _Alias != CAIAliasTranslator::Invalid );

	// get the bounding box
	float minX = VPoints[0].x;
	float minY = VPoints[0].y;
	float maxX = VPoints[0].x;
	float maxY = VPoints[0].y;
	for ( uint i = 1; i < VPoints.size(); i++)
	{
		if ( VPoints[i].x < minX )minX = VPoints[i].x;
		if ( VPoints[i].y < minY )minY = VPoints[i].y;
		if ( VPoints[i].x > maxX )maxX = VPoints[i].x;
		if ( VPoints[i].y > maxY )maxY = VPoints[i].y;
	}
	// get the center of the Box
	_CenterX = sint32 ( ( minX + maxX ) *1000.0f / 2.0f );
	_CenterY = sint32 ( ( minY + maxY ) *1000.0f / 2.0f );
	_GooPath = true;
	_GooActive = true;
	_Reported = false;

	return true;
}


//-----------------------------------------------
// CPlace build
//-----------------------------------------------
bool CPlace::build(const NLLIGO::CPrimZone * zone,uint16 id, bool reportAutorised )
{
	_Id = id;
	if (!zone->VPoints.empty())
		*( (NLLIGO::CPrimZone*)this ) = *zone;

	if ( !zone->getPropertyByName("name",_Name) )
	{
		nlwarning("<CPlace build> : no name in place %u", id);
		return false;
	}

	string val;
	if (zone->getPropertyByName("reported", val))
		_Reported = val == "true";
	else
		_Reported = true & reportAutorised;

	if (zone->getPropertyByName("main place", val))
		_MainPlace = val == "true";
	else
		_MainPlace = true;

	if (!CPrimitivesParser::getAlias(zone, _Alias))
	{
		nlwarning("Fatal Error, no alias in primitive '%s'",
			buildPrimPath(zone).c_str());
		nlstop;
	}
//	_Alias = NLMISC::fromString( val.c_str() );
	nlassert( _Alias != CAIAliasTranslator::Invalid );

	 
	
	if ( VPoints.empty() )
	{
		nlwarning("<CPlace build> :no points in place %u/%s", id, _Name.c_str());
		return false;
	}
	// get the bounding box
	float minX = VPoints[0].x;
	float minY = VPoints[0].y;
	float maxX = VPoints[0].x;
	float maxY = VPoints[0].y;
	for ( uint i = 1; i < VPoints.size(); i++)
	{
		if ( VPoints[i].x < minX )minX = VPoints[i].x;
		if ( VPoints[i].y < minY )minY = VPoints[i].y;
		if ( VPoints[i].x > maxX )maxX = VPoints[i].x;
		if ( VPoints[i].y > maxY )maxY = VPoints[i].y;
	}
	// get the center of the Box
	_CenterX = sint32 ( ( minX + maxX ) *1000.0f / 2.0f );
	_CenterY = sint32 ( ( minY + maxY ) *1000.0f / 2.0f );
	_GooPath = false;
	_GooActive = false;

	// get place_type of re-spawn point
	PLACE_TYPE::TPlaceType placeType;
	if( zone->getPropertyByName("place_type", val) )
	{
		placeType = PLACE_TYPE::fromString(val);
		if( placeType == PLACE_TYPE::Undefined )
			placeType = PLACE_TYPE::Place;
	}
	else
		placeType = PLACE_TYPE::Place;

	// get children respawn points
	bool ret = true;
	for (uint i=0; i< getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( getChild(child,i) && child )
		{
			child->getPropertyByName("name",val);
			uint16 idx = CZoneManager::getInstance().getTpSpawnZoneIdByName(val);
			if ( idx != InvalidSpawnZoneId )
			{
				const CTpSpawnZone * spawn = CZoneManager::getInstance().getTpSpawnZone( idx );
				if ( spawn == NULL )
				{
					nlwarning( "Invalid spawn zone '%s' in place '%s' : bad index %u",val.c_str(), _Name.c_str(), idx );
					ret = false;
				}
				// if the respawn point is validated when user enters the place, add it to the place.
				// we dont add special respawn points ( outposts,... ) because they are validated separatly from the place where they are
				else if ( spawn->getType() == RESPAWN_POINT::KAMI ||
					spawn->getType() == RESPAWN_POINT::KARAVAN ||
					spawn->getType() == RESPAWN_POINT::NEWBIELAND ||
					spawn->getType() == RESPAWN_POINT::RESPAWNABLE )
				{
					(const_cast<CTpSpawnZone *>(spawn))->setPlaceType(placeType);
					_RespawnPoints.push_back( idx );
				}
			}
		}
	}
	return ret;
}// CPlace build

//-----------------------------------------------
// CRegion build
//-----------------------------------------------
bool CRegion::build(const NLLIGO::CPrimZone * zone,uint16 id)
{
	if ( CPlace::build(zone,id) )
	{
		string value;
		if ( !zone->getPropertyByName("newbie_region",value) )
		{
			nlwarning("<CRegion build> : failed : no newbie_region property in primitive");
			_NewbieRegion = false;
		}
		else
		{
			_NewbieRegion = (!nlstricmp( value,"true"))?true:false;
		}
		return true;
	}
	else
	{
		nlwarning("<CRegion build> : failed in place build");
		return false;
	}
}// CRegion build

//-----------------------------------------------
// CRegion dtor
//-----------------------------------------------
CRegion::~CRegion()
{
	
}// CRegion dtor

//-----------------------------------------------
// CRegion registerChatGroup
//-----------------------------------------------
void CRegion::registerChatGroup()
{
	TGroupId idGroup = CHAT_GROUPS_IDS::getRegionChatGroupId(_Id);
	CMessage msgout("ADD_GROUP");
	msgout.serial( idGroup );
	CChatGroup::TGroupType type = CChatGroup::region;
	msgout.serialEnum( type );
	sendMessageViaMirror( "IOS", msgout );
	
	// add online members ( if IOS crashed... )
	set< CEntityId >::iterator it = _Players.begin();
	for (; it != _Players.end(); ++it )
	{
		TDataSetRow row = TheDataset.getDataSetRow( (*it) );
		// if we can get the user from this row, the user is ingame
		CCharacter* user = PlayerManager.getChar( row );
		if ( user )
		{
			CMessage msgout("ADD_TO_GROUP");
			msgout.serial( idGroup );
			msgout.serial( const_cast<NLMISC::CEntityId &>(user->getId()) );
			sendMessageViaMirror( "IOS", msgout );
		}
	}
}

//-----------------------------------------------
// CRegion addPlayer
//-----------------------------------------------
void CRegion::addPlayer( const NLMISC::CEntityId & id )
{
	_Players.insert(id);
	TGroupId idGroup = CHAT_GROUPS_IDS::getRegionChatGroupId(_Id);
	CMessage msgout("ADD_TO_GROUP");
	msgout.serial( idGroup );
	msgout.serial( const_cast<NLMISC::CEntityId &>(id) );
	sendMessageViaMirror( "IOS", msgout );
}

//-----------------------------------------------
// CRegion removePlayer
//-----------------------------------------------
void CRegion::removePlayer( const NLMISC::CEntityId & id )
{
	_Players.erase(id);
	TGroupId idGroup = CHAT_GROUPS_IDS::getRegionChatGroupId(_Id);
	CMessage msgout("REMOVE_FROM_GROUP");
	msgout.serial( idGroup );
	msgout.serial( const_cast<NLMISC::CEntityId &>(id) );
	sendMessageViaMirror( "IOS", msgout );
}

//-----------------------------------------------
// CContinent build
//-----------------------------------------------
bool CContinent::build(const NLLIGO::CPrimZone * zone)
{
	string value;
	if ( zone->getPropertyByName("id",value) )
	{
		_Id = CONTINENT::toContinent( value );
		if ( _Id != CONTINENT::UNKNOWN )
		{
			return CPlace::build( zone,_Id );
		}
		else
			nlwarning("<CContinent build> : invalid continent id '%s'",value.c_str());
	}
	else
		nlwarning("<CContinent build> : no id in a continent");
	return false;
}// CContinent build

//-----------------------------------------------
// CContinent dtor
//-----------------------------------------------
CContinent::~CContinent()
{
	
}// CContinent dtor


//-----------------------------------------------
// CZoneManager init
//-----------------------------------------------
void CZoneManager::init()
{
	RandomGenerator.srand( CTickEventHandler::getGameCycle() );
	initInstance();
}// CZoneManager init

//-----------------------------------------------
// CZoneManager release
//-----------------------------------------------
void CZoneManager::release()
{
	for (uint i = 0; i < _Deposits.size(); i++ )
		delete _Deposits[i];
	for ( uint i = 0; i < _Places.size(); i++ )
		delete _Places[i];
	clearEcotypes();
}// CZoneManager release

//-----------------------------------------------
// CZoneManager initInstance
//-----------------------------------------------
void CZoneManager::initInstance()
{
	_NextDepositIndexUpdated = 0;
	_SpreadUpdateLoopBeginTick = CTickEventHandler::getGameCycle();
	
	// get the loaded primitives
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();	

	_PlacesByAlias.clear();
	// parse the zones
	nlinfo("CZoneManager : parsing the zones");
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;
	
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseContinents(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the continents");
		}
	}

	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseRegions(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the regions");
		}
	}

	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseTpSpawnZones(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the tp spawn Zones");
		}
	}

	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseZones(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the zones");
		}
	}

	// Parse ecotypes, that will be used when building deposits
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseEcotypes( first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the ecotypes");
		}
	}

	// Parse (and build) deposits
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseDeposits(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the zones");
		}
	}

	// Don't keep ecotypes in memory, the information is already in the deposits
	CDeposit::clearEcotypes();
 
	// Parse stables
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseStables(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the stables");
		}
	}
	
	// Parse the goo border
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseGooBorder(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the Goo Border");
		}
	}

	// preallocate 5 start points per newbieland
	_StartPoints.reserve( 20 );
	// Parse the start points
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if (! parseStartPoints(first->Primitive.RootNode ) )
		{
			nlwarning("<CZoneManager constructor> Error while building the start points");
		}
	}

	// parse the PVP zones, must be before parsing the PVP safe zones!
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if ( !parsePVPZones(first->Primitive.RootNode) )
		{
			nlwarning("<CZoneManager constructor> Error while building the PVP zones");
		}
	}

	// parse the PVP safe zones (except for outpost zones)
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		if ( !parsePVPSafeZones(first->Primitive.RootNode) )
		{
			nlwarning("<CZoneManager constructor> Error while building the PVP safe zones");
		}
	}

	// apply config to PVP zones (must be done after loading primitives), except outpost ones
	CPVPManager::getInstance()->applyConfigToPVPZones();

	// for backward compatibility
	//BACK_COMPAT::initSpawnZonesCompat();
	BACK_COMPAT::initPlacesCompat();


	// Initialize messages from other services

	// array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "SET_ZONE_STATE",		cbSetZoneState },
	};
	// register call back for zone manager
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
	
}// CZoneManager ctor

//-----------------------------------------------
// CZoneManager dtor
//-----------------------------------------------
CZoneManager::~CZoneManager()
{
	// remove deposits
	for (uint i = 0; i < _Deposits.size(); i++ )
		delete _Deposits[i];
	// all must have unregistered from the set of autospawn update
	nlassert(_DepositNeedingAutoSpawnUpdate.empty());

	// remove places
	for ( uint i = 0; i < _Places.size(); i++ )
		delete _Places[i];

	// clear ecotypes
	clearEcotypes();
}// CZoneManager dtor

//-----------------------------------------------
// CZoneManager iosConnection
//-----------------------------------------------
void CZoneManager::iosConnection()
{
	for ( uint i = 0; i < _Continents.size(); i++ )
	{
		for ( uint j = 0; j < _Continents[i].getRegions().size(); j++ )
		{
			_Continents[i].getRegions()[j]->registerChatGroup();
		}
	}
}// CZoneManager iosConnection

//-----------------------------------------------
// CZoneManager getTpSpawnZoneIdByName
//-----------------------------------------------
uint16 CZoneManager::getTpSpawnZoneIdByName( const std::string & name )
{
	map<string,uint16>::const_iterator it = _TpSpawnZoneIdByName.find( name );
	if ( it == _TpSpawnZoneIdByName.end() )
	{
		return InvalidSpawnZoneId;
	}

	const uint16 id = (*it).second;
	BOMB_IF( (id >= _TpSpawnZones.size()), "<SPAWN_ZONE> invalid spawn zone id", return InvalidSpawnZoneId );
	BOMB_IF( (name != _TpSpawnZones[id].getName()), "<SPAWN_ZONE> spawn zone name does not match", return InvalidSpawnZoneId );

	return id;
}// CZoneManager getTpSpawnZoneIdByName


//-----------------------------------------------
// CZoneManager parseContinents
//-----------------------------------------------
bool CZoneManager::parseContinents( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	const CPrimZone* zone = dynamic_cast<const CPrimZone*>( prim);
	// if the primitive is a continent, parse it, build it and add it to this manager if it is correct
	if ( zone && zone->getPropertyByName("class",value) && value == "continent" )
	{
		CContinent continent;
		if ( continent.build( zone ) )
		{
			uint i = 0;
			for (; i < _Continents.size(); i++ )
			{
				if ( _Continents[i].getId() == continent.getId() )
					break;
			}
			if ( i == _Continents.size() )
			{
				_Continents.push_back( continent );
			}
		}
	}
		// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseContinents(child) && ok;
	}
	return ok;
}// CZoneManager parseContinents

//-----------------------------------------------
// CZoneManager parseRegions
//-----------------------------------------------
bool CZoneManager::parseRegions( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	const CPrimZone* zone = dynamic_cast<const CPrimZone*>(prim);
	// if the primitive is a continent, parse it, build it and add it to this manager if it is correct
	if ( zone && prim->getPropertyByName("class",value) && value == "region" )
	{
		CRegion * region = new CRegion();
		if ( region->build( zone,(uint16)_Places.size() ) )
		{
			bool found = false;
			for (uint i = 0; i < _Continents.size(); i++ )
			{
				const std::vector<CPrimVector> & regionPoints = zone->VPoints;
				for (uint j = 0; j < regionPoints.size(); j++ )
				{
					if ( _Continents[i].contains( regionPoints[j] ) )
					{
						found = true;
						_Continents[i].addRegion( region );
						region->setContinent( (CONTINENT::TContinent) _Continents[i].getId() );
						break;
					}
				}
			}
			nlassertex(found, (toString("No continent found that includes the region %s", prim->getName().c_str()).c_str() ) );
			_Places.push_back( region );
			_PlacesByAlias.insert( make_pair(region->getAlias(), region) );
		}
		else
			delete region;
	}
	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseRegions(child) && ok;
	}
	return ok;
}// CZoneManager parseRegions

//-----------------------------------------------
// CZoneManager parseZones
//-----------------------------------------------
bool CZoneManager::parseZones( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	const CPrimZone* zone = dynamic_cast<const CPrimZone*>(prim);
	// if the primitive is a zone, parse it
	if ( zone )
	{
		// build the zone and add it to this manager if it is correct
		if (prim->getPropertyByName("class",value) )
		{
			if ( value == "place" )
			{
				CPlace* place = new CPlace();
				if ( place->build( zone,(uint16)_Places.size() ) )
				{
					for (uint i = 0; i < _Continents.size(); i++ )
					{
						for (uint j = 0; j < _Continents[i].getRegions().size(); j++ )
						{
							for ( uint k = 0; k < place->VPoints.size(); k++ )
							{
								if ( _Continents[i].getRegions()[j]->contains( place->VPoints[k] ) )
								{
									_Continents[i].getRegions()[j]->addPlace( place );
									break;
								}
							}
						}
					}
					_Places.push_back( place );
					_PlacesByAlias.insert( make_pair(place->getAlias(), place) );
				}
				else
					delete place;
			}
		}
	}
	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseZones(child) && ok;
	}
	return ok;
} // CZoneManager parseZones

//-----------------------------------------------
// CZoneManager parseEcotypes
//-----------------------------------------------
bool CZoneManager::parseEcotypes( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "ecotypezone_list" )
		{
			std::string ecotypeZoneListName;
			prim->getPropertyByName( "name", ecotypeZoneListName );
			
			// Parse the children ecotypes
			for( uint i = 0; i < prim->getNumChildren(); ++i )
			{
				const IPrimitive * childPrim;
				if ( prim->getChild( childPrim, i ) )
				{
					// If the primitive is a zone, parse it
					const CPrimZone* zone = dynamic_cast<const CPrimZone*>(childPrim);		
					if ( zone )
					{	
						string primType;
						childPrim->getPropertyByName( "class", primType );
						if( primType == "ecotypezone" )
						{
							CEcotypeZone *ecotype = new CEcotypeZone();
							if ( ecotype->build( zone ) )
							{
								CDeposit::addEcotype( ecotype );
								CEcotypeZone *ecotype2 = new CEcotypeZone();
								if( ecotype2->build( zone ) )
									CZoneManager::addEcotype( ecotype2 );
								else
									delete ecotype2;
							}
							else
								delete ecotype;
						}
					}
				}
			}
		}
	}

	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseEcotypes(child) && ok;
	}
	return ok;
}

//-----------------------------------------------
// CZoneManager parseDeposits
// Assumes the ecotypes have been parsed before
//-----------------------------------------------
bool CZoneManager::parseDeposits( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "depositzone_list" )
		{
			std::string depositZoneListName;
			prim->getPropertyByName( "name", depositZoneListName );
			
			// Parse the children deposits
			for( uint i = 0; i < prim->getNumChildren(); ++i )
			{
				const IPrimitive * childPrim;
				if ( prim->getChild( childPrim, i ) )
				{
					// If the primitive is a zone, parse it
					const CPrimZone* zone = dynamic_cast<const CPrimZone*>(childPrim);		
					if ( zone )
					{	
						string primType;
						childPrim->getPropertyByName( "class", primType );
						if( primType == "depositzone" )
						{
							bool found = false;
							CDeposit * deposit = new CDeposit;
							if ( deposit->build( zone ) )
							{
								for (uint j = 0; j < _Continents.size(); j++ )
								{
									for (uint k = 0; k < _Continents[j].getRegions().size(); k++ )
									{
										uint l = 0;
										for ( ; l < deposit->VPoints.size(); l++ )
										{
											if ( _Continents[j].getRegions()[k]->contains( deposit->VPoints[l] ) )
											{
												_Continents[j].getRegions()[k]->addDeposit( deposit );
												found = true;
												break;
											}
										}
									}
									if ( !found )
									{
										for ( uint k = 0; k < _Continents[j].getRegions().size(); k++ )
										{
											for ( uint l = 0 ; l < _Continents[j].getRegions()[k]->VPoints.size(); l++ )
											{
												if ( deposit->contains( _Continents[j].getRegions()[k]->VPoints[l] ) )
												{
													_Continents[j].getRegions()[k]->addDeposit( deposit );
													found = true;
													break;
												}
											}
										}
									}
								}
								if ( !found  )
									nlwarning("<CZoneManager parseDeposits> a deposit of %s is not in a region", depositZoneListName.c_str());
								_Deposits.push_back(deposit);
							}
							else
								delete deposit;
						}
					}
				}
			}
		}
	}
				
	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseDeposits(child) && ok;
	}
	return ok;
} // CZoneManager parseDeposits

//-----------------------------------------------
// CZoneManager parseStables
//-----------------------------------------------
bool CZoneManager::parseStables( const NLLIGO::IPrimitive* prim )
{
// Primitives format:	
// <!-- stables -->
// <PRIMITIVE CLASS_NAME="stables" TYPE="node" AUTO_INIT="true" DELETABLE="true">
// <DYNAMIC_CHILD CLASS_NAME="stable"/>
// </PRIMITIVE>
//
// <!-- stable -->
// <PRIMITIVE CLASS_NAME="stable" TYPE="zone"  R="0" G="255" B="255" A="128" AUTO_INIT="false" DELETABLE="true">>
// <PARAMETER NAME="name" TYPE="string" VISIBLE="true"/>
//
// <STATIC_CHILD CLASS_NAME="stable_entry" NAME ="stable entry"/>
// </PRIMITIVE>
//
// <!-- stable entry point -->
// <PRIMITIVE CLASS_NAME="stable_entry" TYPE="point" R="128" G="50" B="200" A="128" AUTO_INIT="true" DELETABLE="false">
// </PRIMITIVE>

	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "stables" )
		{
			string stableName, continent;

			prim->getPropertyByName( "name", stableName );

			for( uint c = 0; c < prim->getNumChildren(); ++c )
			{
				const NLLIGO::IPrimitive* child;
				prim->getChild(child,c);
				if (child->getPropertyByName("class",value) )
				{
					if( value == "stable" )
					{
						//geometry of stable point (normally one dot with directional information)
						const NLLIGO::CPrimZone* primZone = dynamic_cast<const CPrimZone*>(child);
						if( primZone )
						{
							bool found = false;

							CPlace* place = new CPlace();
							if ( place->build( primZone,(uint16)_Places.size(), true ) )
							{
								for (uint i = 0; i < _Continents.size(); i++ )
								{
									continent = CONTINENT::toString( (CONTINENT::TContinent)_Continents[i].getId() );
									for (uint j = 0; j < _Continents[i].getRegions().size(); ++j )
									{
										for ( uint k = 0; k < place->VPoints.size(); ++k )
										{
											if ( _Continents[i].getRegions()[j]->contains( place->VPoints[k] ) )
											{
												_Continents[i].getRegions()[j]->addPlace( place );
												found = true;
												// Displaying the aliases of all the stables:
												//string branchPath = _Continents[i].getName() + "." + _Continents[i].getRegions()[j]->getName() + "." + place->getName();
												//InfoLog->displayRawNL( "STBL:%s: %u", branchPath.c_str(), place->getAlias() );
												break;
											}
										}
									}
								}
							}

							if( place->getId() == 0x35 )
							{
								nlinfo("catch it!");
							}

							if( found == false )
							{
								nlwarning("<CZoneManager::parseStables> Stable %s is not in a region", stableName.c_str());
								delete place;
							}
							else
							{
								_Places.push_back( place );
								_PlacesByAlias.insert( make_pair(place->getAlias(), place) );

								for( uint c2 = 0; c2 < child->getNumChildren(); ++c2 )
								{
									const NLLIGO::IPrimitive* child2;
									child->getChild(child2,c2);

									if (child2->getPropertyByName("class",value) )
									{
										if( value == "stable_entry" )
										{
											const NLLIGO::CPrimPoint *stableEntryPoint = dynamic_cast< const CPrimPoint * >(child2);
											if( stableEntryPoint == 0 )
											{
												nlwarning("<CZoneManager::parseStables> Stable %s not contained stable entry point", stableName.c_str());
												return false;
											}
											CStable::getInstance()->addStable( stableName, place->getId(), continent, stableEntryPoint->Point.x, stableEntryPoint->Point.y, stableEntryPoint->Point.z, stableEntryPoint->Angle );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseStables(child) && ok;
	}
	return ok;
}

//-----------------------------------------------
// CZoneManager parseTpSpawnZones
//-----------------------------------------------
bool CZoneManager::parseTpSpawnZones( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "teleport_spawn_zone" )
		{
			const NLLIGO::CPrimPoint* point = dynamic_cast<const CPrimPoint*>(prim);
			if ( !point )
			{
				nlwarning("CZoneManager::parseTpSpawnZones -> teleport_trigger should be prim points");
				return false;
			}
			CTpSpawnZone zone;
			if( !zone.build( point ) )
				return false;

			// if it is a respawn point get the continent where the zone is
			if ( zone.getType() == RESPAWN_POINT::KAMI ||
				zone.getType() == RESPAWN_POINT::KARAVAN ||
				zone.getType() == RESPAWN_POINT::NEWBIELAND ||
				zone.getType() == RESPAWN_POINT::RESPAWNABLE )
			{
				bool found = false;
				uint j;
				for ( j = 0; j < _Continents.size() && !found; ++j )
				{
					if ( _Continents[j].contains( point->Point ) )
					{
						zone.setContinent((CONTINENT::TContinent) _Continents[j].getId());
						found = true;
						break;
					}
				}
				if( !found )	
				{
					nlwarning("<CZoneManager::parseTpSpawnZones> Re-spawn point '%s' is not in a continent", zone.getName().c_str());
					return false;
				}

				found = false;
				for ( uint k = 0; k < _Continents[j].getRegions().size(); k++ )
				{
					if( _Continents[j].getRegions()[k]->contains( point->Point ) )
					{
						zone.setRegion( _Continents[j].getRegions()[k]->getId() );
						found = true;
						break;
					}
				}
				if( ! found )
				{
					nlwarning("<CZoneManager::parseTpSpawnZones> Re-spawn point '%s' is not in a region", zone.getName().c_str());
					return false;
				}
			}

			nlassert( _TpSpawnZones.size() < 0xffff );

			map<string,uint16>::const_iterator it = _TpSpawnZoneIdByName.find( zone.getName() );
			BOMB_IF( (it != _TpSpawnZoneIdByName.end()),
				toString( "<SPAWN_ZONE> spawn zone name '%s' is already used!!!", zone.getName().c_str() ),
				return false
				);

			_TpSpawnZoneIdByName[zone.getName()] = (uint16)_TpSpawnZones.size();
			_TpSpawnZones.push_back( zone );
			return true;
		}
	}
	
	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseTpSpawnZones(child) && ok;
	}
	return ok;
}

//-----------------------------------------------
// CZoneManager parseGooBorder
//-----------------------------------------------
bool CZoneManager::parseGooBorder( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "goo_border" )
		{
			string gooBorderName, continent;
			prim->getPropertyByName( "name", gooBorderName );

			const CPrimPath * path = dynamic_cast<const CPrimPath*>(prim);
			if( path )
			{
				CPlace* place = new CPlace();
				if ( place->build( path,(uint16)_Places.size() ) ) //assume CPrimPath and CPrimZone has the same members, method needed are only in CPrimZone
				{
					for (uint i = 0; i < _Continents.size(); i++ )
					{
						for (uint j = 0; j < _Continents[i].getRegions().size(); j++ )
						{
							for ( uint k = 0; k < place->VPoints.size(); k++ )
							{
								if ( _Continents[i].getRegions()[j]->contains( place->VPoints[k] ) )
								{
									_Continents[i].getRegions()[j]->addPlace( place );
									break;
								}
							}
						}
					}
					_Places.push_back( place );
					_PlacesByAlias.insert( make_pair(place->getAlias(), place) );
				}
				else
				{
					delete place;
				}
			}
		}
	}

	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseGooBorder(child) && ok;
	}
	return ok;
}

//-----------------------------------------------
// CZoneManager parseStartPoints
//-----------------------------------------------
bool CZoneManager::parseStartPoints( const NLLIGO::IPrimitive* prim )
{
	std::string value;
	if (prim->getPropertyByName("class",value) )
	{
		if ( value == "start_village" )
		{
			string value;
			prim->getPropertyByName( "name", value );
			RYZOM_STARTING_POINT::TStartPoint start = RYZOM_STARTING_POINT::toStartPoint( value );
			if ( start == RYZOM_STARTING_POINT::NB_START_POINTS )
			{
				nlwarning("<parseStartPoints> invalid start point %s",value.c_str() );
				return false;
			}
			uint16 startIdx = (uint16) start;
			if ( startIdx >= _StartPoints.size() )
				_StartPoints.resize( startIdx + 1);

			bool ok = true;
			for (uint i=0;i<prim->getNumChildren();++i)	
			{
				const IPrimitive *child = NULL;
				if ( prim->getChild(child,i) && child && child->getPropertyByName("class",value) && value == "start_point")
				{
					CStartPoint point;
					child->getPropertyByName( "mission", value );
					point.Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( value );
					if ( point.Mission == CAIAliasTranslator::Invalid )
					{
						nlwarning("<parseStartPoints> invalid mission %s",value.c_str() );
					}
					child->getPropertyByName( "welcomer", value );
					point.Welcomer = CAIAliasTranslator::Invalid;
					vector<TAIAlias> aliases;
					CAIAliasTranslator::getInstance()->getNPCAliasesFromName( value, aliases );
					if ( aliases.empty() )
					{
						nlwarning("<parseStartPoints> invalid welcomer %s",value.c_str() );
					}
					else
						point.Welcomer = aliases[0];
					if ( point.Welcomer == CAIAliasTranslator::Invalid )
					{
						nlwarning("<parseStartPoints> invalid welcomer %s",value.c_str() );
					}
					child->getPropertyByName( "spawn_zone", value );
					point.SpawnZoneId = getTpSpawnZoneIdByName( value );
					if ( point.SpawnZoneId == InvalidSpawnZoneId )
					{
						nlwarning("<parseStartPoints> invalid spawn zone %s",value.c_str() );
						ok = false;
					}
					if ( ok )
					{
						_StartPoints[startIdx].push_back( point );
					}
				}
			}
			return ok;
		}
	}
	// Lookup recursively in the children
	bool ok = true;
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			ok = parseStartPoints(child) && ok;
	}
	return ok;
}

CVariable<bool> LoadPVPFreeZones("egs", "LoadPVPFreeZones", "If true PVP free zones will be loaded", false, 0, true );
CVariable<bool> LoadPVPVersusZones("egs", "LoadPVPVersusZones", "If true PVP versus zones will be loaded", false, 0, true );
CVariable<bool> LoadPVPGuildZones("egs", "LoadPVPGuildZones", "If true PVP guild zones will be loaded", false, 0, true );

//-----------------------------------------------
// CZoneManager parsePVPZones
//-----------------------------------------------
bool CZoneManager::parsePVPZones( const NLLIGO::IPrimitive * prim )
{
	std::string value;
	prim->getPropertyByName("class",value);
	const CPrimZone * zone = dynamic_cast<const CPrimZone *>(prim);

	// if the primitive is a zone, parse it
	if ( zone && (value == "pvp_zone") )
	{
		CSmartPtr<IPVPZone> pvpZone = IPVPZone::build( zone );
		bool load = false;

		switch (pvpZone->getPVPZoneType())
		{
		case PVP_ZONE_TYPE::FreeZone:
			load = LoadPVPFreeZones;
			break;

		case PVP_ZONE_TYPE::VersusZone:
			load = LoadPVPVersusZones;
			break;

		case PVP_ZONE_TYPE::GuildZone:
			load = LoadPVPGuildZones;
			break;

		default:
			nlwarning( "Invalid %s zone in pvp_zone primitive %s", PVP_ZONE_TYPE::toString( pvpZone->getPVPZoneType() ).c_str(), prim->getName().c_str() );
			load = false;
		}
		if (load)
			CPVPManager::getInstance()->addPVPZone( pvpZone );
	}


	// lookup recursively in the children
	bool result = true;
	for (uint i = 0; i < prim->getNumChildren(); i++)
	{
		const IPrimitive * child;
		if ( prim->getChild(child, i) )
			result &= parsePVPZones(child);
	}

	return result;
} // CZoneManager parsePVPZones

//-----------------------------------------------
// CZoneManager parsePVPSafeZones
//-----------------------------------------------
bool CZoneManager::parsePVPSafeZones( const NLLIGO::IPrimitive * prim )
{
	std::string value;
	const CPrimPoint * point = dynamic_cast<const CPrimPoint *>(prim);

	// if the primitive is a point, parse it
	if ( point && prim->getPropertyByName("class",value) && value == "safe_zone" )
	{
		value = "true";
		prim->getPropertyByName("safe_from_pvp", value);
		if ( value == "true" )
		{
			CSmartPtr<CPVPSafeZone> safeZone = CPVPSafeZone::build( point );
			if ( !safeZone.isNull() )
			{
				CPVPManager::getInstance()->addPVPSafeZone( safeZone );
				CPVPManager2::getInstance()->addPVPSafeZone( safeZone );
			}
		}
	}

	// lookup recursively in the children
	bool result = true;
	for (uint i = 0; i < prim->getNumChildren(); i++)
	{
		const IPrimitive * child;
		if ( prim->getChild(child, i) )
			result &= parsePVPSafeZones(child);
	}

	return result;
} // CZoneManager parsePVPSafeZones

//-----------------------------------------------
// CZoneManager getContinent
//-----------------------------------------------
CContinent * CZoneManager::getContinent( sint32 x, sint32 y )
{
	CVector vect( x * 0.001f, y * 0.001f, 0.0f );
	for ( uint i = 0; i < _Continents.size(); i++ )
	{
		if ( _Continents[i].contains(vect) )
			return &_Continents[i];
	}
	return NULL;
	return getContinent(vect);
}// CZoneManager getContinent*


//-----------------------------------------------
// Same with vector
//-----------------------------------------------
CContinent * CZoneManager::getContinent( const NLMISC::CVector& pos )
{
	for ( uint i = 0; i < _Continents.size(); i++ )
	{
		if ( _Continents[i].contains(pos) )
			return &_Continents[i];
	}
	return NULL;
}// CZoneManager getContinent


//-----------------------------------------------
// CZoneManager getRegion
//-----------------------------------------------
bool CZoneManager::getRegion( sint32 x, sint32 y, const CRegion ** region, const CContinent ** continent)
{
	nlassert(region);
	CVector vect( x * 0.001f, y * 0.001f, 0.0f );
	for ( uint i = 0; i < _Continents.size(); i++ )
	{
		if ( _Continents[i].contains(vect) )
		{
			for (uint j = 0; j < _Continents[i].getRegions().size(); j++ )
			{
				if ( _Continents[i].getRegions()[j]->contains( vect ) )
				{
					if ( continent )
						*continent = &_Continents[i];

					*region =  _Continents[i].getRegions()[j];
					return true;
				}
			}
		}
	}
	return false;
}// CZoneManager getRegion

#include <limits>

//-----------------------------------------------
// CZoneManager getPlace
//-----------------------------------------------
bool CZoneManager::getPlace( sint32 x, sint32 y, float& gooDistance, const CPlace ** stable, std::vector<const CPlace *>& places, const CRegion ** region , const CContinent ** continent )
{
	nlassert(stable);

	if( continent ) *continent = NULL;
	if( region ) *region = NULL;
	*stable = NULL;

	float nearGooDistance = numeric_limits<float>::max();

	CVector vect( x * 0.001f, y * 0.001f, 0.0f );
	for ( uint i = 0; i < _Continents.size(); i++ )
	{
		if ( _Continents[i].contains(vect) )
		{
			if ( continent )
				*continent = &_Continents[i];
			for (uint j = 0; j < _Continents[i].getRegions().size(); j++ )
			{
				if ( _Continents[i].getRegions()[j]->contains( vect ) )
				{
					if ( region )
					{
						*region = _Continents[i].getRegions()[j];
					}
					for (uint k = 0; k < _Continents[i].getRegions()[j]->getPlaces().size(); k++ )
					{
						if(!_Continents[i].getRegions()[j]->getPlaces()[k]->isGooActive())
						{
							if ( _Continents[i].getRegions()[j]->getPlaces()[k]->contains( vect ) )
							{
								const CPlace * p;
								p = _Continents[i].getRegions()[j]->getPlaces()[k];

								CStable::TStableData stableData;
								if( CStable::getInstance()->getStableData( p->getId(), stableData ) )
								{
									*stable = p;
								}
								places.push_back( p );
							}
						}
						else
						{
							CVector p1, p2, nearPos;
							float distance;

							CPrimZone::contains (vect, _Continents[i].getRegions()[j]->getPlaces()[k]->VPoints, distance, nearPos, true);
							if( distance < nearGooDistance )
							{
								nearGooDistance = distance;
							}
						}
					}
					gooDistance = nearGooDistance;
					return true;
				}
			}
		}
	}
	gooDistance = nearGooDistance;
	return false;
}// CZoneManager getPlace


//-----------------------------------------------
// CZoneManager getRegion
//-----------------------------------------------
CRegion * CZoneManager::getRegion( const NLMISC::CVector& pos )
{
	const CRegion *cregion = 0;
	if ( (! getRegion( (sint32)(pos.x*1000.0f), (sint32)(pos.y*1000.0f), &cregion )) || (! cregion) )
	{
		nlwarning( "Invalid region for pos %s", pos.asString().c_str() );
		return NULL;
	}
	return (const_cast<CRegion*>(cregion));
}


//-----------------------------------------------
// CZoneManager getDepositsUnderUser
//-----------------------------------------------
void CZoneManager::getDepositsUnderPos( const CVector& pos, std::vector<CDeposit*>& deposits, bool warnIfOutsideOfRegion )
{
	const CRegion *cregion = 0;
	CRegion *region;
	if ( (! getRegion( (sint32)(pos.x*1000.0f), (sint32)(pos.y*1000.0f), &cregion )) || (! cregion) )
	{
		if ( warnIfOutsideOfRegion )
			nlwarning( "<CZoneManager getDepositsUnderPos> invalid region for pos %s", pos.asString().c_str() );
		return;
	}
	region = (const_cast<CRegion*>(cregion));
	deposits.clear();
	for (uint i=0; i!=region->getDeposits().size(); ++i )
	{
		if ( region->getDeposits()[i]->contains( pos ) )
			deposits.push_back( region->getDeposits()[i] );
	}
}// CZoneManager getDepositsUnderPos


//
// get the first deposit found under the position (faster than getDepositsUnderPos()), or NULL if not found
//
CDeposit* CZoneManager::getFirstFoundDepositUnderPos( const NLMISC::CVector& pos )
{
	const CRegion *cregion = 0;
	CRegion *region;
	if ( (! getRegion( (sint32)(pos.x*1000.0f), (sint32)(pos.y*1000.0f), &cregion )) || (! cregion) )
	{
		nlwarning( "<CZoneManager getFirstFoundDepositUnderPos> invalid region for pos %s", pos.asString().c_str() );
		return NULL;
	}
	region = (const_cast<CRegion*>(cregion));
	for (uint i=0; i!=region->getDeposits().size(); ++i )
	{
		if ( region->getDeposits()[i]->contains( pos ) )
			return region->getDeposits()[i];
	}
	return NULL;
}

//-----------------------------------------------
// CZoneManager getContinentFromId
//-----------------------------------------------
CPlace* CZoneManager::getPlaceFromAlias( TAIAlias alias )
{
	std::map< TAIAlias, CPlace* >::iterator it = _PlacesByAlias.find( alias );
	if ( it != _PlacesByAlias.end() )
	{
		return ( (*it).second );
	}
	return NULL;
}

//-----------------------------------------------
// CZoneManager getPlaceFromName
//-----------------------------------------------
CPlace* CZoneManager::getPlaceFromName( const std::string & name )
{
	for ( uint i = 0; i < _Places.size(); i++ )
	{
		if ( _Places[i] && _Places[i]->getName() == name )
		{
			return _Places[i];
		}
	}
	return NULL;
}// CZoneManager getPlaceFromName

//-----------------------------------------------
// CZoneManager getContinentFromId
//-----------------------------------------------
CContinent* CZoneManager::getContinentFromId( CONTINENT::TContinent id )
{
	if (id == CONTINENT::UNKNOWN )
		return NULL;
	for ( uint i = 0; i< _Continents.size(); i++ )
	{
		if ( _Continents[i].getId() == id )
			return &_Continents[i];
	}
	nlwarning("<CZoneManager getContinentFromId> continent id %u is out of bound",id);
	return NULL;
}// CZoneManager getContinentFromId

//-----------------------------------------------
// CZoneManager updateCharacterPosition
//-----------------------------------------------
void CZoneManager::updateCharacterPosition( CCharacter * user )
{
	nlassert(user);

	// if user is in an instance, do not update the places where he is
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;	
	if ( cell <= - 2 )
		return;
	if ( user->getState().X <= 0 || user->getState().Y >= 0 )
		return;

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::place);
	SM_STATIC_PARAMS_2(params2, STRING_MANAGER::place, STRING_MANAGER::faction);

	// get the current location of the character
	const CPlace * stable;
	std::vector<const CPlace*> places;
	const CRegion * region ;
	const CContinent * continent;
	float gooDistance;
	getPlace( user, gooDistance, &stable, places, &region, &continent);

	// SOURCE: user->getCurrentContinent()
	// DESTINATION: continent->getId()
	//	{	
		// update the continent if necessary
		if ( continent == NULL )
		{
			CPlace * oldPlace = getContinentFromId( user->getCurrentContinent() ); // returns NULL for CONTINENT::UNKNOWN
			if (oldPlace)
			{
				params[0].Identifier = oldPlace->getName();
				PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_CONTINENT",params);	
			}
			user->setCurrentContinent( CONTINENT::UNKNOWN );
		}
		else if ( (CONTINENT::TContinent)continent->getId() != user->getCurrentContinent() )
		{
			// if current continent is NEWBIELAND, send an 'newbie' update to the SU
			bool updateSU = false;
			if (user->getCurrentContinent() == CONTINENT::NEWBIELAND)
				updateSU = true;

			const CONTINENT::TContinent oldContinent = user->getCurrentContinent();
			CPlace * oldPlace = getContinentFromId( oldContinent ); // returns NULL for CONTINENT::UNKNOWN
			if (oldPlace)
			{
				params[0].Identifier = oldPlace->getName();
				PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_CONTINENT",params);	
			}
			params[0].Identifier = continent->getName();
			PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_CONTINENT",params);	
			user->setCurrentContinent( (CONTINENT::TContinent)continent->getId() );
			// notify player respawn points system that continent changed
			user->getRespawnPoints().cbContinentChanged(oldContinent);
			// update newbieland flag (defailt to 1 if there's aproblem determining the true value)
			bool newbie = user->isNewbie();
//			user->_PropertyDatabase.setProp("USER:IS_NEWBIE", newbie);
			CBankAccessor_PLR::getUSER().setIS_NEWBIE(user->_PropertyDatabase, newbie);

			bool trialPlayer = false;
			CPlayer *player = PlayerManager.getPlayer( PlayerManager.getPlayerId(user->getId()));
			if (!player)
			{
				nlwarning("Error %s was not found in player manager.",user->getId().toString().c_str());
				trialPlayer = player->isTrialPlayer();
			}
//			user->_PropertyDatabase.setProp("USER:IS_TRIAL", trialPlayer);
			CBankAccessor_PLR::getUSER().setIS_TRIAL(user->_PropertyDatabase, trialPlayer);

			if (updateSU)
			{
				if (IShardUnifierEvent::getInstance() != NULL)
					IShardUnifierEvent::getInstance()->onUpdateCharNewbieFlag(user->getId(), newbie);
			}
		}
		
		// do the same for region
		// first send message for leaving previous region
		CRegion * oldRegion = dynamic_cast<CRegion *> ( getPlaceFromId( user->getCurrentRegion() ) );
		if( user->getCurrentRegion() != 0xFFFF )
		{
			// previously in region
			if( region == 0 || ( region->getId() != user->getCurrentRegion()) )
			{
				// and not the same than actual region
				if (oldRegion)
				{
					const CTotemBase* pTotem = CPVPFactionRewardManager::getInstance().getTotemBaseFromId( user->getCurrentRegion() );
					if( pTotem )
					{
						PVP_CLAN::TPVPClan regionFaction = pTotem->getOwnerFaction();
						if( regionFaction == PVP_CLAN::Neutral )
						{
							params[0].Identifier = oldRegion->getName();
							PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_NEUTRAL_REGION",params);
						}
						else
						{
							params2[0].Identifier = oldRegion->getName();
							params2[1].Enum = PVP_CLAN::getFactionIndex(regionFaction);
							PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_FACTION_REGION",params2);
						}
					}
					else
					{
						params[0].Identifier = oldRegion->getName();
						PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_NEUTRAL_REGION",params);
					}

					oldRegion->removePlayer( user->getId() );

					// check if user leaves newbieland
					if ( region != 0 && oldRegion->isNewbieRegion() && !region->isNewbieRegion() )
					{
						user->getRespawnPoints().clearRespawnPoints();
					}
				}

				if( region == 0 )
				{
					user->setCurrentRegion( 0xFFFF );
					// remove spire effects for Pvp-flagged players
					if ( user->getPVPFlag() )
						CPVPFactionRewardManager::getInstance().removeTotemsEffects( user );
				}
			}
		}
		
		if ( region != 0 && (user->getCurrentRegion() != region->getId()) )
		{
			const CTotemBase* pTotem = CPVPFactionRewardManager::getInstance().getTotemBaseFromId( region->getId() );
			if( pTotem )
			{
				PVP_CLAN::TPVPClan regionFaction = pTotem->getOwnerFaction();
				if( regionFaction == PVP_CLAN::Neutral )
				{
					params[0].Identifier = region->getName();
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_NEUTRAL_REGION",params);
				}
				else
				{
					params2[0].Identifier = region->getName();
					params2[1].Enum = PVP_CLAN::getFactionIndex(regionFaction);
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_FACTION_REGION",params2);
				}
			}
			else
			{
				params[0].Identifier = region->getName();
				PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_NEUTRAL_REGION",params);
			}

			user->setCurrentRegion( region->getId() );
			((CRegion*)region)->addPlayer( user->getId() );
			
			// check if user leaves newbieland
			if ( oldRegion != 0 && oldRegion->isNewbieRegion() && !region->isNewbieRegion() )
			{
				user->getRespawnPoints().clearRespawnPoints();
			}

			// add new spire effects for Pvp-flagged players
			if ( user->getPVPFlag() )
				CPVPFactionRewardManager::getInstance().giveTotemsEffects( user );

			// Now the VisitPlace missions are checked in CMissionManager::checkVisitPlaceMissions()
			//CMissionEventVisitPlace event(region->getId() );
			//user->processMissionMultipleEvent(event);
		}
		
		// get new places
		const uint newPlacesSize = (uint)places.size();
		//bool sendWarning = false;
		bool changed = false;
		for ( uint i = 0; i < newPlacesSize; i++ )
		{
			if ( !user->isInPlace( places[i]->getId() ) )
			{
				if( places[i]->getReported() )
				{
					params[0].Identifier = places[i]->getName();
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_PLACE",params);
				}
				// Now the VisitPlace missions are checked in CMissionManager::checkVisitPlaceMissions()
				//CMissionEventVisitPlace event( places[i]->getId() );
				//user->processMissionMultipleEvent(event);
				
				// validate respawn points
				for ( uint j =  0; j< places[i]->getRespawnPoints().size(); j++ )
				{
					user->getRespawnPoints().addRespawnPoint( places[i]->getRespawnPoints()[j] );
				}

				changed = true;

				/// tells mission system that user enters a place
				for ( map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
				{
					CMissionManager::getInstance()->enterPlace( (*it).second,places[i]->getAlias(), places[i]->getId() );
				}
			}
		}

		// get left places
		const uint oldPlacesSize = (uint)user->getPlaces().size();
		for ( uint i = 0; i < oldPlacesSize; i++ )
		{
			uint j = 0;
			for (; j < newPlacesSize; j++ )
			{
				if ( places[j]->getId() == user->getPlaces()[i] )
					break;
			}
			if ( j == newPlacesSize )
			{
				CPlace * place = getPlaceFromId( user->getPlaces()[i] );
				if ( place )
				{
					if( place->getReported() )
					{
						params[0].Identifier = place->getName();
						PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_PLACE",params);
					}

					/// tells mission system that user enters a place
					for ( map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
					{
						CMissionManager::getInstance()->leavePlace( (*it).second,place->getAlias(), place->getId() );
					}
				}
				changed = true;
			}
			
		}

		// enter/leave PVP zones
		TAIAlias pvpZoneAlias = CPVPManager::getInstance()->getPVPZoneFromUserPosition( user );
		if ( pvpZoneAlias != user->getCurrentPVPZone() && !user->isDead() )
		{
			if ( user->getCurrentPVPZone() != CAIAliasTranslator::Invalid )
			{
				CPVPManager::getInstance()->leavePVPZone( user );
			}

			if ( pvpZoneAlias != CAIAliasTranslator::Invalid )
			{
				CPVPManager::getInstance()->enterPVPZone( user, pvpZoneAlias );
			}

			user->setCurrentPVPZone( pvpZoneAlias );
		}

		// enter/leave outpost zones
		TAIAlias outpostAlias = COutpostManager::getInstance().getOutpostFromUserPosition( user );
		if ( outpostAlias != user->getCurrentOutpostZone() && !user->isDead() )
		{
			if ( user->getCurrentOutpostZone() != CAIAliasTranslator::Invalid )
			{
				COutpostManager::getInstance().leaveOutpostZone( user );
			}
			
			user->setCurrentOutpostZone( outpostAlias );
			
			if ( outpostAlias != CAIAliasTranslator::Invalid )
			{
				COutpostManager::getInstance().enterOutpostZone( user );
			}
		}
		
		if ( changed )
			user->setPlaces( places );

		// and stable
		// if the player is now in an unknown stable
		if( stable == NULL )
		{
			// disable this message because stables are already treated as places
			/*CPlace * oldStable = getPlaceFromId( user->getCurrentStable() );
			if (oldStable)
			{
				if( oldStable->getReported() )
				{
					params[0].Identifier = oldStable->getName();
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_PLACE",params);
				}
			}*/

			user->setCurrentStable( 0xFFFF,0xFFFF);	
		}
		// check if player left old current stable for enter to another
		else if( user->getCurrentStable() != stable->getId() )
		{
			// disable this message because stables are already treated as places
			/*CPlace * oldStable = getPlaceFromId( user->getCurrentStable() );
			if (oldStable)
			{
				if( oldStable->getReported() )
				{
					params[0].Identifier = oldStable->getName();
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_LEAVE_PLACE",params);
				}
			}
			if( stable->getReported() )
			{
				params[0].Identifier = stable->getName();
				PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"EGS_ENTER_PLACE",params);
			}*/

			for ( uint i = 0; i < newPlacesSize; i++ )
			{
				if ( places[i]->isMainPlace() )
				{
					user->setCurrentStable( stable->getId(), places[i]->getId() );
					break;
				}
			}
			//  Now the VisitPlace missions are checked in CMissionManager::checkVisitPlaceMissions()
			//CMissionEventVisitPlace event(stable->getId() );
			//user->processMissionMultipleEvent(event);
		}

		// apply goo damage if needed
		user->applyGooDamage( gooDistance );
	//}
}// CZoneManager updateCharacterPosition

//-----------------------------------------------
// CZoneManager tickUpdate
//-----------------------------------------------
void CZoneManager::tickUpdate()
{
	// *** Update deposits at low frequency (each deposit is updated once per DepositUpdateFrequency (which is a period in game cycles, actually))
	// CPU is smoothed if there are more deposits than cycles in the period.
	uint32 nbDeposit = (uint32)_Deposits.size();
	if( ( (_NextDepositIndexUpdated != 0) || (CTickEventHandler::getGameCycle() - _SpreadUpdateLoopBeginTick ) >= DepositUpdateFrequency.get()) )
	{
		if ( _NextDepositIndexUpdated == 0 )
		{
			_SpreadUpdateLoopBeginTick = CTickEventHandler::getGameCycle();
		}
		uint32 i, nbDepositUpdatedByTick = (uint32 ) ( nbDeposit / DepositUpdateFrequency.get() ) + 1;
		for( i = _NextDepositIndexUpdated; i < _NextDepositIndexUpdated + nbDepositUpdatedByTick; ++i )
		{
			if( i < nbDeposit )
			{
				_Deposits[ i ]->lowFreqUpdate();
			}
		}
		
		if( i >= nbDeposit )
			_NextDepositIndexUpdated = 0;
		else
			_NextDepositIndexUpdated = i;
	}

	// *** Update the deposits that need an update for auto spawn
	std::set< CDeposit* >::iterator			itDeposit= _DepositNeedingAutoSpawnUpdate.begin();
	while(itDeposit!= _DepositNeedingAutoSpawnUpdate.end())
	{
		CDeposit	*deposit= *itDeposit;
		nlassert(deposit);
		deposit->autoSpawnUpdate();

		// remove the deposit from the set, and go next to update
		std::set< CDeposit* >::iterator		itNext= itDeposit;
		itNext++;
		_DepositNeedingAutoSpawnUpdate.erase(itDeposit);
		itDeposit= itNext;
	}

}// CZoneManager tickUpdate

//-----------------------------------------------
// CZoneManager dumpWorld
//-----------------------------------------------
void CZoneManager::dumpWorld(CLog & log)
{
	log.displayNL("%u continents", _Continents.size() );
	for (uint i = 0; i < _Continents.size(); i++ )
	{
		log.displayNL("CONTINENT %u : id=%u, name=%s", i, _Continents[i].getId(),_Continents[i].getName().c_str() );
		const std::vector< CRegion* > & regions =  _Continents[i].getRegions();
		for (uint j = 0; j < regions.size();j++ )
		{
			log.displayNL("      region %u : id=%u, name=%s",j, regions[j]->getId(),regions[j]->getName().c_str() );
			const std::vector< CPlace* > & places =  regions[j]->getPlaces();
			for (uint k = 0; k < places.size();k++ )
			{
				log.displayNL("            place %u : id=%u, name=%s", k, places[k]->getId(),places[k]->getName().c_str() );
			}
			log.displayNL("");          
			const std::vector< CDeposit* > & deposits =  regions[j]->getDeposits();
			log.displayNL("%u deposits", deposits.size() );
			//for (uint k = 0; k < deposits.size();k++ )
			//{
			//	log.displayNL("            deposits %u : id=%u", k, /*deposits[k]->getId()*/0, /*deposits[k]->getName().c_str()*/  );
			//}
		}

	}
}

//-----------------------------------------------
// CZoneManager dumpTpSpawnZones
//-----------------------------------------------
void CZoneManager::dumpTpSpawnZones(CLog & log)
{
	log.displayNL("%u tp spawn zones", _TpSpawnZones.size());
	for (uint i = 0; i < _TpSpawnZones.size(); i++)
	{
		log.displayNL("TP SPAWN ZONE %u : name=%s", i, _TpSpawnZones[i].getName().c_str());
	}
}

//-----------------------------------------------
// CZoneManager answerWhere
//-----------------------------------------------
void CZoneManager::answerWhere(const NLMISC::CEntityId & eId)
{
	///\todo nico manage building
	CCharacter * c = PlayerManager.getChar( eId );
	if (c )
	{
		c->setAfkState(false);
		CContinent * cont = getContinentFromId(c->getCurrentContinent());
		if ( !cont )
		{
			nlwarning("<CZoneManager answerWhere> invalid continent %u for entity %s",c->getCurrentContinent(),eId.toString().c_str());
			return;
		}
		CPlace * region = getPlaceFromId( c->getCurrentRegion() );
		if ( !region )
		{
			nlwarning("<CZoneManager answerWhere> invalid region %u for entity %s",c->getCurrentRegion(),eId.toString().c_str());
			return;
		}

		uint size = (uint)c->getPlaces().size();
		CPlace * place = NULL;
		for ( uint i = 0; i < size; i++ )
		{
			CPlace * placeTest = getPlaceFromId( c->getPlaces()[i] );
			nlassert( placeTest );
			if ( placeTest->isMainPlace() )
			{
				place = placeTest;
				break;
			}
		}
		
		STRING_MANAGER::TParam param;
		param.Type = STRING_MANAGER::place;
		if (place)
		{
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::place, STRING_MANAGER::place, STRING_MANAGER::place);
			params[0].Identifier = cont->getName();
			params[1].Identifier = region->getName();
			params[2].Identifier = place->getName();
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(eId),"EGS_ANSWER_WHERE",params);
		}
		else
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::place);
			params[0].Identifier = cont->getName();
			params[1].Identifier = region->getName();
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(eId),"EGS_ANSWER_WHERE_NO_PLACE",params);
		}

		/*
		std::vector< NLMISC::CEntityId > bots;
		for ( uint i = 0; i < c->getPickedMissions().size(); i++ )
		{
			if ( c->getPickedMissions()[i] )
			{
				CStaticMission * mission = c->getPickedMissions()[i]->getTemplate();
				NLMISC::CEntityId bot;
				if ( CAIAliasTranslator::getInstance()->getEntityId(c->getPickedMissions()[i]->getRewardGiver(), bot ) )
				{
					uint  j = 0;
					for (; j < bots.size(); j++ )
					{
						if ( bots[j] == bot  )
							break;
					}
					if ( j == bots.size() )
						bots.push_back( bot );
				}
				for (uint j  = 0; j < mission->getSteps().size(); j++ )
				{
					mission->getSteps()[j]->addImpliedBots( bots );
				}
			}
		}
		if ( !bots.empty() )
		{
			CCharacter::sendMessageToClient( eId,"EGS_MISSION_DIR_INTRO" );
			double xUser = (double)c->getState().X;
			double yUser = (double)c->getState().Y;
			for ( uint i = 0; i < bots.size(); i++ )
			{
				CCreature * botPtr = CreatureManager.getCreature( bots[i] );
				if ( botPtr )
				{
					string botName;
					if ( CAIAliasTranslator::getInstance()->getNPCNameFromAlias( botPtr->getAlias(), botName ) )
					{	
						double dx = (double)botPtr->getState().X/1000.0-(double)xUser/1000.0;
						double dy = (double)botPtr->getState().Y/1000.0-(double)yUser/1000.0;
						double dist = sqrt(dx*dx+dy*dy);
						uint32 distshort = (uint16)dist;
						
						double angle = atan2 (dy, dx) + NLMISC::Pi;
						sint direction =(sint) floor( 0.5 + 8.0 * angle /(NLMISC::Pi) );
						direction = direction %16;
						
						static string txts[]=
						{
								"W",
								"WSW",
								"SW",
								"SSW",
								"S",
								"SSE",
								"SE",
								"ESE",
								"E",
								"ENE",
								"NE",
								"NNE",
								"N",
								"NNW",
								"NW",
								"WNW",
						};
						static string msgBot = ("EGS_MISSION_DIR_BOT");
						
						CMessage msgout("STATIC_STRING");
						msgout.serial( const_cast<CEntityId&>(eId) );
						std::set<CEntityId> empty;
						msgout.serialCont( empty );		
						msgout.serial( msgBot );
						msgout.serial( botName );
						msgout.serial( distshort );
						msgout.serial( txts[direction] );
						sendMessageViaMirror( "IOS", msgout );
					}
				}
			}
		}
		*/
	}
	else
	{
		nlwarning( "<CZoneManager answerWhere>Invalid char %s",eId.toString().c_str() );
	}
}


//-----------------------------------------------
// CZoneManager harvestDeposit
//-----------------------------------------------
/*void CZoneManager::harvestDeposit(CCharacter * user)
{
	nlassert(user);
	vector<CDeposit*> deposits;
	nlerror( "getDepositsUnderUser(user,deposits);" );
	if ( deposits.size() )
	{
		uint nbMps = 0;
		for (uint i = 0; i < deposits.size(); i++ )
			nbMps+= deposits[i]->getContentSize();
		uint result = RandomGenerator.rand( nbMps - 1 );
		nbMps = 0;
		for (uint i = 0; i < deposits.size(); i++ )
		{
			if ( result <  nbMps + deposits[i]->getContentSize() )
			{
				nlerror( "deposits[i]->harvestInfo(user->getId(),user->getHarvestInfos());" );
				break;
			}
			nbMps += deposits[i]->getContentSize();
		}
	}
	if (user->getHarvestInfos().Sheet == CSheetId::Unknown)
	{
		// no raw material found, return
		return;
	}

	user->getHarvestInfos().EndCherchingTime = CTickEventHandler::getGameCycle() + DepositSearchTime;
	user->setCurrentAction( CLIENT_ACTION_TYPE::Harvest, user->getHarvestInfos().EndCherchingTime );

}// CZoneManager harvestDeposit
*/


//-----------------------------------------------
// CZoneManager displayAllDeposit
//-----------------------------------------------
void CZoneManager::dumpDeposits( NLMISC::CLog & log, const std::string& depName, bool extendedInfo )
{
	bool displayAll = (depName == "ALL");
	for( uint i = 0; i < _Deposits.size(); i++ )
	{
		if ( _Deposits[i] )
		{
			if ( displayAll || (_Deposits[i]->name() == depName) )
				_Deposits[i]->displayContent( &log, extendedInfo );
		}
		else
			log.displayNL( "===== Deposit %d is NULL" );
	}
}// CZoneManager displayAllDeposit


//-----------------------------------------------
// CZoneManager getStartPointVector
//-----------------------------------------------
vector<CZoneManager::CStartPoint> CZoneManager::getStartPointVector( uint16 startPointIdx ) const
{
	if ( startPointIdx >= _StartPoints.size() )
	{
		nlwarning("bad start point index %u ( count %u )",startPointIdx , _StartPoints.size() );
		return vector<CStartPoint>();
	}

	return _StartPoints[startPointIdx];
}// CZoneManager getStartPointVector


//-----------------------------------------------
// Get the ecotype zone under the position.
// If not found, a NULL pointer is returned.
//-----------------------------------------------
ECOSYSTEM::EECosystem CZoneManager::getEcotype( const NLMISC::CVector& pos )
{
	// The ecotypes must not be overlapped: only the first one found is returned
	for ( CEcotypeZones::iterator it=_EcotypeZones.begin(); it!=_EcotypeZones.end(); ++it )
	{
		CEcotypeZone *ecotypeZone = (*it);
		if ( ecotypeZone->contains( pos ) )
		{
			return ecotypeZone->ecotype();
		}
	}
	return ECOSYSTEM::unknown;
}


//-----------------------------------------------
// Clear ecotype information
//-----------------------------------------------
void CZoneManager::clearEcotypes()
{
	for ( CEcotypeZones::iterator iez=_EcotypeZones.begin(); iez!=_EcotypeZones.end(); ++iez )
	{
		delete (*iez);
	}
	_EcotypeZones.clear();
}


//-----------------------------------------------
// deposit auto spawn
//-----------------------------------------------
void	CZoneManager::registerDepositToAutoSpawnUpdate(CDeposit *deposit)
{
	if(deposit)
		_DepositNeedingAutoSpawnUpdate.insert(deposit);
}
void	CZoneManager::unregisterDepositToAutoSpawnUpdate(CDeposit *deposit)
{
	if(deposit)
		_DepositNeedingAutoSpawnUpdate.erase(deposit);
}



// dump the world organisation
NLMISC_COMMAND(dumpWorld," dump the world organisation","")
{
	if (args.size() == 0)
	{
		CZoneManager::getInstance().dumpWorld(log);
		return true;
	}
	return false;
}

NLMISC_COMMAND(dumpTpSpawnZones, "dump the tp spawn zones", "")
{
	if (args.size() == 0)
	{
		CZoneManager::getInstance().dumpTpSpawnZones(log);
		return true;
	}
	return false;
}
