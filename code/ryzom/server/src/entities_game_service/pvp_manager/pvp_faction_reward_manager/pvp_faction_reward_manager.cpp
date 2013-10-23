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
#include "pvp_faction_reward_manager.h"
#include "totem_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"
#include "primitives_parser.h"
#include "pvp_manager/pvp_manager_2.h"
#include "nel/misc/path.h"
#include "game_share/fame.h"
#include "server_share/used_continent.h"
#include "world_instances.h"
#include "phrase_manager/phrase_utilities_functions.h"
//#include "creature_manager/creature_manager.h"

using namespace std;
using namespace EFFECT_FAMILIES;
using namespace NLLIGO;
using namespace NLMISC;
using namespace NLNET;

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily

const uint32 NbSpireEffectsDatabaseEntry = 59;

sint32 CPVPFactionRewardManager::EffectValues[ EndTotemEffects+1 ];
CVariable<sint32> FactionPoolSizeToBuild( "egs", "NbTotemPointToBuild", "Number of points in the faction pool needed to build a totem", 10000, 0, true );
CVariable<NLMISC::TGameCycle> DelayBetweenAttackMessage( "egs", "DelayBetweenAttackMessage", "Number of tick between each message about spire attacked", 3000, 0, true );

NLMISC_SAFE_SINGLETON_IMPL(CPVPFactionRewardManager);

//----------------------------------------------------------------------------

CPVPFactionRewardManager::CPVPFactionRewardManager()
{
	for ( int i=0; i<PVP_CLAN::NbClans; i++)
	{
		_FactionsPossessions[i].NbTotems = 0;
		_FactionsPossessions[i].Level = 0;
		_FactionsPossessions[i].FactionPointsPool = 0;
	}		

	_InitDone = false;
	_DBLoaded = false;
	_LastSave = 0;
	_NbTotems = 0;
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::_BuildTotemBasesRec( const IPrimitive* prim,
													std::map<CTotemBase*, set<string> >& neighboursNames,
													std::map<std::string, CTotemBase*>& totemBasesPerName )
{
	if ( !prim )
		return;

	// look for spire nodes in the primitives
	string spireClass;
	if (prim->getPropertyByName( "class", spireClass ) && spireClass=="spire")
	{
		string name, effect;
		vector<string> const* neighbours = NULL;
		if ( prim->getPropertyByName( "name", name ) && !name.empty()
			&& prim->getPropertyByName( "effect", effect ) && !effect.empty()
			&& prim->getPropertyByName( "neighbours", neighbours ) && neighbours && !neighbours->empty() )
		{
			CTotemBase* pTotemBase = new CTotemBase(name);
			
			TEffectFamily family = toEffectFamily( effect );
			pTotemBase->setTotemEffectFamily( family );
			
			// link totem to its region
			CRegion* region = CZoneManager::getInstance().getRegion( *( prim->getPrimVector() ) );
		//	CZoneManager::getInstance().getRegion();
			
			if ( region )
			{
				_TotemBasesPerRegion[ region->getAlias() ] = pTotemBase;
				pTotemBase->setRegionAlias( region->getAlias() );
				++_NbTotems;
			}
			
			totemBasesPerName[ name ] = pTotemBase;
			
			// register neighbours
			vector<string>::const_iterator it, itEnd;
			for (it=neighbours->begin(), itEnd=neighbours->end(); it!=itEnd; ++it)
			{
				string const& neighbour = *it;
				if (!neighbour.empty())
					neighboursNames[ pTotemBase ].insert(neighbour);
			}
		}
	}
	
	//this is not a spire node, so lookup recursively in the children
	for ( uint i=0; i<prim->getNumChildren(); i++ )	
	{
		const IPrimitive *child;
		if ( prim->getChild( child, i ) )
			_BuildTotemBasesRec( child, neighboursNames, totemBasesPerName );
	}
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::init()
{
	// can be called only once
	if ( _InitDone )
		return;
	
	// init totem effects with default values
	for (uint32 i=0; i<=EndTotemEffects; i++)
	{
		EffectValues[i] = 5;
	}
	
	EffectValues[ TotemHarvestZRs ] = 10;
	EffectValues[ TotemCombatDS ] = 10;
	EffectValues[ TotemMiscMov ] = 20;

	// For each totem, store its neighbors' names
	map<CTotemBase*, set<string> >	neighboursNames;

	// Totem bases sorted by name (used to build the totem neighborhood graph)
	map<std::string, CTotemBase*> totemBasesPerName;

	// get the primitive list and parse each primitive
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;
	for ( first = primsList.begin(), last = primsList.end(); first != last; ++first )
	{
		//if ( first->FileName == "pvp_towers" )
	//	{
			// get the spire nodes
			for ( uint i = 0; i < first->Primitive.RootNode->getNumChildren(); ++i )	
			{
				const NLLIGO::IPrimitive* node = NULL;
				if ( first->Primitive.RootNode->getChild( node, i ) && node != NULL )
					_BuildTotemBasesRec( node, neighboursNames, totemBasesPerName );
			}

		//	break;
		//}		
	}

	// second pass : set neighbours
	map<CTotemBase*, set<string> >::iterator itTotem, itTotemEnd;
	for (itTotem=neighboursNames.begin(), itTotemEnd=neighboursNames.end(); itTotem!=itTotemEnd; ++itTotem)
	{
		CTotemBase* totem = itTotem->first;
		set<string>& neighbours = itTotem->second;
		
		set<string>::const_iterator itNeighbour, itNeighbourEnd;
		for (itNeighbour=neighbours.begin(), itNeighbourEnd=neighbours.end(); itNeighbour!=itNeighbourEnd; ++itNeighbour)
		{
			string const& name = *itNeighbour;
			if ( totemBasesPerName.find(name)!=totemBasesPerName.end() )
				totem->addNeighbour( totemBasesPerName[ name ]->getRegionAlias() );
			else
				nlinfo( "%s is not a valid region", name.c_str() );
		}
	}

	_LoadFromPDR();
	

	// update effects for everyone
	for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
	{
		if ((*it).second.Player != 0)
		{
			CCharacter *c = (*it).second.Player->getActiveCharacter();
			if(c)
				CPVPFactionRewardManager::getInstance().giveTotemsEffects( c );
		}
	}

	// create directory
	string sPath = Bsi.getLocalPath() + "totems";
	if (!CFile::isExists(sPath))
		CFile::createDirectory(sPath);

	for( map<TAIAlias, CTotemBase*>::iterator it = _TotemBasesPerRegion.begin(); it != _TotemBasesPerRegion.end(); ++it )
	{
		CTotemBase * totemBase = (*it).second;
		if(totemBase)
		{
			if( totemBase->isBuildingFinished() )
			{
				sendEventToAI( totemBase, string("construit") );
			}
		}
		else
		{
			nlstopex(("Pointer null in map of totem base found !!!!"));
		}
	}

	_InitDone = true;
	_DataUpdated = true;
}

//----------------------------------------------------------------------------

struct TTotemFileCallback : public IBackupFileReceiveCallback
{
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		CPVPFactionRewardManager::getInstance()._totemFileCallback(fileDescription, dataStream);
	}

};

void CPVPFactionRewardManager::_totemFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	static CPersistentDataRecordRyzomStore pdr;
	pdr.clear();
	pdr.fromBuffer(dataStream);
//	pdr.readFromFile( sFilePath.c_str() );

	apply( pdr );
}

bool CPVPFactionRewardManager::_LoadFromPDR()
{
	H_AUTO(CPVPFactionRewardManager_load);

	if ( _DBLoaded )
		return true;


	TTotemFileCallback *cb = new TTotemFileCallback;
	Bsi.syncLoadFile("totems/totems_pdr.bin", cb);

//	string sFilePath = Bsi.getLocalPath();
//	sFilePath += toString( "totems/totems_pdr.bin" );
//	
//	if ( !CFile::fileExists( sFilePath ) )
//		// file does not exist, there is nothing to load
//		return false;
//
//	static CPersistentDataRecordRyzomStore pdr;
//	pdr.clear();
//	pdr.readFromFile( sFilePath.c_str() );
//
//	if ( pdr.isEndOfData() )
//	{
//		// no data ?
//		return false;
//	}
//
//	apply( pdr );
	
	map<uint32, CTotemBase*>::iterator it = _TotemBasesPerRegion.begin();
	while ( it != _TotemBasesPerRegion.end() )
	{
		CTotemBase* pTotemBase = (*it).second;
		pTotemBase->loadFromPDR();
		it++;
	}
	
	_DBLoaded = true;

	return true;
}

//----------------------------------------------------------------------------

CTotemBase* CPVPFactionRewardManager::_GetTotemBaseFromId( uint16 regionId )
{
	CRegion* region = static_cast<CRegion*>( CZoneManager::getInstance().getPlaceFromId( regionId ) );
	
	if ( region )
		return _TotemBasesPerRegion[ region->getAlias() ];
	else
		return NULL;
}

//----------------------------------------------------------------------------

CTotemBase* CPVPFactionRewardManager::_GetTotemBase( TAIAlias regionAlias )
{
	return _TotemBasesPerRegion[ regionAlias ];
}

//----------------------------------------------------------------------------

sint32 CPVPFactionRewardManager::_GetLevelBonus( EFFECT_FAMILIES::TEffectFamily, uint8 level )
{
	static sint32 baseValuesStat[] = { 0, 2, 5, 10, 15 };
	static sint32 baseValuesPercent[] = { 0, 2, 5, 8, 10 };

	bool statEffect = false;

	if ( statEffect )
		return baseValuesStat[ level ];
	else
		return baseValuesPercent[ level ];
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::_UpdateLevel( PVP_CLAN::TPVPClan faction )
{
	int total = _FactionsPossessions[faction].NbTotems;
	if ( total >= LEVEL_4 )
		_FactionsPossessions[faction].Level = 4;
	else if ( total >= LEVEL_3 )
		_FactionsPossessions[faction].Level = 3;
	else if ( total >= LEVEL_2 )
		_FactionsPossessions[faction].Level = 2;
	else if ( total >= LEVEL_1 )
		_FactionsPossessions[faction].Level = 1;
	else
		_FactionsPossessions[faction].Level = 0;

	// update effects for everyone
	for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
	{
		if ( (*it).second.Player != 0 )
		{
			CCharacter *c = (*it).second.Player->getActiveCharacter();
			if ( c )
			{
				giveTotemsEffects( c );
			}
		}
	}

	_DataUpdated = true;
}

//----------------------------------------------------------------------------

bool CPVPFactionRewardManager::startTotemBuilding( uint16 regionIndex, CCharacter* builder )
{
	CTotemBase* pTotemBase = _GetTotemBaseFromId( regionIndex );
	
	if ( !pTotemBase )
	{
		nlwarning("Totem build aborted (2): no totem in that region");
		return false;
	}
	if ( ( pTotemBase->getOwnerFaction() != PVP_CLAN::Neutral ) )
	{
		nlwarning("Totem build aborted (2): totem is not neutral");
		return false;
	}
	
	pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = builder->getAllegiance();
	if ( allegiance.first == PVP_CLAN::Neutral )
	{
		nlwarning("Totem build aborted (2): builder cult allegiance is neutral");
		return false;
	}
	
	TEffectFamily family = pTotemBase->getTotemEffectFamily();
	
	// harvest totems use random effects
	if (   ( family == TotemHarvestAgg )
		|| ( family == TotemHarvestQty )
		|| ( family == TotemHarvestZRs )
	   )
	{
		family = (TEffectFamily)( TotemHarvestAgg + RandomGenerator.rand( 2 ) );
		pTotemBase->setTotemEffectFamily( family );
	}
		
	pTotemBase->setOwnerFaction( allegiance.first );
	
	_FactionsPossessions[ allegiance.first ].NbTotems++;
	
	// update HP for all totems & faction points
	// &send message to clients
	updateFactionPointPool( allegiance.first, -FactionPoolSizeToBuild );
	
	SM_STATIC_PARAMS_1(params1, STRING_MANAGER::integer);
	params1[0].Int = FactionPoolSizeToBuild;
	PHRASE_UTILITIES::sendDynamicSystemMessage(builder->getEntityRowId(),"PVP_SPIRE_SPEND_FACTION_POINT",params1);
	
	// start building the totem
	pTotemBase->startBuilding(builder);
	
	// send message to clients
	SM_STATIC_PARAMS_4(params, STRING_MANAGER::place, STRING_MANAGER::faction, STRING_MANAGER::integer, STRING_MANAGER::integer);
	const CRegion * region = dynamic_cast<CRegion *>(CZoneManager::getInstance().getPlaceFromId( regionIndex ));
	if(region)
	{
		params[0].Identifier = region->getName();
		params[1].Enum = PVP_CLAN::getFactionIndex(allegiance.first);
		params[2].Int = TotemBuildTime / 600;
		params[3].Int = (TotemBuildTime - params[2].Int * 600);
		PHRASE_UTILITIES::sendDynamicSystemMessage(builder->getEntityRowId(),"PVP_SPIRE_START_BUILDING",params);
		STRING_MANAGER::sendSystemStringToClientAudience(builder->getEntityRowId(),std::vector<NLMISC::CEntityId>(),CChatGroup::arround,"PVP_SPIRE_START_BUILDING",params);
	}
	_DataUpdated = true;
	return true;
}

//----------------------------------------------------------------------------

bool CPVPFactionRewardManager::destroyTotem( uint16 regionIndex, TDataSetRow killerRowId )
{
	CTotemBase* pTotemBase = _GetTotemBaseFromId( regionIndex );

	if ( !pTotemBase || ( pTotemBase->getOwnerFaction() == PVP_CLAN::Neutral ) )
		return false;

	PVP_CLAN::TPVPClan oldFaction = pTotemBase->getOwnerFaction();

	// send message to clients
	SM_STATIC_PARAMS_3(params, STRING_MANAGER::place, STRING_MANAGER::faction, STRING_MANAGER::integer);
	const CRegion * region = dynamic_cast<CRegion *>(CZoneManager::getInstance().getPlaceFromAlias(pTotemBase->getRegionAlias()));
	if(region )
	{
		params[0].Identifier = region->getName();
		params[1].Enum = PVP_CLAN::getFactionIndex(oldFaction);
		params[2].Int = 0;
		PHRASE_UTILITIES::sendDynamicSystemMessage(killerRowId,"PVP_SPIRE_DESTROYED",params);
		STRING_MANAGER::sendSystemStringToClientAudience(killerRowId,std::vector<NLMISC::CEntityId>(),CChatGroup::region,"PVP_SPIRE_DESTROYED",params);

		// Send message to faction channel
		params[2].Int = 1;
		// TODO TChanID channelId = CPVPManager2::getInstance()->getFactionDynChannel(pTotemBase->getOwnerFaction());
//		if( channelId != DYN_CHAT_INVALID_CHAN )
//		{
			// TODO Send message PVP_SPIRE_DESTROYED to faction channel and enemies factions
//		}
	}

	

	pTotemBase->destroyTotem();
	sendEventToAI( pTotemBase, string("detruit") );
	_FactionsPossessions[oldFaction].NbTotems--;
	_UpdateLevel( oldFaction );
	
	// update HP for all totems
	updateFactionPointPool( oldFaction, 0 );

	_DataUpdated = true;

	return true;
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::_GetTotemsEffectsRec( CCharacter* user, CTotemBase* pTotem, 
												    vector<CSEffect*>& outEffects,
												    vector<CTotemBase*>& processed )
{
	// user and pTotem has already been checked by calling methods
	
	processed.push_back( pTotem );
	
	if ( pTotem->getOwnerFaction() == PVP_CLAN::Neutral )
		return;

	pTotem->getTotemEffect( user, outEffects );
	
	// loop through all neighbour
	for ( uint32 i=0; i<pTotem->getNumNeighbours(); i++ )
	{
		CTotemBase* pNeighbour = _TotemBasesPerRegion[ pTotem->getNeighbour( i ) ];

		if ( pNeighbour && ( pNeighbour->getOwnerFaction() == pTotem->getOwnerFaction() ) )
		{
			// look if the neighbour has already been processed
			bool alreadyProcessed = false;
			uint32 j=0; 
			while ( !alreadyProcessed && ( j<processed.size() ) )
			{
				alreadyProcessed = ( processed[j] == pNeighbour );
				j++;
			}
			
			if ( !alreadyProcessed )
				_GetTotemsEffectsRec( user, pNeighbour, outEffects, processed );
		}
	}
}

//----------------------------------------------------------------------------

vector<CSEffect*> CPVPFactionRewardManager::getTotemsEffects( CCharacter* user, vector<CTotemBase*>& processed )
{
	vector<CSEffect*> effects;
	CTotemBase* pTotem = _GetTotemBaseFromId( user->getCurrentRegion() );
	uint8 level = 0;

	// get totems effect values
	if ( user && pTotem )
	{
		_GetTotemsEffectsRec( user, pTotem, effects, processed );
		level = _FactionsPossessions[ pTotem->getOwnerFaction() ].Level;

		// clamp values ans add level bonuses
		for ( uint32 i=0; i<effects.size(); i++)
		{
			sint32 effectValue = effects[i]->getParamValue();
			
			if ( effectValue > 0 )
				effectValue += _GetLevelBonus( effects[i]->getFamily(), level );
			else
				effectValue -= _GetLevelBonus( effects[i]->getFamily(), level );	

			effects[i]->setParamValue( effectValue );
		}
	}	

	return effects;
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::giveTotemsEffects( CCharacter* user )
{
	vector<CSEffect*> effects;
	vector<CTotemBase*> processed;
	CTotemBase* pTotem = _GetTotemBaseFromId( user->getCurrentRegion() );
		
	if ( !user || !pTotem )
		return;
	
	_removeTotemsEffects( user );

	effects = getTotemsEffects( user, processed );

	for ( uint i=0; i<effects.size(); i++ )
	{
		user->addSabrinaEffect( effects[i] );
	}

	uint i;
	for( i = 0; i<processed.size(); ++i )
	{
		CBankAccessor_PLR::TPVP_EFFECTS::TArray &pvpElem = CBankAccessor_PLR::getPVP_EFFECTS().getArray(i);
		vector<CSEffect*> outEffects;
		processed[i]->getTotemEffect( user, outEffects );
		if( outEffects.size() > 0 )
		{
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ID", i), processed[i]->getRegionAlias() );
			pvpElem.setID(user->_PropertyDatabase, processed[i]->getRegionAlias() );
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ISBONUS", i), outEffects[0]->getParamValue() >= 0 );
			pvpElem.setISBONUS(user->_PropertyDatabase, outEffects[0]->getParamValue() >= 0 );
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:PARAM", i), abs(outEffects[0]->getParamValue()) );
			pvpElem.setPARAM(user->_PropertyDatabase, abs(outEffects[0]->getParamValue()) );
		}
		else
		{
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ID", i), 0 );
			pvpElem.setID(user->_PropertyDatabase, 0 );
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ISBONUS", i), 0 );
			pvpElem.setISBONUS(user->_PropertyDatabase, 0 );
//			user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:PARAM", i), 0 );
			pvpElem.setPARAM(user->_PropertyDatabase, 0 );
		}
	}

	for( uint j = i; j < NbSpireEffectsDatabaseEntry; ++j )
	{
		CBankAccessor_PLR::TPVP_EFFECTS::TArray &pvpElem = CBankAccessor_PLR::getPVP_EFFECTS().getArray(j);
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ID", j), 0 );
		pvpElem.setID(user->_PropertyDatabase, 0 );
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ISBONUS", j), 0 );
		pvpElem.setISBONUS(user->_PropertyDatabase, 0 );
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:PARAM", j), 0 );
		pvpElem.setPARAM(user->_PropertyDatabase, 0 );
	}
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::_removeTotemsEffects( CCharacter* user )
{
	if ( !user )
		return;
	
	vector<CSEffectPtr> effects = user->getSEffects();

	for ( uint i=0; i<effects.size(); i++ )
	{
		CSEffectPtr pEffect = effects[i];

		if ( ( pEffect->getFamily() >= EFFECT_FAMILIES::BeginTotemEffects ) 
			   && ( pEffect->getFamily() <= EFFECT_FAMILIES::EndTotemEffects ) )
		{
			user->removeSabrinaEffect( pEffect );
		}
	}
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::removeTotemsEffects( CCharacter* user )
{
	_removeTotemsEffects( user );

	for( uint j = 0; j < NbSpireEffectsDatabaseEntry; ++j )
	{
		CBankAccessor_PLR::TPVP_EFFECTS::TArray &pvpElem = CBankAccessor_PLR::getPVP_EFFECTS().getArray(j);
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ID", j), 0 );
		pvpElem.setID(user->_PropertyDatabase, 0 );
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:ISBONUS", j), 0 );
		pvpElem.setISBONUS(user->_PropertyDatabase, 0 );
//		user->_PropertyDatabase.setProp( toString("PVP_EFFECTS:%u:PARAM", j), 0 );
		pvpElem.setPARAM(user->_PropertyDatabase, 0 );
	}
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::tickUpdate()
{
	H_AUTO( COutpostManagertickUpdate );
	
	// update every totem each tick
	map<uint32, CTotemBase*>::iterator it = _TotemBasesPerRegion.begin();
	while ( it != _TotemBasesPerRegion.end() )
	{
		CTotemBase* pTotem = (*it).second;

		if ( pTotem && ( pTotem->tickUpdate() ) )
		{
			if ( pTotem->isBuildingFinished() )
			{
				// the totem has finished building during last tick
				_UpdateLevel( pTotem->getOwnerFaction() );

				// update all totem HP
				updateFactionPointPool( pTotem->getOwnerFaction(), 0 );

				// spawn totem npc (guards)
				sendEventToAI( pTotem, string("construit") );

				SM_STATIC_PARAMS_3(params, STRING_MANAGER::place, STRING_MANAGER::faction, STRING_MANAGER::integer);
				const CRegion * region = dynamic_cast<CRegion *>(CZoneManager::getInstance().getPlaceFromAlias( (*it).first ));
				if(region)
				{
					params[0].Identifier = region->getName();
					params[1].Enum = pTotem->getOwnerFaction();
					params[2].Int = 0;
					PHRASE_UTILITIES::sendDynamicSystemMessage(pTotem->getBuilder(),"PVP_SPIRE_BUILD_FINISHED",params);
					STRING_MANAGER::sendSystemStringToClientAudience(pTotem->getBuilder(),std::vector<NLMISC::CEntityId>(),CChatGroup::region,"PVP_SPIRE_BUILD_FINISHED",params);

					// Send it in faction channel too
					params[2].Int = 1;
					//TODO TChanID channelId = CPVPManager2::getInstance()->getFactionDynChannel(pTotem->getOwnerFaction());
					//if( channelId != DYN_CHAT_INVALID_CHAN )
					//{
						// TODO Send message PVP_SPIRE_BUILD_FINISHED to faction channel
					//}
				}
			}
		}
		++it;
	}
	
	// save if needed
	
	NLMISC::TGameCycle currentCycle = CTickEventHandler::getGameCycle();

	if ( currentCycle - _LastSave > 100 ) // save updates every 10 seconds
	{
		if ( _DataUpdated && _InitDone )
		{
			string sFilePath = toString( "totems/totems_pdr.bin" );
			
			static CPersistentDataRecordRyzomStore pdr;
			pdr.clear();

			store( pdr );

			CBackupMsgSaveFile msg( sFilePath, CBackupMsgSaveFile::SaveFile, Bsi );
			
			uint32 bufSize= pdr.totalDataSize();
			vector<char> buffer;
			buffer.resize(bufSize);
			pdr.toBuffer(&buffer[0],bufSize);
			msg.DataMsg.serialBuffer((uint8*)&buffer[0], bufSize);

			Bsi.sendFile( msg );

			_LastSave = currentCycle;
			_DataUpdated = false;
		}
	}
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::updateFactionPointPool( PVP_CLAN::TPVPClan faction, sint32 fpDelta )
{
	if( faction >= PVP_CLAN::NbClans )
		return;

	_FactionsPossessions[faction].FactionPointsPool += fpDelta;

	if ( _FactionsPossessions[faction].NbTotems > 0 )
	{
		sint32 newTotemHp = _FactionsPossessions[faction].FactionPointsPool / _FactionsPossessions[faction].NbTotems;

		map<uint32, CTotemBase*>::iterator it = _TotemBasesPerRegion.begin();
		while ( it != _TotemBasesPerRegion.end() )
		{
			CTotemBase* pTotem = (*it).second;
			if ( pTotem && ( pTotem->getOwnerFaction() == faction ) && ( pTotem->isBuildingFinished() == true ) )
			{
				if ( pTotem->getBotObject() )
					pTotem->getBotObject()->getScores()._PhysicalScores[SCORES::hit_points].Max = newTotemHp;
			}
			it++;
		}
	}
	_DataUpdated = true;
}

//----------------------------------------------------------------------------

sint32 CPVPFactionRewardManager::getFactionPointPool( PVP_CLAN::TPVPClan faction )
{
	if( faction >= 0 && faction < PVP_CLAN::NbClans )
		return _FactionsPossessions[faction].FactionPointsPool;
	else
		return 0;
}


//----------------------------------------------------------------------------

bool CPVPFactionRewardManager::isAttackable( CCharacter* actor, CEntityBase* target )
{
	CCreature * totemBot = dynamic_cast<CCreature *>(target);
			
	if ( !totemBot || !actor || !actor->getPVPFlag() )
		return false;

	// get totem owner
	CVector vec( (float)totemBot->getX() / 1000.0f, (float)totemBot->getY() / 1000.0f, 0.0f );
	
	CRegion* region = CZoneManager::getInstance().getRegion( vec );

	if ( region )
	{
		CTotemBase* pTotemBase = _TotemBasesPerRegion[ region->getAlias() ];

		if ( pTotemBase )
		{
			// the totem can only be attacked by enemy factions
			pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = actor->getAllegiance();

			if( CPVPManager2::getInstance()->factionWarOccurs( allegiance.first, pTotemBase->getOwnerFaction() ) || 
				CPVPManager2::getInstance()->factionWarOccurs( allegiance.second, pTotemBase->getOwnerFaction() ) )
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool CPVPFactionRewardManager::canBuildTotem( CCharacter* actor )
{
	pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = actor->getAllegiance();
	
	// neutral player ?
	if ( allegiance.first == PVP_CLAN::Neutral || allegiance.first == PVP_CLAN::Unknown )
	{
		nlwarning("Can't build totem: character has no cult allegiance");
		return false;
	}

	// faction pool big enough ?
	if ( _FactionsPossessions[ allegiance.first ].FactionPointsPool < FactionPoolSizeToBuild )
	{
		nlwarning("Can't build totem: character cult has not enough faction points");
		return false;
	}
	
	// invalid region ?
	NLMISC::CVector pos( (float)actor->getX() / 1000.0f,  (float)actor->getY() / 1000.0f, 0 );
	CRegion* region = CZoneManager::getInstance().getRegion( pos );

	if ( !region )
	{
		nlwarning("Can't build totem: no region where character is");
		return false;
	}
	
	CTotemBase* pTotem = _GetTotemBaseFromId( region->getId() );
	if ( !pTotem )
	{
		nlwarning("Can't build totem: no totem in that region");
		return false;
	}

	return pTotem->canStartBuilding( actor );
}

//----------------------------------------------------------------------------

void CPVPFactionRewardManager::addBotObject( CCreature* botObject )
{
	CVector vec( botObject->getX() / 1000.0f, botObject->getY() / 1000.0f, 0 );
	CRegion* region = CZoneManager::getInstance().getRegion( vec );

	if ( region )
	{
		CTotemBase* pTotemBase = _TotemBasesPerRegion[ region->getAlias() ];

		if ( pTotemBase )
		{
			pTotemBase->setBotObject( botObject );
			if( pTotemBase->isBuildingFinished() && pTotemBase->getOwnerFaction()!=PVP_CLAN::Neutral )
			{
				sendEventToAI( pTotemBase, string("construit") );
			}
			// nlinfo( "the totem bot object has been linked to region %s", region->getName().c_str() );
		}
	}
}

//----------------------------------------------------------------------------

bool CPVPFactionRewardManager::isATotem( CCreature* botObject )
{
	bool found = false;
	
	map<uint32, CTotemBase*>::iterator it = _TotemBasesPerRegion.begin();
	while ( ( it != _TotemBasesPerRegion.end() ) && !found )
	{
		CTotemBase* pTotem = (*it).second;
		if ( pTotem )
			found = pTotem->getBotObject() == botObject;
		it++;
	}

	return found;
}

//----------------------------------------------------------------------------

PVP_CLAN::TPVPClan CPVPFactionRewardManager::getRegionOwner( uint16 regionId ) 
{ 
	CTotemBase* pTotemBase = _GetTotemBaseFromId( regionId );

	if ( pTotemBase )
		return pTotemBase->getOwnerFaction();
	else
		return PVP_CLAN::Unknown;
}

//----------------------------------------------------------------------------
void CPVPFactionRewardManager::sendEventToAI( const CTotemBase * pTotem, const string& event )
{
	CPlace * place = CZoneManager::getInstance().getPlaceFromAlias( pTotem->getRegionAlias() );
	nlassert( place );
	CRegion * region = dynamic_cast<CRegion *>(place);
	nlassert( region);
	CONTINENT::TContinent continent = region->getContinent();
	string groupSpireName = "spire_group_" + pTotem->getName();
	vector< TAIAlias > aliases;
	CAIAliasTranslator::getInstance()->getGroupAliasesFromName(groupSpireName,aliases);
	for (uint i = 0; i < aliases.size(); ++i )
	{
		CUserEventMsg eventMsg;
		eventMsg.InstanceNumber = CUsedContinent::instance().getInstanceForContinent( continent );
		eventMsg.GrpAlias = aliases[i];
		eventMsg.EventId = 1;
		eventMsg.Params.push_back( strlwr( PVP_CLAN::toString(pTotem->getOwnerFaction()) ));
		eventMsg.Params.push_back(event);
		CWorldInstances::instance().msgToAIInstance(eventMsg.InstanceNumber, eventMsg);
	}
}


//----------------------------------------------------------------------------
void CPVPFactionRewardManager::spireAttacked( CCharacter * actor, CCreature * spire )
{
	CTotemBase * pTotemBase = _GetTotemBaseFromId( actor->getCurrentRegion() );
	if( pTotemBase )
	{
		nlassert( pTotemBase->getBotObject() == spire );

		if( pTotemBase->getLastTickAttackMessageSended() > CTickEventHandler::getGameCycle() + DelayBetweenAttackMessage )
		{
			pTotemBase->setLastTickAttackMessageSended(CTickEventHandler::getGameCycle());

			SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::faction);
			const CRegion * region = dynamic_cast<CRegion *>(CZoneManager::getInstance().getPlaceFromId( actor->getCurrentRegion() ));
			if(region)
			{
				params[0].Identifier = region->getName();
				params[1].Enum = pTotemBase->getOwnerFaction();
				PHRASE_UTILITIES::sendDynamicSystemMessage(pTotemBase->getBuilder(),"PVP_SPIRE_BUILD_FINISHED",params);
				STRING_MANAGER::sendSystemStringToClientAudience(actor->getEntityRowId(),std::vector<NLMISC::CEntityId>(),CChatGroup::region,"PVP_SPIRE_ATTACKED",params);
			}
		}

	}
}

//-----------------------------------------------
// Build a spire
//-----------------------------------------------
NLMISC_COMMAND(buildSpire, "build a spire","<Eid> <region>")
{
	if( args.size() != 2 )
		return false;
	else
	{
		CEntityId id;
		id.fromString( args[0].c_str() );
		
		CCharacter *c = PlayerManager.getChar(id);
		if (!c || !c->getEnterFlag())
		{
			log.displayNL("Invalid character %s", id.toString().c_str());
			return true;
		}

		float x;
		float y;
		
		if ( args[1] == "void" )
		{
			x = 11018.0f;
			y = -2330.0f;
		}
		else if ( args[1] == "haven" )
		{
			x = 9041.0f;
			y = -2473.0f;
		}
		else if ( args[1] == "maiden" )
		{
			x = 11000.0f;
			y = -3422.0f;
		}
		else if ( args[1] == "knot" )
		{
			x = 7632.0f;
			y = -2120.0f;
		}
		else if ( args[1] == "grove" )
		{
			x = 11105.0f;
			y = -4400.0f;
		}
			

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			c->setCurrentRegion( region->getId() );
			if ( CPVPFactionRewardManager::getInstance().canBuildTotem( c ) )
			{
				if( CPVPFactionRewardManager::getInstance().startTotemBuilding( region->getId(), c ) )
					log.displayNL( "buildSpire : You built a spire in %s", region->getName().c_str() );
				else
					log.displayNL( "buildSpire : built a spire in %s failed", region->getName().c_str() );
			}
			else
				log.displayNL( "buildSpire : You couldn't build the spire in region %s", region->getName().c_str() );
			return true;
		}
		else
			log.displayNL( "buildSpire : Can't found a region at coordinates %f %f", x, y );
	}
	return false;
}


//-----------------------------------------------
// destroy a spire
//-----------------------------------------------
NLMISC_COMMAND(destroySpire, "destroy a spire","<region>")
{
	if( args.size() != 1 )
		return false;
	else
	{
		float x;
		float y;
		
		if ( args[0] == "void" )
		{
			x = 11018.0f;
			y = -2330.0f;
		}
		else if ( args[0] == "haven" )
		{
			x = 9041.0f;
			y = -2473.0f;
		}
		else if ( args[0] == "maiden" )
		{
			x = 11000.0f;
			y = -3422.0f;
		}
		else if ( args[0] == "knot" )
		{
			x = 7632.0f;
			y = -2120.0f;
		}
		else if ( args[0] == "grove" )
		{
			x = 11105.0f;
			y = -4400.0f;
		}
			
		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			CPVPFactionRewardManager::getInstance().destroyTotem( region->getId(), TDataSetRow() );
			log.displayNL( "destroySpire : You destroyed a spire in %s", region->getName().c_str() );
			return true;
		}
		else
			log.displayNL( "getEffects : Can't found a region at coordinates %f %f", x, y );
	}
	return false;
}

//-----------------------------------------------
// get effects on a region
//-----------------------------------------------
NLMISC_COMMAND(getEffects, "get effects of a spire","<x> <y>" )
{
	if( args.size() != 2 )
		return false;
	else
	{
		float x = (float)atof(args[0].c_str());
		float y = (float)atof(args[1].c_str());

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			const CTotemBase* totem = CPVPFactionRewardManager::getInstance().getTotemBaseFromId( region->getId() );

			if( totem != 0 )
			{
				log.displayNL( "getEffects : Spire on region %s have effect %s", region->getName().c_str(), EFFECT_FAMILIES::toString( totem->getTotemEffectFamily() ).c_str() );	
			}
			else
				log.displayNL( "getEffects : Spire not found on region %s ", region->getName().c_str() );	
		}
		else
			log.displayNL( "getEffects : Can't found a region at coordinates %f %f", x, y );
	}
	return false;
}

//-----------------------------------------------
// Build a spire
//-----------------------------------------------
NLMISC_COMMAND(buildSpirePos, "build a spire","<Eid> <x> <y>")
{
	if( args.size() != 3 )
		return false;
	else
	{
		CEntityId id;
		id.fromString( args[0].c_str() );
		
		CCharacter *c = PlayerManager.getChar(id);
		if (!c || !c->getEnterFlag())
		{
			log.displayNL("Invalid character %s", id.toString().c_str());
			return true;
		}
		
		float x = (float)atof(args[1].c_str());
		float y = (float)atof(args[2].c_str());
			
		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			c->setCurrentRegion( region->getId() );
			if ( CPVPFactionRewardManager::getInstance().canBuildTotem( c ) )
			{
				if( CPVPFactionRewardManager::getInstance().startTotemBuilding( region->getId(), c ) )
					log.displayNL( "buildSpire : You built a spire in %s", region->getName().c_str() );
				else 
					log.displayNL( "buildSpire : built a spire in %s failed", region->getName().c_str() );
			}
			else
				log.displayNL( "buildSpire : You can't built a spire in %s", region->getName().c_str() );
			return true;
		}
		else
			log.displayNL( "getEffects : Can't found a region at coordinates %f %f", x, y );
	}
	return false;
}


//-----------------------------------------------
// destroy a spire
//-----------------------------------------------
NLMISC_COMMAND(destroySpirePos, "destroy a spire","<x> <y>")
{
	if( args.size() != 2 )
		return false;
	else
	{
		float x = (float)atof(args[0].c_str());
		float y = (float)atof(args[1].c_str());			

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );
		if ( region )
		{
			if( CPVPFactionRewardManager::getInstance().destroyTotem( region->getId(), TDataSetRow() ) )
				log.displayNL( "destroySpire : You destroyed a spire in %s", region->getName().c_str() );
			else
				log.displayNL( "destroySpire : Spire not found on region %s", region->getName().c_str() );
			return true;
		}
		else
			log.displayNL( "destroySpire : Can't build a spire here" );
	}
	return false;
}


//-----------------------------------------------
// get effects on a region
//-----------------------------------------------
NLMISC_COMMAND(getEffectsPos, "get effects of a spire","<Eid> <x> <y>" )
{
	if( args.size() != 3 )
		return false;
	else
	{
		CEntityId id;
		id.fromString( args[0].c_str() );
		
		CCharacter *c = PlayerManager.getChar(id);
		if (!c || !c->getEnterFlag())
		{
			log.displayNL("Invalid character %s", id.toString().c_str());
			return true;
		}

		float x = (float)atof(args[1].c_str());
		float y = (float)atof(args[2].c_str());	

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			c->setCurrentRegion( region->getId() );
			vector<CTotemBase*> processed;
			vector<CSEffect*> effects = CPVPFactionRewardManager::getInstance().getTotemsEffects( c, processed );
			for ( uint8 i=0; i<effects.size(); i++ )
			{
				CSEffect* pEffect = effects[i];
				log.displayNL( "%s %d", EFFECT_FAMILIES::toString( pEffect->getFamily() ).c_str(), pEffect->getParamValue() );
			}
			return true;
		}
		else
			log.displayNL( "getEffects : Can't found a region at coordinates %f %f", x, y );
	}
	
	return false;
}

//-----------------------------------------------
// Change Spire Effects value
//-----------------------------------------------
NLMISC_COMMAND(setSpireEffectValue, "change base value of a spire effect","<effectName> <value>" )
{
	if( args.size() != 2 )
		return false;
	else
	{
		EFFECT_FAMILIES::TEffectFamily family = EFFECT_FAMILIES::toEffectFamily( args[0] );

		if ( ( family < EFFECT_FAMILIES::BeginTotemEffects ) || ( family > EFFECT_FAMILIES::EndTotemEffects ) )
		{
			log.displayNL( "Not a valid effect family." );
			return false;
		}

		NLMISC::fromString(args[1], CPVPFactionRewardManager::EffectValues[ family ]);
	}

	// update effects for everyone
	for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
	{
		if ((*it).second.Player != 0)
		{
			CCharacter *c = (*it).second.Player->getActiveCharacter();
			CPVPFactionRewardManager::getInstance().giveTotemsEffects( c );
		}
	}
	return true;
}

//-----------------------------------------------
// Display current Spire Effects values
//-----------------------------------------------
NLMISC_COMMAND(getSpireEffectValues, "display current base value for all spire effects", "" )
{
	if( args.size() != 0 )
		return false;
	else
	{
		for ( uint i=EFFECT_FAMILIES::BeginTotemEffects; i<=EFFECT_FAMILIES::EndTotemEffects; i++ )
		{
			EFFECT_FAMILIES::TEffectFamily family = (EFFECT_FAMILIES::TEffectFamily)i;
			log.displayNL( "%s : %d", toString( family ).c_str(),
				                      CPVPFactionRewardManager::EffectValues[ i ] );
		}
	}
	
	return false;
}

//-----------------------------------------------
// Display the region owner
//-----------------------------------------------
NLMISC_COMMAND(getRegionOwner, "display the faction which currently owns the region", "<x> <y>" )
{
	if( args.size() != 2 )
		return false;
	else
	{
		float x = (float)atof(args[0].c_str());
		float y = (float)atof(args[1].c_str());	

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			log.displayNL( "The region %s is owned by the %s faction", 
				region->getName().c_str(),
				PVP_CLAN::toString(CPVPFactionRewardManager::getInstance().getRegionOwner( region->getId() ) ).c_str());
			return true;
		}
		else
			log.displayNL( "getRegionOwner : Can't found a region at coordinates %f %f", x, y );
	}
	return false;
}

//-----------------------------------------------
// Stats of spire in region
//-----------------------------------------------
NLMISC_COMMAND(getSpireStats, "display the status of spire in region corresponding to given coordinates", "<x> <y>" )
{
	if( args.size() != 2 )
		return false;
	else
	{
		float x = (float)atof(args[0].c_str());
		float y = (float)atof(args[1].c_str());	

		NLMISC::CVector vec( x, y, 0.0f );

		CRegion* region = CZoneManager::getInstance().getRegion( vec );

		if ( region )
		{
			const CTotemBase* totem = CPVPFactionRewardManager::getInstance().getTotemBaseFromId( region->getId() );

			if( totem != 0 )
			{
				if( totem->getOwnerFaction() == PVP_CLAN::Neutral )
				{
					log.displayNL( "getSpireStats: Region %s are neutral", region->getName().c_str() );
				}
				else
				{
					if( totem->isBuildingFinished() )
					{
						log.displayNL( "getSpireStats: Spire is builded in region %s, allegeance %s", region->getName().c_str(), PVP_CLAN::toString(totem->getOwnerFaction()).c_str() );
					}
					else
					{
						log.displayNL( "getSpireStats: Spire are building in region %s, allegeance %s, time left before finished %d seconds", region->getName().c_str(), PVP_CLAN::toString(totem->getOwnerFaction()).c_str(), totem->buildTimeLeft() / 100 );
					}

				}
			}
		}
		else
			log.displayNL( "getSpireStats : Can't found a region at coordinates %f %f", x, y );
	}
	return true;
}

//-----------------------------------------------
// Display the region owner
//-----------------------------------------------
NLMISC_COMMAND(addFactionPoint, "add (or substract) faction point in faction pool", "<+- delta faction point> <faction>" )
{
	if( args.size() != 2 )
		return false;
	else
	{
		sint32 deltaFactionPoint;
		NLMISC::fromString(args[0], deltaFactionPoint);
		PVP_CLAN::TPVPClan faction = PVP_CLAN::fromString(args[1].c_str());

		if(faction == PVP_CLAN::Unknown)
		{
			log.displayNL( "% is unknown faction", args[1].c_str() );
			return true;
		}
		
		CPVPFactionRewardManager::getInstance().updateFactionPointPool( faction, deltaFactionPoint );
	}
	return true;
}

// ----------------------------------------------------------------------------
// PERSISTENCE METHODS
// ----------------------------------------------------------------------------

#define PERSISTENT_MACROS_AUTO_UNDEF

//-----------------------------------------------------------------------------
// Persistent data for TFactionPossessions
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS TFactionPossessions

#define PERSISTENT_PRE_STORE\
	H_AUTO(TFactionPossessionsStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(TFactionPossessionsApply);\

#define PERSISTENT_DATA\
	PROP(uint32,	NbTotems)\
	PROP(uint8,		Level)\
	PROP(sint32,	FactionPointsPool)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CPVPFactionRewardManager
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPVPFactionRewardManager

#define PERSISTENT_PRE_STORE\
	H_AUTO(CPVPFactionRewardManagerStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CPVPFactionRewardManagerApply);\

#define PERSISTENT_DATA\
	PROP( uint32,						_NbTotems )\
	STRUCT_ARRAY( _FactionsPossessions, PVP_CLAN::NbClans )\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
